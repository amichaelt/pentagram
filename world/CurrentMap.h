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

#ifndef CURRENTMAP_H
#define CURRENTMAP_H

#include <list>
#include "intrinsics.h"

class Map;
class Item;
class UCList;
class TeleportEgg;
class EggHatcherProcess;

class CurrentMap
{
public:
	CurrentMap();
	~CurrentMap();

	void clear();
	void writeback();
	void loadMap(Map* map);

	uint32 getNum() const;

	void addItem(Item* item);
	void removeItemFromList(Item* item, sint32 oldx, sint32 oldy);

	void areaSearch(UCList* itemlist, const uint8* loopscript,
					uint32 scriptsize, Item* item, uint16 range, bool recurse,
					sint32 x=0, sint32 y=0);

	// Collision detection. Returns true if the box [x,y,z]-[x-xd,y-yd,z+zd]
	// does not collide with any solid items.
	// Additionally, if support is not NULL, *support is set to the item
	// supporting the given box, or 0 if it isn't supported.
	// If under_roof is not NULL, *roof is set to the roof item with the lowest
	// z coordinate that's over the box, or 0 if there is no roof above box.
	// NB: isValidPosition doesn't consider item 'item'.
	bool isValidPosition(sint32 x, sint32 y, sint32 z,
						 int xd, int yd, int zd, uint16 item,
						 uint16* support=0, uint16* roof=0);

	struct SweepItem {
		SweepItem(uint16 it, sint32 ht, sint32 et) : 
			item(it), hit_time(ht), end_time(et) { }

		uint16	item;		// Item that was hit

		//
		// The time values here are 'normalized' fixed point values
		// They range from 0 for the start of the move to 0x4000 for the end of
		// The move.
		//
		// Linear interpolate between the start and end positions using
		// hit_time to find where the moving item was when the hit occurs
		//

		sint32	hit_time;	// if 0, already hitting when sweep started. 
		sint32	end_time;	// if 0x4000, still hitting when sweep finished

		// Use this func to get the interpolated location of the hit
		void GetInterpolatedCoords(uint32 out[3], uint32 start[3], uint32 end[3])
		{
			for (int i = 0; i < 3; i++)
				out[i] = start[i] + ((end[i]-start[i])*0x4000)/hit_time;
		}
	};

	// Do a sweepTest of an item from start to end point.
	// Bounding size is dims.
	// item is the item that will be moving
	// skip will skip all items until item num skip is reached
	// Returns item hit or 0 if no hit.
	// end is to the colision point
	bool sweepTest(const sint32 start[3], const sint32 end[3], const sint32 dims[3],
			uint16 item, bool solid_only, std::list<SweepItem> *hit);

	TeleportEgg* findDestination(uint16 id);

	// Not allowed to modify the list. Remember to use const_iterator
	const std::list<Item*>* getItemList (sint32 gx, sint32 gy)
	{
		// CONSTANTS!
		if (gx < 0 || gy < 0 || gx >= 128 || gy >= 128) return 0;
		return &items[gx][gy];
	}

	INTRINSIC(I_canExistAt);

private:
	void loadItems(std::list<Item*> itemlist);

	Map* current_map;

	// item lists. Lots of them :-)
	// items[x][y]
	std::list<Item*>** items;

	EggHatcherProcess* egghatcher;
};

#endif
