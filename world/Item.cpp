/*
Copyright (C) 2003 The Pentagram team

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

#include "Item.h"
#include "Application.h"
#include "Usecode.h"
#include "GameData.h"
#include "UCMachine.h"
#include "World.h"
#include "DelayProcess.h"
#include "Container.h"
#include "Kernel.h"

#include "MainShapeFlex.h"
#include "ShapeInfo.h"
#include "ItemFactory.h"
#include "CurrentMap.h"

#include "UCStack.h"

/*
My current idea on how to construct items: an ItemFactory class that
creates the right kind of Item (Item, Container, ???) based on
the item that it's supposed to create. (This would be a friend of the Item
classes)

This is why the Item (and Container) constructors are currently rather empty.
-wjp
*/

// p_dynamic_cast stuff
DEFINE_DYNAMIC_CAST_CODE(Item,Object);

Item::Item()
	: shape(0), frame(0), x(0), y(0), z(0),
	  flags(0), quality(0), npcnum(0), mapnum(0),
	  extendedflags(0), parent(0), glob_next(0)
{

}


Item::~Item()
{

}

void Item::setLocation(sint32 X, sint32 Y, sint32 Z)
{
	x = X;
	y = Y;
	z = Z;
}

void Item::move(sint32 X, sint32 Y, sint32 Z)
{
	//! constant

	if (!parent && (x / 512 != X / 512) || (y / 512 != Y / 512)) {
		// ! check if item was/should be in CurrentMap in the first place...
		// just checking for !parent may not be enough

		World::get_instance()->getCurrentMap()->removeItemFromList(this,x,y);
		setLocation(X, Y, Z);
		World::get_instance()->getCurrentMap()->addItem(this);
		return;
	}

	setLocation(X, Y, Z);
}

void Item::getLocation(sint32& X, sint32& Y, sint32 &Z) const
{
	X = x;
	Y = y;
	Z = z;
}

ShapeInfo* Item::getShapeInfo() const
{
	return GameData::get_instance()->getMainShapes()->getShapeInfo(shape);
}

uint16 Item::getFamily() const
{
	return (uint16)(getShapeInfo()->family);
}


bool Item::checkLoopScript(const uint8* script, uint32 scriptsize)
{
	// if really necessary this could be made static to prevent news/deletes
	UCStack stack(0x40); // 64bytes should be plenty of room

	unsigned int i = 0;

	uint16 ui16a, ui16b;

	stack.push2(1); // default to true if script is empty

	while (i < scriptsize) {
		switch(script[i]) {
		case 0: // false
			stack.push2(0); break;
		case 1: // true
			stack.push2(1); break;
		case '$': // end
			ui16a = stack.pop2();
			return (ui16a != 0);
		case '%': // int
			//! check for i out of bounds
			ui16a = script[i+1] + (script[i+2]<<8);
			stack.push2(ui16a);
			i += 2;
			break;
		case '&': // and
			ui16a = stack.pop2();
			ui16b = stack.pop2();
			if (ui16a != 0 && ui16b != 0)
				stack.push2(1);
			else
				stack.push2(0);
			break;
		case '+': // or
			ui16a = stack.pop2();
			ui16b = stack.pop2();
			if (ui16a != 0 || ui16b != 0)
				stack.push2(1);
			else
				stack.push2(0);
			break;			
		case '!': // not
			ui16a = stack.pop2();
			if (ui16a != 0)
				stack.push2(0);
			else
				stack.push2(1);
			break;
		case '?': // item status
			stack.push2(getFlags());
			break;
		case '*': // item quality
			stack.push2(getQuality());
			break;
		case '#': // npc num
			stack.push2(getNpcNum());
			break;
		case '=': // equal
			ui16a = stack.pop2();
			ui16b = stack.pop2();
			if (ui16a == ui16b)
				stack.push2(1);
			else
				stack.push2(0);
			break;
		case '>': // greater than
			ui16a = stack.pop2();
			ui16b = stack.pop2();
			if (ui16b > ui16a)
				stack.push2(1);
			else
				stack.push2(0);
			break;
		case '<': // less than
			ui16a = stack.pop2();
			ui16b = stack.pop2();
			if (ui16b < ui16a)
				stack.push2(1);
			else
				stack.push2(0);
			break;
		case ']': // greater or equal
			ui16a = stack.pop2();
			ui16b = stack.pop2();
			if (ui16b >= ui16a)
				stack.push2(1);
			else
				stack.push2(0);
			break;
		case '[': // smaller or equal
			ui16a = stack.pop2();
			ui16b = stack.pop2();
			if (ui16b <= ui16a)
				stack.push2(1);
			else
				stack.push2(0);
			break;
		case ':': // item family
			stack.push2(getFamily());
			break;
		case '@': // item shape
			stack.push2(static_cast<uint16>(getShape()));
			break;
		case 'A': case 'B': case 'C': case 'D': case 'E':
		case 'F': case 'G': case 'H': case 'I': case 'J':
		case 'K': case 'L': case 'M': case 'N': case 'O':
		case 'P': case 'Q': case 'R': case 'S': case 'T':
		case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z':
		{
			bool match = false;
			int count = script[i] - '@';
			for (int j = 0; j < count; i++) {
				//! check for i out of bounds
				if (getShape() == (uint32)(script[i+1] + (script[i+2]<<8)))
					match = true;
				i += 2;
			}
			if (match)
				stack.push2(1);
			else
				stack.push2(0);
		}
		break;
		case '`': // item frame
			stack.push2(static_cast<uint16>(getFrame()));
			break;
		case 'a': case 'b': case 'c': case 'd': case 'e':
		case 'f': case 'g': case 'h': case 'i': case 'j':
		case 'k': case 'l': case 'm': case 'n': case 'o':
		case 'p': case 'q': case 'r': case 's': case 't':
		case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
		{
			bool match = false;
			int count = script[i] - '`';
			for (int j = 0; j < count; i++) {
				//! check for i out of bounds
				if (getFrame() == (uint32)(script[i+1] + (script[i+2]<<8)))
					match = true;
				i += 2;
			}
			if (match)
				stack.push2(1);
			else
				stack.push2(0);
		}
		break;
		default:
			perr.printf("Unknown loopscript opcode %02X\n", script[i]);
		}

		i++;
	}
	perr.printf("Didn't encounter $ in loopscript\n");
	return false;
}



uint32 Item::callUsecodeEvent(uint32 event)
{
	uint32	class_id = shape;

	// Non monster NPCs use objid/npcnum + 1024
	if (objid < 256 && !(flags & FLG_MONSTER_NPC)) class_id = objid + 1024;

	// UnkEggs have quality+0x47F
	if (getFamily() == ShapeInfo::SF_UNKEGG) class_id = quality + 0x47F;

	Usecode* u = GameData::get_instance()->getMainUsecode();
	uint32 offset = u->get_class_event(class_id, event);
	if (!offset) return 0;

	return callUsecode(static_cast<uint16>(class_id), static_cast<uint16>(offset), u);
}


//
// Item::setupLerp(uint32 factor)
//
// Desc: Setup the lerped info for this frame
//
// factor: Range 0 (prev) to 256 (next)
// camera: Camera object
//
void Item::setupLerp(sint32 cx, sint32 cy, sint32 cz)
{
	// Setup prev values
	l_prev = l_next;

	l_next.x = ix = x - cx;
	l_next.y = iy = y - cy;
	l_next.z = iz = z - cz;
	l_next.shape = shape;
	l_next.frame = frame;
}

uint32 Item::I_getX(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM(item);
	if (!item) return 0;

	return item->x;
}

uint32 Item::I_getY(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM(item);
	if (!item) return 0;

	return item->y;
}

uint32 Item::I_getZ(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM(item);
	if (!item) return 0;

	return item->z;
}

uint32 Item::I_getCX(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM(item);
	if (!item) return 0;

	if (item->flags & FLG_FLIPPED)
		return item->x - item->getShapeInfo()->y * 16;
	else
		return item->x - item->getShapeInfo()->x * 16;
}

uint32 Item::I_getCY(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM(item);
	if (!item) return 0;

	if (item->flags & FLG_FLIPPED)
		return item->y - item->getShapeInfo()->x * 16;
	else
		return item->y - item->getShapeInfo()->y * 16;
}

uint32 Item::I_getCZ(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM(item);
	if (!item) return 0;

	return item->z + item->getShapeInfo()->z * 4;
}

uint32 Item::I_getPoint(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM(item);
	ARG_UINT32(ptr);
	if (!item) return 0;

	uint8 buf[5];

	buf[0] = static_cast<uint8>(item->x);
	buf[1] = static_cast<uint8>(item->x >> 8);
	buf[2] = static_cast<uint8>(item->y);
	buf[3] = static_cast<uint8>(item->y >> 8);
	buf[4] = static_cast<uint8>(item->z);

	UCMachine::get_instance()->assignPointer(ptr, buf, 5);

	return 0;
}

uint32 Item::I_getShape(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM(item);
	if (!item) return 0;

	return item->getShape();
}

uint32 Item::I_setShape(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM(item);
	ARG_UINT16(shape);
	if (!item) return 0;

	item->setShape(shape);
	return 0;
}

uint32 Item::I_getFrame(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM(item);
	if (!item) return 0;

	return item->getFrame();
}

uint32 Item::I_setFrame(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM(item);
	ARG_UINT16(frame);
	if (!item) return 0;

	item->setFrame(frame);
	return 0;
}

uint32 Item::I_getQuality(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM(item);
	if (!item) return 0;

	if (item->getFamily() != ShapeInfo::SF_GENERIC)
		return item->getQuality();
	else
		return 0;
}

uint32 Item::I_getUnkEggType(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM(item);
	if (!item) return 0;

	if (item->getFamily() == ShapeInfo::SF_UNKEGG)
		return item->getQuality();
	else
		return 0;
}

uint32 Item::I_getQuantity(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM(item);
	if (!item) return 0;

	if (item->getFamily() == ShapeInfo::SF_QUANTITY ||
		item->getFamily() == ShapeInfo::SF_REAGENT)
		return item->getQuality();
	else
		return 0;
}

uint32 Item::I_getContainer(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM(item);
	if (!item) return 0;

	Container *parent = item->getParent();

	//! What do we do if item has no parent?

	if (parent)
		return parent->getObjId();
	else
		return 0;
}

uint32 Item::I_getRootContainer(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM(item);
	if (!item) return 0;

	Container *parent = item->getParent();

	//! What do we do if item has no parent?

	if (!parent) return 0;

	while (parent->getParent()) {
		parent = parent->getParent();
	}

	return parent->getObjId();
}

uint32 Item::I_getQ(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM(item);
	if (!item) return 0;

	return item->getQuality();
}

uint32 Item::I_setQ(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM(item);
	ARG_UINT16(q);
	if (!item) return 0;

	item->setQuality(q);
	return 0;
}

uint32 Item::I_setQuality(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM(item);
	ARG_UINT16(q);
	if (!item) return 0;

	if (item->getFamily() != ShapeInfo::SF_GENERIC)
		item->setQuality(q);

	return 0;
}

uint32 Item::I_setQuantity(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM(item);
	ARG_UINT16(q);
	if (!item) return 0;

	if (item->getFamily() == ShapeInfo::SF_QUANTITY ||
		item->getFamily() == ShapeInfo::SF_REAGENT)
		item->setQuality(q);

	return 0;
}

uint32 Item::I_getFamily(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM(item);
	if (!item) return 0;

	return item->getFamily();
}

uint32 Item::I_getTypeFlag(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM(item);
	ARG_UINT16(typeflag);
	if (!item) return 0;

	ShapeInfo *info = item->getShapeInfo();

	if (typeflag >= 64)
		perr << "Invalid TypeFlag greater than 63 requested (" << typeflag << ") by Usecode" << std::endl;

	if (info->getTypeFlag(typeflag))
		return 1;
	else
		return 0;
}

uint32 Item::I_getStatus(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM(item);
	if (!item) return 0;

	return item->getFlags();
}

uint32 Item::I_orStatus(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM(item);
	ARG_UINT16(mask);
	if (!item) return 0;

	item->flags |= mask;
	return 0;
}

uint32 Item::I_andStatus(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM(item);
	ARG_UINT16(mask);
	if (!item) return 0;

	item->flags &= mask;
	return 0;
}


uint32 Item::I_getWeight(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM(item);
	if (!item) return 0;

	return item->getShapeInfo()->weight;
}

uint32 Item::I_bark(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM(item);
	ARG_STRING(str);
	if (!item) return 0;

	pout << std::endl << std::endl << str  << std::endl << std::endl;
	
	// wait 4 ticks
	return Kernel::get_instance()->addProcess(new DelayProcess(4));

	// of course, in the final version of bark, we'll have to actually do
	// something after the timeout occurs.
	// Some kind of 'callback-delay-process' maybe?
}

uint32 Item::I_look(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM(item);
	if (!item) return 0;

	// constant?
	return item->callUsecodeEvent(0);
}

uint32 Item::I_use(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM(item);
	if (!item) return 0;

	// constant?
	return item->callUsecodeEvent(1);
}

uint32 Item::I_enterFastArea(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM(item);
	if (!item) return 0;

	// constant?
	return item->callUsecodeEvent(15);
}

//!!!!! major hack
UCList* answerlist;
int userchoice;
class UserChoiceProcess : public Process
{
public:
	virtual bool run(const uint32 /*framenum*/) {
		if (userchoice >= 0 && static_cast<uint32>(userchoice) < answerlist->getSize()) {
			result = answerlist->getStringIndex(userchoice);
			// we're leaking strings and memory here... (not that I care)
			pout << "User answer = " << result << ": " << UCMachine::get_instance()->getString(static_cast<uint16>(result)) << std::endl;
			terminate();
		}
		return false;
	}
};


uint32 Item::I_ask(const uint8* args, unsigned int /*argsize*/)
{
	ARG_NULL32(); // ARG_ITEM(item); // currently unused.
	ARG_LIST(answers);

	if (!answers) return 0;

	// display answers and spawn new process (return pid)
	// process waits for user input and returns the selected answer (as string)

	pout << std::endl << std::endl;
	for (unsigned int i = 0; i < answers->getSize(); ++i) {
		pout << i << ": " << UCMachine::get_instance()->getString(answers->getStringIndex(i)) << std::endl;
	}

	userchoice = -1;
	answerlist = new UCList(2);
	answerlist->copyStringList(*answers);

	return Kernel::get_instance()->addProcess(new UserChoiceProcess());
}

uint32 Item::I_legalCreateAtPoint(const uint8* args, unsigned int /*argsize*/)
{
	ARG_NULL32(); // ARG_ITEM(item); // unused?
	ARG_UINT16(shape);
	ARG_UINT16(frame);
	ARG_UINT32(ptr);

	//! haven't checked if this does what it should do.
	// It just creates an item at a worldpoint currently and returns the id.
	// This may have to check for room at the give spot

	uint8 buf[5];
	if (!UCMachine::get_instance()->dereferencePointer(ptr, buf, 5)) {
		perr << "Illegal WorldPoint pointer passed to I_legalCreateAtPoint."
			 << std::endl;
		return 0;
	}

	uint16 x = buf[0] + (buf[1]<<8);
	uint16 y = buf[2] + (buf[3]<<8);
	uint16 z = buf[4];

	Item* newitem = ItemFactory::createItem(shape, frame, 0, 0, 0, 0, 0);
	if (!newitem) {
		perr << "I_legalCreateAtPoint failed to create item (" << shape
			 <<	"," << frame << ")." << std::endl;
		return 0;
	}
	newitem->setLocation(x, y, z);
	uint16 objID = newitem->assignObjId();
	World::get_instance()->getCurrentMap()->addItem(newitem);

	return objID;
}

uint32 Item::I_legalCreateAtCoords(const uint8* args, unsigned int /*argsize*/)
{
	ARG_NULL32(); // ARG_ITEM(item); // unused?
	ARG_UINT16(shape);
	ARG_UINT16(frame);
	ARG_UINT16(x);
	ARG_UINT16(y);
	ARG_UINT16(z);

	//! haven't checked if this does what it should do.
	// It just creates an item at given coords currently and returns the id
	// This may have to check for room at the give spot

	Item* newitem = ItemFactory::createItem(shape, frame, 0, 0, 0, 0, 0);
	if (!newitem) {
		perr << "I_legalCreateAtCoords failed to create item (" << shape
			 <<	"," << frame << ")." << std::endl;
		return 0;
	}
	newitem->setLocation(x, y, z);
	uint16 objID = newitem->assignObjId();
	World::get_instance()->getCurrentMap()->addItem(newitem);

	return objID;
}

uint32 Item::I_legalCreateInCont(const uint8* args, unsigned int /*argsize*/)
{
	ARG_NULL32(); // ARG_ITEM(item); // unused?
	ARG_UINT16(shape);
	ARG_UINT16(frame);
	ARG_CONTAINER(container);
	ARG_UINT16(unknown); // ?

	//! haven't checked if this does what it should do.
	// It just creates an item, tries to add it to the given container.
	// If it first, return id; otherwise return 0.

	Item* newitem = ItemFactory::createItem(shape, frame, 0, 0, 0, 0, 0);
	if (!newitem) {
		perr << "I_legalCreateInCont failed to create item (" << shape
			 <<	"," << frame << ")." << std::endl;
		return 0;
	}

	if (container->AddItem(newitem)) {
		uint16 objID = newitem->assignObjId();

		return objID;
	} else {
		// failed to add (weight, volume, ...)
		// clean up
		delete newitem;

		return 0;
	}
}
