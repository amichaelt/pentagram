/*
 *  Copyright (C) 2003  Pentagram Team
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

#ifndef XFORMBLEND_H
#define XFORMBLEND_H

#include "Texture.h"
#include "RenderSurface.h"

#ifndef P_FASTCALL
#ifdef _MSC_VER
#define P_FASTCALL __fastcall
#elif (defined(WIN32) && defined(FASTCALL))
#define P_FASTCALL FASTCALL
#else
#define P_FASTCALL
#endif
#endif

extern const uint8 U8XFormPal[1024];

inline uint32 P_FASTCALL BlendPreModulated(uint32 xform, uint32 dst)
{
	uint32 r, g, b;
	UNPACK_RGB8(dst,r,g,b);
	r*=256-TEX32_A(xform);
	g*=256-TEX32_A(xform);
	b*=256-TEX32_A(xform);
	r+=256*TEX32_R(xform);
	g+=256*TEX32_G(xform);
	b+=256*TEX32_B(xform);
	return PACK_RGB16(r>65535?65535:r, g>65535?65535:g, b>65535?65535:b);
}

// This does the red highlight blending. 
inline uint32 P_FASTCALL BlendHighlight(uint32 src, uint32 cr, uint32 cg, uint32 cb, uint32 ca, uint32 ica)
{
	uint32 sr, sg, sb;
	UNPACK_RGB8(src,sr,sg,sb);
	return PACK_RGB16(sr*ica+cr*ca, sg*ica+cg*ca, sb*ica+cb*ca);
}

// This does the invisible blending. I've set it to about 40%
inline uint32 P_FASTCALL BlendInvisible(uint32 src, uint32 dst)
{
	uint32 sr, sg, sb;
	uint32 dr, dg, db;
	UNPACK_RGB8(src,sr,sg,sb);
	UNPACK_RGB8(dst,dr,dg,db);
	return PACK_RGB16(sr*100+dr*156,
						sg*100+dg*156,
						sb*100+db*156);
}

// This does the translucent highlight blending. (50%) 
inline uint32 P_FASTCALL BlendHighlightInvis(uint32 src, uint32 dst, uint32 cr, uint32 cg, uint32 cb, uint32 ca, uint32 ica)
{
	uint32 sr, sg, sb;
	uint32 dr, dg, db;
	UNPACK_RGB8(src,sr,sg,sb);
	UNPACK_RGB8(dst,dr,dg,db);
	return PACK_RGB16(((sr*ica+cr*ca)>>1)+(dr<<7), 
						((sg*ica+cg*ca)>>1)+(dg<<7), 
						((sb*ica+cb*ca)>>1)+(db<<7));
}


#endif
