/*
 *  Copyright (C) 2003-2005  The Pentagram Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "pent_include.h"
#include "ItemRelativeGump.h"
#include "GameMapGump.h"
#include "Item.h"
#include "World.h"
#include "Container.h"
#include "ShapeInfo.h"
#include "GUIApp.h"
#include "IDataSource.h"
#include "ODataSource.h"

DEFINE_RUNTIME_CLASSTYPE_CODE(ItemRelativeGump,Gump);

ItemRelativeGump::ItemRelativeGump()
	: Gump(), ix(0), iy(0)
{
}

ItemRelativeGump::ItemRelativeGump(int x, int y, int width, int height,
								   uint16 owner, uint32 _Flags, sint32 _Layer)
	: Gump(x, y, width, height, owner, _Flags, _Layer), ix(0), iy(0)
{
}

ItemRelativeGump::~ItemRelativeGump(void)
{
}

void ItemRelativeGump::InitGump(Gump* newparent, bool take_focus)
{
	Gump::InitGump(newparent, take_focus);

	GetItemLocation(0);

	if (!newparent && parent)
		MoveOnScreen();
}

void ItemRelativeGump::MoveOnScreen()
{
	assert(parent);
	Pentagram::Rect sd, gd;
	parent->GetDims(sd);

	// first move back to our desired location
	x = 0;
	y = 0;

	// get rectangle that gump occupies in scalerGump's coordinate space
	sint32 left,right,top,bottom;
	left = -dims.x;
	right = left + dims.w;
	top = -dims.y;
	bottom = top + dims.h;
	GumpToParent(left,top);
	GumpToParent(right,bottom);

	sint32 movex = 0, movey = 0;

	if (left < -sd.x)
		movex = -sd.x - left;
	else if (right > -sd.x + sd.w)
		movex = -sd.x + sd.w - right;

	if (top < -sd.y)
		movey = -sd.y - top;
	else if (bottom > -sd.y + sd.h)
		movey = -sd.y + sd.h - bottom;

	Move(left+movex, top+movey);
}

// Paint the Gump (RenderSurface is relative to parent).
// Calls PaintThis and PaintChildren
void ItemRelativeGump::Paint(RenderSurface*surf, sint32 lerp_factor)
{
	GetItemLocation(lerp_factor);
	Gump::Paint(surf,lerp_factor);
}


// Convert a parent relative point to a gump point
void ItemRelativeGump::ParentToGump(int &px, int &py)
{
	px -= ix; 
	py -= iy;
	Gump::ParentToGump(px,py);
}

// Convert a gump point to parent relative point
void ItemRelativeGump::GumpToParent(int &gx, int &gy)
{
	Gump::GumpToParent(gx,gy);
	gx += ix;
	gy += iy;
}

void ItemRelativeGump::GetItemLocation(sint32 lerp_factor)
{
	Item *it = 0;
	Item *next = 0;
	Item *prev = 0;
	Gump *gump = 0;

	it = World::get_instance()->getItem(owner);

	if (!it) {
		// This shouldn't ever happen, the GumpNotifyProcess should
		// close us before we get here
		Close();
		return;
	}

	while ((next = it->getParentAsContainer()) != 0)
	{
		prev = it;
		it = next;
		gump = GUIApp::get_instance()->getGump(it->getGump());
		if (gump) break;
	}

	int gx, gy;

	if (!gump)
	{
		gump = GetRootGump()->FindGump(GameMapGump::ClassType);

		if (!gump) {
			perr << "ItemRelativeGump::GetItemLocation(): "
				 << "Unable to find GameMapGump!?!?" << std::endl;
			return;
		}

		gump->GetLocationOfItem(owner, gx, gy, lerp_factor);
	}
	else
	{
		gump->GetLocationOfItem(prev->getObjId(), gx, gy, lerp_factor);
	}

	// Convert the GumpSpaceCoord relative to the world/item gump
	// into screenspace coords
	gy = gy-it->getShapeInfo()->z*8-16;
	gump->GumpToScreenSpace(gx,gy);

	// Convert the screenspace coords into the coords of us
	if (parent) parent->ScreenSpaceToGump(gx,gy);

	// Set x and y, and center us over it
	ix = gx-dims.w/2;
//	iy = gy-dims.h-it->getShapeInfo()->z*8-16;
	iy = gy-dims.h;


	if (flags & FLAG_KEEP_VISIBLE)
		MoveOnScreen();
}

void ItemRelativeGump::Move(int x_, int y_)
{
	ParentToGump(x_, y_);
	x += x_;
	y += y_;
}

void ItemRelativeGump::saveData(ODataSource* ods)
{
	Gump::saveData(ods);
}

bool ItemRelativeGump::loadData(IDataSource* ids, uint32 version)
{
	if (!Gump::loadData(ids, version)) return false;

	return true;
}

// Colourless Protection
