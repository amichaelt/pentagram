/*
 *  Copyright (C) 2002-2003 The Pentagram Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef CONVERTUSECODEU8_H
#define CONVERTUSECODEU8_H

#include "Convert.h"

class ConvertUsecodeU8 : public ConvertUsecode
{
	public:
		const char* const *intrinsics()  { return _intrinsics;  };
		const char* const *event_names() { return _event_names; };
		void readheader(IFileDataSource *ucfile, UsecodeHeader &uch, uint32 &curOffset);
		void readevents(IFileDataSource *ucfile, const UsecodeHeader &/*uch*/)
		{
			for (uint32 i=0; i<32; ++i)
			{
				uint32 offset = read4(ucfile);
				EventMap[offset] = i;
				#ifdef DISASM_DEBUG
				pout << "Event " << i << ": " << std::hex << std::setw(4) << offset << std::dec << endl;
				#endif
			}
		}

		void readOp(TempOp &op, IFileDataSource *ucfile, uint32 &dbg_symbol_offset, std::vector<DebugSymbol> &debugSymbols, bool &done)
		{ readOpGeneric(op, ucfile, dbg_symbol_offset, debugSymbols, done, false); };
		Node *readOp(IFileDataSource *ucfile, uint32 &dbg_symbol_offset, std::vector<DebugSymbol> &debugSymbols, bool &done)
		{ return readOpGeneric(ucfile, dbg_symbol_offset, debugSymbols, done, false); };

	
	private:
		static const char* const _intrinsics[];
		static const char* const _event_names[];
};

const char* const ConvertUsecodeU8::_intrinsics[] = {
	// 0000
	"target()",
	"Item::getNext()",							// Unused
	"Item::touch()",
	"word Item::getX()",
	"word Item::getY()",
	"word Item::getZ()",
	"word Item::getCX()",
	"word Item::getCY()",
	"word Item::getCZ()",
	"word Item::getGumpX()",					// Unused
	"word Item::getGumpY()",					// Unused
	"void Item::setGumpXY(word x, word y)",	// Unused
	"Item::getPoint(WorldPoint&)",
	"uword Item::getType()",
	"void Item::setType(uword type)",
	"uword Item::getFrame()",
	// 0010
	"void Item::setFrame(uword frame)",
	"uword Item::getQuality()",				// Get Q (Not generic familiy)
	"uword Item::getUnkEggType()",				// Get Q (UnkEgg family)
	"uword Item::getQuantity()",				// Get Q (Quantity or Reagent family)
	"Item::getContents()",						// Unused
	"Item::getContainer()",
	"Item::getRootContainer()",
	"uword Item::getGlobNum()",				// Get Q (GlobEgg family) - Unused
	"void Item::setGlobNum(uword)",				// Set Q (GlobEgg family) - Unused
	"uword Item::getQ()",						// Get Q
	"void Item::setQ(uword)",					// Set Q
	"uword Item::setQuality(word value)",		// Get Q (Not generic familiy)
	"void Item::setUnkEggType(word value)",	// Set Q (UnkEgg family) - Unused
	"void Item::setQuantity(word value)",		// Set Q (Quantity and Reagent families)
	"word Item::getFamily()",
	"bool Item::getTypeFlag(word bit)",		// Get TypeFlag (0 to 63 - not a mask!)
	// 0020
	"word Item::getStatus()",					// Get Status Flags
	"void Item::orStatus(word mask)",			// Status |= mask;
	"void Item::andStatus(word mask)",			// Status &= mask;
	"Item::getFootpad(word&, word&, word&)",
	"Item::touches(uword)",					// Unused
	"Item::overlaps(uword)",
	"Item::overlapsXY(uword)",					// Unused
	"Item::isOn(uword)",
	"Item::isCompletelyOn(uword)",				// Unused
	"Item::isAbove(uword)",					// Unused
	"Item::isUnder(uword)",					// Unused
	"Item::ascend(word)",
	"Item::getWeight()",
	"Item::getWeightIncludingContents()",
	"Item::getSurfaceWeight()",
	"Item::getVolume()",						// Unused
	// 0030
	"Item::getCapacity()",						// Unuses
	"Item::legal_create(uword, uword, uword, uword, uword)",
	"Item::create(uword, uword)",
	"Item::legal_create(uword, uword, WorldPoint&)",
	"Item::legal_create(uword, uword, uword, word)",
	"Item::push()",
	"Item::pop(uword, uword, ubyte)",
	"Item::pop(uword)",
	"Item::pop()",
	"Item::popToEnd(uword)",
	"Item::destroy()",
	"Item::removeContents()",
	"Item::destroyContents()",
	"Item::isExplosive()",
	"Item::move(uword, uword, ubyte)",
	"Item::move(WorldPoint&)",					// Unused
	// 0040
	"Item::legal_move(WorldPoint&, uword, uword)",
	"Item::legal_move(uword&, uword)",
	"Item::isNpc()",
	"Item::isInNpc()",							// Unused
	"Item::hurl(word, word, word, word)",
	"Item::shoot(WorldPoint&, word, word)",
	"Item::fall()",
	"Item::grab()",
	"Item::findTarget(word, word)",				// Unused
	"process Item::bark(char* str)",
	"strptr process Item::ask(uword slist)",
	"word Item::getSliderInput(word min, word max, word step)",
	"Item::openGump(word)",
	"Item::closeGump()",
	"Item::isGumpOpen()",						// Unused
	"Item::getNpcArray()",						// Unused
	// 0050
	"Item::getMapArray()",
	"Item::setNpcArray(word)",					// Unused
	"Item::setMapArray(word)",
	"Item::receiveHit(uword, byte, word, uword)",
	"Item::explode()",
	"Item::canReach(uword, word)",
	"Item::getRange(uword)",
	"Item::getRange(uword, uword, uword)",	// Unused
	"Item::getDirToCoords(uword, uword)",
	"Item::getDirFromCoords(uword, uword)",
	"Item::getDirToItem(uword)",
	"Item::getDirFromItem(uword)",
	"process Item::look()",						// Call event 0x0
	"process Item::use()",						// Call event 0x1
	"Item::anim()",								// Unused
	"Item::cachein()",							// Unused
	// 0060
	"Item::hit(uword, word)",					// Unused
	"process Item::gotHit(uword, word)",		// Call event ???
	"Item::release()",							// Unused
	"Item::equip()",							// Unused
	"Item::unequip()",							// Unused
	"Item::combine()",							// Unused
	"Item::calledFromAnim()",					// Unused
	"process Item::enterFastArea()",			// Call event 0xF
	"Item::leaveFastArea()",					// Unused
	"Item::cast(uword)",						// Unused
	"Item::justMoved()",						// Unused
	"Item::AvatarStoleSomething(uword)",		// Unused
	"Item::animGetHit(uword)",					// Unused
	"process Item::guardianBark(word)",			// Call even 0x15
	"Book::read(char*)",
	"Scroll::read(char*)",
	// 0070
	"Grave::read(word,char*)",
	"Plaque::read(word,char*)",
	"Egg::getEggXRange()",
	"Egg::getEggYRange()",
	"Egg::setEggXRange(uword)",
	"Egg::setEggYRange(uword)",				// Unused
	"Egg::getEggId()",
	"Egg::setEggId(uword)",
	"Egg::hatch()",								// Unused
	"MonsterEgg::hatch()",
	"MonsterEgg::getMonId()",
	"MonsterEgg::getActivity()",				// Unused
	"MonsterEgg::getShapeType()",				// Unused
	"MonsterEgg::setMonId(word)",				// Unused
	"MonsterEgg::setActivity(word)",			// Unused
	"MonsterEgg::setShapeType(word)",			// Unused
	// 0080
	"Npc::isBusy()",
	"Npc::areEnemiesNear()",
	"Npc::isInCombat()",
	"Npc::setInCombat()",
	"Npc::clrInCombat()",
	"Npc::setTarget(uword)",
	"Npc::getTarget()",
	"Npc::setAlignment(ubyte)",
	"Npc::getAlignment()",						// Unused
	"Npc::setEnemyAlignment(ubyte)",
	"Npc::getEnemyAlignment()",					// Unused
	"Npc::isEnemy(uword)",
	"Npc::isDead()",
	"Npc::setDead()",
	"Npc::clrDead()",
	"Npc::isImmortal()",						// Unused
	// 0090
	"Npc::setImmortal()",
	"Npc::clrImmortal()",
	"Npc::isWithstandDeath()",					// Unused
	"Npc::setWithstandDeath()",
	"Npc::clrWithstandDeath()",
	"Npc::isFeignDeath()",						// Unused
	"Npc::setFeignDeath()",
	"Npc::clrFeignDeath()",						// Unused
	"Npc::freeEquip(uword)",					// Unused
	"Npc::clearEquip()",						// Unused
	"Npc::getNpcSlot()",						// Unused
	"Npc::freeNpcSlot()",						// Unused
	"Npc::getDir()",
	"Npc::getMap()",
	"Npc::teleport(uword, uword, ubyte, ubyte)",
	"Npc::doAnim(AnimSet, word, word, ubyte)",
	// 00A0
	"Npc::getLastAnimSet()",
	"Npc::pathfind(uword, uword, uword, uword)",
	"Npc::pathfind(uword, uword)",
	"byte Npc::getStr()",
	"byte Npc::getInt()",
	"byte Npc::getDex()",
	"ubyte Npc::getHp()",
	"word Npc::getMana()",
	"void Npc::setStr(byte str)",
	"void Npc::setInt(byte int)",
	"void Npc::setDex(byte dex)",
	"void Npc::setHp(ubyte hp)",
	"void Npc::setMana(word mana)",
	"Npc::create(uword, uword)",
	"Npc::cSetActivity(Activity)",
	"Npc::setAirWalkEnabled(ubyte)",
	// 00B0
	"Npc::getAirWalkEnabled()",
	"Npc::schedule(ulong)",
	"Npc::getEquip(word)",
	"Npc::setEquip(word, uword)",
	"closeAllGumps()",
	"Camera::scrollTo(uword, uword, ubyte, word)",
	"urandom(word)",
	"rndRange(word,word)",
	"castGrantPeaceSpell()",
	"numToStr(uword)",
	"strToNum(char*)",							// Unused
	"playMusic(byte)",
	"getName()",								// Returns the Avatar's name
	"igniteChaos(uword, uword, ubyte)",
	"Camera::setCenterOn(uword)",
	"Camera::move_to(uword, uword, ubyte, word)",
	// 00C0
	"Camera::move_rel(word, word, word)",			// Unused
	"Camera::set_roof(word)",					// Unused
	"Camera::roof()",							// Unused
	"Camera::getX()",							// Unused
	"Camera::getY()",							// Unused
	"Camera::getZ()",							// Unused
	"Camera::startQuake(word)",
	"Camera::stopQuake()",
	"Camera::invertScreen(ubyte)",
	"U8MousePointer::getDir()",					// Unused
	"Kernel::getNumProcesses(uword, ProcessType)",
	"Kernel::resetRef(uword, ProcessType)",
	"teleportToEgg(word, word, ubyte)",
	"resetRef(uword, uword)",
	"setRef(uword, uword, uword)",
	"getAvatarInStasis()",
	// 00D0
	"setAvatarInStasis(word)",
	"getEtherealTop()",
	"getCurrentTimerTick()",
	"canGetThere(uword, uword, uword)",		// Unused
	"canExistAt(uword, uword, uword, uword, ubyte, uword, word)",
	"createSprite(word, word, word, word, word, word, uword, uword, ubyte)",
	"createSprite(word, word, word, word, uword, uword, ubyte)",
	"word getFamilyOfType(uword type)",
	"TimeInGameHours()",
	"TimeInMinutes()",
	"TimeInSeconds()",							// Unused
	"SetTimeInGameHours(word)",
	"SetTimeInMinutes(long)",					// Unused
	"SetTimeInSeconds(long)",					// unused
	"FadeToBlack()",
	"FadeFromBlack()",
	// 00E0
	"FadeToPalette(word, word)",
	"LightningBolt()",
	"FadeToWhite()",
	"FadeFromWhite()",
	"playEndgame()",
	"FeedAvatar(word)",
	"AccumulateStrength(word)",				// Unused
	"AccumulateIntelligence(word)",
	"AccumulateDexterity(word)",
	"ClrAvatarInCombat()",
	"SetAvatarInCombat()",
	"IsAvatarInCombat()",						// Unused
	"playSFX(word)",
	"playSFX(word, ubyte)",
	"playSFX(word, word, uword)",
	"playAmbientSFX(word)",						// Unused
	// 00F0
	"playAmbientSFX(word, word)",					// Unused
	"playAmbientSFX(word, word, uword)",
	"isSFXPlaying(word)",
	"setVolumeSFX(word, word)",
	"stopSFX(word)",
	"stopSFX(word, uword)",
	"soundInit(word, word, word)",					// Unused
	"soundDeInit()",							// Unused
	"musicStop()",
	"musicSlowStop()",							// Unused
	"musicPlay(word)",							// Unused
	"TonysBalls(word, word, uword, uword, uword)",
	"AvatarCanCheat()",
	"MakeAvatarACheater()",
	"isGameRunning()",
	"unused",
	// 0100
	"unused",
	0
};

const char * const ConvertUsecodeU8::_event_names[] = {
	"look()",						// 0x00
	"use()",						// 0x01
	"anim()",						// 0x02
	"setActivity()",				// 0x03
	"cachein()",					// 0x04
	"hit(uword, word)",				// 0x05
	"gotHit(uword, word)",			// 0x06
	"hatch()",						// 0x07
	"schedule()",					// 0x08
	"release()",					// 0x09
	"equip()",						// 0x0A
	"unequip()",					// 0x0B
	"combine()",					// 0x0C
	"func0D",						// 0x0D
	"calledFromAnim()",				// 0x0E
	"enterFastArea()",				// 0x0F

	"leaveFastArea()",				// 0x10
	"cast(uword)",					// 0x11
	"justMoved()",					// 0x12
	"AvatarStoleSomething(uword)",	// 0x13
	"animGetHit()",					// 0x14
	"guardianBark(word)",			// 0x15
	"func16",						// 0x16
	"func17",						// 0x17
	"func18",						// 0x18
	"func19",						// 0x19
	"func1A",						// 0x1A
	"func1B",						// 0x1B
	"func1C",						// 0x1C
	"func1D",						// 0x1D
	"func1E",						// 0x1E
	"func1F",						// 0x1F
	0
};

void ConvertUsecodeU8::readheader(IFileDataSource *ucfile, UsecodeHeader &uch, uint32 &curOffset)
{
	#ifdef DISASM_DEBUG
	perr << std::setfill('0') << std::hex;
	perr << "unknown1: " << std::setw(4) << read4(ucfile) << endl; // unknown
	uch.maxOffset = read4(ucfile) - 0x0C; // file size
	perr << "maxoffset: " << std::setw(4) << maxOffset << endl;
	perr << "unknown2: " << std::setw(4) << read4(ucfile) << endl; // unknown
	curOffset = 0;
	#else
	read4(ucfile); // unknown
	uch.maxOffset = read4(ucfile) - 0x0C; // file size
	read4(ucfile); // unknown
	curOffset = 0;
	#endif
};

#endif
