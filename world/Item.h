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

#ifndef ITEM_H
#define ITEM_H

#include "Object.h"

#include "intrinsics.h"
class Container;

class Item : public Object
{
	friend class ItemFactory;

public:
	Item();
	virtual ~Item();

	// p_dynamic_cast stuff
	ENABLE_DYNAMIC_CAST(Item);

	Container* getParent() const { return parent; }
	void setLocation(sint32 x, sint32 y, sint32 z);
	void getLocation(sint32& x, sint32& y, sint32& z) const;
	uint32 getFlags() const { return flags; }
	uint32 getExtFlags() const { return extendedflags; }
	uint32 getShape() const { return shape; }
	uint32 getFrame() const { return frame; }

	void callUsecodeEvent(uint32 event);

	void setupLerp(/* Camera &camera */);	// Setup the lerped info for this frame
	inline void doLerp(uint32 factor) 		// Does lerping for an in between frame (0-256)
	{
		// Should be noted that this does indeed limit us to 'only' 24bit coords
#if 1
		// This way while possibly slower is more accurate
		ix = (l_prev.x*(256-factor) + l_next.x*factor)>>8;
		iy = (l_prev.y*(256-factor) + l_next.y*factor)>>8;
		iz = (l_prev.z*(256-factor) + l_next.z*factor)>>8;
#else
		ix = l_prev.x + (((l_next.x-l_prev.x)*factor)>>8);
		iy = l_prev.y + (((l_next.y-l_prev.y)*factor)>>8);
		iz = l_prev.z + (((l_next.z-l_prev.z)*factor)>>8);
#endif
	}


	// Intrinsics
	INTRINSIC(I_getX);
	INTRINSIC(I_getY);
	INTRINSIC(I_getZ);
	INTRINSIC(I_getShape);
	INTRINSIC(I_getFrame);
	INTRINSIC(I_getQ);
	INTRINSIC(I_bark);

protected:
	uint32 shape;
	uint32 frame;

	sint32 x,y,z; // world coordinates
	uint32 flags;
	uint16 quality;
	uint32 npcnum;
	uint32 mapnum;

	uint32 extendedflags; // pentagram's own flags

	Container* parent; // container this item is in (or 0 for top-level items)

	// This is stuff that is used for displaing and interpolation
	struct Lerped
	{
		sint32 x,y,z;
		uint32 shape,frame;
	};
	
	Lerped	l_prev;			// Previous state (relative to camera)
	Lerped	l_next;			// Next (current) state (relative to camera)
	sint32	ix,iy,iz;		// Interpolated position in camera space

public:
	enum {
		EXT_FIXED = 0x0001  // item came from FIXED
	} extflags;
};

#endif
