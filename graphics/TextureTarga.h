/*
 *  Copyright (C) 2002  Ryan Nunn and The Pentagram Team
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

#ifndef TEXTURETARGA_H
#define TEXTURETARGA_H

#include "Texture.h"

struct TGA;

// container structure for bitmaps .BMP file
struct TextureTarga : public Texture
{
	TextureTarga() : Texture()
	{
	}

	// False on Error
	virtual bool Read(DataSource &ds);

protected:

	void ConvertFormat(uint8 *buffer, TGA &tga);
};


#endif
