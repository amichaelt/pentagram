/*
 *  Copyright (C) 2005  The Pentagram Team
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
#include "CreditsGump.h"

#include "GUIApp.h"
#include "DesktopGump.h"
#include "RenderSurface.h"
#include "RenderedText.h"
#include "Font.h"
#include "FontManager.h"
#include "MusicProcess.h"
#include "SettingManager.h"

DEFINE_RUNTIME_CLASSTYPE_CODE(CreditsGump,ModalGump);

CreditsGump::CreditsGump()
	: ModalGump()
{

}

CreditsGump::CreditsGump(const std::string& text_, int parskip_,
						 uint32 _Flags, sint32 layer)
	: ModalGump(0, 0, 320, 200, 0, _Flags, layer)
{
	text = text_;
	parskip = parskip_;

	timer = 0;
	title = 0;
	nexttitle = 0;
	state = CS_PLAYING;
}

CreditsGump::~CreditsGump()
{
	delete scroll[0];
	delete scroll[1];
	delete scroll[2];
	delete scroll[3];

	delete title;
	delete nexttitle;
}

void CreditsGump::InitGump(Gump* newparent, bool take_focus)
{
	ModalGump::InitGump(newparent, take_focus);

	scroll[0] = RenderSurface::CreateSecondaryRenderSurface(256, 200);
	scroll[1] = RenderSurface::CreateSecondaryRenderSurface(256, 200);
	scroll[2] = RenderSurface::CreateSecondaryRenderSurface(256, 200);
	scroll[3] = RenderSurface::CreateSecondaryRenderSurface(256, 200);
	scroll[0]->Fill32(0xFF000000,0,0,256,200); // black background
	scroll[1]->Fill32(0xFF000000,0,0,256,200);
	scroll[2]->Fill32(0xFF000000,0,0,256,200);
	scroll[3]->Fill32(0xFF000000,0,0,256,200);
	scrollheight[0] = 156;
	scrollheight[1] = 0;
	scrollheight[2] = 0;
	scrollheight[3] = 0;

	currentsurface = 0;
	currenty = 0;

	GUIApp::get_instance()->pushMouseCursor();
	GUIApp::get_instance()->setMouseCursor(GUIApp::MOUSE_NONE);
}

void CreditsGump::Close(bool no_del)
{
	GUIApp::get_instance()->popMouseCursor();

	ModalGump::Close(no_del);

	MusicProcess* musicproc = MusicProcess::get_instance();
	if (musicproc) musicproc->playMusic(0);
}

void CreditsGump::extractLine(std::string& text,
							  char& modifier, std::string& line)
{
	if (text.empty()) {
		line = "";
		modifier = 0;
		return;
	}

	if (text[0] == '+' || text[0] == '&' || text[0] == '}' || text[0] == '~' ||
		text[0] == '@')
	{
		modifier = text[0];
		text.erase(0,1);
	} else {
		modifier = 0;
	}

	std::string::size_type starpos = text.find('*');

	line = text.substr(0, starpos);

	// replace '%%' by '%'.
	// (Original interpreted these strings as format strings??)
	std::string::size_type ppos;
	while ((ppos = line.find("%%")) != std::string::npos) {
		line.replace(ppos, 2, "%");
	}

	if (starpos != std::string::npos) starpos++;
	text.erase(0, starpos);
}


void CreditsGump::run()
{
	ModalGump::run();

	if (timer) {
		timer--;
		return;
	}

	if (state == CS_CLOSING) {
		// pout << "CreditsGump: closing" << std::endl;
		Close();
		return;
	}

	timer = 1;

	int available = scrollheight[currentsurface] - currenty;
	int nextblock = -1;
	for (int i = 1; i < 4; i++) {
		available += scrollheight[(currentsurface+i)%4];
		if (nextblock == -1 && scrollheight[(currentsurface+i)%4] == 0)
			nextblock = (currentsurface+i)%4;
	}
	if (available == 0) nextblock = 0;

	if (state == CS_FINISHING && available <= 156) {
		// pout << "CreditsGump: waiting before closing" << std::endl;
		timer = 120;
		state = CS_CLOSING;

		if (!configkey.empty()) {
			SettingManager* settingman = SettingManager::get_instance();
			settingman->set(configkey, true);
			settingman->write();
		}

		return;
	}

	if (state == CS_PLAYING && available <= 160) {
		// time to render next block

		scroll[nextblock]->Fill32(0xFF000000,0,0,256,200);
		// scroll[nextblock]->Fill32(0xFFFFFFFF,0,0,256,5); // block marker
		scrollheight[nextblock] = 0;
		
		Pentagram::Font *redfont, *yellowfont;
		
		redfont = FontManager::get_instance()->getGameFont(6, true);
		yellowfont = FontManager::get_instance()->getGameFont(8, true);
		
		bool done = false;
		bool firstline = true;
		while (!text.empty() && !done) {
			std::string::size_type endline = text.find('\n');
			std::string line = text.substr(0, endline);
			
			if (line.empty()) {
				text.erase(0, 1);
				continue;
			}
			
			// pout << "Rendering paragraph: " << line << std::endl;
			
			if (line[0] == '+') {
				// set title
				if (!firstline) {
					// if this isn't the first line of the block,
					// postpone setting title until next block
					done = true;
					continue;
				}
				
				std::string titletext;
				char modifier;
				
				extractLine(line, modifier, titletext);
				
				unsigned int remaining;
				nexttitle = redfont->renderText(titletext, remaining, 192, 0,
												Pentagram::Font::TEXT_CENTER);
				
				if (!title) {
					title = nexttitle;
					nexttitle = 0;
				} else {
					nexttitlesurf = nextblock;
					scrollheight[nextblock] = 160; // skip some space
				}
				
			} else {
				
				int height = 0;
				
				Pentagram::Font* font = redfont;
				Pentagram::Font::TextAlign align = Pentagram::Font::TEXT_LEFT;
				int indent = 0;
				
				while (!line.empty()) {
					std::string outline;
					char modifier;
					unsigned int remaining;
					extractLine(line, modifier, outline);
					
					// pout << "Rendering line: " << outline << std::endl;
					
					switch (modifier) {
					case '&':
						font = yellowfont;
						align = Pentagram::Font::TEXT_CENTER;
						break;
					case '}':
						font = redfont;
						align = Pentagram::Font::TEXT_CENTER;
						break;
					case '~':
						font = yellowfont;
						align = Pentagram::Font::TEXT_LEFT;
						indent = 32;
						break;
					case '@':
						// pout << "CreditsGump: done, finishing" << std::endl;
						state = CS_FINISHING;
						break;
					default:
						break;
					}

					if (!modifier && outline.empty()) {
						height += 48;
						continue;
					}

					if (outline[0] == '&') { 
						// horizontal line
						
						if (scrollheight[nextblock]+height+7 > 200) {
							done = true;
							break;
						}
						
						int linewidth = outline.size() * 8;
						if (linewidth > 192) linewidth = 192;
						
						scroll[nextblock]->
							Fill32(0xFFD43030,128-(linewidth/2),
								   scrollheight[nextblock]+height+3,
								   linewidth,1);
						height += 7;
						continue;
					}
					
					RenderedText* rt = font->renderText(outline, remaining,
														256-indent, 0,
														align);
					int xd,yd;
					rt->getSize(xd,yd);
					
					if (scrollheight[nextblock]+height+yd > 200) {
						delete rt;
						done = true;
						break;
					}
					
					rt->draw(scroll[nextblock], indent,
							 scrollheight[nextblock]+height+
							 font->getBaseline());
					
					height += yd + rt->getVlead();
					
					delete rt;
				}
				
				if (state == CS_PLAYING)
					height += parskip;

				if (scrollheight[nextblock] + height > 200) {
					if (firstline) {
						height = 200 - scrollheight[nextblock];
						assert(height >= 0);
					} else {
						done = true;
					}
				}
				
				if (done) break; // no room
				
				scrollheight[nextblock] += height;
			}
			
			
			if (endline != std::string::npos) endline++;
			text.erase(0, endline);
			
			firstline = false;
		}
	}

	currenty++;

	if (currenty >= scrollheight[currentsurface]) {
		// next surface
		currenty -= scrollheight[currentsurface];
		scrollheight[currentsurface] = 0;
		currentsurface = (currentsurface+1)%4;

		if (nexttitle && currentsurface == nexttitlesurf) {
			delete title;
			title = nexttitle;
			nexttitle = 0;
		}
	}
}

void CreditsGump::PaintThis(RenderSurface* surf, sint32 lerp_factor, bool scaled)
{
	surf->Fill32(0xFF000000,0,0,320,200); // black background
	surf->Fill32(0xFFD43030,64,41,192,1); // line between title and scroller

	if (title)
		title->draw(surf, 64, 34);

	Texture* tex = scroll[currentsurface]->GetSurfaceAsTexture();
	int h = scrollheight[currentsurface] - currenty;
	if (h > 156) h = 156;
	if (h > 0)
		surf->Blit(tex, 0, currenty, 256, h, 32, 44);

	int y = h;
	for (int i = 1; i < 4; i++) {
		if (h == 156) break;

		int s = (currentsurface+i)%4;
		tex = scroll[s]->GetSurfaceAsTexture();
		h = scrollheight[s];
		if (h > 156-y) h = 156-y;
		if (h > 0)
			surf->Blit(tex, 0, 0, 256, h, 32, 44+y);
		y += h;
	}
}

bool CreditsGump::OnKeyDown(int key, int mod)
{
	switch(key)
	{
	case SDLK_ESCAPE:
	{
		Close();
	} break;
	default:
		break;
	}

	return true;
}
