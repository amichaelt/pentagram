/*
Copyright (C) 2002-2003 The Pentagram team

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

#include "GUIApp.h"

//!! a lot of these includes are just for some hacks... clean up sometime
#include "Kernel.h"
#include "FileSystem.h"
#include "Configuration.h"

#include "RenderSurface.h"
#include "Texture.h"
#include "PaletteManager.h"
#include "GameData.h"
#include "World.h"

#include "U8Save.h"

#include "Gump.h"
#include "DesktopGump.h"
#include "ConsoleGump.h"
#include "GameMapGump.h"
#include "BarkGump.h"

#include "Actor.h"
#include "ActorAnimProcess.h"
#include "Font.h"
#include "FontShapeFlex.h"
#include "u8intrinsics.h"
#include "Egg.h"
#include "CurrentMap.h"
#include "UCList.h"
#include "LoopScript.h"
#include <SDL.h>

#include "DisasmProcess.h"
#include "CompileProcess.h"

#if defined(WIN32) && defined(COLOURLESS_WANTS_A_DATA_FREE_PENATGRAM)
#include <windows.h>
#include "resource.h"
#endif

using std::string;

DEFINE_RUNTIME_CLASSTYPE_CODE(GUIApp,CoreApp);

GUIApp::GUIApp(const int argc, const char * const * const argv)
	: CoreApp(argc, argv, "u8", true), ucmachine(0), screen(0),
	  palettemanager(0), gamedata(0), world(0), desktopGump(0),
	  consoleGump(0), gameMapGump(0),
	  runGraphicSysInit(false), runSDLInit(false),
	  frameSkip(false), frameLimit(true), interpolate(true),
	  animationRate(33), avatarInStasis(false), paintEditorItems(false),
	  painting(false), dragging(false), timeOffset(0)
{
	// Set the console to auto paint, till we have finished initing
	con.SetAutoPaint(conAutoPaint);

	application = this;

	pout << "Create UCMachine" << std::endl;
	ucmachine = new UCMachine(U8Intrinsics);

	postInit(argc, argv);

	GraphicSysInit();

	U8Playground();

	// Unset the console auto paint, since we have finished initing
	con.SetAutoPaint(0);

	for (int i = 0; i < NUM_MOUSEBUTTONS+1; ++i) {
		mouseDownGump[i] = 0;
		lastMouseDown[i] = 0;
		mouseState[i] = MBS_HANDLED;
	}
}

GUIApp::~GUIApp()
{
	FORGET_OBJECT(ucmachine);
	FORGET_OBJECT(palettemanager);
}

void GUIApp::run()
{
	isRunning = true;

	sint32 next_ticks = SDL_GetTicks();	// Next time is right now!
	
	// Ok, the theory is that if this is set to true then we must do a repaint
	// At the moment only it's ignored
	bool repaint;

	SDL_Event event;
	while (isRunning) {
		inBetweenFrame = true;	// Will get set false if it's not an inBetweenFrame

		if (!frameLimit) {
			repaint = false;
			
			if (kernel->runProcesses(framenum)) repaint = true;
			desktopGump->Run(framenum);
			framenum++;
			inBetweenFrame = false;
			next_ticks = animationRate + SDL_GetTicks();
			lerpFactor = 256;
		}
		else 
		{
			sint32 ticks = SDL_GetTicks();
			sint32 diff = next_ticks - ticks;
			repaint = false;

			while (diff < 0) {
				next_ticks += animationRate;
				if (kernel->runProcesses(framenum)) repaint = true;
				desktopGump->Run(framenum);
				framenum++;
	
				inBetweenFrame = false;

				ticks = SDL_GetTicks();

				// If frame skipping is off, we will only recalc next
				// ticks IF the frames are taking up 'way' too much time. 
				if (!frameSkip && diff <= -animationRate*2) next_ticks = animationRate + ticks;

				diff = next_ticks - ticks;
				if (!frameSkip) break;
			}

			// Calculate the lerp_factor
			lerpFactor = ((animationRate-diff)*256)/animationRate;
			//pout << "lerpFactor: " << lerpFactor << " framenum: " << framenum << std::endl;
			if (!interpolate || lerpFactor > 256) lerpFactor = 256;
		}


		repaint = true;

		// get & handle all events in queue
		while (isRunning && SDL_PollEvent(&event)) {
			handleEvent(event);
		}
		handleDelayedEvents();

		// Paint Screen
		paint();
	}
}

void GUIApp::U8Playground()
{
	inBetweenFrame = 0;
	lerpFactor = 256;

	// Load palette
	pout << "Load Palette" << std::endl;
	palettemanager = new PaletteManager(screen);
	IDataSource *pf = filesystem->ReadFile("@u8/static/u8pal.pal");
	if (!pf) {
		perr << "Unable to load static/u8pal.pal. Exiting" << std::endl;
		std::exit(-1);
	}
	pf->seek(4); // seek past header
	palettemanager->load(PaletteManager::Pal_Game, *pf, U8XFormFuncs);

	pout << "Load GameData" << std::endl;
	gamedata = new GameData();
	gamedata->loadU8Data();

	// Initialize world
	pout << "Initialize World" << std::endl;
	world = new World();

	IDataSource *saveds = filesystem->ReadFile("@u8/savegame/u8save.000");
	U8Save *u8save = new U8Save(saveds);

	IDataSource *nfd = u8save->get_datasource("NONFIXED.DAT");
	if (!nfd) {
		perr << "Unable to load savegame/u8save.000/NONFIXED.DAT. Exiting" << std::endl;
		std::exit(-1);
	}
	world->initMaps();
	world->loadNonFixed(nfd);
	delete nfd;
	IDataSource *icd = u8save->get_datasource("ITEMCACH.DAT");
	if (!icd) {
		perr << "Unable to load savegame/u8save.000/ITEMCACH.DAT. Exiting" << std::endl;
		std::exit(-1);
	}
	IDataSource *npcd = u8save->get_datasource("NPCDATA.DAT");
	if (!npcd) {
		perr << "Unable to load savegame/u8save.000/NPCDATA.DAT. Exiting" << std::endl;
		std::exit(-1);
	}

	world->loadItemCachNPCData(icd, npcd);
	delete icd;
	delete npcd;
	delete u8save;
	delete saveds;

	Actor* av = world->getNPC(1);
//	av->teleport(40, 16240, 15240, 64); // central Tenebrae
//	av->teleport(3, 11391, 1727, 64); // docks, near gate
//	av->teleport(39, 16240, 15240, 64); // West Tenebrae
//	av->teleport(41, 12000, 15000, 64); // East Tenebrae
//	av->teleport(8, 14462, 15178, 48); // before entrance to Mythran's house

	if (av)
		world->switchMap(av->getMapNum());
	else
		world->switchMap(3);

	// Create GameMapGump
	Rect dims;
	screen->GetSurfaceDims(dims);
	
	pout << "Create GameMapGump" << std::endl;
	gameMapGump = new GameMapGump(0, 0, dims.w, dims.h);
	gameMapGump->InitGump();
	desktopGump->AddChild(gameMapGump);

	pout << "Create Camera" << std::endl;
	CameraProcess::SetCameraProcess(new CameraProcess(1)); // Follow Avatar

	pout << "Paint Inital display" << std::endl;
	consoleGump->HideConsole();
	paint();
}

// conAutoPaint hackery
void GUIApp::conAutoPaint(void)
{
	GUIApp *app = GUIApp::get_instance();
	if (app && !app->isPainting()) app->paint();
}

// Paint the screen
void GUIApp::paint()
{
	if(!runGraphicSysInit) // need to worry if the graphics system has been started. Need nicer way.
		return;

	painting = true;

	// Begin painting
	screen->BeginPainting();

	// We need to get the dims
	Rect dims;
	screen->GetSurfaceDims(dims);

	long before_gumps = SDL_GetTicks();
	desktopGump->Paint(screen, lerpFactor);
	long after_gumps = SDL_GetTicks();

	static long prev = 0;
	long now = SDL_GetTicks();
	long diff = now - prev;
	prev = now;

	char buf[256];
	snprintf(buf, 255, "Rendering time %li ms %li FPS - Paint Gumps %li ms - t %02d:%02d gh %i ", diff, 1000/diff, after_gumps-before_gumps, I_getTimeInMinutes(0,0), I_getTimeInSeconds(0,0)%60, I_getTimeInGameHours(0,0));
	screen->PrintTextFixed(con.GetConFont(), buf, 8, dims.h-16);

	// End painting
	screen->EndPainting();

	painting = false;
}

void GUIApp::GraphicSysInit()
{
	// if we've already done this...
	if(runGraphicSysInit) return;
	//else...

	// Set Screen Resolution
	pout << "Set Video Mode" << std::endl;

	std::string fullscreen;
	config->value("config/video/fullscreen", fullscreen, "no");
	if (fullscreen!="yes") fullscreen="no";
	int width = 640, height = 480;
	screen = RenderSurface::SetVideoMode(width, height, 32, fullscreen=="yes", false);

	if (!screen)
	{
		perr << "Unable to set video mode. Exiting" << std::endl;
		std::exit(-1);
	}

	pout << "Create Desktop" << std::endl;
	desktopGump = new DesktopGump(0,0, width, height);
	desktopGump->InitGump();
	desktopGump->MakeFocus();

	pout << "Create Graphics Console" << std::endl;
	consoleGump = new ConsoleGump(0, 0, width, height);
	consoleGump->InitGump();
	desktopGump->AddChild(consoleGump);

	LoadConsoleFont();

	// Create desktop
	Rect dims;
	screen->GetSurfaceDims(dims);

	runGraphicSysInit=true;

	// Do initial console paint
	paint();
}

void GUIApp::LoadConsoleFont()
{
#if defined(WIN32) && defined(COLOURLESS_WANTS_A_DATA_FREE_PENATGRAM)
	HRSRC res = FindResource(NULL,  MAKEINTRESOURCE(IDR_FIXED_FONT_TGA), RT_RCDATA);
	if (res) filesystem->MountFileInMemory("@data/fixedfont.tga", static_cast<uint8*>(LockResource(LoadResource(NULL, res))), SizeofResource(NULL, res));

	res = FindResource(NULL, MAKEINTRESOURCE(IDR_FIXED_FONT_CFG), RT_RCDATA);
	if (res) filesystem->MountFileInMemory("@data/fixedfont.cfg", static_cast<uint8*>(LockResource(LoadResource(NULL, res))), SizeofResource(NULL, res));
#endif

	std::string data;
	std::string confontfile;
	std::string confontcfg("@data/fixedfont.cfg");

	pout << "Searching for alternate console font... ";
	config->value("config/general/console-font", data, "");
	if (data != "")
	{
		confontcfg = data;
		pout << "Found." << std::endl;
	}
	else
		pout << "Not Found." << std::endl;

	// try to load the file
	pout << "Loading console font config: " << confontcfg << "... ";
	Configuration *fontconfig = new Configuration();
	if(fontconfig->readConfigFile(confontcfg, "font"))
		pout << "Ok" << std::endl;
	else
		pout << "Failed" << std::endl;

	// search for the path to the font...
	fontconfig->value("font/path", confontfile, "");
	if(confontfile=="")
	{
		pout << "Error: Console font path not found! Unable to continue. Exiting." << std::endl;
		std::exit(-1);
	}

	// clean up
	delete fontconfig;

	// Load confont
	pout << "Loading Confont: " << confontfile << std::endl;
	IDataSource *cf = filesystem->ReadFile(confontfile.c_str());
	Texture *confont;
	if (cf) confont = Texture::Create(*cf, confontfile.c_str());
	else confont = 0;
	if (!confont)
	{
		perr << "Unable to load " << confontfile << ". Exiting" << std::endl;
		std::exit(-1);
	}
	delete cf;

	con.SetConFont(confont);
}

//
// Hacks are us!
//

class AvatarMoverProcess : public Process
{
	int dx, dy, dz, dir;
public:
	static  AvatarMoverProcess	*amp[6];
	static	bool				clipping;
	static	bool				quarter;
	static	bool				hitting;

	AvatarMoverProcess(int x, int y, int z, int _dir) : Process(1), dx(x), dy(y), dz(z), dir(_dir)
	{
		if (amp[dir]) amp[dir]->terminate();
		amp[dir] = this;
	}

	virtual bool run(const uint32 framenum)
	{
		if (GUIApp::get_instance()->isAvatarInStasis()) 
		{
			terminate();
			return false;
		}
		Actor* avatar = World::get_instance()->getNPC(1);
		sint32 x,y,z;
		avatar->getLocation(x,y,z);
		sint32 ixd,iyd,izd;
		avatar->getFootpad(ixd, iyd, izd);
		ixd *= 32; iyd *= 32; izd *= 8; //!! constants

		CurrentMap* cm = World::get_instance()->getCurrentMap();

		for (int j = 0; j < 3; j++)
		{
			sint32 dx = this->dx;
			sint32 dy = this->dy;
			sint32 dz = this->dz;

			if (j == 1) dx = 0;
			else if (j == 2) dy = 0;

			if (quarter) 
			{
				dx /= 4;
				dy /= 4;
				dz /= 4;
			}

			sint32 start[3] = { x, y, z };
			uint16 skip = 0;

			//pout << "Avatar at (" << start[0] << ", " << start[1] << ", " << start[2] << ")" << std::endl;
			//pout << "Avatar to (" << start[0]+dx << ", " << start[1]+dy << ", " << start[2]+dz << ")" << std::endl;

			if (hitting) for (;;)
			{
				sint32 end[3] = { x+dx, y+dy, z+dz };
				sint32 dims[3] = { ixd, iyd, izd };
				uint16 hit = cm->sweepTest(start,end,dims,1,false,skip);

				if (!hit) break;

				//pout << "Hit item " << hit << " at (" << end[0] << ", " << end[1] << ", " << end[2] << ")" << std::endl;
				start[0] = end[0];
				start[1] = end[1];
				start[2] = end[2];
				skip = hit;

				Item *item = World::get_instance()->getItem(hit);
				item->callUsecodeEvent_gotHit(1,0);
			}

			while (dx || dy || dz) {

				if (!clipping || cm->isValidPosition(x+dx,y+dy,z+dz,ixd,iyd,izd,1,0,0))
				{
					if (clipping && !dz)
					{
						if (cm->isValidPosition(x+dx,y+dy,z-8,ixd,iyd,izd,1,0,0) &&
								!cm->isValidPosition(x,y,z-8,ixd,iyd,izd,1,0,0))
							avatar->move(x+dx,y+dy,z-8);
						else if (cm->isValidPosition(x+dx,y+dy,z-16,ixd,iyd,izd,1,0,0) &&
								!cm->isValidPosition(x,y,z-16,ixd,iyd,izd,1,0,0))
							avatar->move(x+dx,y+dy,z-16);
						else if (cm->isValidPosition(x+dx,y+dy,z-24,ixd,iyd,izd,1,0,0) &&
								!cm->isValidPosition(x,y,z-24,ixd,iyd,izd,1,0,0))
							avatar->move(x+dx,y+dy,z-24);
						else if (cm->isValidPosition(x+dx,y+dy,z-32,ixd,iyd,izd,1,0,0) &&
								!cm->isValidPosition(x,y,z-32,ixd,iyd,izd,1,0,0))
							avatar->move(x+dx,y+dy,z-32);
						else
							avatar->move(x+dx,y+dy,z+dz);
					}
					else
						avatar->move(x+dx,y+dy,z+dz);
					
					return true;
				}
				else if (cm->isValidPosition(x+dx,y+dy,z+dz+8,ixd,iyd,izd,1,0,0))
				{
					avatar->move(x+dx,y+dy,z+dz+8);
					return true;
				}
				dx/=2;
				dy/=2;
				dz/=2;
			}
		}
		return true;
	}

	virtual void terminate()
	{
		Process::terminate();
		amp[dir] = 0;
	}
};

AvatarMoverProcess	*AvatarMoverProcess::amp[6] = { 0, 0, 0, 0, 0, 0 };
bool AvatarMoverProcess::clipping = false;
bool AvatarMoverProcess::quarter = false;
bool AvatarMoverProcess::hitting = false;

void GUIApp::handleEvent(const SDL_Event& event)
{
	uint32 now = SDL_GetTicks();

	switch (event.type) {
	case SDL_QUIT:
	{
		isRunning = false;
	}
	break;

	case SDL_ACTIVEEVENT:
	{
		// pause when lost focus?
	}
	break;


	// most of these events will probably be passed to a gump manager,
	// since almost all (all?) user input will be handled by a gump

	case SDL_MOUSEBUTTONDOWN:
	{
		int button = event.button.button;
		int mx = event.button.x;
		int my = event.button.y;
		assert(button >= 0 && button <= NUM_MOUSEBUTTONS);

		Gump *mousedowngump = desktopGump->OnMouseDown(button, mx, my);
		if (mousedowngump)
			mouseDownGump[button] = mousedowngump->getObjId();
		else
			mouseDownGump[button] = 0;

		mouseDownX[button] = mx;
		mouseDownY[button] = my;
		mouseState[button] |= MBS_DOWN;
		mouseState[button] &= ~MBS_HANDLED;

		if (now - lastMouseDown[button] < 200) { //!! constant
			Gump* gump = getGump(mouseDownGump[button]);
			if (gump)
				gump->OnMouseDouble(button, mx, my);
			mouseState[button] |= MBS_HANDLED;
		}
		lastMouseDown[button] = now;
	}
	break;

	case SDL_MOUSEBUTTONUP:
	{
		int button = event.button.button;
		int mx = event.button.x;
		int my = event.button.y;
		assert(button >= 0 && button <= NUM_MOUSEBUTTONS);

		mouseState[button] &= ~MBS_DOWN;

		if (button == BUTTON_LEFT && dragging) {
			// stop dragging

			// for a Gump: notify parent
			Gump *gump = getGump(dragging_objid);
			if (gump) {
				Gump *parent = gump->GetParent();
				assert(parent); // can't drag root gump
				parent->StopDraggingChild(gump);
			}

			// for an item: notify GameMapGump:
			//!! TODO
				
			dragging = false;
			break;
		}

		Gump* gump = getGump(mouseDownGump[button]);
		if (gump)
			gump->OnMouseUp(button, mx, my);
	}
	break;

	case SDL_MOUSEMOTION:
	{
		int mx = event.button.x;
		int my = event.button.y;
		if (!dragging) {
			if (mouseState[BUTTON_LEFT] & MBS_DOWN) {
				int startx = mouseDownX[BUTTON_LEFT];
				int starty = mouseDownY[BUTTON_LEFT];
				if (abs(startx - mx) > 2 ||
					abs(starty - my) > 2)
				{
					dragging_objid = desktopGump->TraceObjID(startx, starty);
					perr << "Dragging object " << dragging_objid << std::endl;

					//!! check if Object is draggable
					//!! (also need to check if item is in range)
					dragging = true;

					//!! need to notify mouseDownGump that the last
					//!! mousedown event was used for dragging

					//!! need to pause the kernel

					// for a Gump, notify the Gump's parent that we started
					// dragging:
					Gump *gump = getGump(dragging_objid);
					if (gump) {
						Gump *parent = gump->GetParent();
						assert(parent); // can't drag root gump
						int px = startx, py = starty;
						parent->ScreenSpaceToGump(px, py);
						parent->StartDraggingChild(gump, px, py);
					}

					// for an Item, notify the GameMapGump that we started
					// dragging
					Item *item= World::get_instance()->getItem(dragging_objid);
					if (item) {
						// TODO
					}

					mouseState[BUTTON_LEFT] |= MBS_HANDLED;
				}
			}
		}

		if (dragging) {
			// for a gump, notify Gump's parent that it was dragged
			Gump* gump = getGump(dragging_objid);
			if (gump) {
				Gump *parent = gump->GetParent();
				assert(parent); // can't drag root gump
				int px = mx, py = my;
				parent->ScreenSpaceToGump(px, py);
				parent->DraggingChild(gump, px, py);
			}

			// for an item, notify GameMapGump:
			Item *item= World::get_instance()->getItem(dragging_objid);
			if (item) {
				// TODO
			}
		}
	}
	break;

	case SDL_KEYDOWN:
	{
		switch (event.key.keysym.sym) {
			case SDLK_LSHIFT: 
			case SDLK_RSHIFT: {
				AvatarMoverProcess::quarter = true;
				pout << "AvatarMoverProcess::quarter = " << AvatarMoverProcess::quarter << std::endl; 
			} break;
			case SDLK_c: {
				AvatarMoverProcess::clipping = !AvatarMoverProcess::clipping;
				pout << "AvatarMoverProcess::clipping = " << AvatarMoverProcess::clipping << std::endl; 
			} break;
			case SDLK_h: {
				AvatarMoverProcess::hitting = !AvatarMoverProcess::hitting;
				pout << "AvatarMoverProcess::hitting = " << AvatarMoverProcess::hitting << std::endl; 
			} break;
			case SDLK_UP: {
				if (!avatarInStasis) { 
					Process *p = new AvatarMoverProcess(-64,-64,0,0);
					Kernel::get_instance()->addProcess(p);
				} else { 
					pout << "Can't: avatarInStasis" << std::endl; 
				}
			} break;
			case SDLK_DOWN: {
				if (!avatarInStasis) { 
					Process *p = new AvatarMoverProcess(+64,+64,0,1);
					Kernel::get_instance()->addProcess(p);
				} else { 
					pout << "Can't: avatarInStasis" << std::endl; 
				}
			} break;
			case SDLK_LEFT: {
				if (!avatarInStasis) { 
					Process *p = new AvatarMoverProcess(-64,+64,0,2);
					Kernel::get_instance()->addProcess(p);
				} else { 
					pout << "Can't: avatarInStasis" << std::endl; 
				}
			} break;
			case SDLK_RIGHT: {
				if (!avatarInStasis) { 
					Process *p = new AvatarMoverProcess(+64,-64,0,3);
					Kernel::get_instance()->addProcess(p);
				} else { 
					pout << "Can't: avatarInStasis" << std::endl; 
				}
			} break;
			case SDLK_a: {
				if (!avatarInStasis) { 
					Process *p = new AvatarMoverProcess(0,0,8,4);
					Kernel::get_instance()->addProcess(p);
				} else { 
					pout << "Can't: avatarInStasis" << std::endl; 
				}
			} break;
			case SDLK_l: {
				pout << "Flushing fast area" << std::endl; 
				gameMapGump->FlushFastArea();
			} break;
			case SDLK_z: {
				if (!avatarInStasis) { 
					Process *p = new AvatarMoverProcess(0,0,-8,5);
					Kernel::get_instance()->addProcess(p);
				} else { 
					pout << "Can't: avatarInStasis" << std::endl; 
				}
			} break;

			default:
				break;
		}
	}
	break;

	case SDL_KEYUP:
	{
		switch (event.key.keysym.sym) {
		case SDLK_LSHIFT: 
		case SDLK_RSHIFT: {
			AvatarMoverProcess::quarter = false;
			pout << "AvatarMoverProcess::quarter = " << AvatarMoverProcess::quarter << std::endl; 
		} break;
		case SDLK_UP: {
			if (AvatarMoverProcess::amp[0]) AvatarMoverProcess::amp[0]->terminate();
		} break;
		case SDLK_DOWN: {
			if (AvatarMoverProcess::amp[1]) AvatarMoverProcess::amp[1]->terminate();
		} break;
		case SDLK_LEFT: {
			if (AvatarMoverProcess::amp[2]) AvatarMoverProcess::amp[2]->terminate();
		} break;
		case SDLK_RIGHT: {
			if (AvatarMoverProcess::amp[3]) AvatarMoverProcess::amp[3]->terminate();
		} break;
		case SDLK_a: {
			if (AvatarMoverProcess::amp[4]) AvatarMoverProcess::amp[4]->terminate();
		} break;
		case SDLK_z: {
			if (AvatarMoverProcess::amp[5]) AvatarMoverProcess::amp[5]->terminate();
		} break;

		case SDLK_BACKQUOTE: {
			consoleGump->ToggleConsole();
			break;
		}
		case SDLK_LEFTBRACKET: gameMapGump->IncSortOrder(-1); break;
		case SDLK_RIGHTBRACKET: gameMapGump->IncSortOrder(+1); break;
		case SDLK_ESCAPE: case SDLK_q: isRunning = false; break;
		case SDLK_PAGEUP: {
			if (!consoleGump->ConsoleIsVisible()) con.ScrollConsole(-3);
			break;
		}
		case SDLK_PAGEDOWN: {
			if (!consoleGump->ConsoleIsVisible()) con.ScrollConsole(3); 
			break;
		}
		case SDLK_s: { // toggle avatarInStasis

			avatarInStasis = !avatarInStasis;
			pout << "avatarInStasis = " << avatarInStasis << std::endl; 
		} break;
		case SDLK_t: { // engine stats
			Kernel::get_instance()->kernelStats();
			UCMachine::get_instance()->usecodeStats();
			World::get_instance()->worldStats();
		} break;
		case SDLK_e: { // editor objects toggle
			paintEditorItems = !paintEditorItems;
			pout << "paintEditorItems = " << paintEditorItems << std::endl;
		} break;
		case SDLK_f: { // trigger 'first' egg
			if (avatarInStasis) {
				pout << "Can't: avatarInStasis" << std::endl;
				break;
			}

			CurrentMap* currentmap = World::get_instance()->getCurrentMap();
			UCList uclist(2);
			// (shape == 73 && quality == 36)
			//const uint8 script[] = "@%\x49\x00=*%\x24\x00=&$";
			LOOPSCRIPT(script, LS_AND(LS_SHAPE_EQUAL1(73), LS_Q_EQUAL(36)));
			currentmap->areaSearch(&uclist, script, sizeof(script),
								   0, 256, false, 16188, 7500);
			if (uclist.getSize() < 1) {
				perr << "Unable to find FIRST egg!" << std::endl;
				break;
			}
			uint16 objid = uclist.getuint16(0);

			Egg* egg = p_dynamic_cast<Egg*>(
				Kernel::get_instance()->getObject(objid));
			sint32 ix, iy, iz;
			egg->getLocation(ix,iy,iz);
			// Center on egg
			CameraProcess::SetCameraProcess(new CameraProcess(ix,iy,iz));
			egg->hatch();
		} break;
		case SDLK_g: { // trigger 'execution' egg
			if (avatarInStasis) {
				pout << "Can't: avatarInStasis" << std::endl;
				break;
			}

			CurrentMap* currentmap = World::get_instance()->getCurrentMap();
			UCList uclist(2);
			// (shape == 73 && quality == 4)
			//const uint8* script = "@%\x49\x00=*%\x04\x00=&$";
			LOOPSCRIPT(script, LS_AND(LS_SHAPE_EQUAL1(73), LS_Q_EQUAL(4)));
			currentmap->areaSearch(&uclist, script, sizeof(script),
								   0, 256, false, 11732, 5844);
			if (uclist.getSize() < 1) {
				perr << "Unable to find EXCUTION egg!" << std::endl;
				break;
			}
			uint16 objid = uclist.getuint16(0);

			Egg* egg = p_dynamic_cast<Egg*>(
				Kernel::get_instance()->getObject(objid));
			Actor* avatar = World::get_instance()->getNPC(1);
			sint32 x,y,z;
			egg->getLocation(x,y,z);
			avatar->move(x,y,z);
			egg->hatch();
		} break;
		default: break;
		}
	}
	break;

	// any more useful events?

	default:
		break;
	}
}

void GUIApp::handleDelayedEvents()
{
	uint32 now = SDL_GetTicks();
	for (int button = 0; button <= NUM_MOUSEBUTTONS; ++button) {
		if (!(mouseState[button] & (MBS_HANDLED | MBS_DOWN)) &&
			now - lastMouseDown[button] > 200) // !constant
		{
			Gump* gump = getGump(mouseDownGump[button]);
			if (gump)
				gump->OnMouseClick(button, mouseDownX[button],
								   mouseDownY[button]);

			mouseDownGump[button] = 0;
			mouseState[button] &= ~MBS_HANDLED;
		}
	}

}

Gump* GUIApp::getGump(uint16 gumpid)
{
	return p_dynamic_cast<Gump*>(Kernel::get_instance()->getObject(gumpid));
}

uint32 GUIApp::I_getCurrentTimerTick(const uint8* /*args*/,
										unsigned int /*argsize*/)
{
	// number of ticks of a 60Hz timer, with the default animrate of 30Hz
	return get_instance()->getFrameNum()*2;
}

uint32 GUIApp::I_setAvatarInStasis(const uint8* args, unsigned int /*argsize*/)
{
	ARG_SINT16(statis);
	get_instance()->setAvatarInStasis(statis!=0);
	return 0;
}

uint32 GUIApp::I_getAvatarInStasis(const uint8* /*args*/, unsigned int /*argsize*/)
{
	if (get_instance()->avatarInStasis)
		return 1;
	else
		return 0;
}

uint32 GUIApp::I_getTimeInGameHours(const uint8* /*args*/,
										unsigned int /*argsize*/)
{
	// 1 game hour per every 27000 frames
	return (get_instance()->getFrameNum()+get_instance()->timeOffset)/27000;
}

uint32 GUIApp::I_getTimeInMinutes(const uint8* /*args*/,
										unsigned int /*argsize*/)
{
	// 1 minute per every 1800 frames
	return (get_instance()->getFrameNum()+get_instance()->timeOffset)/1800;
}

uint32 GUIApp::I_getTimeInSeconds(const uint8* /*args*/,
										unsigned int /*argsize*/)
{
	// 1 second per every 30 frames
	return (get_instance()->getFrameNum()+get_instance()->timeOffset)/30;
}

uint32 GUIApp::I_setTimeInGameHours(const uint8* args,
										unsigned int /*argsize*/)
{
	ARG_UINT16(newhour);

	// 1 game hour per every 27000 frames
	sint32	absolute = newhour*27000;
	get_instance()->timeOffset = absolute-get_instance()->getFrameNum();

	return 0;
}

