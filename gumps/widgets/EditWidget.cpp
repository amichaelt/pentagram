/*
 *  Copyright (C) 2005-2006  The Pentagram Team
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
#include "EditWidget.h"
#include "ShapeFont.h"
#include "RenderedText.h"
#include "RenderSurface.h"
#include "FontManager.h"
#include "IDataSource.h"
#include "ODataSource.h"
#include "TTFont.h"
#include "encoding.h"

#include "SDL.h"

DEFINE_RUNTIME_CLASSTYPE_CODE(EditWidget,Gump);

EditWidget::EditWidget(int X, int Y, std::string txt, bool gamefont_, int font,
					   int w, int h, unsigned int maxlength_, bool multiline_)
	: Gump(X, Y, w, h), text(txt), gamefont(gamefont_), fontnum(font),
	  maxlength(maxlength_), multiline(multiline_),
	  cursor_changed(0), cursor_visible(true), cached_text(0)
{
	cursor = text.size();
}

EditWidget::~EditWidget(void)
{
	delete cached_text;
	cached_text = 0;
}

// Init the gump, call after construction
void EditWidget::InitGump(Gump* newparent, bool take_focus)
{
	Gump::InitGump(newparent, take_focus);

	Pentagram::Font *font = getFont();

	// Y offset is always baseline
	dims.y = -font->getBaseline();

	// No X offset
	dims.x = 0;

	if (gamefont) 
	{
		Pentagram::Font *font = getFont();
		if (font->isHighRes())
		{
			int ssx = 0, ssy = font->getBaseline();
			ScreenSpaceToGumpVec(ssx,ssy);
			dims.y = -ssy;
		}
	}
}

Pentagram::Font* EditWidget::getFont() const
{
	if (gamefont)
		return FontManager::get_instance()->getGameFont(fontnum, true);
	else
		return FontManager::get_instance()->getTTFont(fontnum);
}

void EditWidget::setText(const std::string& t)
{
	text = t;
	cursor = text.size();
	FORGET_OBJECT(cached_text);
}

void EditWidget::ensureCursorVisible()
{
	cursor_visible = true;
	cursor_changed = SDL_GetTicks();
}

bool EditWidget::textFits(std::string& t)
{
	Pentagram::Font *font = getFont();
	
	unsigned int remaining;
	int width, height;

	int max_width = multiline ? dims.w : 0;
	int max_height = dims.h;
	if (gamefont && font->isHighRes())
		GumpVecToScreenSpace(max_width,max_height);

	font->getTextSize(t, width, height, remaining,
					  max_width, max_height,
					  Pentagram::Font::TEXT_LEFT, false);

	if (gamefont && font->isHighRes())
		ScreenSpaceToGumpVec(width,height);

	if (multiline)
		return (remaining >= t.size());
	else
		return (width <= dims.w);
}

void EditWidget::renderText()
{
	bool cv = cursor_visible;
	if (!IsFocus()) {
		cv = false;
	} else {
		uint32 now = SDL_GetTicks();
		if (now > cursor_changed + 750) {
			cv = !cursor_visible;
			cursor_changed = now;
		}
	}

	if (cv != cursor_visible) {
		FORGET_OBJECT(cached_text);
		cursor_visible = cv;
	}

	if (!cached_text) {
		Pentagram::Font *font = getFont();

		int max_width = multiline ? dims.w : 0;
		int max_height = dims.h;
		if (gamefont && font->isHighRes())
			GumpVecToScreenSpace(max_width,max_height);

		unsigned int remaining;
		cached_text = font->renderText(text, remaining,
									   max_width, max_height,
									   Pentagram::Font::TEXT_LEFT,
									   false, cv ? cursor : std::string::npos);
	}
}

// Overloadable method to Paint just this Gump (RenderSurface is relative to this)
void EditWidget::PaintThis(RenderSurface*surf, sint32 lerp_factor, bool scaled)
{
	Gump::PaintThis(surf,lerp_factor, scaled);

	renderText();

	if (scaled && gamefont && getFont()->isHighRes())
	{
		surf->FillAlpha(0xFF,dims.x,dims.y,dims.w, dims.h);
		return;
	}

	cached_text->draw(surf, 0, 0);
}

void EditWidget::PaintCompositing(RenderSurface* surf, sint32 lerp_factor, sint32 sx, sint32 sy)
{
	// Don't paint if hidden
	if (IsHidden()) return;

	// Get old Origin
	int ox=0, oy=0;
	surf->GetOrigin(ox, oy);

	// FIXME - Big accuracy problems here with the origin and clipping rect

	// Set the new Origin
	int nx=0, ny=0;
	GumpToParent(nx,ny);
	surf->SetOrigin(ox+ScaleCoord(nx,sx), oy+ScaleCoord(ny,sy));

	// Get Old Clipping Rect
	Pentagram::Rect old_rect;
	surf->GetClippingRect(old_rect);

	// Set new clipping rect
	Pentagram::Rect new_rect( 0, -getFont()->getBaseline(), ScaleCoord(dims.w,sx), ScaleCoord(dims.h,sy) );
	new_rect.Intersect(old_rect);
	surf->SetClippingRect(new_rect);

	// Iterate all children
	std::list<Gump*>::reverse_iterator it = children.rbegin();
	std::list<Gump*>::reverse_iterator end = children.rend();

	while (it != end)
	{
		Gump *g = *it;
		// Paint if not closing
		if (!g->IsClosing()) 
			g->PaintCompositing(surf, lerp_factor, sx, sy);

		++it;
	}	

	// Paint This
	PaintComposited(surf, lerp_factor, sx, sy);

	// Reset The Clipping Rect
	surf->SetClippingRect(old_rect);

	// Reset The Origin
	surf->SetOrigin(ox, oy);
}

// Overloadable method to Paint just this gumps unscaled components that require compositing (RenderSurface is relative to parent).
void EditWidget::PaintComposited(RenderSurface* surf, sint32 lerp_factor, sint32 sx, sint32 sy)
{
	Pentagram::Font *font = getFont();

	if (!gamefont || !font->isHighRes())
	{
		return;
	}

	cached_text->draw(surf, 0, 0, true);

	surf->FillAlpha(0x00, 0, -getFont()->getBaseline(), ScaleCoord(dims.w,sx), ScaleCoord(dims.h,sy));
}

// don't handle any mouse motion events, so let parent handle them for us.
Gump* EditWidget::OnMouseMotion(int mx, int my)
{
	return 0;
}

bool EditWidget::OnKeyDown(int key, int mod)
{
	switch (key) {
	case SDLK_RETURN:
	case SDLK_KP_ENTER:
		parent->ChildNotify(this, EDIT_ENTER);
		break;
	case SDLK_ESCAPE:
		parent->ChildNotify(this, EDIT_ESCAPE);
		break;
	case SDLK_BACKSPACE:
		if (cursor > 0) {
			text.erase(--cursor, 1);
			FORGET_OBJECT(cached_text);
			ensureCursorVisible();
		}
		break;
	case SDLK_DELETE:
		if (cursor != text.size()) {
			text.erase(cursor, 1);
			FORGET_OBJECT(cached_text);
		}
		break;
	case SDLK_LEFT:
		if (cursor > 0) {
			cursor--;
			FORGET_OBJECT(cached_text);
			ensureCursorVisible();
		}
		break;
	case SDLK_RIGHT:
		if (cursor < text.size()) {
			cursor++;
			FORGET_OBJECT(cached_text);
			ensureCursorVisible();
		}
		break;
	default:
		break;
	}

	return true;
}

bool EditWidget::OnKeyUp(int key)
{
	return true;
}


bool EditWidget::OnTextInput(int unicode)
{
	if (maxlength > 0 && text.size() >= maxlength)
		return true;

	char c = 0;
	if (unicode >= 0 && unicode < 256)
		c = Pentagram::reverse_encoding[unicode];
	if (!c) return true;

	std::string newtext = text;
	newtext.insert(cursor, 1, c);

	if (textFits(newtext)) {
		text = newtext;
		cursor++;
		FORGET_OBJECT(cached_text);
	}

	return true;
}
