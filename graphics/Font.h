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

#ifndef FONT_H
#define FONT_H

#include "Shape.h"

#include <string>

class Font : public Shape
{
public:
	Font(const uint8* data, uint32 size, const ConvertShapeFormat *format)
		: Shape(data, size, format) { }
	virtual ~Font() { }

	int getWidth(char c);


	ENABLE_DYNAMIC_CAST(Font);
};

#endif
