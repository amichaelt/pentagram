/*
Copyright (C) 2004 The Pentagram team

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

#ifndef TTFONT_H
#define TTFONT_H

#ifdef USE_SDLTTF

#include "Font.h"

#include "SDL_ttf.h"

class IDataSource;

class TTFont : public Pentagram::Font
{
public:
	TTFont(IDataSource* font, uint32 rgb, int pointsize);
	virtual ~TTFont();

	virtual int getHeight();
	virtual int getBaseline();
	virtual int getBaselineSkip();

	virtual void getStringSize(std::string& text, int& width, int& height);

	virtual RenderedText* renderText(std::string text,
									 unsigned int& remaining,
									 int width=0, int height=0,
									 TextAlign align=TEXT_LEFT);

	ENABLE_RUNTIME_CLASSTYPE();
protected:
	TTF_Font* ttf_font;
	uint32 rgb;
};


#endif

#endif
