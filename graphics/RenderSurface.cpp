/*
RenderSurface.cpp : RenderSurface Interface source file

Copyright (C) 2002, 2003 The Pentagram Team

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

#include "pent_include.h"
#include "RenderSurface.h"
#include "SoftRenderSurface.h"
#include <SDL.h>


//
// RenderSurface::SetVideoMode()
//
// Desc: Create a standard RenderSurface
// Returns: Created RenderSurface or 0
//

RenderSurface *RenderSurface::SetVideoMode(uint32 width,		// Width of desired mode
									uint32 height,		// Height of desired mode
									uint32 bpp,			// Bits Per Pixel of desired mode
									bool fullscreen,	// Fullscreen if true, Windowed if false
									bool use_opengl)	// Use OpenGL if true, Software if false
{
	// TODO: Add in OpenGL
	if (use_opengl)
	{
		pout << "OpenGL Mode not enabled" << std::endl;
		// TODO: Set Error Code
		return 0;
	}

	// check to make sure a 16 bit or 32 bit Mode has been requested
	if (bpp != 16 && bpp != 32)
	{
		pout << "Only 16 bit and 32 bit video modes supported" << std::endl;
		// TODO: Set Error Code
		return 0;
	}

	// SDL Flags to set
	uint32 flags = 0;

	// Get Current Video Mode details
	const SDL_VideoInfo *vinfo = SDL_GetVideoInfo();

	// Specific Windowed code
	if (!fullscreen) 
	{
		// Use the BPP of the desktop
		bpp = vinfo->vfmt->BitsPerPixel;

		// check to make sure we are in 16 bit or 32 bit
		if (bpp != 16 && bpp != 32)
		{
			pout << bpp << " bit windowed mode unsupported" << std::endl;
			// TODO: Set Error Code
			return 0;
		}

		// Enable Resizable
		flags |= SDL_RESIZABLE;
	}
	// Fullscreen Specific 
	else
	{
		// Enable Fullscreen
		flags |= SDL_FULLSCREEN;
	}

	// Double buffered (sdl will emulate if we don't have)
	flags |= SDL_HWSURFACE|SDL_DOUBLEBUF;

	SDL_Surface *sdl_surf = SDL_SetVideoMode(width, height, bpp, flags);
	
	if (!sdl_surf)
	{
		// TODO: Set Error Code
		return 0;
	}

	// Now create the SoftRenderSurface
	RenderSurface *surf;

	// TODO: Change this
	if (bpp == 32) surf = new SoftRenderSurface<uint32>(sdl_surf);
	else surf = new SoftRenderSurface<uint16>(sdl_surf);

	return surf;
}


RenderSurface::~RenderSurface()
{
}
