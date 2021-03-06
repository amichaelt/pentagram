/*
Copyright (C) 2003-2007 The Pentagram team

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

#ifndef CURRENTMAP_H
#define CURRENTMAP_H

#include <list>
#include "intrinsics.h"

class Map;
class Item;
class UCList;
class TeleportEgg;
class EggHatcherProcess;
class IDataSource;
class ODataSource;

#define MAP_NUM_CHUNKS	64

class CurrentMap
{
	friend class World;
public:
	CurrentMap();
	~CurrentMap();

	void clear();
	void writeback();
	void loadMap(Map* map);

	//! sets the currently loaded map, without any processing.
	//! (Should only be used for loading.)
	void setMap(Map* map) { current_map = map; }

	//! Get the map number of the CurrentMap
	uint32 getNum() const;

	unsigned int getChunkSize() const { return mapChunkSize; }

	//! Add an item to the beginning of the item list
	void addItem(Item* item);

	//! Add an item to the end of the item list
	void addItemToEnd(Item* item);

	void removeItemFromList(Item* item, sint32 oldx, sint32 oldy);
	void removeItem(Item* item);

	//! Update the fast area for the cameras position
	void updateFastArea(sint32 from_x, sint32 from_y, sint32 from_z, sint32 to_x, sint32 to_y, sint32 to_z);

	//! search an area for items matching a loopscript
	//! \param itemlist the list to return objids in
	//! \param loopscript the script to check items against
	//! \param scriptsize the size (in bytes) of the loopscript
	//! \param item the item around which you want to search, or 0.
	//!             if item is 0, search around (x,y)
	//! \param range the (square) range to search
	//! \param recurse if true, search in containers too
	//! \param x x coordinate of search center if item is 0.
	//! \param y y coordinate of search center if item is 0.
	void areaSearch(UCList* itemlist, const uint8* loopscript,
					uint32 scriptsize, Item* item, uint16 range, bool recurse,
					sint32 x=0, sint32 y=0);

	// Surface search: Search above and below an item.
	void surfaceSearch(UCList* itemlist, const uint8* loopscript,
					uint32 scriptsize, Item* item, bool above, bool below,
					bool recurse=false);

	// Surface search: Search above and below an item.
	void surfaceSearch(UCList* itemlist, const uint8* loopscript,
					uint32 scriptsize, ObjId id,
					sint32 origin[3], sint32 dims[2],
					bool above, bool below, bool recurse=false);

	// Collision detection. Returns true if the box [x,y,z]-[x-xd,y-yd,z+zd]
	// does not collide with any solid items.
	// Additionally, if support is not NULL, *support is set to the item
	// supporting the given box, or 0 if it isn't supported.
	// If under_roof is not NULL, *roof is set to the roof item with the lowest
	// z coordinate that's over the box, or 0 if there is no roof above box.
	// Ignores collisions which were already occurring at the start position.
	// NB: isValidPosition doesn't consider item 'item'.
	bool isValidPosition(sint32 x, sint32 y, sint32 z,
						 sint32 startx, sint32 starty, sint32 startz,
						 int xd, int yd, int zd, uint32 shapeflags,
						 ObjId item,
						 Item** support=0, ObjId* roof=0);

	// Note that this version of isValidPosition does not look for start
	// position collisions.
	bool isValidPosition(sint32 x, sint32 y, sint32 z,
						 int xd, int yd, int zd, uint32 shapeflags,
						 ObjId item,
						 Item** support=0, ObjId* roof=0);

	// Note that this version of isValidPosition can not take 'flipped'
	// into account!
	bool isValidPosition(sint32 x, sint32 y, sint32 z, uint32 shape,
						 ObjId item, Item** support=0, ObjId* roof=0);

	//! Scan for a valid position for item in directions orthogonal to movedir
	bool scanForValidPosition(sint32 x, sint32 y, sint32 z, Item* item,
							  int movedir, bool wantsupport,
							  sint32& tx, sint32& ty, sint32& tz);

	struct SweepItem {
		SweepItem(ObjId it, sint32 ht, sint32 et, bool touch,
				  bool touchfloor, bool block, uint8 dir)
			: item(it), hit_time(ht), end_time(et), touching(touch),
			  touching_floor(touchfloor), blocking(block), dirs(dir) { }

		ObjId	item;		// Item that was hit

		//
		// The time values here are 'normalized' fixed point values
		// They range from 0 for the start of the move to 0x4000 for the end of
		// The move.
		//
		// Linear interpolate between the start and end positions using
		// hit_time to find where the moving item was when the hit occurs
		//

		sint32	hit_time;	// if -1, already hitting when sweep started. 
		sint32	end_time;	// if 0x4000, still hitting when sweep finished

		bool	touching;	// We are only touching (don't actually overlap)
		bool	touching_floor; // touching and directly below the moving item

		bool	blocking;	// This item blocks the moving item

		uint8	dirs; // Directions in which the item is being hit.
				      // Bitmask. Bit 0 is x, 1 is y, 2 is z.

		// Use this func to get the interpolated location of the hit
		void GetInterpolatedCoords(sint32 out[3], sint32 start[3], sint32 end[3])
		{
			for (int i = 0; i < 3; i++)
				out[i] = start[i] + ((end[i]-start[i])*(hit_time>=0?hit_time:0)+(end[i] > start[i] ? 0x2000 : -0x2000) )/0x4000;
		}
	};

	//! Perform a sweepTest for an item move
	//! \param start Start point to sweep from.
	//! \param end End point to sweep to.
	//! \param dims Bounding size of item to check.
	//! \param shapeflags shapeflags of item to check.
	//! \param item ObjId of the item being checked. This will allow item to
	//!             be skipped from being tested against. Use 0 for no item.
	//! \param solid_only If true, only test solid items.
	//! \param hit Pointer to a list to fill with items hit. Items are sorted
	//!            by SweepItem::hit_time
	//! \return false if no items were hit.
	//!         true if any items were hit.
	bool sweepTest(const sint32 start[3], const sint32 end[3],
				   const sint32 dims[3], uint32 shapeflags,
				   ObjId item, bool solid_only, std::list<SweepItem> *hit);

	TeleportEgg* findDestination(uint16 id);

	// Not allowed to modify the list. Remember to use const_iterator
	const std::list<Item*>* getItemList (sint32 gx, sint32 gy)
	{
		// CONSTANTS!
		if (gx < 0 || gy < 0 || gx >= MAP_NUM_CHUNKS || gy >= MAP_NUM_CHUNKS) 
			return 0;
		return &items[gx][gy];
	}

	bool isChunkFast(sint32 cx, sint32 cy)
	{
		// CONSTANTS!
		if (cx < 0 || cy < 0 || cx >= MAP_NUM_CHUNKS || cy >= MAP_NUM_CHUNKS) 
			return false;
		return (fast[cy][cx/32]&(1<<(cx&31))) != 0;
	}

	// A simple trace to find the top item at a specific xy point
	Item *traceTopItem(sint32 x, sint32 y, sint32 ztop, sint32 zbot, ObjId ignore, uint32 shflags);

	// Set the entire map as being 'fast' 
	void setWholeMapFast();

	void save(ODataSource* ods);
	bool load(IDataSource* ids, uint32 version);

	INTRINSIC(I_canExistAt);

private:
	void loadItems(std::list<Item*> itemlist, bool callCacheIn);
	void createEggHatcher();

	Map* current_map;

	// item lists. Lots of them :-)
	// items[x][y]
	std::list<Item*>** items;

	ProcId egghatcher;

	// Fast area bit masks -> fast[ry][rx/32]&(1<<(rx&31));
	uint32** fast;	
	sint32 fast_x_min, fast_y_min, fast_x_max, fast_y_max;

	int mapChunkSize;

	void setChunkFast(sint32 cx, sint32 cy);
	void unsetChunkFast(sint32 cx, sint32 cy);
};

#endif
