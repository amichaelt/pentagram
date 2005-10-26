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

#include "CurrentMap.h"
#include "Map.h"
#include "Item.h"
#include "GlobEgg.h"
#include "Egg.h"
#include "Actor.h"
#include "World.h"
#include "Rect.h"
#include "Container.h"
#include "UCList.h"
#include "ShapeInfo.h"
#include "TeleportEgg.h"
#include "EggHatcherProcess.h"
#include "Kernel.h"
#include "GameData.h"
#include "MainShapeArchive.h"
#include "GUIApp.h"
#include "GameMapGump.h"
#include "Direction.h"
#include "getObject.h"

#include "IDataSource.h"	
#include "ODataSource.h"

using std::list; // too messy otherwise
using Pentagram::Rect;
typedef list<Item*> item_list;

CurrentMap::CurrentMap()
	: current_map(0), egghatcher(0),
		fast_x_min(-1), fast_y_min(-1),
		fast_x_max(-1), fast_y_max(-1)
{
	items = new list<Item*>*[MAP_NUM_CHUNKS];
	fast = new uint32*[MAP_NUM_CHUNKS];
	for (unsigned int i = 0; i < MAP_NUM_CHUNKS; i++) {
		items[i] = new list<Item*>[MAP_NUM_CHUNKS];
		fast[i] = new uint32[MAP_NUM_CHUNKS/32];
		std::memset(fast[i],false,sizeof(uint32)*MAP_NUM_CHUNKS/32);
	}
}


CurrentMap::~CurrentMap()
{
//	clear();

	for (unsigned int i = 0; i < MAP_NUM_CHUNKS; i++) {
		delete[] items[i];
		delete[] fast[i];
	}
	delete[] items;
	delete[] fast;
}

void CurrentMap::clear()
{
	for (unsigned int i = 0; i < MAP_NUM_CHUNKS; i++) {
		for (unsigned int j = 0; j < MAP_NUM_CHUNKS; j++) {
			item_list::iterator iter;
			for (iter = items[i][j].begin(); iter != items[i][j].end(); ++iter)
				delete *iter;
			items[i][j].clear();
		}
		std::memset(fast[i],false,sizeof(uint32)*MAP_NUM_CHUNKS/32);
	}

	fast_x_min =  fast_y_min = fast_x_max = fast_y_max = -1;
	current_map = 0;

	Process* ehp = Kernel::get_instance()->getProcess(egghatcher);
	if (ehp)
		ehp->terminate();
	egghatcher = 0;
}

uint32 CurrentMap::getNum() const
{
	if (current_map == 0)
		return 0;

	return current_map->mapnum;
}

void CurrentMap::createEggHatcher()
{
	// get rid of old one, if any
	Process* ehp = Kernel::get_instance()->getProcess(egghatcher);
	if (ehp)
		ehp->terminate();

    ehp = new EggHatcherProcess();
	egghatcher = Kernel::get_instance()->addProcess(ehp);
}

void CurrentMap::writeback()
{
	if (!current_map)
		return;

	for (unsigned int i = 0; i < MAP_NUM_CHUNKS; i++) {
		for (unsigned int j = 0; j < MAP_NUM_CHUNKS; j++) {
			item_list::iterator iter;
			for (iter = items[i][j].begin(); iter != items[i][j].end(); ++iter)
			{
				Item* item = *iter;

				// item is being removed from the CurrentMap item lists
				item->clearExtFlag(Item::EXT_INCURMAP);

				// delete all fast only and disposable items
				if ((item->getFlags() & Item::FLG_FAST_ONLY) ||
					(item->getFlags() & Item::FLG_DISPOSABLE))
				{
					delete item;
					continue;
				}

				// Reset the egg
				Egg* egg = p_dynamic_cast<Egg*>(item);
				if (egg) {
					egg->reset();
				}

				// this item isn't from the Map. (like NPCs)
				if (item->getFlags() & Item::FLG_IN_NPC_LIST)
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

	// delete egghatcher
	Process* ehp = Kernel::get_instance()->getProcess(egghatcher);
	if (ehp)
		ehp->terminate();
	egghatcher = 0;
}

void CurrentMap::loadItems(list<Item*> itemlist, bool callCacheIn)
{
	item_list::iterator iter;
	for (iter = itemlist.begin(); iter != itemlist.end(); ++iter)
	{
		Item* item = *iter;

		item->assignObjId();

		// No fast area for you!
		item->clearFlag(Item::FLG_FASTAREA);

		// add item to internal object list
		addItemToEnd(item);

		if (callCacheIn)
			item->callUsecodeEvent_cachein();
	}
}

void CurrentMap::loadMap(Map* map)
{
	// don't call the cachein events at startup or when loading a savegame
	bool callCacheIn = (current_map != 0);

	current_map = map;

	createEggHatcher();

	// Clear fast area
	for (unsigned int i = 0; i < MAP_NUM_CHUNKS; i++) {
		std::memset(fast[i],false,sizeof(uint32)*MAP_NUM_CHUNKS/32);
	}
	fast_x_min = -1;
	fast_y_min = -1;
	fast_x_max = -1;
	fast_y_max = -1;

	loadItems(map->fixeditems, callCacheIn);
	loadItems(map->dynamicitems, callCacheIn);

	// we take control of the items in map, so clear the pointers
	map->fixeditems.clear();
	map->dynamicitems.clear();

	// load relevant NPCs to the item lists
	// !constant
	for (uint16 i = 0; i < 256; ++i) {
		Actor* actor = getActor(i);
		if (!actor) continue;

		// Schedule
		// CHECKME: is this the right time to pass?
		if (callCacheIn)
			actor->schedule(GUIApp::get_instance()->getGameTimeInSeconds()/60);
		
		if (actor->getMapNum() == getNum()) {
			addItemToEnd(actor);

#if 0
			// the avatar's cachein function is very strange; disabled for now
			if (callCacheIn)
				actor->callUsecodeEvent_cachein();
#endif
		}
	}
}

void CurrentMap::addItem(Item* item)
{
	sint32 ix, iy, iz;

	item->getLocation(ix, iy, iz);

	if (ix < 0 || ix >= MAP_CHUNK_SIZE*MAP_NUM_CHUNKS || 
		iy < 0 || iy >= MAP_CHUNK_SIZE*MAP_NUM_CHUNKS) {
		perr << "Skipping item: out of range (" 
			 << ix << "," << iy << ")" << std::endl;
		return;
	}

	sint32 cx = ix / MAP_CHUNK_SIZE;
	sint32 cy = iy / MAP_CHUNK_SIZE;

	items[cx][cy].push_front(item);
	item->setExtFlag(Item::EXT_INCURMAP);

	Egg* egg = p_dynamic_cast<Egg*>(item);
	if (egg) {
		EggHatcherProcess* ehp = p_dynamic_cast<EggHatcherProcess*>(Kernel::get_instance()->getProcess(egghatcher));
		assert(ehp);
		ehp->addEgg(egg);
	}
}

void CurrentMap::addItemToEnd(Item* item)
{
	sint32 ix, iy, iz;

	item->getLocation(ix, iy, iz);

	if (ix < 0 || ix >= MAP_CHUNK_SIZE*MAP_NUM_CHUNKS || 
		iy < 0 || iy >= MAP_CHUNK_SIZE*MAP_NUM_CHUNKS) {
		perr << "Skipping item: out of range (" 
			 << ix << "," << iy << ")" << std::endl;
		return;
	}

	sint32 cx = ix / MAP_CHUNK_SIZE;
	sint32 cy = iy / MAP_CHUNK_SIZE;

	items[cx][cy].push_back(item);
	item->setExtFlag(Item::EXT_INCURMAP);

	Egg* egg = p_dynamic_cast<Egg*>(item);
	if (egg) {
		EggHatcherProcess* ehp = p_dynamic_cast<EggHatcherProcess*>(Kernel::get_instance()->getProcess(egghatcher));
		assert(ehp);
		ehp->addEgg(egg);
	}
}

void CurrentMap::removeItem(Item* item)
{
	sint32 ix, iy, iz;

	item->getLocation(ix, iy, iz);

	removeItemFromList(item, ix, iy);
}


void CurrentMap::removeItemFromList(Item* item, sint32 oldx, sint32 oldy)
{
	//! This might a bit too inefficient
	// if it's really a problem we could change the item lists into sets
	// or something, but let's see how it turns out

	if (oldx < 0 || oldx >= MAP_CHUNK_SIZE*MAP_NUM_CHUNKS || 
		oldy < 0 || oldy >= MAP_CHUNK_SIZE*MAP_NUM_CHUNKS) {
		perr << "Skipping item: out of range (" 
			 << oldx << "," << oldy << ")" << std::endl;
		return;
	}

	sint32 cx = oldx / MAP_CHUNK_SIZE;
	sint32 cy = oldy / MAP_CHUNK_SIZE;

	items[cx][cy].remove(item);
	item->clearExtFlag(Item::EXT_INCURMAP);
}

// Check to see if the chunk is on the screen 
static inline bool ChunkOnScreen(sint32 cx, sint32 cy, sint32 sleft, sint32 stop, sint32 sright, sint32 sbot)
{
	sint32 scx = (cx*MAP_CHUNK_SIZE - cy*MAP_CHUNK_SIZE)/4;
	sint32 scy = ((cx*MAP_CHUNK_SIZE + cy*MAP_CHUNK_SIZE)/8);

	// Screenspace bounding box left extent    (LNT x coord)
	sint32 cxleft = scx-MAP_CHUNK_SIZE/4;
	// Screenspace bounding box right extent   (RFT x coord)
	sint32 cxright= scx+MAP_CHUNK_SIZE/4;

	// Screenspace bounding box top extent     (LFT y coord)
	sint32 cytop = scy - 256;
	// Screenspace bounding box bottom extent  (RNB y coord)
	sint32 cybot = scy + 128;

	const bool right_clear = cxright <= sleft;
	const bool left_clear = cxleft >= sright;
	const bool top_clear = cytop >= sbot;
	const bool bot_clear = cybot <= stop;

	const bool clear = right_clear|left_clear|top_clear|bot_clear;

	return !clear;
}

static inline void CalcFastAreaLimits( sint32 &sx_limit, 
										sint32 &sy_limit, 
										sint32 &xy_limit,
										const Pentagram::Rect &dims)
{
	// By default the fastArea is the screensize plus a border of no more
	// than 256 pixels wide and 384 pixels high
	// dims.w and dims.h need to be divided by 2 for crusader
	sx_limit = dims.w/256 + 3;
	sy_limit = dims.h/128 + 7;
	xy_limit = (sy_limit+sx_limit)/2;
}

void CurrentMap::updateFastArea(sint32 from_x, sint32 from_y, sint32 from_z, sint32 to_x, sint32 to_y, sint32 to_z)
{
	int x_min = from_x;
	int x_max = to_x;

	if (x_max < x_min)  {
		x_min = to_x;
		x_max = from_x;
	}

	int y_min = from_y;
	int y_max = to_y;

	if (y_max < y_min)  {
		y_min = to_y;
		y_max = from_y;
	}

	int z_min = from_z;
	int z_max = to_z;

	if (z_max < z_min)  {
		z_min = to_z;
		z_max = from_z;
	}

	// Work out Fine (screenspace) Limits of chunks with half chunk border
	Pentagram::Rect dims;
	GUIApp::get_instance()->getGameMapGump()->GetDims(dims);

	sint32 sleft  = ((x_min - y_min)/4)         - (dims.w/2 + MAP_CHUNK_SIZE/4);
	sint32 stop   = ((x_min + y_min)/8 - z_max) - (dims.h/2 + MAP_CHUNK_SIZE/8);
	sint32 sright = ((x_max - y_max)/4)         + (dims.w/2 + MAP_CHUNK_SIZE/4);
	sint32 sbot   = ((x_max + y_max)/8 - z_min) + (dims.h/2 + MAP_CHUNK_SIZE/8);

	// Don't do anything IF the regions are the same
	if (fast_x_min == sleft && fast_y_min == stop &&
		fast_x_max == sright && fast_y_max == sbot )
		return;

	// Update the saved region
	fast_x_min = sleft;
	fast_y_min = stop;
	fast_x_max = sright;
	fast_y_max = sbot;

	// Get Coarse Limits
	sint32 sx_limit;
	sint32 sy_limit;
	sint32 xy_limit;

	CalcFastAreaLimits(sx_limit, sy_limit, xy_limit, dims);

	x_min = x_min/MAP_CHUNK_SIZE - xy_limit;
	x_max = x_max/MAP_CHUNK_SIZE + xy_limit;
	y_min = y_min/MAP_CHUNK_SIZE - xy_limit;
	y_max = y_max/MAP_CHUNK_SIZE + xy_limit;

	for (sint32 cy = 0; cy < MAP_NUM_CHUNKS; cy++) {
		for (sint32 cx = 0; cx < MAP_NUM_CHUNKS; cx++) {

			// Coarse
			bool want_fast = cx>=x_min && cx<=x_max && cy>=y_min && cy<=y_max;

			// Fine
			if (want_fast) want_fast = ChunkOnScreen(cx,cy,sleft,stop,sright,sbot);

			bool currently_fast = isChunkFast(cx,cy);

			// Don't do anything, they are the same
			if (want_fast == currently_fast) continue;

			// leave fast area
			if (!want_fast) unsetChunkFast(cx,cy);
			// Enter fast area
			else setChunkFast(cx,cy);
		}
	}
}

void CurrentMap::setChunkFast(sint32 cx, sint32 cy)
{
	fast[cy][cx/32] |= 1<<(cx&31);

	item_list::iterator iter;
	for (iter = items[cx][cy].begin();
			iter != items[cx][cy].end(); ++iter) {
				(*iter)->enterFastArea();
			}
}

void CurrentMap::unsetChunkFast(sint32 cx, sint32 cy)
{
	fast[cy][cx/32] &= ~(1<<(cx&31));

	item_list::iterator iter = items[cx][cy].begin();
	while (iter != items[cx][cy].end())
	{
		Item* item = *iter;
		++iter;
		item->leaveFastArea();	// Can destroy the item
	}
}

void CurrentMap::areaSearch(UCList* itemlist, const uint8* loopscript,
							uint32 scriptsize, Item* check, uint16 range,
							bool recurse, sint32 x, sint32 y)
{
	sint32 z;
	sint32 xd = 0, yd = 0, zd = 0;

	// if item != 0, search an area around item. Otherwise, search an area
	// around (x,y)
	if (check) {
		check->getLocation(x,y,z);
		check->getFootpadWorld(xd,yd,zd);
	}

	Rect searchrange(x-xd-range,y-yd-range,2*range+xd,2*range+yd);

	int minx, miny, maxx, maxy;

	minx = ((x-xd-range)/MAP_CHUNK_SIZE) - 1;
	maxx = ((x+range)/MAP_CHUNK_SIZE) + 1;
	miny = ((y-yd-range)/MAP_CHUNK_SIZE) - 1;
	maxy = ((y+range)/MAP_CHUNK_SIZE) + 1;
	if (minx < 0) minx = 0;
	if (maxx >= MAP_NUM_CHUNKS) maxx = MAP_NUM_CHUNKS-1;
	if (miny < 0) miny = 0;
	if (maxy >= MAP_NUM_CHUNKS) maxy = MAP_NUM_CHUNKS-1;

	for (int cx = minx; cx <= maxx; cx++) {
		for (int cy = miny; cy <= maxy; cy++) {
			item_list::iterator iter;
			for (iter = items[cx][cy].begin();
				 iter != items[cx][cy].end(); ++iter) {

				Item* item = *iter;

				if (item->getExtFlags() & Item::EXT_SPRITE) continue;

				// check if item is in range?
				sint32 ix, iy, iz;
				item->getLocation(ix, iy, iz);

				ShapeInfo* info = item->getShapeInfo();
				sint32 ixd, iyd;

				//!! constants
				if (item->getFlags() & Item::FLG_FLIPPED) {
					ixd = 32 * info->y;
					iyd = 32 * info->x;
				} else {
					ixd = 32 * info->x;
					iyd = 32 * info->y;
				}

				Rect itemrect(ix - ixd, iy - iyd, ixd, iyd);

				if (!itemrect.Overlaps(searchrange)) continue;
				
				// check item against loopscript
				if ((*iter)->checkLoopScript(loopscript, scriptsize)) {
					uint16 objid = (*iter)->getObjId();
					uint8 buf[2];
					buf[0] = static_cast<uint8>(objid);
					buf[1] = static_cast<uint8>(objid >> 8);
					itemlist->append(buf);				
				}

				if (recurse) {
					// recurse into child-containers
					Container *container = p_dynamic_cast<Container*>(*iter);
					if (container)
						container->containerSearch(itemlist, loopscript,
												   scriptsize, recurse);
				}
			}
		}
	}
}

void CurrentMap::surfaceSearch(UCList* itemlist, const uint8* loopscript,
					uint32 scriptsize, Item* check, bool above, bool below,
					bool recurse)
{
	sint32 origin[3];
	sint32 dims[3];
	check->getLocation(origin[0], origin[1], origin[2]);
	check->getFootpadWorld(dims[0], dims[1], dims[2]);
	surfaceSearch(itemlist, loopscript, scriptsize, check->getObjId(),
				origin, dims, above, below, recurse);
}

void CurrentMap::surfaceSearch(UCList* itemlist, const uint8* loopscript,
					uint32 scriptsize, ObjId check,
					sint32 origin[3], sint32 dims[3],
					bool above, bool below, bool recurse)
{
	Rect searchrange(origin[0] - dims[0], origin[1] - dims[1],
					dims[0], dims[1]);

	sint32 minx, miny, maxx, maxy;

	minx = ((origin[0] - dims[0])/MAP_CHUNK_SIZE) - 1;
	maxx = ((origin[0])/MAP_CHUNK_SIZE) + 1;
	miny = ((origin[1] - dims[1])/MAP_CHUNK_SIZE) - 1;
	maxy = ((origin[1])/MAP_CHUNK_SIZE) + 1;
	if (minx < 0) minx = 0;
	if (maxx >= MAP_NUM_CHUNKS) maxx = MAP_NUM_CHUNKS-1;
	if (miny < 0) miny = 0;
	if (maxy >= MAP_NUM_CHUNKS) maxy = MAP_NUM_CHUNKS-1;

	for (sint32 cx = minx; cx <= maxx; cx++) {
		for (sint32 cy = miny; cy <= maxy; cy++) {
			item_list::iterator iter;
			for (iter = items[cx][cy].begin();
				 iter != items[cx][cy].end(); ++iter) {

				Item* item = *iter;

				if (item->getObjId() == check) continue;
				if (item->getExtFlags() & Item::EXT_SPRITE) continue;

				// check if item is in range?
				sint32 ix, iy, iz;
				item->getLocation(ix, iy, iz);
				sint32 ixd, iyd, izd;
				item->getFootpadWorld(ixd, iyd, izd);

				Rect itemrect(ix - ixd, iy - iyd, ixd, iyd);

				if (!itemrect.Overlaps(searchrange)) continue;

				bool ok = false;
				
				if (above && iz == (origin[2] + dims[2]))
				{
					ok = true;
					// Only recursive if tops aren't same (i.e. NOT flat)
					if (recurse && (izd+iz != origin[2] + dims[2]) )
						surfaceSearch(itemlist, loopscript, scriptsize, item, true, false, true);
				}
				
				if (below && origin[2] == (iz + izd))
				{
					ok = true;
					// Only recursive if bottoms aren't same (i.e. NOT flat)
					if (recurse && (izd != dims[2]) )
						surfaceSearch(itemlist, loopscript, scriptsize, item, false, true, true);
				}

				if (!ok) continue;

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
	for (unsigned int i = 0; i < MAP_NUM_CHUNKS; i++) {
		for (unsigned int j = 0; j < MAP_NUM_CHUNKS; j++) {
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

bool CurrentMap::isValidPosition(sint32 x, sint32 y, sint32 z,
								 uint32 shape,
								 ObjId item, uint16* support, uint16* roof)
{
	int xd, yd, zd;
	ShapeInfo* si = GameData::get_instance()->
		getMainShapes()->getShapeInfo(shape);
	//!! constants
	xd = si->x * 32;
	yd = si->y * 32;
	zd = si->z * 8;

	return isValidPosition(x,y,z, xd,yd,zd, si->flags, item, support, roof);
}

bool CurrentMap::isValidPosition(sint32 x, sint32 y, sint32 z,
								 int xd, int yd, int zd,
								 uint32 shapeflags,
								 ObjId item_, uint16* support_, uint16* roof_)
{
	const uint32 flagmask = (ShapeInfo::SI_SOLID | ShapeInfo::SI_DAMAGING |
							 ShapeInfo::SI_ROOF);

	bool valid = true;
	ObjId support = 0;
	ObjId roof = 0;
	sint32 roofz = 1 << 24; //!! semi-constant

	int minx, miny, maxx, maxy;

	minx = ((x-xd)/MAP_CHUNK_SIZE) - 1;
	maxx = (x/MAP_CHUNK_SIZE) + 1;
	miny = ((y-yd)/MAP_CHUNK_SIZE) - 1;
	maxy = (y/MAP_CHUNK_SIZE) + 1;
	if (minx < 0) minx = 0;
	if (maxx >= MAP_NUM_CHUNKS) maxx = MAP_NUM_CHUNKS-1;
	if (miny < 0) miny = 0;
	if (maxy >= MAP_NUM_CHUNKS) maxy = MAP_NUM_CHUNKS-1;

	for (int cx = minx; cx <= maxx; cx++) {
		for (int cy = miny; cy <= maxy; cy++) {
			item_list::iterator iter;
			for (iter = items[cx][cy].begin();
				 iter != items[cx][cy].end(); ++iter)
			{
				Item* item = *iter;
				if (item->getObjId() == item_) continue;
				if (item->getExtFlags() & Item::EXT_SPRITE) continue;

				ShapeInfo* si = item->getShapeInfo();
				//!! need to check is_sea() and is_land() maybe?
				if (!(si->flags & flagmask))
					continue; // not an interesting item

				sint32 ix, iy, iz, ixd, iyd, izd;
				item->getLocation(ix, iy, iz);
				item->getFootpadWorld(ixd, iyd, izd);

#if 0
				if (item->getShape() == 145) {
					perr << "Shape 145: (" << ix-ixd << "," << iy-iyd << ","
						 << iz << ")-(" << ix << "," << iy << "," << iz+izd
						 << ")" << std::endl;
					if (!si->is_solid()) perr << "not solid" << std::endl;
				}
#endif

				// check overlap
				if ((si->flags & shapeflags & flagmask) &&
					!(x <= ix - ixd || x - xd >= ix ||
					  y <= iy - iyd || y - yd >= iy ||
					  z + zd <= iz || z >= iz + izd))
				{
					// overlapping an item. Invalid position
#if 0
					item->dumpInfo();
#endif					
					valid = false;
				}

				// check xy overlap
				if (!(x <= ix - ixd || x - xd >= ix ||
					  y <= iy - iyd || y - yd >= iy))
				{
					// check support
					if (support == 0 && si->is_solid() &&
						iz + izd == z)
					{
						support = item->getObjId();
					}

					// check roof
					if (si->is_roof() && iz < roofz && iz >= z + zd) {
						roof = item->getObjId();
						roofz = iz;
					}
				}
			}
		}
	}

	if (support_)
		*support_ = support;
	if (roof_)
		*roof_ = roof;

	return valid;
}

bool CurrentMap::scanForValidPosition(sint32 x, sint32 y, sint32 z, Item* item,
									  int movedir, bool wantsupport,
									  sint32& tx, sint32& ty, sint32& tz)
{
	const uint32 flagmask = (ShapeInfo::SI_SOLID | ShapeInfo::SI_DAMAGING |
							 ShapeInfo::SI_ROOF);
	static uint32 validmask[17];
	static uint32 supportmask[17];

	int searchdir = (movedir + 2) % 4;

	int xdir = (x_fact[searchdir] != 0) ? 1 : 0;
	int ydir = (y_fact[searchdir] != 0) ? 1 : 0;

	// mark everything as valid, but without support
	for (int i = 0; i < 17; ++i) {
		validmask[i] = 0x1FFFF;
		supportmask[i] = 0;
	}

	sint32 xd,yd,zd;
	item->getFootpadWorld(xd, yd, zd);	

	// next, we'll loop over all objects in the area, and mark the areas
	// overlapped and supported by each object

	int minx, miny, maxx, maxy;

	minx = ((x-xd)/MAP_CHUNK_SIZE) - 1;
	maxx = (x/MAP_CHUNK_SIZE) + 1;
	miny = ((y-yd)/MAP_CHUNK_SIZE) - 1;
	maxy = (y/MAP_CHUNK_SIZE) + 1;
	if (minx < 0) minx = 0;
	if (maxx >= MAP_NUM_CHUNKS) maxx = MAP_NUM_CHUNKS-1;
	if (miny < 0) miny = 0;
	if (maxy >= MAP_NUM_CHUNKS) maxy = MAP_NUM_CHUNKS-1;

	for (int cx = minx; cx <= maxx; cx++) {
		for (int cy = miny; cy <= maxy; cy++) {
			item_list::iterator iter;
			for (iter = items[cx][cy].begin();
				 iter != items[cx][cy].end(); ++iter)
			{
				Item* citem = *iter;
				if (citem->getObjId() == item->getObjId()) continue;
				if (citem->getExtFlags() & Item::EXT_SPRITE) continue;

				ShapeInfo* si = citem->getShapeInfo();
				//!! need to check is_sea() and is_land() maybe?
				if (!(si->flags & flagmask))
					continue; // not an interesting item

				sint32 ix, iy, iz, ixd, iyd, izd;
				citem->getLocation(ix, iy, iz);
				citem->getFootpadWorld(ixd, iyd, izd);

				int minv = iz-z-zd+1;
				int maxv = iz+izd-z-1;
				if (minv < -8) minv = -8;
				if (maxv > 8) maxv = 8;

				int sminx,smaxx,sminy,smaxy;
				sminx = ix-ixd+1 -x;
				smaxx = ix+xd-1  -x;
				sminy = iy-iyd+1 -y;
				smaxy = iy+yd-1  -y;

				int minh = -100;
				int maxh = 100;
				if (!xdir && (sminx > 0 || smaxx < 0)) continue;
				if (!ydir && (sminy > 0 || smaxy < 0)) continue;

				if (xdir && minh < sminx)
					minh = sminx;
				if (xdir && maxh > smaxx)
					maxh = smaxx;
				if ((ydir && searchdir != 3) && minh < sminy)
					minh = sminy;
				if ((ydir && searchdir != 3) && maxh > smaxy)
					maxh = smaxy;
				if (searchdir == 3 && minh < -smaxy)
					minh = -smaxy;
				if (searchdir == 3 && maxh > -sminy)
					maxh = -sminy;
				
				if (minh < -8) minh = -8;
				if (maxh > 8) maxh = 8;

				for (int j = minv; j <= maxv; ++j)
					for (int i = minh; i <= maxh; ++i)
						validmask[j+8] &= ~(1 << (i+8));

				if (wantsupport && si->is_solid() &&
					iz+izd >= z-8 && iz+izd <= z+8)
				{
					for (int i = minh; i <= maxh; ++i)
						supportmask[iz+izd-z+8] |= (1 << (i+8));

				}
			}
		}
	}

	bool foundunsupported = false;

#if 0
	for (unsigned int i = 0; i < 17; ++i) {
		pout.printf("%05x | %05x\n", validmask[16-i], supportmask[16-i]);
	}
	pout.printf("-----------\n");
#endif

	for (unsigned int i = 0; i < 5; ++i) {
		int horiz;
		if (i % 2 == 0)
			horiz = 4*(i/2);
		else
			horiz = -1 - 4*(i/2);
		
		for (unsigned int j = 0; j < 5; ++j) {
			int vert;
			if (j % 2 == 0)
				vert = 4*(j/2);
			else
				vert = -4 - 4*(j/2);

			if (validmask[vert+8] & (1<<(horiz+8))) {
				if (!wantsupport || !foundunsupported || 
					(supportmask[vert+8] & (1<<(horiz+8))))
				{
					tz = z + vert;
					tx = x + x_fact[searchdir]*horiz;
					ty = y + y_fact[searchdir]*horiz;
				}
				if (!wantsupport || (supportmask[vert+8] & (1<<(horiz+8))))
					return true;
				foundunsupported = true;
			}
		}
	}

	// no supported location found, so return unsupported unblocked one
	if (foundunsupported)
		return true;

	return false;
}


// Do a sweepTest of an item from start to end point.
// dims is the bounding box size.
// item is the item that we are checking to move
// blocking_only forces us to check against blocking items only.
// skip will skip all items until item num skip is reached
// Returns item hit or 0 if no hit.
// end is set to the colision point
bool CurrentMap::sweepTest(const sint32 start[3], const sint32 end[3],
						   const sint32 dims[3], uint32 shapeflags,
						   ObjId item, bool blocking_only,
						   std::list<SweepItem> *hit)
{
	const uint32 flagmask = (ShapeInfo::SI_SOLID | ShapeInfo::SI_DAMAGING |
							 ShapeInfo::SI_ROOF);

	int i;

	int minx, miny, maxx, maxy;
	minx = ((start[0]-dims[0])/MAP_CHUNK_SIZE) - 1;
	maxx = (start[0]/MAP_CHUNK_SIZE) + 1;
	miny = ((start[1]-dims[1])/MAP_CHUNK_SIZE) - 1;
	maxy = (start[1]/MAP_CHUNK_SIZE) + 1;

	{
		int dminx, dminy, dmaxx, dmaxy;
		dminx = ((end[0]-dims[0])/MAP_CHUNK_SIZE) - 1;
		dmaxx = (end[0]/MAP_CHUNK_SIZE) + 1;
		dminy = ((end[1]-dims[1])/MAP_CHUNK_SIZE) - 1;
		dmaxy = (end[1]/MAP_CHUNK_SIZE) + 1;
		if (dminx < minx) minx = dminx;
		if (dmaxx > maxx) maxx = dmaxx;
		if (dminy < miny) miny = dminy;
		if (dmaxy > maxy) maxy = dmaxy;
	}

	if (minx < 0) minx = 0;
	if (maxx >= MAP_NUM_CHUNKS) maxx = MAP_NUM_CHUNKS-1;
	if (miny < 0) miny = 0;
	if (maxy >= MAP_NUM_CHUNKS) maxy = MAP_NUM_CHUNKS-1;

	// Get velocity of item
	sint32 vel[3];
	sint32 ext[3];
	for (i = 0; i < 3; i++) 
	{
		vel[i] = end[i] - start[i];
		ext[i] = dims[i]/2;
	}

	// Centre of object
	sint32 centre[3];
	centre[0] = start[0] - ext[0];
	centre[1] = start[1] - ext[1];
	centre[2] = start[2] + ext[2];

//	pout << "Sweeping from (" << -ext[0] << ", " << -ext[1] << ", " << -ext[2] << ")" << std::endl;
//	pout << "              (" << ext[0] << ", " << ext[1] << ", " << ext[2] << ")" << std::endl;
//	pout << "Sweeping to   (" << vel[0]-ext[0] << ", " << vel[1]-ext[1] << ", " << vel[2]-ext[2] << ")" << std::endl;
//	pout << "              (" << vel[0]+ext[0] << ", " << vel[1]+ext[1] << ", " << vel[2]+ext[2] << ")" << std::endl;

	std::list<SweepItem>::iterator sw_it;
	if (hit) sw_it = hit->end();

	for (int cx = minx; cx <= maxx; cx++) {
		for (int cy = miny; cy <= maxy; cy++) {
			item_list::iterator iter;
			for (iter = items[cx][cy].begin();
				 iter != items[cx][cy].end(); ++iter)
			{
				Item* other_item = *iter;
				if (other_item->getObjId()==item) continue;
				if (other_item->getExtFlags() & Item::EXT_SPRITE) continue;

				uint32 othershapeflags = other_item->getShapeInfo()->flags;
				bool blocking = (othershapeflags & shapeflags & flagmask) != 0;

				// This WILL hit everything and return them unless
				// blocking_only is set
				if (blocking_only && !blocking)
					continue;

				sint32 other[3], oext[3];
				other_item->getLocation(other[0], other[1], other[2]);
				other_item->getFootpadWorld(oext[0], oext[1], oext[2]);
				oext[0] /= 2; oext[1] /= 2; oext[2] /= 2;

				// Put other into our coord frame
				other[0] -= oext[0]+centre[0];
				other[1] -= oext[1]+centre[1];
				other[2] += oext[2]-centre[2];

				//first times of overlap along each axis
				sint32 u_1[3] = {0,0,0};

				//last times of overlap along each axis 
				sint32 u_0[3] = {0x4000,0x4000,0x4000}; // CONSTANTS

				bool touch = false;
				bool touch_floor = false;

				//find the possible first and last times
				//of overlap along each axis
				for( long i=0 ; i<3 ; i++ )
				{
					sint32 A_max = ext[i];	
					sint32 A_min = -ext[i];	
					sint32 B_max = other[i]+oext[i];	
					sint32 B_min = other[i]-oext[i];	

					if ( vel[i] < 0 && A_max>=B_min )		// A_max>=B_min not required
					{
						if (A_max==B_min) touch = true; // touch at start
						if (A_min+vel[i]==B_max) touch = true; // touch at end

						// - want to know when rear of A passes front of B
						u_0[i] = ((B_max - A_min)*0x4000) / vel[i];
						// - want to know when front of A passes rear of B
						u_1[i] = ((B_min - A_max)*0x4000) / vel[i]; 
					}
					else if( vel[i] > 0 && A_min<=B_max)	// A_min<=B_max not required
					{
						if (A_min==B_max) touch = true; // touch at start
						if (A_max-vel[i]==B_min) touch = true; // touch at end

						// + want to know when front of A passes rear of B
						u_0[i] = ((B_min - A_max)*0x4000) / vel[i];
						// + want to know when rear of A passes front of B
						u_1[i] = ((B_max - A_min)*0x4000) / vel[i]; 
					}
					else if( vel[i] == 0 && A_max >= B_min && A_min <= B_max)
					{
						if (A_min==B_max || A_max==B_min) touch = true;
						if (i == 2 && A_min == B_max) touch_floor = true;

						u_0[i] = -1;
						u_1[i] = 0x4000;
					}
					else
					{
						u_0[i] = 0x4001;
						u_1[i] = -1;
					}

					if (u_1[i] >= u_0[i] && (u_0[i] > 0x4000 || u_1[i] < 0))
					{
						u_0[i] = 0x4001;
						u_1[i] = -1;
					}
				}

				//possible first time of overlap
				sint32 first = u_0[0];
				if (u_0[1] > first) first = u_0[1];
				if (u_0[2] > first) first = u_0[2];

				//possible last time of overlap
				sint32 last = u_1[0];
				if (u_1[1] < last) last = u_1[1];
				if (u_1[2] < last) last = u_1[2];

				//they could have only collided if
				//the first time of overlap occurred
				//before the last time of overlap
				if (first <= last)
				{
					//pout << "Hit item " << other_item->getObjId() << " at first: " << first << "  last: " << last << std::endl;

					if (!hit) return true;

					// Clamp
					if (first < -1) first = -1;
					if (last > 0x4000) last = 0x4000;

					// Ok, what we want to do here is add to the list.
					// Sorted by hit_time. 

					// Small speed up.
					if (sw_it != hit->end())
					{
						SweepItem &si = *sw_it;
						if (si.hit_time > first) sw_it = hit->begin();
					}
					else
						sw_it = hit->begin();

					for (;sw_it != hit->end(); ++sw_it) 
						if ((*sw_it).hit_time > first) break;

					// Now add it
					sw_it = hit->insert(sw_it, SweepItem(other_item->getObjId(),first,last,touch,touch_floor,blocking));
//					pout << "Hit item " << other_item->getObjId() << " at (" << first << "," << last << ")" << std::endl;
//					pout << "hit item      (" << other[0] << ", " << other[1] << ", " << other[2] << ")" << std::endl;
//					pout << "hit item time (" << u_0[0] << "-" << u_1[0] << ") (" << u_0[1] << "-" << u_1[1] << ") ("
//						 << u_0[2] << "-" << u_1[2] << ")" << std::endl;
				}
			}
		}
	}

	return hit && hit->size();
}


Item *CurrentMap::traceTopItem(sint32 x, sint32 y, sint32 ztop, sint32 zbot, ObjId ignore, uint32 shflags)
{
	Item* top = 0;

	if (ztop < zbot) {
		sint32 temp = ztop;
		ztop = zbot;
		zbot = temp;
	}

	int minx, miny, maxx, maxy;
	minx = (x/MAP_CHUNK_SIZE);
	maxx = (x/MAP_CHUNK_SIZE) + 1;
	miny = (y/MAP_CHUNK_SIZE);
	maxy = (y/MAP_CHUNK_SIZE) + 1;
	if (minx < 0) minx = 0;
	if (maxx >= MAP_NUM_CHUNKS) maxx = MAP_NUM_CHUNKS-1;
	if (miny < 0) miny = 0;
	if (maxy >= MAP_NUM_CHUNKS) maxy = MAP_NUM_CHUNKS-1;

	for (int cx = minx; cx <= maxx; cx++) {
		for (int cy = miny; cy <= maxy; cy++) {
			item_list::iterator iter;
			for (iter = items[cx][cy].begin();
				 iter != items[cx][cy].end(); ++iter)
			{
				Item* item = *iter;
				if (item->getObjId() == ignore) continue;
				if (item->getExtFlags() & Item::EXT_SPRITE) continue;

				ShapeInfo* si = item->getShapeInfo();
				if (!(si->flags & shflags) || si->is_editor() || si->is_translucent()) continue;

				sint32 ix, iy, iz, ixd, iyd, izd;
				item->getLocation(ix, iy, iz);
				item->getFootpadWorld(ixd, iyd, izd);

				if ((ix-ixd) >= x || ix <= x) continue;
				if ((iy-iyd) >= y || iy <= y) continue;
				if (iz >= ztop || (iz+izd) <= zbot) continue;

				if (top) {
					sint32 tix, tiy, tiz, tixd, tiyd, tizd;
					top->getLocation(tix, tiy, tiz);
					top->getFootpadWorld(tixd, tiyd, tizd);

					if ((tiz+tizd) < (iz+izd)) top = 0;
				}

				if (!top) top = item;
			}
		}
	}
	return top;
}

void CurrentMap::setWholeMapFast()
{
	for (unsigned int i = 0; i < MAP_NUM_CHUNKS; ++i) {
		for (unsigned int j = 0; j < MAP_NUM_CHUNKS; ++j) {
			if (!isChunkFast(j,i)) setChunkFast(j,i);
		}
	}
}

void CurrentMap::save(ODataSource* ods)
{
	for (unsigned int i = 0; i < MAP_NUM_CHUNKS; ++i) {
		for (unsigned int j = 0; j < MAP_NUM_CHUNKS/32; ++j) {
			ods->write4(fast[i][j]);
		}
	}
}

bool CurrentMap::load(IDataSource* ids, uint32 version)
{
	for (unsigned int i = 0; i < MAP_NUM_CHUNKS; ++i) {
		for (unsigned int j = 0; j < MAP_NUM_CHUNKS/32; ++j) {
			fast[i][j] = ids->read4();
		}
	}

	fast_x_min = -1;
	fast_y_min = -1;
	fast_x_max = -1;
	fast_y_max = -1;

	return true;
}

uint32 CurrentMap::I_canExistAt(const uint8* args, unsigned int /*argsize*/)
{
	ARG_UINT16(shape);
	ARG_UINT16(x);
	ARG_UINT16(y);
	ARG_UINT16(z);
	//!! TODO: figure these out
	ARG_UINT16(unk1); // is either 1 or 4
	ARG_UINT16(unk2); // looks like it could be an objid
	ARG_UINT16(unk3); // always zero

	CurrentMap* cm = World::get_instance()->getCurrentMap();
	bool valid = cm->isValidPosition(x, y, z, shape, 0, 0, 0);

	if (valid)
		return 1;
	else
		return 0;
}
