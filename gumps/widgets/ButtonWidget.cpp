/*
 *  Copyright (C) 2003-2004  The Pentagram Team
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
#include "ButtonWidget.h"
#include "TextWidget.h"
#include "GUIApp.h"
#include "GameData.h"
#include "ShapeFrame.h"
#include "ShapeArchive.h"
#include "Shape.h"
#include "Mouse.h"

#include "IDataSource.h"
#include "ODataSource.h"

// p_dynamic_class stuff 
DEFINE_RUNTIME_CLASSTYPE_CODE(ButtonWidget,Gump);

ButtonWidget::ButtonWidget()
	: Gump()
{
}

ButtonWidget::ButtonWidget(int X, int Y, std::string txt, int font,
						   int w, int h) :
	Gump(X,Y,w,h), shape_up(0), shape_down(0), mouseOver(false)
{
	TextWidget* widget = new TextWidget(0,0,txt,font,w,h);
	textwidget = widget->getObjId();
}

ButtonWidget::ButtonWidget(int X, int Y, FrameID frame_up, FrameID frame_down, 							  bool _mouseOver)
	: Gump(X,Y,5,5), textwidget(0), mouseOver(_mouseOver)
{
	shape_up = GameData::get_instance()->getShape(frame_up);
	shape_down = GameData::get_instance()->getShape(frame_down);
	framenum_up = frame_up.framenum;
	framenum_down = frame_down.framenum;
}


ButtonWidget::~ButtonWidget(void)
{
}

void ButtonWidget::InitGump()
{
	if (textwidget != 0) {
		Gump* widget = GUIApp::get_instance()->getGump(textwidget);
		assert(widget);
		widget->InitGump();

		widget->GetDims(dims); // transfer child dimension to self
		AddChild(widget);
		widget->Move(0,dims.y); // move it to the correct height
	} else {
		assert(shape_up != 0);
		assert(shape_down != 0);

		shape = shape_up;
		framenum = framenum_up;

		ShapeFrame* sf = shape->getFrame(framenum);
		assert(sf);
		dims.w = sf->width;
		dims.h = sf->height;
	}
}

bool ButtonWidget::PointOnGump(int mx, int my)
{
	// CHECKME: this makes the ButtonWidget accept any point in its rectangle,
	// effectively ignoring shape transparency. For some buttons (like those
	// in U8's diary), this is desirable. For others it may not be, so this
	// behaviour may need to be changed. (bool ignoreShapeTransparency or
	// something)

	int gx = mx, gy = my;
	ParentToGump(gx,gy);

	return dims.InRect(gx,gy);
}

Gump *ButtonWidget::OnMouseDown(int button, int mx, int my)
{
	Gump *ret = Gump::OnMouseDown(button,mx,my);
	if (ret) return ret;
	if (button == BUTTON_LEFT)
	{
		// CHECKME: change dimensions or not?
		if (!mouseOver) {
			shape = shape_down;
			framenum = framenum_down;
		}
		return this;
    }
	return 0;
}

uint16 ButtonWidget::TraceObjId(int mx, int my)
{
	if (PointOnGump(mx, my))
		return getObjId();
	else
		return 0;
}


void ButtonWidget::OnMouseUp(int button, int mx, int my)
{
	if (button == BUTTON_LEFT) {
		if (!mouseOver) {
			shape = shape_up;
			framenum = framenum_up;
		}
		parent->ChildNotify(this,BUTTON_UP);
	}
}

void ButtonWidget::OnMouseClick(int button, int mx, int my)
{
	int gx = mx, gy = my;
	parent->ScreenSpaceToGump(gx,gy);
	if (PointOnGump(gx, gy))
		parent->ChildNotify(this,BUTTON_CLICK);
}

void ButtonWidget::OnMouseDouble(int button, int mx, int my)
{
	parent->ChildNotify(this,BUTTON_DOUBLE);
}

void ButtonWidget::OnMouseOver()
{
	if (mouseOver) {
		shape = shape_down;
		framenum = framenum_down;
	}
}

void ButtonWidget::OnMouseLeft()
{
	if (mouseOver) {
		shape = shape_up;
		framenum = framenum_up;
	}
}

void ButtonWidget::saveData(ODataSource* ods)
{
	Gump::saveData(ods);

	uint16 flex = 0;
	uint32 shapenum = 0;
	if (shape_up)
	{
		shape_up->getShapeId(flex, shapenum);
	}
	ods->write2(flex);
	ods->write4(shapenum);
	ods->write4(framenum_up);

	if (shape_down)
	{
		shape_down->getShapeId(flex, shapenum);
	}
	ods->write2(flex);
	ods->write4(shapenum);
	ods->write4(framenum_down);
	ods->write2(textwidget);

	uint8 m = (mouseOver ? 1 : 0);
	ods->write1(m);
}

bool ButtonWidget::loadData(IDataSource* ids, uint32 version)
{
	if (!Gump::loadData(ids, version)) return false;

	shape_up = 0;
	ShapeArchive * flex = GameData::get_instance()->getShapeFlex(ids->read2());
	uint32 shapenum = ids->read4();
	if (flex)
	{
		shape_up = flex->getShape(shapenum);
	}
	framenum_up = ids->read4();

	shape_down = 0;
	flex = GameData::get_instance()->getShapeFlex(ids->read2());
	shapenum = ids->read4();
	if (flex)
	{
		shape_down = flex->getShape(shapenum);
	}
	framenum_down = ids->read4();
	textwidget = ids->read2();
	mouseOver = (ids->read1() != 0);

	return true;
}
