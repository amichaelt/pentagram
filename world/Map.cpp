/*
Copyright (C) 2003-2006 The Pentagram team

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

#include <stack>

#include "Map.h"
#include "IDataSource.h"
#include "ODataSource.h"
#include "ItemFactory.h"
#include "Item.h"
#include "Container.h"
#include "ObjectManager.h"
#include "CoreApp.h"
#include "GameInfo.h"

#include "ShapeInfo.h" // debugging only
#include "GameData.h" // ""
#include "MainShapeArchive.h" // ""

//#define DUMP_ITEMS

Map::Map(uint32 mapnum_)
	: mapnum(mapnum_)
{

}


Map::~Map()
{
	clear();
}

void Map::clear()
{
	std::list<Item*>::iterator iter;

	for (iter = fixeditems.begin(); iter != fixeditems.end(); ++iter) {
		delete *iter;
	}
	fixeditems.clear();

	for (iter = dynamicitems.begin(); iter != dynamicitems.end(); ++iter) {
		delete *iter;
	}
	dynamicitems.clear();
}

void Map::loadNonFixed(IDataSource* ds)
{
	loadFixedFormatObjects(dynamicitems, ds, 0);
}

void Map::loadFixed(IDataSource* ds)
{
	loadFixedFormatObjects(fixeditems, ds, Item::EXT_FIXED);


	// U8 hack for missing ground tiles on map 25. See docs/u8bugs.txt
	if (GAME_IS_U8 && mapnum == 25)
	{
		// TODO
	}

	// U8 hack for missing ground/wall tiles on map 62. See docs/u8bugs.txt
	if (GAME_IS_U8 && mapnum == 62)
	{
		Item* item = ItemFactory::createItem(301, 1, 0, 0, 0, 0,
											Item::EXT_FIXED, false);
		item->setLocation(16255, 6143, 48);
		fixeditems.push_back(item);

		item = ItemFactory::createItem(301, 1, 0, 0, 0, 0,
									   Item::EXT_FIXED, false);
		item->setLocation(16639, 6143, 48);
		fixeditems.push_back(item);

		item = ItemFactory::createItem(301, 1, 0, 0, 0, 0,
									   Item::EXT_FIXED, false);
		item->setLocation(16511, 6143, 48);
		fixeditems.push_back(item);


		item = ItemFactory::createItem(301, 1, 0, 0, 0, 0,
									   Item::EXT_FIXED, false);
		item->setLocation(15999, 6143, 48);
		fixeditems.push_back(item);

		item = ItemFactory::createItem(301, 1, 0, 0, 0, 0,
									   Item::EXT_FIXED, false);
		item->setLocation(15871, 6143, 48);
		fixeditems.push_back(item);

		item = ItemFactory::createItem(301, 1, 0, 0, 0, 0,
									   Item::EXT_FIXED, false);
		item->setLocation(15743, 6143, 48);
		fixeditems.push_back(item);

		item = ItemFactory::createItem(301, 1, 0, 0, 0, 0,
									   Item::EXT_FIXED, false);
		item->setLocation(15615, 6143, 48);
		fixeditems.push_back(item);



		item = ItemFactory::createItem(301, 1, 0, 0, 0, 0,
									   Item::EXT_FIXED, false);
		item->setLocation(15999, 6015, 48);
		fixeditems.push_back(item);

		item = ItemFactory::createItem(301, 1, 0, 0, 0, 0,
									   Item::EXT_FIXED, false);
		item->setLocation(15871, 6015, 48);
		fixeditems.push_back(item);

		item = ItemFactory::createItem(301, 1, 0, 0, 0, 0,
									   Item::EXT_FIXED, false);
		item->setLocation(15743, 6015, 48);
		fixeditems.push_back(item);

		item = ItemFactory::createItem(301, 1, 0, 0, 0, 0,
									   Item::EXT_FIXED, false);
		item->setLocation(15615, 6015, 48);
		fixeditems.push_back(item);

		item = ItemFactory::createItem(301, 1, 0, 0, 0, 0,
									   Item::EXT_FIXED, false);
		item->setLocation(20095, 6911, 48);
		fixeditems.push_back(item);

		item = ItemFactory::createItem(301, 1, 0, 0, 0, 0,
									   Item::EXT_FIXED, false);
		item->setLocation(20223,6911,48);
		fixeditems.push_back(item);

		item = ItemFactory::createItem(301, 1, 0, 0, 0, 0,
									   Item::EXT_FIXED, false);
		item->setLocation(20095,6783,48);
		fixeditems.push_back(item);

		item = ItemFactory::createItem(301, 1, 0, 0, 0, 0,
									   Item::EXT_FIXED, false);
		item->setLocation(20223,6783,48);
		fixeditems.push_back(item);

		item = ItemFactory::createItem(301, 1, 0, 0, 0, 0,
									   Item::EXT_FIXED, false);
		item->setLocation(19839,6655,48);
		fixeditems.push_back(item);

		item = ItemFactory::createItem(301, 1, 0, 0, 0, 0,
									   Item::EXT_FIXED, false);
		item->setLocation(19967,6655,48);
		fixeditems.push_back(item);

		item = ItemFactory::createItem(301, 1, 0, 0, 0, 0,
									   Item::EXT_FIXED, false);
		item->setLocation(19839,6527,48);
		fixeditems.push_back(item);

		item = ItemFactory::createItem(301, 1, 0, 0, 0, 0,
									   Item::EXT_FIXED, false);
		item->setLocation(19967,6527,48);
		fixeditems.push_back(item);

		item = ItemFactory::createItem(301, 1, 0, 0, 0, 0,
									   Item::EXT_FIXED, false);
		item->setLocation(20095,6527,48);
		fixeditems.push_back(item);

		item = ItemFactory::createItem(301, 1, 0, 0, 0, 0,
									   Item::EXT_FIXED, false);
		item->setLocation(19967,6399,48);
		fixeditems.push_back(item);

		item = ItemFactory::createItem(301, 1, 0, 0, 0, 0,
									   Item::EXT_FIXED, false);
		item->setLocation(19839,6399,48);
		fixeditems.push_back(item);

		item = ItemFactory::createItem(301, 1, 0, 0, 0, 0,
									   Item::EXT_FIXED, false);
		item->setLocation(19711,6399,48);
		fixeditems.push_back(item);



		item = ItemFactory::createItem(497, 0, 0, 0, 0, 0,
									   Item::EXT_FIXED, false);
		item->setLocation(15487, 6271, 48);
		fixeditems.push_back(item);

		item = ItemFactory::createItem(497, 0, 0, 0, 0, 0,
									   Item::EXT_FIXED, false);
		item->setLocation(15359, 6271, 48);
		fixeditems.push_back(item);

		item = ItemFactory::createItem(409, 32, 0, 0, 0, 0,
									   Item::EXT_FIXED, false);
		item->setLocation(14975, 6399, 0);
		fixeditems.push_back(item);

		item = ItemFactory::createItem(409, 32, 0, 0, 0, 0,
									   Item::EXT_FIXED, false);
		item->setLocation(14975, 6015, 0);
		fixeditems.push_back(item);

		item = ItemFactory::createItem(409, 32, 0, 0, 0, 0,
									   Item::EXT_FIXED, false);
		item->setLocation(15103, 6015, 0);
		fixeditems.push_back(item);
	}
}

void Map::unloadFixed()
{
	std::list<Item*>::iterator iter;

	for (iter = fixeditems.begin(); iter != fixeditems.end(); ++iter) {
		delete *iter;
	}
	fixeditems.clear();
}

void Map::loadFixedFormatObjects(std::list<Item*>& itemlist, IDataSource* ds,
								 uint32 extendedflags)
{
	if (!ds) return;
	uint32 size = ds->getSize();
	if (size == 0) return;

	uint32 itemcount = size / 16;

	std::stack<Container*> cont;
	int contdepth = 0;

	for (uint32 i = 0; i < itemcount; ++i)
	{
		// These are ALL unsigned on disk
		sint32 x = static_cast<sint32>(ds->readX(2));
		sint32 y = static_cast<sint32>(ds->readX(2));
		sint32 z = static_cast<sint32>(ds->readX(1));

		if (GAME_IS_CRUSADER) {
			x *= 2;
			y *= 2;
		}

		uint32 shape = ds->read2();
		uint32 frame = ds->read1();
		uint16 flags = ds->read2();
		uint16 quality = ds->read2();
		uint16 npcnum = static_cast<uint16>(ds->read1());
		uint16 mapnum = static_cast<uint16>(ds->read1());
		uint16 next = ds->read2(); // do we need next for anything?

		// find container this item belongs to, if any.
		// the x coordinate stores the container-depth of this item,
		// so pop items from the container stack until we reach x,
		// or, if x is too large, the item is added to the top-level list
		while (contdepth != x && contdepth > 0) {
			cont.pop();
			contdepth--;
#ifdef DUMP_ITEMS
			pout << "---- Ending container ----" << std::endl;
#endif
		}

#ifdef DUMP_ITEMS
		pout << shape << "," << frame << ":\t(" << x << "," << y << "," << z << "),\t" << std::hex << flags << std::dec << ", " << quality << ", " << npcnum << ", " << mapnum << ", " << next << std::endl;
#endif

		Item *item = ItemFactory::createItem(shape,frame,quality,flags,npcnum,
											 mapnum,extendedflags,false);
		if (!item) {
			pout << shape << "," << frame << ":\t(" << x << "," << y << "," << z << "),\t" << std::hex << flags << std::dec << ", " << quality << ", " << npcnum << ", " << mapnum << ", " << next;

			ShapeInfo *info = GameData::get_instance()->getMainShapes()->
				getShapeInfo(shape);
			if (info) pout << ", family = " << info->family;
			pout << std::endl;

			pout << "Couldn't create item" << std::endl;
			continue;
		}
		item->setLocation(x,y,z);

		if (contdepth > 0) {
			cont.top()->addItem(item);
		} else {
			itemlist.push_back(item);
		}

		Container *c = p_dynamic_cast<Container*>(item);
		if (c) {
			// container, so prepare to read contents
			contdepth++;
			cont.push(c);
#ifdef DUMP_ITEMS
			pout << "---- Starting container ----" << std::endl;
#endif
		}
	}
}


void Map::save(ODataSource* ods)
{
	ods->write4(static_cast<uint32>(dynamicitems.size()));

	std::list<Item*>::iterator iter;
	for (iter = dynamicitems.begin(); iter != dynamicitems.end(); ++iter) {
		(*iter)->save(ods);
	}
}


bool Map::load(IDataSource* ids, uint32 version)
{
	uint32 itemcount = ids->read4();

	for (unsigned int i = 0; i < itemcount; ++i) {
		Object* obj = ObjectManager::get_instance()->loadObject(ids, version);
		Item* item = p_dynamic_cast<Item*>(obj);
		if (!item) return false;
		dynamicitems.push_back(item);
	}

	return true;
}
