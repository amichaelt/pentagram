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

#include "CurrentMap.h"
#include "Map.h"
#include "Item.h"
#include "GlobEgg.h"
#include "Actor.h"
#include "World.h"
#include "Rect.h"
#include "Container.h"
#include "UCList.h"
#include "ShapeInfo.h"
#include "TeleportEgg.h"
#include "EggHatcherProcess.h"
#include "Kernel.h"

using std::list; // too messy otherwise
typedef list<Item*> item_list;

CurrentMap::CurrentMap()
	: current_map(0), egghatcher(0)
{
	items = new list<Item*>*[128]; //! get rid of constants
	for (unsigned int i = 0; i < 128; i++) {
		items[i] = new list<Item*>[128];
	}
}


CurrentMap::~CurrentMap()
{
	clear();

	//! get rid of constants
	for (unsigned int i = 0; i < 128; i++) {
		delete[] items[i];
	}
	delete[] items;
}

void CurrentMap::clear()
{
	// We need to be careful about who exactly deletes the items in a map
	// (CurrentMap or Map)
	// It should probably be CurrentMap, which means a Map has to be
	// emptied when it's loaded into CurrentMap

	//! get rid of constants
	for (unsigned int i = 0; i < 128; i++) {
		for (unsigned int j = 0; j < 128; j++) {
			item_list::iterator iter;
			for (iter = items[i][j].begin(); iter != items[i][j].end(); ++iter)
				delete *iter;
			items[i][j].clear();
		}
	}

	current_map = 0;

	if (egghatcher)
		egghatcher->terminate(); // kernel will delete egghatcher
	egghatcher = 0;
}

uint32 CurrentMap::getNum() const
{
	if (current_map == 0)
		return 0;

	return current_map->mapnum;
}

void CurrentMap::writeback()
{
	if (!current_map)
		return;

	//! constants

	for (unsigned int i = 0; i < 128; i++) {
		for (unsigned int j = 0; j < 128; j++) {
			item_list::iterator iter;
			for (iter = items[i][j].begin(); iter != items[i][j].end(); ++iter)
			{
				Item* item = *iter;

				// delete items inside globs
				if (item->getExtFlags() & Item::EXT_INGLOB) {
					item->clearObjId();
					delete item;
					continue;
				}

				// unexpand all globeggs (note that this doesn't do much)
				GlobEgg* globegg = p_dynamic_cast<GlobEgg*>(item);
				if (globegg) {
					globegg->unexpand();
				}

				// this item isn't from the Map. (like NPCs)
				if (item->getExtFlags() & Item::EXT_NOTINMAP)
					continue;

				item->clearObjId();
				if (item->getExtFlags() & Item::EXT_FIXED) {
					// item came from fixed
					current_map->fixeditems.push_back(item);
				} else {
					current_map->dynamicitems.push_back(item);
				}
			}
			items[i][j].clear();
		}
	}

	if (egghatcher)
		egghatcher->terminate(); // kernel will delete egghatcher
	egghatcher = 0;
}

void CurrentMap::loadItems(list<Item*> itemlist)
{
	item_list::iterator iter;
	for (iter = itemlist.begin(); iter != itemlist.end(); ++iter)
	{
		Item* item = *iter;

		item->assignObjId();

		// add item to internal object list
		addItem(item);

		GlobEgg* globegg = p_dynamic_cast<GlobEgg*>(item);
		if (globegg) {
			globegg->expand();
		}
	}
}

void CurrentMap::loadMap(Map* map)
{
	current_map = map;

	if (egghatcher)
		egghatcher->terminate();
	egghatcher = new EggHatcherProcess();
	Kernel::get_instance()->addProcess(egghatcher);

	loadItems(map->fixeditems);
	loadItems(map->dynamicitems);

	// we take control of the items in map, so clear the pointers
	map->fixeditems.clear();
	map->dynamicitems.clear();

	// load relevant NPCs to the item lists
	// !constant
	for (uint16 i = 0; i < 256; ++i) {
		Actor* actor = World::get_instance()->getNPC(i);
		if (actor && actor->getMapNum() == getNum()) {
			addItem(actor);
		}
	}
}

void CurrentMap::addItem(Item* item)
{
	sint32 ix, iy, iz;

	item->getLocation(ix, iy, iz);

	//! constants
	if (ix < 0 || iy < 0 || ix >= 512*128 || iy >= 512*128) {
		perr << "Skipping item: out of range (" 
			 << ix << "," << iy << ")" << std::endl;
		return;
	}

	sint32 cx = ix / 512;
	sint32 cy = iy / 512;

	items[cx][cy].push_back(item);

	Egg* egg = p_dynamic_cast<Egg*>(item);
	if (egg) {
		assert(egghatcher);
		egghatcher->addEgg(egg);
	}
}

void CurrentMap::removeItemFromList(Item* item, sint32 oldx, sint32 oldy)
{
	//! This might a bit too inefficient
	// if it's really a problem we could change the item lists into sets
	// or something, but let's see how it turns out

	//! constants
	if (oldx < 0 || oldy < 0 || oldx >= 512*128 || oldy >= 512*128) {
		perr << "Skipping item: out of range (" 
			 << oldx << "," << oldy << ")" << std::endl;
		return;
	}

	sint32 cx = oldx / 512;
	sint32 cy = oldy / 512;

	items[cx][cy].remove(item);
}


void CurrentMap::areaSearch(UCList* itemlist, const uint8* loopscript,
							uint32 scriptsize, Item* item, uint16 range,
							bool recurse, sint32 x, sint32 y)
{
	sint32 z;
	// if item != 0, search an area around item. Otherwise, search an area
	// around (x,y)
	if (item)
		item->getLocation(x,y,z);

	//!! do the dimensions of item have to be included too?
	Rect searchrange(x-range,y-range,2*range,2*range);

	int minx, miny, maxx, maxy;

	//! constants
	minx = ((x-range)/512) - 1;
	maxx = ((x+range)/512) + 1;
	miny = ((y-range)/512) - 1;
	maxy = ((y+range)/512) + 1;
	if (minx < 0) minx = 0;
	if (maxx > 127) maxx = 127;
	if (miny < 0) miny = 0;
	if (miny > 127) maxy = 127;

	for (int cx = minx; cx <= maxx; cx++) {
		for (int cy = miny; cy <= maxy; cy++) {
			item_list::iterator iter;
			for (iter = items[cx][cy].begin();
				 iter != items[cx][cy].end(); ++iter) {

				Item* item = *iter;


				// check if item is in range?
				sint32 itemx, itemy, itemz;
				item->getLocation(itemx, itemy, itemz);

				ShapeInfo* info = item->getShapeInfo();
				sint32 xd, yd;

				if (item->getFlags() & Item::FLG_FLIPPED) {
					xd = 32 * info->y;
					yd = 32 * info->x;
				} else {
					xd = 32 * info->x;
					yd = 32 * info->y;
				}

				Rect itemrect(x - xd, y - yd, xd, yd);

				if (!itemrect.Overlaps(searchrange)) continue;

				if (recurse) {
					// recurse into child-containers
					Container *container = p_dynamic_cast<Container*>(*iter);
					if (container)
						container->containerSearch(itemlist, loopscript,
												   scriptsize, recurse);
				}
				
				// check item against loopscript
				if ((*iter)->checkLoopScript(loopscript, scriptsize)) {
					uint16 objid = (*iter)->getObjId();
					uint8 buf[2];
					buf[0] = static_cast<uint8>(objid);
					buf[1] = static_cast<uint8>(objid >> 8);
					itemlist->append(buf);				
				}
			}
		}
	}
}

TeleportEgg* CurrentMap::findDestination(uint16 id)
{
	//! constants
	for (unsigned int i = 0; i < 128; i++) {
		for (unsigned int j = 0; j < 128; j++) {
			item_list::iterator iter;
			for (iter = items[i][j].begin();
				 iter != items[i][j].end(); ++iter)
			{
				TeleportEgg* egg = p_dynamic_cast<TeleportEgg*>(*iter);
				if (egg) {
					if (!egg->isTeleporter() && egg->getTeleportId() == id)
						return egg;
				}
			}
		}
	}
	return 0;
}
