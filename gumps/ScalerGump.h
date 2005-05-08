/*
 *  Copyright (C) 2004-2005  The Pentagram Team
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

#ifndef ScalerGump_H
#define ScalerGump_H

#include "DesktopGump.h"
#include <vector>
class RenderSurface;

namespace Pentagram {
	class Scaler;
};

class ScalerGump : public DesktopGump
{
public:
	ENABLE_RUNTIME_CLASSTYPE();

	//! ScalerGump constructor
	//! \param x x coordinate of our top-left corner in parent
	//! \param y y coordinate
	//! \param width width after scaling (usually screenwidth)
	//! \param height height after scaling (usually screenheight)
	//! \param swidth1 width before scaling (usually game-width)
	//! \param sheight1 height before scaling (usually game-height)
	//! \param scaler1 scaler to use
	ScalerGump(sint32 x, sint32 y, sint32 width, sint32 height, sint32 swidth1, sint32 sheight1, const Pentagram::Scaler *scaler1, sint32 swidth2=0, sint32 sheight2=0, const Pentagram::Scaler *scaler2=0);
	virtual ~ScalerGump(void);

	virtual void Paint(RenderSurface* surf, sint32 lerp_factor);

	virtual void ParentToGump(int &px, int &py);
	virtual void GumpToParent(int &gx, int &gy);
	
	void GetScaledSize(sint32 &sw, sint32 &sh) const { sw = swidth1; sh = sheight1; }

protected:
	sint32					swidth1;
	sint32					sheight1;
	const Pentagram::Scaler	*scaler1;
	RenderSurface			*buffer1;

	sint32					swidth2;
	sint32					sheight2;
	const Pentagram::Scaler	*scaler2;
	RenderSurface			*buffer2;

	sint32					width;
	sint32					height;
};

#endif
