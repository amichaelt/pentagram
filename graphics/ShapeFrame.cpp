/*
 *  Copyright (C) 2003 The Pentagram Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "pent_include.h"

#include "ShapeFrame.h"


/*
  parse data and fill class
 */
ShapeFrame::ShapeFrame(const uint8* data, uint32 size)
{
	//NB: This is U8 style!

	compressed = data[8];
	width = data[10] + (data[11]<<8);
	height = data[12] + (data[13]<<8);
	sint16 xoff = data[14] + (data[15]<<8);
	sint16 yoff = data[16] + (data[17]<<8);

	xoffset = xoff;
	yoffset = yoff;

	this->data = data + 18;
	this->size = size - 18;
}

ShapeFrame::~ShapeFrame()
{

}
