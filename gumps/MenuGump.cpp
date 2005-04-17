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

#include "pent_include.h"
#include "MenuGump.h"

#include "GameData.h"
#include "GumpShapeArchive.h"
#include "Shape.h"
#include "ShapeFrame.h"
#include "GUIApp.h"
#include "DesktopGump.h"
#include "ButtonWidget.h"
#include "TextWidget.h"
#include "QuitGump.h"
#include "ControlsGump.h"
#include "OptionsGump.h"
#include "PagedGump.h"
#include "Game.h"
#include "MainActor.h"
#include "World.h"
#include "Font.h"
#include "RenderedText.h"
#include "FontManager.h"
#include "SettingManager.h"

#include "IDataSource.h"
#include "ODataSource.h"

DEFINE_RUNTIME_CLASSTYPE_CODE(MenuGump,ModalGump);

MenuGump::MenuGump(bool nameEntryMode_)
	: ModalGump(0, 0, 5, 5, 0, FLAG_DONT_SAVE)
{
	nameEntryMode = nameEntryMode_;

	GUIApp * app = GUIApp::get_instance();
	app->pushMouseCursor();
	if (!nameEntryMode)
		app->setMouseCursor(GUIApp::MOUSE_HAND);
	else
		app->setMouseCursor(GUIApp::MOUSE_NONE);

	nametext = 0;
	namechanged = true;
}

MenuGump::~MenuGump()
{
	delete nametext;
}

void MenuGump::Close(bool no_del)
{
	GUIApp* guiapp = GUIApp::get_instance();
	guiapp->popMouseCursor();

	ModalGump::Close(no_del);
}

static const int gumpShape = 35;
static const int paganShape = 32;
static const int menuEntryShape = 37;

void MenuGump::InitGump()
{
	ModalGump::InitGump();

	shape = GameData::get_instance()->getGumps()->getShape(gumpShape);
	ShapeFrame* sf = shape->getFrame(0);
	assert(sf);

	dims.w = sf->width;
	dims.h = sf->height;

	Shape* logoShape;
	logoShape = GameData::get_instance()->getGumps()->getShape(paganShape);
	sf = logoShape->getFrame(0);
	assert(sf);

	Gump * logo = new Gump(42, 10, sf->width, sf->height);
	logo->SetShape(logoShape, 0);
	logo->InitGump();
	AddChild(logo);

	if (!nameEntryMode) {
		SettingManager* settingman = SettingManager::get_instance();
		bool endgame;
		settingman->get("endgame", endgame);

		int x = dims.w / 2 + 14;
		int y = 18;
		Gump * widget;
		for (int i = 0; i < 8; ++i)
		{
			if (!endgame && i == 6) break;

			FrameID frame_up(GameData::GUMPS, menuEntryShape, i * 2);
			FrameID frame_down(GameData::GUMPS, menuEntryShape, i * 2 + 1);
			frame_up = _TL_SHP_(frame_up);
			frame_down = _TL_SHP_(frame_down);
			widget = new ButtonWidget(x, y, frame_up, frame_down, true);
			widget->InitGump();
			widget->SetIndex(i + 1);
			AddChild(widget);
			y+= 14;
		}
		
		MainActor* av = World::get_instance()->getMainActor();
		std::string name;
		if (av)
			name = av->getName();

		if (!name.empty()) {
			Pentagram::Rect rect;
			widget = new TextWidget(0, 0, name, 6);
			widget->InitGump();
			widget->GetDims(rect);
			widget->Move(90 - rect.w / 2, dims.h - 40);
			AddChild(widget);
		}
	} else {
		Gump * widget;
		widget = new TextWidget(0, 0, _TL_("Give thy name:"), 6); // CONSTANT!
		widget->InitGump();
		widget->Move(dims.w / 2 + 6, 10);
		AddChild(widget);
	}
}


void MenuGump::PaintThis(RenderSurface* surf, sint32 lerp_factor)
{
	Gump::PaintThis(surf, lerp_factor);

	if (nameEntryMode) {
		if (!nametext || namechanged) {
			// CONSTANT!
			Font* font = FontManager::get_instance()->getGameFont(6);
			unsigned int remaining;
			delete nametext;
			nametext = font->renderText(name + "-", remaining, 0, 0);
			namechanged = false;
		}

		nametext->draw(surf, dims.w / 2 + 6, 30);
	}
}

bool MenuGump::OnKeyDown(int key, int mod)
{
	if (!nameEntryMode) {

		if (key == SDLK_ESCAPE) {
			MainActor* av = World::get_instance()->getMainActor();
			if (av && !(av->getActorFlags() & Actor::ACT_DEAD))
				Close(); // don't allow closing if dead/game over
		} else if (key >= SDLK_1 && key <=SDLK_9) {
			selectEntry(key - SDLK_1 + 1);
		}

	} else {

		// hack (should have a 'TextInputWidget')
		if (key == SDLK_RETURN || key == SDLK_KP_ENTER) {
			if (!name.empty()) {
				MainActor* av = World::get_instance()->getMainActor();
				av->setName(name);
				Close();
			}
		} else if (key == SDLK_BACKSPACE) {
			if (!name.empty()) {
				namechanged = true;
				name.erase(name.size()-1,1);
			}
		}

	}

	return true;
}

void MenuGump::ChildNotify(Gump *child, uint32 message)
{
	if (message == ButtonWidget::BUTTON_CLICK)
	{
		selectEntry(child->GetIndex());
	}
}

void MenuGump::selectEntry(int entry)
{
	switch (entry)
	{
	case 1: // Intro
		Game::get_instance()->playIntroMovie();
		break;
	case 2: // Read Diary
		GUIApp::get_instance()->loadGame("@save/quicksave");
		break;
	case 3: // Write Diary
		GUIApp::get_instance()->saveGame("@save/quicksave", true);
		break;
	case 4: // Options
	{
		OptionsGump * options = new OptionsGump();
		options->InitGump();
		PagedGump * gump = new PagedGump(34, -38, 3, gumpShape);
		gump->InitGump();
		gump->addPage(options);
		AddChild(gump);
		gump->setRelativePosition(CENTER);
	} break;
	case 5: // Credits
		Game::get_instance()->playCredits();
		break;
	case 6: // Quit
		QuitGump::verifyQuit();
		break;
	case 7: // Quotes
		Game::get_instance()->playQuotes();
		break;
	case 8: // End Game
		Game::get_instance()->playEndgameMovie();
		break;
	default:
		break;
	}
}

bool MenuGump::OnTextInput(int unicode)
{
	// hack (should have a 'TextInputWidget')
	if (nameEntryMode) {
		if (unicode >= 32 && unicode < 128 && name.size() < 15) {
			namechanged = true;
			name += static_cast<char>(unicode);
		}
	}

	return true;
}

//static
void MenuGump::showMenu()
{
	ModalGump* gump = new MenuGump();
	gump->InitGump();
	GUIApp::get_instance()->addGump(gump);
	gump->setRelativePosition(CENTER);
}

//static
void MenuGump::inputName()
{
	ModalGump* gump = new MenuGump(true);
	gump->InitGump();
	GUIApp::get_instance()->addGump(gump);
	gump->setRelativePosition(CENTER);
}
