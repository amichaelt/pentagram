/*
Copyright (C) 2003-2005 The Pentagram team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "pent_include.h"

#include "ActorAnimProcess.h"
#include "GameData.h"
#include "Animation.h"
#include "AnimDat.h"
#include "AnimAction.h"
#include "MainActor.h"
#include "Direction.h"
#include "World.h"
#include "GravityProcess.h"
#include "Kernel.h"
#include "UCList.h"
#include "LoopScript.h"
#include "CurrentMap.h"
#include "ShapeInfo.h"
#include "AnimationTracker.h"
#include "AudioProcess.h"
#include "SettingManager.h"
#include "CombatProcess.h"
#include "SpriteProcess.h"
#include "PaletteFaderProcess.h"
#include "CreateItemProcess.h"
#include "DestroyItemProcess.h"
#include "DelayProcess.h"
#include "getObject.h"

#include "IDataSource.h"
#include "ODataSource.h"

#include <cstdlib>

//#define WATCHACTOR 1

#ifdef WATCHACTOR
static const int watchactor = WATCHACTOR;
#endif

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(ActorAnimProcess,Process);

ActorAnimProcess::ActorAnimProcess() : Process(), tracker(0)
{

}

ActorAnimProcess::ActorAnimProcess(Actor* actor_, Animation::Sequence action_,
								   uint32 dir_, uint32 steps_)
{
	assert(actor_);
	item_num = actor_->getObjId();
	dir = dir_;
	action = action_;
	steps = steps_;

	type = 0x00F0; // CONSTANT !
	firstframe = true;
	tracker = 0;
	currentstep = 0;
}

bool ActorAnimProcess::init()
{
	if (dir > 7) {
		// invalid direction
		return false;
	}

	repeatcounter = 0;
	animAborted = false;
	attackedSomething = false;

	Actor* actor = getActor(item_num);
	assert(actor);

	if (!(actor->getFlags() & Item::FLG_FASTAREA)) {
		// not in the fast area? Can't play an animation then.
		// (If we do, the actor will likely fall because the floor is gone.)
		return false;
	}

	if (actor->getActorFlags() & Actor::ACT_ANIMLOCK) {
		//! What do we do if actor was already animating?
		//! don't do this animation or kill the previous one?
		//! Or maybe wait until the previous one finishes?

		perr << "ActorAnimProcess [" << getPid() << "]: ANIMLOCK set on actor "
			 << item_num << std::endl;

		// for now, just don't play this one.
		return false;
	}

	tracker = new AnimationTracker();
	if (!tracker->init(actor, action, dir))
		return false;

	actor->setActorFlag(Actor::ACT_ANIMLOCK);

	actor->lastanim = action;
	actor->direction = dir;


#ifdef WATCHACTOR
	if (item_num == watchactor)
		pout << "Animation [" << Kernel::get_instance()->getFrameNum()
			 << "] ActorAnimProcess " << getPid() << " created ("
			 << action << "," << dir << ")" << std::endl;
#endif

	return true;
}


bool ActorAnimProcess::run(const uint32 /*framenum*/)
{
	if (firstframe) {
		bool ret = init();
		if (!ret) {
			// initialization failed
			terminateDeferred();
			return false;
		}
	}

	if (animAborted) {
		terminate();
		return false;
	}

	assert(tracker);

	if (!firstframe)
		repeatcounter++;
	if (repeatcounter > tracker->getAnimAction()->framerepeat)
		repeatcounter = 0;

	Actor *a = getActor(item_num);
	if (!a) {
		// actor gone
		terminate();
		return false;
	}

	firstframe = false;

	if (!(a->getFlags() & Item::FLG_FASTAREA)) {
		// not in the fast area? Kill the animation then.
		//! TODO: Decide if this is the right move.
		//  Animation could do one of three things: pause, move
		//  without allowing actor to fall, or pretend to move and
		//  complete the entire movement as the actor reappears
		//  in fast area (still may need to pause when
		//  AnimationTracker is done.)
#ifdef WATCHACTOR
		if (item_num == watchactor)
			pout << "Animation ["
				 << Kernel::get_instance()->getFrameNum()
				 << "] ActorAnimProcess left fastarea; terminating"
				 << std::endl;
#endif
		terminate();
		return true;
	}

	bool result = true;
	if (repeatcounter == 0) {
		// next step:
		sint32 x,y,z;
		a->getLocation(x,y,z);
		result = tracker->stepFrom(x,y,z);
		tracker->updateActorFlags();
		currentstep++;

		if (!result) {
			// check possible error conditions

			if (tracker->isDone() || (steps && currentstep >= steps) ) {
				// all done
#ifdef WATCHACTOR
				if (item_num == watchactor)
					pout << "Animation ["
						 << Kernel::get_instance()->getFrameNum()
						 << "] ActorAnimProcess done; terminating"
						 << std::endl;
#endif
				terminate();
				return true;
			}


			if (tracker->isBlocked() &&
				!(tracker->getAnimAction()->flags&AnimAction::AAF_UNSTOPPABLE))
			{
#ifdef WATCHACTOR
				if (item_num == watchactor)
					pout << "Animation ["
						 << Kernel::get_instance()->getFrameNum()
						 << "] ActorAnimProcess blocked; terminating"
						 << std::endl;
#endif
				terminate();
				return true;
			}
		}

		AnimFrame* curframe = tracker->getAnimFrame();
		if (curframe && curframe->sfx) {
			AudioProcess* audioproc = AudioProcess::get_instance();
			if (audioproc) audioproc->playSFX(curframe->sfx,0x60,item_num,0);
		}

		if (curframe && (curframe->flags & AnimAction::AAF_SPECIAL)) {
			// Flag to trigger a special action
			// E.g.: play draw/sheathe SFX for avatar when weapon equipped,
			// throw skull-fireball when ghost attacks, ...
			doSpecial();
		}

	}

	sint32 x,y,z;
	tracker->getInterpolatedPosition(x,y,z,repeatcounter+1);

#ifdef WATCHACTOR
	if (repeatcounter == 0 && item_num == watchactor)
		pout << "Animation [" << Kernel::get_instance()->getFrameNum()
			 << "] showing next frame (" << x << "," << y << "," << z << ")"
			 << " shape (" << a->getShape() << "," << tracker->getFrame()
			 << ") sfx " << tracker->getAnimFrame()->sfx << std::endl;
#endif


	a->collideMove(x, y, z, false, true); // forced move
	a->setFrame(tracker->getFrame());

	if (repeatcounter == 0) {
		if (!result && tracker->isUnsupported()) {
			animAborted = true;

			a->fall();
			// TODO: inertia
			
			// Note: do not wait for the fall to finish: this breaks
			// the scene where Devon kills Mordea
			return true;
		}

		// attacking?
		if (!attackedSomething) {
			ObjId hit = tracker->hitSomething();
			if (hit) {
				attackedSomething = true;
				Item* hit_item = getItem(hit);
				assert(hit_item);
				hit_item->receiveHit(item_num, (dir+4)%8, 0, 0);
				doHitSpecial(hit_item);
			}
		}
	}

	return true;
}

void ActorAnimProcess::doSpecial()
{
	Actor *a = getActor(item_num);
	assert(a);

	// play SFX when Avatar draws/sheathes weapon
	if (item_num == 1 && (action == Animation::readyWeapon ||
						  action == Animation::unreadyWeapon) &&
		a->getEquip(ShapeInfo::SE_WEAPON) != 0)
	{
		int sfx = (std::rand() % 2) ? 0x51 : 0x52; // constants!
		AudioProcess* audioproc = AudioProcess::get_instance();
		if (audioproc) audioproc->playSFX(sfx, 0x60, 1, 0);
		return;
	}

	// ghosts
	if (a->getShape() == 0x19b)
	{
		Actor* hostile = 0;
		if (action == Animation::attack) {
			// fireball on attack
			unsigned int skullcount = a->countNearby(0x19d, 6*256);
			if (skullcount > 5) return;

			Actor* skull = Actor::createActor(0x19d);
			if (!skull) return;
			skull->setFlag(Item::FLG_FAST_ONLY);
			sint32 x,y,z;
			a->getLocation(x,y,z);
			int dir = a->getDir();
			skull->move(x+32*x_fact[dir],y+32*y_fact[dir],z);
			hostile = skull;
		} else if (a->getMapNum() != 54) { // Khumash-Gor doesn't summon ghouls
			// otherwise, summon ghoul
			unsigned int ghoulcount = a->countNearby(0x8e, 8*256);
			if (ghoulcount > 2) return;

			sint32 x,y,z;
			a->getLocation(x,y,z);
			x += (std::rand() % (6*256)) - 3*256;
			y += (std::rand() % (6*256)) - 3*256;

			Actor* ghoul = Actor::createActor(0x8e);
			if (!ghoul) return;
			ghoul->setFlag(Item::FLG_FAST_ONLY);
			if (!ghoul->canExistAt(x,y,z,true)) {
				ghoul->destroy();
				return;
			}
			ghoul->move(x,y,z);
			ghoul->doAnim(Animation::standUp, 0);
			hostile = ghoul;
		}
		
		if (hostile) {
			hostile->setInCombat();
			CombatProcess* hostilecp = hostile->getCombatProcess();
			CombatProcess* cp = a->getCombatProcess();
			if (hostilecp && cp)
				hostilecp->setTarget(cp->getTarget());
		}

		return;
	}

	// ghost's fireball
	if (a->getShape() == 0x19d)
	{
		Actor* av = getMainActor();
		if (a->getRange(*av) < 96) {
			a->setActorFlag(Actor::ACT_DEAD);
			a->explode(); // explode if close to the avatar
		}
		return;
	}

	// play PC/NPC footsteps
	SettingManager* settingman = SettingManager::get_instance();
	bool playavfootsteps;
	settingman->get("footsteps", playavfootsteps);
	if (item_num != 1 || playavfootsteps)
	{
		UCList itemlist(2);
		LOOPSCRIPT(script, LS_TOKEN_TRUE);		
		CurrentMap* cm = World::get_instance()->getCurrentMap();

		// find items directly below
		cm->surfaceSearch(&itemlist, script, sizeof(script), a, false, true);
		if (itemlist.getSize() == 0) return;

		Item* f = getItem(itemlist.getuint16(0));
		assert(f);

		uint32 floor = f->getShape();
		bool running = (action == Animation::run);
		bool splash = false;
		int sfx = 0;
		switch (floor) { // lots of constants!!
		case 0x03: case 0x04: case 0x09: case 0x0B: case 0x5C: case 0x5E:
			sfx = 0x2B;
			break;
		case 0x7E: case 0x80:
			sfx = 0xCD;
			splash = true;
			break;
		case 0xA1: case 0xA2: case 0xA3: case 0xA4:
			sfx = (running ? 0x99 : 0x91);
			break;
		default:
			sfx = (running ? 0x97 : 0x90);
			break;
		}

		if (sfx) {
			AudioProcess* audioproc = AudioProcess::get_instance();
			if (audioproc) audioproc->playSFX(sfx, 0x60, item_num, 0, false, 0x10000 + (std::rand()&0x1FFF) - 0x1000);
		}

		if (splash) {
			sint32 x,y,z;
			a->getLocation(x,y,z);			
			Process *sp = new SpriteProcess(475, 0, 7, 1, 1, x, y, z);
			Kernel::get_instance()->addProcess(sp);
		}
	}

}


void ActorAnimProcess::doHitSpecial(Item* hit)
{
	Actor *a = getActor(item_num);
	assert(a);

	Actor* attacked = p_dynamic_cast<Actor*>(hit);

	if (item_num == 1 && action == Animation::attack) {
		// some magic weapons have some special effects

		AudioProcess* audioproc = AudioProcess::get_instance();

		MainActor* av = getMainActor();
		ObjId weaponid = av->getEquip(ShapeInfo::SE_WEAPON);
		Item* weapon = getItem(weaponid);

		if (!weapon) return;

		uint32 weaponshape = weapon->getShape();

		switch (weaponshape) {
		case 0x32F: // magic hammer
			if (audioproc) audioproc->playSFX(23, 0x60, 1, 0, false,
								  0x10000 + (std::rand()&0x1FFF) - 0x1000);
			break;
		case 0x330: // Slayer
		{
			// if we killed somebody, thunder&lightning
			if (attacked && (attacked->getActorFlags() & Actor::ACT_DEAD)) {
				// calling intrinsic...
				PaletteFaderProcess::I_lightningBolt(0, 0);
				int sfx;
				switch (std::rand() % 3) {
				case 0: sfx = 91; break;
				case 1: sfx = 94; break;
				default: sfx = 96; break;
				}
				if (audioproc) audioproc->playSFX(sfx, 0x60, 1, 0);
			}
			break;
		}
		case 0x331: // Flame Sting
		{
			int sfx = 33;
			if (std::rand()%2 == 0) sfx = 101;
			if (audioproc) audioproc->playSFX(sfx, 0x60, 1, 0, false,
								  0x10000 + (std::rand()&0x1FFF) - 0x1000);

			sint32 x,y,z;
			a->getLocation(x,y,z);
			// 1: create flame sprite
			// 2: create flame object
			// 3: wait
			// 4a: destroy flame object
			// 4b: create douse-flame sprite
			Kernel* kernel = Kernel::get_instance();

			sint32 fx,fy,fz;
			fx = x + 96 * x_fact[dir];
			fy = y + 96 * y_fact[dir];
			fz = z;

			// CONSTANTS!! (lots of them)

			SpriteProcess* sp1 = new SpriteProcess(480, 0, 9, 1, 2, fx,fy,fz);
			kernel->addProcess(sp1);

			DelayProcess* dp1 = new DelayProcess(3);
			ProcId dp1id = kernel->addProcess(dp1);

			CreateItemProcess* cip = new CreateItemProcess(400, 0, 0,
														   Item::FLG_FAST_ONLY,
														   0, 0, 0, fx,fy,fz);
			ProcId cipid = kernel->addProcess(cip);

			DelayProcess* dp2 = new DelayProcess(60 + (std::rand()%60)); //2-4s
			ProcId dp2id = kernel->addProcess(dp2);

			DestroyItemProcess* dip = new DestroyItemProcess(0);
			kernel->addProcess(dip);

			SpriteProcess* sp2 = new SpriteProcess(381, 0, 9, 1, 1,
												   fx,fy,fz, true);
			kernel->addProcess(sp2);

			cip->waitFor(dp1id);
			dp2->waitFor(cipid);
			dip->waitFor(dp2id);
			sp2->waitFor(dp2id);

			break;
		}
		default:
			break;
		}

		return ;
	}

}



void ActorAnimProcess::terminate()
{
#ifdef WATCHACTOR
				if (item_num == watchactor)
					pout << "Animation ["
						 << Kernel::get_instance()->getFrameNum()
						 << "] ActorAnimProcess terminating"
						 << std::endl;
#endif
	Actor *a = getActor(item_num);
	if (a) {
		if (tracker) // if we were really animating...
			a->clearActorFlag(Actor::ACT_ANIMLOCK);
	}

	delete tracker;

	Process::terminate();
}

void ActorAnimProcess::dumpInfo()
{
	Process::dumpInfo();
	pout << "action: " << action << ", dir: " << dir << std::endl;
}

void ActorAnimProcess::saveData(ODataSource* ods)
{
	Process::saveData(ods);

	uint8 ff = firstframe ? 1 : 0;
	ods->write1(ff);
	uint8 ab = animAborted ? 1 : 0;
	ods->write1(ab);
	uint8 attacked = attackedSomething ? 1 : 0;
	ods->write1(attacked);
	ods->write1(static_cast<uint8>(dir));
	ods->write2(static_cast<uint16>(action));
	ods->write2(static_cast<uint16>(steps));
	ods->write2(static_cast<uint16>(repeatcounter));
	ods->write2(static_cast<uint16>(currentstep));

	if (tracker) {
		ods->write1(1);
		tracker->save(ods);
	} else
		ods->write1(0);
}

bool ActorAnimProcess::loadData(IDataSource* ids, uint32 version)
{
	if (!Process::loadData(ids, version)) return false;

	firstframe = (ids->read1() != 0);
	animAborted = (ids->read1() != 0);
	attackedSomething = (ids->read1() != 0);
	dir = ids->read1();
	action = static_cast<Animation::Sequence>(ids->read2());
	steps = ids->read2();
	repeatcounter = ids->read2();
	currentstep = ids->read2();

	assert(tracker == 0);
	if (ids->read1() != 0) {
		tracker = new AnimationTracker();
		if (!tracker->load(ids, version))
			return false;
	}

	return true;
}
