/*
Copyright (C) 2004-2005 The Pentagram team

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
#include "AnimationTracker.h"

#include "GameData.h"
#include "Actor.h"
#include "World.h"
#include "CurrentMap.h"
#include "MainShapeArchive.h"
#include "AnimAction.h"
#include "Direction.h"
#include "ShapeInfo.h"
#include "UCList.h"
#include "LoopScript.h"
#include "getObject.h"

#include "IDataSource.h"
#include "ODataSource.h"

//#define WATCHACTOR 1

#ifdef WATCHACTOR
static const int watchactor = WATCHACTOR;
#endif

AnimationTracker::AnimationTracker()
{

}

AnimationTracker::~AnimationTracker()
{

}

bool AnimationTracker::init(Actor* actor_, Animation::Sequence action_,
							uint32 dir_, PathfindingState* state_)
{
	assert(actor_);
	actor = actor_->getObjId();
	uint32 shape = actor_->getShape();
	animaction = GameData::get_instance()->getMainShapes()->
		getAnim(shape, action_);
	if (!animaction) return false;

	dir = dir_;

	if (state_ == 0) {
		animaction->getAnimRange(actor_, dir, startframe, endframe);
		actor_->getLocation(x, y, z);
		flipped = (actor_->getFlags() & Item::FLG_FLIPPED) != 0;
		firststep = (actor_->getActorFlags() & Actor::ACT_FIRSTSTEP) != 0;
	} else {
		animaction->getAnimRange(state_->lastanim, state_->direction,
								 state_->firststep, dir, startframe, endframe);
		flipped = state_->flipped;
		firststep = state_->firststep;
		x = state_->x;
		y = state_->y;
		z = state_->z;
	}

#ifdef WATCHACTOR
	if (actor_ && actor_->getObjId() == watchactor) {
		pout << "AnimationTracker: playing " << startframe << "-" << endframe
			 << " (animaction flags: " << std::hex << animaction->flags
			 << std::dec << ")" << std::endl;
		
	}
#endif

	firstframe = true;

	done = false;
	blocked = false;
	unsupported = false;
	hitobject = 0;
	mode = NormalMode;

	return true;
}

unsigned int AnimationTracker::getNextFrame(unsigned int frame)
{
	frame++;

	if (frame == endframe)
		return endframe;

	// loop if necessary
	if (frame >= animaction->size) {
		if (animaction->flags & (AnimAction::AAF_LOOPING |
								 AnimAction::AAF_LOOPING2)) {
			// CHECKME: unknown flag
			frame = 1;
		} else {
			frame = 0;
		}
	}

	return frame;
}

bool AnimationTracker::stepFrom(sint32 x_, sint32 y_, sint32 z_)
{
	x = x_;
	y = y_;
	z = z_;

	return step();
}

bool AnimationTracker::step()
{
	if (done) return false;

	Actor* a = getActor(actor);
	assert(a);

	if (firstframe)
		currentframe = startframe;
	else
		currentframe = getNextFrame(currentframe);

	if (currentframe == endframe) {
		done = true;

		// toggle ACT_FIRSTSTEP flag if necessary
		if (animaction->flags & AnimAction::AAF_TWOSTEP)
			firststep = !firststep;
		else
			firststep = true;

		return false;
	}

	prevx = x;
	prevy = y;
	prevz = z;

	// reset status flags
	unsupported = false;
	blocked = false;


	firstframe = false;

	AnimFrame& f = animaction->frames[dir][currentframe];

	shapeframe = f.frame;
	flipped = f.is_flipped();

	// determine movement for this frame
	sint32 dx = 4 * x_fact[dir] * f.deltadir;
	sint32 dy = 4 * y_fact[dir] * f.deltadir;
	sint32 dz = f.deltaz;

	if (mode == TargetMode && !(f.flags & AnimFrame::AFF_ONGROUND))
	{
		dx += target_dx;
		dy += target_dy;
	}

	// determine footpad
	bool actorflipped = (a->getFlags() & Item::FLG_FLIPPED) != 0;
	sint32 xd, yd, zd;
	a->getFootpadWorld(xd, yd, zd);
	if (actorflipped != flipped) {
		sint32 t = xd;
		xd = yd;
		yd = t;
	}
	CurrentMap* cm = World::get_instance()->getCurrentMap();

	// TODO: check if this step is allowed
	// * can move?
	//   if not:
	//     - try to step up a bit
	//     - try to shift left/right a bit
	//     CHECKME: how often can we do these minor adjustments?
	//     CHECKME: for which animation types can we do them?
	//   if still fails: blocked
	// * if ONGROUND
	//     - is supported if ONGROUND?
	//       if not:
	//         * try to step down a bit
	//         * try to shift left/right a bit
	//       if still fails: unsupported
	//     - if supported by non-land item: unsupported

	// It might be worth it creating a 'scanForValidPosition' function
	// (in CurrentMap maybe) that scans a small area around the given
	// coordinates for a valid position (with 'must be supported' as a flag).
	// Note that it should only check in directions orthogonal to the movement
	// direction (to prevent it becoming impossible to step off a ledge).

	// I seem to recall that the teleporter from the Upper Catacombs teleporter
	// to the Upper Catacombs places you inside the floor. Using this
	// scanForValidPosition after a teleport would work around that problem.

	sint32 tx,ty,tz;
	tx = x+dx;
	ty = y+dy;
	tz = z+dz;

	ObjId support;
	bool targetok = cm->isValidPosition(tx,ty,tz, xd,yd,zd,
										a->getShapeInfo()->flags,
										actor, &support, 0);

	if (!targetok || ((f.flags & AnimFrame::AFF_ONGROUND) && !support)) {

		// If Avatar, and on ground, try to adjust properly
		if (actor == 1 && (f.flags & AnimFrame::AFF_ONGROUND)) {
			targetok = cm->scanForValidPosition(tx,ty,tz, a, dir,
												true, tx,ty,tz);

			if (!targetok) {
				blocked = true;
				return false;
			} else {
#ifdef WATCHACTOR
				if (a->getObjId() == watchactor) {
					pout << "AnimationTracker: adjusted step: "
						 << tx-(x+dx) << "," << ty-(y+dy) << "," << tz-(z+dz)
						 << std::endl;
				}
#endif
			}
		} else {
			if (!targetok) {
				blocked = true;
				return false;
			}
		}
	}

#ifdef WATCHACTOR
	if (a->getObjId() == watchactor) {
		pout << "AnimationTracker: step (" << tx-x << "," << ty-y
			 << "," << tz-z << ")" << std::endl;
	}
#endif
		
	x = tx;
	y = ty;
	z = tz;
	

	// if attack animation, see if we hit something
	if ((animaction->flags & AnimAction::AAF_ATTACK) &&
		(hitobject == 0) && f.attack_range() > 0)
	{
		checkWeaponHit();
	}

	if (f.flags & AnimFrame::AFF_ONGROUND) {
		// needs support

		/*bool targetok = */ cm->isValidPosition(tx,ty,tz, xd,yd,zd,
												 a->getShapeInfo()->flags,
												 actor, &support, 0);
		

		if (!support) {
			unsupported = true;
			return false;
		} else {
#if 0
			// This check causes really weird behaviour when fall()
			// doesn't make things fall off non-land items, so disabled for now

			Item* supportitem = getItem(support);
			assert(supportitem);
			if (!supportitem->getShapeInfo()->is_land()) {
//				pout << "Not land: "; supportitem->dumpInfo();
				// invalid support
				unsupported = true;
				return false;
			}
#endif
		}
	}

	return true;
}

AnimFrame* AnimationTracker::getAnimFrame()
{
	return &animaction->frames[dir][currentframe];
}

void AnimationTracker::setTargetedMode(sint32 x_, sint32 y_, sint32 z_)
{
	unsigned int i;
	int totaldir = 0;
	int offGround = 0;
	sint32 end_dx, end_dy;

	for (i=startframe; i != endframe; i = getNextFrame(i))
	{
		AnimFrame& f = animaction->frames[dir][i];
		totaldir += f.deltadir;  // This line sometimes seg faults.. ????
		if (!(f.flags & AnimFrame::AFF_ONGROUND))
			++offGround;
	}

	end_dx = 4 * x_fact[dir] * totaldir;
	end_dy = 4 * y_fact[dir] * totaldir;

	if (offGround)
	{
		mode = TargetMode;
		target_dx = (x_ - x - end_dx) / offGround;
		target_dy = (y_ - y - end_dy) / offGround;
	}

}

void AnimationTracker::checkWeaponHit()
{
	int range = animaction->frames[dir][currentframe].attack_range();

	Actor *a = getActor(actor);
	assert(a);


	Pentagram::Box abox = a->getWorldBox();
	abox.MoveAbs(x,y,z);
	abox.MoveRel(x_fact[dir]*32*range,y_fact[dir]*32*range,0);

#ifdef WATCHACTOR
	if (a->getObjId() == watchactor) {
		pout << "AnimationTracker: Checking hit, range " << range << ", box "
			 << abox.x << "," << abox.y << "," << abox.z << "," << abox.xd
			 << "," << abox.yd << "," << abox.zd << ": ";
	}
#endif

	CurrentMap* cm = World::get_instance()->getCurrentMap();

	UCList itemlist(2);
	LOOPSCRIPT(script, LS_TOKEN_END);

	cm->areaSearch(&itemlist, script, sizeof(script), 0, 320, false, x, y);

	ObjId hit = 0;
	for (unsigned int i = 0; i < itemlist.getSize(); ++i) {
		ObjId itemid = itemlist.getuint16(i);
		if (itemid == actor) continue; // don't want to hit self

		Actor* item = getActor(itemid);
		if (!item) continue;

		Pentagram::Box ibox = item->getWorldBox();

		if (abox.Overlaps(ibox))
		{
			hit = itemid;
#ifdef WATCHACTOR
			if (a->getObjId() == watchactor) {
				pout << "hit: ";
				item->dumpInfo();
			}
#endif
			break;
		}
	}

#ifdef WATCHACTOR
	if (a->getObjId() == watchactor && !hit) {
		pout << "nothing" << std::endl;
	}
#endif

	hitobject = hit;
}

void AnimationTracker::updateState(PathfindingState& state)
{
	state.x = x;
	state.y = y;
	state.z = z;
	state.flipped = flipped;
	state.firststep = firststep;
}


void AnimationTracker::updateActorFlags()
{
	Actor* a = getActor(actor);
	assert(a);

	if (flipped)
		a->setFlag(Item::FLG_FLIPPED);
	else
		a->clearFlag(Item::FLG_FLIPPED);

	if (firststep)
		a->setActorFlag(Actor::ACT_FIRSTSTEP);
	else
		a->clearActorFlag(Actor::ACT_FIRSTSTEP);

	if (animaction) {
		bool hanging = (animaction->flags & AnimAction::AAF_HANGING) != 0;
		if (hanging)
			a->setFlag(Item::FLG_HANGING);
		else
			a->clearFlag(Item::FLG_HANGING);
	}

	if (currentframe != endframe)
		a->animframe = currentframe;
}

void AnimationTracker::getInterpolatedPosition(sint32& x_, sint32& y_,
											   sint32& z_, int fc)
{
	sint32 dx = x - prevx;
	sint32 dy = y - prevy;
	sint32 dz = z - prevz;

	x_ = prevx + (dx*fc)/(animaction->framerepeat+1);
	y_ = prevy + (dy*fc)/(animaction->framerepeat+1);
	z_ = prevz + (dz*fc)/(animaction->framerepeat+1);
}

void AnimationTracker::save(ODataSource* ods)
{
	ods->write4(startframe);
	ods->write4(endframe);
	uint8 ff = firstframe ? 1 : 0;
	ods->write1(ff);
	ods->write4(currentframe);

	ods->write2(actor);
	ods->write1(static_cast<uint8>(dir));

	if (animaction) {
		ods->write4(animaction->shapenum);
		ods->write4(animaction->action);
	} else {
		ods->write4(0);
		ods->write4(0);
	}

	ods->write4(static_cast<uint32>(prevx));
	ods->write4(static_cast<uint32>(prevy));
	ods->write4(static_cast<uint32>(prevz));
	ods->write4(static_cast<uint32>(x));
	ods->write4(static_cast<uint32>(y));
	ods->write4(static_cast<uint32>(z));

	ods->write2(static_cast<uint16>(mode));
	if (mode == TargetMode) {
		ods->write4(static_cast<uint32>(target_dx));
		ods->write4(static_cast<uint32>(target_dy));
	}
	uint8 fs = firststep ? 1 : 0;
	ods->write1(fs);
	uint8 fl = flipped ? 1 : 0;
	ods->write1(fl);
	ods->write4(shapeframe);

	uint8 flag = done ? 1 : 0;
	ods->write1(flag);
	flag = blocked ? 1 : 0;
	ods->write1(flag);
	flag = unsupported ? 1 : 0;
	ods->write1(flag);
	ods->write2(hitobject);
}

bool AnimationTracker::load(IDataSource* ids, uint32 version)
{
	startframe = ids->read4();
	endframe = ids->read4();
	firstframe = (ids->read1() != 0);
	currentframe = ids->read4();

	actor = ids->read2();
	dir = ids->read1();

	uint32 shapenum = ids->read4();
	uint32 action = ids->read4();
	if (shapenum == 0) {
		animaction = 0;
	} else {
		animaction = GameData::get_instance()->getMainShapes()->
			getAnim(shapenum, action);
		assert(animaction);
	}

	prevx = ids->read4();
	prevy = ids->read4();
	prevz = ids->read4();
	x = ids->read4();
	y = ids->read4();
	z = ids->read4();

	mode = static_cast<Mode>(ids->read2());
	if (mode == TargetMode) {
		target_dx = ids->read4();
		target_dy = ids->read4();
	}

	firststep = (ids->read1() != 0);
	flipped = (ids->read1() != 0);
	shapeframe = ids->read4();

	done = (ids->read1() != 0);
	blocked = (ids->read1() != 0);
	unsupported = (ids->read1() != 0);
	hitobject = ids->read2();

	return true;
}
