/*
 *  Copyright (C) 2004  The Pentagram Team
 *
 *  This program is free software{} you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation{} either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY{} without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program{} if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "pent_include.h"
#include "PagedGump.h"

#include "GameData.h"
#include "GumpShapeFlex.h"
#include "Shape.h"
#include "ShapeFrame.h"
#include "GUIApp.h"
#include "ButtonWidget.h"

DEFINE_RUNTIME_CLASSTYPE_CODE(PagedGump,ModalGump);

PagedGump::PagedGump(int left, int right, int top, int shape):
	ModalGump(0, 0, 5, 5), leftOff(left), rightOff(right), topOff(top),
	gumpShape(shape), nextButton(0), prevButton(0)
{
	current = gumps.end();
	GUIApp * guiapp = GUIApp::get_instance();
	guiapp->pushMouseCursor();
	guiapp->setMouseCursor(GUIApp::MOUSE_HAND);
}

PagedGump::~PagedGump(void)
{
	gumps.clear();
}

void PagedGump::Close(bool no_del)
{
	GUIApp* guiapp = GUIApp::get_instance();
	guiapp->popMouseCursor();

	ModalGump::Close(no_del);
}

static const int pageOverShape = 34;

void PagedGump::InitGump()
{
	ModalGump::InitGump();
	shape = GameData::get_instance()->getGumps()->getShape(gumpShape);
	ShapeFrame* sf = shape->getFrame(0);
	assert(sf);

	dims.w = sf->width;
	dims.h = sf->height;

	FrameID buttonleft(GameData::GUMPS, pageOverShape, 0);
	FrameID buttonright(GameData::GUMPS, pageOverShape, 1);

	//!! Hardcoded gump
	nextButton = new ButtonWidget(0, 0, buttonright, buttonright);
	nextButton->InitGump();
	AddChild(nextButton);
	nextButton->setRelativePosition(TOP_RIGHT, rightOff, topOff);

	prevButton = new ButtonWidget(0, 0, buttonleft, buttonleft);
	prevButton->InitGump();
	AddChild(prevButton);
	prevButton->setRelativePosition(TOP_LEFT, leftOff, topOff);
	prevButton->HideGump();
}

void PagedGump::PaintThis(RenderSurface* surf, sint32 lerp_factor)
{
	Gump::PaintThis(surf, lerp_factor);
}

bool PagedGump::OnKeyDown(int key, int mod)
{
	switch (key)
	{
	case SDLK_ESCAPE:
	{
		Close();
	} break;
	default:
		break;
	}

	return true;
}

void PagedGump::ChildNotify(Gump *child, uint32 message)
{
	ObjId cid = child->getObjId();
	if (gumps.empty()) return;

	if (message == ButtonWidget::BUTTON_CLICK)
	{
		if (cid == nextButton->getObjId())
		{
			if (current + 1 != gumps.end())
			{
				(*current)->HideGump();
				++current;
				(*current)->UnhideGump();

				if (current + 1 == gumps.end())
					nextButton->HideGump();

				prevButton->UnhideGump();
			}
		}
		else if (cid == prevButton->getObjId())
		{
			if (current != gumps.begin())
			{
				(*current)->HideGump();
				--current;
				(*current)->UnhideGump();
				
				if (current == gumps.begin())
					prevButton->HideGump();

				nextButton->UnhideGump();
			}
		}
	}
}

void PagedGump::addPage(Gump * g)
{
	AddChild(g);
	g->setRelativePosition(TOP_CENTER, 0, 14 + topOff);
	g->HideGump();
	gumps.push_back(g);

	current = gumps.begin();
	(*current)->UnhideGump();
	
	if (current + 1 == gumps.end())
		nextButton->HideGump();
	else
		nextButton->UnhideGump();
}

bool PagedGump::loadData(IDataSource* ids)
{
	return false;
}

void PagedGump::saveData(ODataSource* ods)
{
}
