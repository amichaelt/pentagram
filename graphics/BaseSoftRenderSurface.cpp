	/*
BaseSoftRenderSurface.cpp : Abstract BaseSoftRenderSurface source file

Copyright (C) 2003 The Pentagram Team

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
#include <SDL.h>

#include "SoftRenderSurface.h"
#include "Palette.h"
#include "Texture.h"

using Pentagram::Rect;

///////////////////////////
//                       //
// BaseSoftRenderSurface //
//                       //
///////////////////////////

//
// BaseSoftRenderSurface::BaseSoftRenderSurface(SDL_Surface *s)
//
// Desc: Constructor for BaseSoftRenderSurface from a SDL_Surface
//
BaseSoftRenderSurface::BaseSoftRenderSurface(SDL_Surface *s) :
	pixels(0), pixels00(0), zbuffer(0), zbuffer00(0),
	bytes_per_pixel(0), bits_per_pixel(0), format_type(0), 
	ox(0), oy(0), width(0), height(0), pitch(0), zpitch(0),
	flipped(false), clip_window(0,0,0,0), lock_count(0),
	sdl_surf(s), rtt_tex(0)
{
	clip_window.ResizeAbs(width = sdl_surf->w, height = sdl_surf->h);
	pitch = sdl_surf->pitch;
	bits_per_pixel = sdl_surf->format->BitsPerPixel;
	bytes_per_pixel = sdl_surf->format->BytesPerPixel;

	RenderSurface::s_bpp = bits_per_pixel;
	RenderSurface::r_loss = sdl_surf->format->Rloss;
	RenderSurface::g_loss = sdl_surf->format->Gloss;
	RenderSurface::b_loss = sdl_surf->format->Bloss;
	RenderSurface::a_loss = sdl_surf->format->Aloss;
	RenderSurface::r_loss16 = r_loss+8;
	RenderSurface::g_loss16 = g_loss+8;
	RenderSurface::b_loss16 = b_loss+8;
	RenderSurface::a_loss16 = a_loss+8;
	RenderSurface::r_shift = sdl_surf->format->Rshift;
	RenderSurface::g_shift = sdl_surf->format->Gshift;
	RenderSurface::b_shift = sdl_surf->format->Bshift;
	RenderSurface::a_shift = sdl_surf->format->Ashift;
	RenderSurface::r_mask = sdl_surf->format->Rmask;
	RenderSurface::g_mask = sdl_surf->format->Gmask;
	RenderSurface::b_mask = sdl_surf->format->Bmask;
	RenderSurface::a_mask = sdl_surf->format->Amask;

	SetPixelsPointer();
}


//
// BaseSoftRenderSurface::BaseSoftRenderSurface(int w, int h, int bpp, int rsft, int gsft, int bsft)
//
// Desc: Constructor for Generic BaseSoftRenderSurface
//
BaseSoftRenderSurface::BaseSoftRenderSurface(int w, int h, int bpp, 
											int rsft, int gsft, int bsft, int asft) :
	pixels(0), pixels00(0), zbuffer(0), zbuffer00(0),
	bytes_per_pixel(0), bits_per_pixel(0), format_type(0), 
	ox(0), oy(0), width(0), height(0), pitch(0), zpitch(0),
	flipped(false), clip_window(0,0,0,0), lock_count(0), sdl_surf(0), rtt_tex(0)
{
	clip_window.ResizeAbs(width = w, height = h);

	switch (bpp) {
		case 15:
			r_loss = 3;
			g_loss = 3;
			b_loss = 3;
			a_loss = 7;
			bpp = 16;
			break;

		case 16:
			r_loss = 3;
			g_loss = 2;
			b_loss = 3;
			a_loss = 0;
			break;

		case 32:
			r_loss = 0;
			g_loss = 0;
			b_loss = 0;
			a_loss = 0;
			break;

		default:
			break;
	}

	pitch = w*bpp/8;
	bits_per_pixel = bpp;
	bytes_per_pixel = bpp/8;

	RenderSurface::s_bpp = bpp;
	RenderSurface::r_loss16 = r_loss+8;
	RenderSurface::g_loss16 = g_loss+8;
	RenderSurface::b_loss16 = b_loss+8;
	RenderSurface::a_loss16 = a_loss+8;
	RenderSurface::r_shift = rsft;
	RenderSurface::g_shift = gsft;
	RenderSurface::b_shift = bsft;
	RenderSurface::a_shift = asft;
	RenderSurface::r_mask = (0xFF>>r_loss)<<rsft;
	RenderSurface::g_mask = (0xFF>>g_loss)<<gsft;
	RenderSurface::b_mask = (0xFF>>b_loss)<<bsft;
	RenderSurface::a_mask = (0xFF>>a_loss)<<asft;

	SetPixelsPointer();
}


//
// BaseSoftRenderSurface::BaseSoftRenderSurface(int w, int h, uint8 *buf)
//
// Desc: Constructor for Generic BaseSoftRenderSurface which matches screen params
//
BaseSoftRenderSurface::BaseSoftRenderSurface(int w, int h, uint8 *buf) :
	pixels(0), pixels00(0), zbuffer(0), zbuffer00(0),
	bytes_per_pixel(0), bits_per_pixel(0), format_type(0), 
	ox(0), oy(0), width(0), height(0), pitch(0), zpitch(0),
	flipped(false), clip_window(0,0,0,0), lock_count(0), sdl_surf(0), rtt_tex(0)
{
	clip_window.ResizeAbs(width = w, height = h);

	int bpp = RenderSurface::s_bpp;

	pitch = w*bpp/8;
	bits_per_pixel = bpp;
	bytes_per_pixel = bpp/8;
	pixels00 = buf;

	SetPixelsPointer();
}

//
// BaseSoftRenderSurface::BaseSoftRenderSurface(int w, int h, uint8 *buf)
//
// Desc: Constructor for Generic BaseSoftRenderSurface which matches screen params
//
BaseSoftRenderSurface::BaseSoftRenderSurface(int w, int h) :
	pixels(0), pixels00(0), zbuffer(0), zbuffer00(0),
	bytes_per_pixel(0), bits_per_pixel(0), format_type(0), 
	ox(0), oy(0), width(0), height(0), pitch(0), zpitch(0),
	flipped(false), clip_window(0,0,0,0), lock_count(0), sdl_surf(0), rtt_tex(0)
{
	clip_window.ResizeAbs(width = w, height = h);

	int bpp = RenderSurface::s_bpp;

	pitch = w*bpp/8;
	bits_per_pixel = bpp;
	bytes_per_pixel = bpp/8;
	pixels00 = new uint8[pitch * height];

	rtt_tex = new Texture;
	rtt_tex->buffer = reinterpret_cast<uint32*>(pixels00);
	rtt_tex->width = width;
	rtt_tex->height = height;
	rtt_tex->format = TEX_FMT_NATIVE;
	rtt_tex->CalcLOG2s();

	SetPixelsPointer();
}


//
// BaseSoftRenderSurface::~BaseSoftRenderSurface()
//
// Desc: Destructor
//
BaseSoftRenderSurface::~BaseSoftRenderSurface()
{
	if (rtt_tex)
	{
		delete rtt_tex;
		rtt_tex = 0;

		delete [] pixels00;
		pixels00 = 0;

		delete [] zbuffer00;
		zbuffer00 = 0;
	}
}


//
// BaseSoftRenderSurface::BeginPainting()
//
// Desc: Prepare the surface for drawing this frame (in effect lock it for drawing)
// Returns: Non Zero on error
//
ECode BaseSoftRenderSurface::BeginPainting()
{
	if (!lock_count) {

		if (sdl_surf) {

			// SDL_Surface requires locking
			if (SDL_MUSTLOCK(sdl_surf))
			{
				// Did the lock fail?
				if (SDL_LockSurface(sdl_surf)!=0) {
					pixels = pixels00 = 0;
					// TODO: SetLastError(GR_SOFT_ERROR_SDL_LOCK_FAILED, "SDL Surface Lock Failed!");
					perr << "Error: SDL Surface Lock Failed!" << std::endl;
					return GR_SOFT_ERROR_SDL_LOCK_FAILED;
				}
			}

			// Pixels pointer
			pixels00 = static_cast<uint8*>(sdl_surf->pixels);
			pitch = sdl_surf->pitch;
			if (flipped) pitch = -pitch;
		}
		else  {
			ECode ret = GenericLock();
			if (ret.failed()) return ret;
		}
	}

	lock_count++;
	
	if (pixels00 == 0) 
	{
		// TODO: SetLastError(GR_SOFT_ERROR_LOCKED_NULL_PIXELS, "Surface Locked with NULL BaseSoftRenderSurface::pixels pointer!");
		perr << "Error: Surface Locked with NULL BaseSoftRenderSurface::pixels pointer!" << std::endl;
		return GR_SOFT_ERROR_LOCKED_NULL_PIXELS;
	}

	// Origin offset pointers
	SetPixelsPointer();

	// No error
	return P_NO_ERROR;
}


//
// BaseSoftRenderSurface::EndPainting()
//
// Desc: Prepare the surface for drawing this frame (in effect lock it for drawing)
// Returns: Non Zero on error
//
ECode BaseSoftRenderSurface::EndPainting()
{
	// Already Unlocked
	if (!lock_count)
	{
		// TODO: SetLastError(GR_SOFT_ERROR_BEGIN_END_MISMATCH, "BeginPainting()/EndPainting() Mismatch!");
		perr << "Error: BeginPainting()/EndPainting() Mismatch!" << std::endl;
		return GR_SOFT_ERROR_BEGIN_END_MISMATCH;
	}

	// Decrement counter
	--lock_count;

	if (!lock_count) { 
		if (sdl_surf) {
			// Unlock the SDL_Surface if required
			if (SDL_MUSTLOCK(sdl_surf)) SDL_UnlockSurface(sdl_surf);

			// Clear pointers
			pixels=pixels00=0;

			// Present
			SDL_Flip (sdl_surf);
		}
		else {
			ECode ret = GenericUnlock();
			if (ret.failed()) return ret;
		}
	}

	// No error
	return P_NO_ERROR;
}

//
// Texture *BaseSoftRenderSurface::GetSurfaceAsTexture()
//
// Desc: Get the surface as a Texture. Only valid for SecondaryRenderSurfaces
//
Texture *BaseSoftRenderSurface::GetSurfaceAsTexture()
{ 
	if (!rtt_tex)
	{
		perr << "Error: GetSurfaceAsTexture(): Surface doesn't render-to-texture" << std::endl;
	}

	return rtt_tex; 
}

//
// void BaseSoftRenderSurface::CreateNativePalette(Palette* palette)
//
// Desc: Create a palette of colours native to the surface
//
void BaseSoftRenderSurface::CreateNativePalette(Pentagram::Palette* palette)
{
	for (int i = 0; i < 256; i++)
	{
		sint32 r,g,b;

		// Normal palette
		palette->native_untransformed[i] = PACK_RGB8(palette->palette[i*3+0],
													palette->palette[i*3+1],
													palette->palette[i*3+2]);

		r = palette->matrix[0] * palette->palette[i*3+0] +
			palette->matrix[1] * palette->palette[i*3+1] +
			palette->matrix[2] * palette->palette[i*3+2] +
			palette->matrix[3] * 255;
		if (r < 0) r = 0;
		if (r > 0x7F800) r = 0x7F800;

		g = palette->matrix[4] * palette->palette[i*3+0] +
			palette->matrix[5] * palette->palette[i*3+1] +
			palette->matrix[6] * palette->palette[i*3+2] +
			palette->matrix[7] * 255;
		if (g < 0) g = 0;
		if (g > 0x7F800) g = 0x7F800;

		b = palette->matrix[8] * palette->palette[i*3+0] +
			palette->matrix[9] * palette->palette[i*3+1] +
			palette->matrix[10] * palette->palette[i*3+2] +
			palette->matrix[11] * 255;
		if (b < 0) b = 0;
		if (b > 0x7F800) b = 0x7F800;

		// Transformed normal palette
		// FIXME - Wont work on non SDL SRS Implementations
		palette->native[i] = PACK_RGB8(static_cast<uint8>(r>>11),
										static_cast<uint8>(g>>11),
										static_cast<uint8>(b>>11));

		// Transformed XFORM palette (Uses the TEX32 format)
		if (TEX32_A(palette->xform_untransformed[i]))
		{
			r = palette->matrix[0] * TEX32_R(palette->xform_untransformed[i]) +
				palette->matrix[1] * TEX32_G(palette->xform_untransformed[i]) +
				palette->matrix[2] * TEX32_B(palette->xform_untransformed[i]) +
				palette->matrix[3] * 255;
			if (r < 0) r = 0;
			if (r > 0x7F800) r = 0x7F800;

			g = palette->matrix[4] * TEX32_R(palette->xform_untransformed[i]) +
				palette->matrix[5] * TEX32_G(palette->xform_untransformed[i]) +
				palette->matrix[6] * TEX32_B(palette->xform_untransformed[i]) +
				palette->matrix[7] * 255;
			if (g < 0) g = 0;
			if (g > 0x7F800) g = 0x7F800;

			b = palette->matrix[8] * TEX32_R(palette->xform_untransformed[i]) +
				palette->matrix[9] * TEX32_G(palette->xform_untransformed[i]) +
				palette->matrix[10] * TEX32_B(palette->xform_untransformed[i]) +
				palette->matrix[11] * 255;
			if (b < 0) b = 0;
			if (b > 0x7F800) b = 0x7F800;

			palette->xform[i] = TEX32_PACK_RGBA(static_cast<uint8>(r>>11),
												static_cast<uint8>(g>>11),
												static_cast<uint8>(b>>11),
								TEX32_A(palette->xform_untransformed[i]));
		}
		else
			palette->xform[i] = 0;
	}
}

//
// void BaseSoftRenderSurface::GetSurfaceDims(Rect &r)
//
// Desc: Get the Surface Dimentions (and logical origin)
// r: Rect object to fill
//
void BaseSoftRenderSurface::GetSurfaceDims(Rect &r) const
{
	r.Set(ox, oy, width, height);
}

//
// void BaseSoftRenderSurface::SetOrigin(sint32 x, sint32 y)
//
// Desc: Set the Phyiscal Pixel to be the logical origin
//
void BaseSoftRenderSurface::SetOrigin(sint32 x, sint32 y)
{
	// Adjust the clipping window
	clip_window.MoveRel(ox-x, oy-y);

	// Set the origin
	ox = x;
	oy = y;

	// The new pointers
	SetPixelsPointer();
}

//
// void BaseSoftRenderSurface::GetOrigin(sint32 &x, sint32 &y)
//
// Desc: Get the Phyiscal Pixel that is the logical origin
//
void BaseSoftRenderSurface::GetOrigin(sint32 &x, sint32 &y) const
{
	// Set the origin
	x = ox;
	y = oy;
}

//
// void BaseSoftRenderSurface::GetClippingRect(Rect &r)
//
// Desc: Get the Clipping Rectangle
// r: Rect object to fill
//
void BaseSoftRenderSurface::GetClippingRect(Rect &r) const
{
	r = clip_window;
}

//
// void BaseSoftRenderSurface::GetClippingRect(Rect &r)
//
// Desc: Set the Clipping Rectangle
// r: Rect object that contains new Clipping Rectangle
//
void BaseSoftRenderSurface::SetClippingRect(const Rect &r)
{
	// What we need to do is to clip the clipping rect to the phyiscal screen
	clip_window = r;
	clip_window.Intersect(-ox,-oy, width, height);
}

//
// sint16 BaseSoftRenderSurface::CheckClipped(Rect &r)
//
// Desc: Check for a clipped rectangle
// Returns: -1 if off screen, 
//           0 if not clipped, 
//           1 if clipped
//
sint16 BaseSoftRenderSurface::CheckClipped(const Rect &c) const 
{
	Rect r = c;
	r.Intersect(clip_window);

	// Clipped away to the void
	if (!r.IsValid()) return -1;
	else if (r == c) return 0;
	else return 1;
}

//
// void BaseSoftRenderSurface::SetFlipped(bool flipped)
//
// Desc: Flip the surface
//
void BaseSoftRenderSurface::SetFlipped(bool wantFlipped)
{
	// Flipping is not terrible complex
	// But is a bit of a pain to set up

	// First we check to see if we are currently flipped
	if (wantFlipped == flipped) return;

	flipped = wantFlipped;

	// What we 'need' to do is negate the pitches, and flip the clipping window
	// We keep the 'origin' in the same position relative to the clipping window

	oy -= clip_window.y;
	clip_window.y = height - (clip_window.y + clip_window.h);
	oy += clip_window.y;

	pitch = -pitch;
	zpitch = -zpitch;

	SetPixelsPointer();

}

//
// bool BaseSoftRenderSurface::IsFlipped() const
//
// Desc: Has the render surface been flipped?
//
bool BaseSoftRenderSurface::IsFlipped() const
{
	return flipped;
}

