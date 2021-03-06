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
#include "MiniMapGump.h"
#include "GUIApp.h"
#include "World.h"
#include "CurrentMap.h"
#include "Shape.h"
#include "ShapeFrame.h"
#include "MainActor.h"
#include "RenderSurface.h"
#include "ShapeInfo.h"
#include "Palette.h"
#include "getObject.h"

DEFINE_RUNTIME_CLASSTYPE_CODE(MiniMapGump,Gump);

MiniMapGump::MiniMapGump(int x, int y) : 
	Gump(x,y,MAP_NUM_CHUNKS*2+2,MAP_NUM_CHUNKS*2+2,0,
		 FLAG_DRAGGABLE,LAYER_NORMAL), minimap(), lastMapNum(0)
{
	minimap.format = TEX_FMT_NATIVE;
	minimap.width = minimap.height = MAP_NUM_CHUNKS*MINMAPGUMP_SCALE;
	minimap.buffer = texbuffer[0];

	con.AddConsoleCommand("MiniMapGump::generateWholeMap",
						  MiniMapGump::ConCmd_generateWholeMap);
}

MiniMapGump::MiniMapGump() : Gump()
{
	con.RemoveConsoleCommand(MiniMapGump::ConCmd_generateWholeMap);
}

MiniMapGump::~MiniMapGump(void)
{
	con.RemoveConsoleCommand(MiniMapGump::ConCmd_generateWholeMap);
}

void MiniMapGump::PaintThis(RenderSurface* surf, sint32 lerp_factor, bool scaled)
{
	World *world = World::get_instance();
	CurrentMap *currentmap = world->getCurrentMap();
	int mapChunkSize = currentmap->getChunkSize();

	if (currentmap->getNum() != lastMapNum) {
		std::memset(texbuffer,0,sizeof(texbuffer));
		lastMapNum = currentmap->getNum();
	}

	surf->Fill32(0xFFFFAF00,0,0,MAP_NUM_CHUNKS*2+1,1);
	surf->Fill32(0xFFFFAF00,0,1,1,MAP_NUM_CHUNKS*2+1);
	surf->Fill32(0xFFFFAF00,1,MAP_NUM_CHUNKS*2+1,MAP_NUM_CHUNKS*2+1,1);
	surf->Fill32(0xFFFFAF00,MAP_NUM_CHUNKS*2+1,1,1,MAP_NUM_CHUNKS*2+1);

	for (int y = 0; y < MAP_NUM_CHUNKS; y++) for (int x = 0; x < MAP_NUM_CHUNKS; x++)
	{
		if (currentmap->isChunkFast(x,y)) {

			for (int j = 0; j < MINMAPGUMP_SCALE; j++) for (int i = 0; i < MINMAPGUMP_SCALE; i++)
			{
				if (texbuffer[y*MINMAPGUMP_SCALE+j][x*MINMAPGUMP_SCALE+i] == 0)
					texbuffer[y*MINMAPGUMP_SCALE+j][x*MINMAPGUMP_SCALE+i] = sampleAtPoint(
						x*mapChunkSize + mapChunkSize/(MINMAPGUMP_SCALE*2) + (mapChunkSize*i)/MINMAPGUMP_SCALE, 
						y*mapChunkSize + mapChunkSize/(MINMAPGUMP_SCALE*2) + (mapChunkSize*j)/MINMAPGUMP_SCALE,
						currentmap);
			}
		}
	}

	// Center on avatar
	int sx = 0, sy = 0, ox = 0, oy = 0, lx = 0, ly = 0;

	MainActor *av = getMainActor();
	sint32 ax,ay,az;
	av->getLocation(ax,ay,az);

	ax = ax/(mapChunkSize/MINMAPGUMP_SCALE);
	ay = ay/(mapChunkSize/MINMAPGUMP_SCALE);

	sx = ax - (mapChunkSize/(4*2));
	sy = ay - (mapChunkSize/(4*2));
	ax = ax - sx;
	ay = ay - sy;

	if (sx < 0) {
		ox = -sx;
		surf->Fill32(0,1,1,ox,MAP_NUM_CHUNKS*2);
	}
	else if ((sx+MAP_NUM_CHUNKS*2) > (MAP_NUM_CHUNKS*MINMAPGUMP_SCALE)) {
		lx = (sx+MAP_NUM_CHUNKS*2) - (MAP_NUM_CHUNKS*MINMAPGUMP_SCALE);
		surf->Fill32(0,1+(MAP_NUM_CHUNKS*2)-lx,1,lx,MAP_NUM_CHUNKS*2);
	}

	if (sy < 0) {
		oy = -sy;
		surf->Fill32(0,1,1,MAP_NUM_CHUNKS*2,oy);
	}
	else if ((sy+MAP_NUM_CHUNKS*2) > (MAP_NUM_CHUNKS*MINMAPGUMP_SCALE)) {
		ly = (sy+MAP_NUM_CHUNKS*2) - (MAP_NUM_CHUNKS*MINMAPGUMP_SCALE);
		surf->Fill32(0,1,1+(MAP_NUM_CHUNKS*2)-ly,MAP_NUM_CHUNKS*2,ly);
	}

	surf->Blit(&minimap,sx+ox,sy+oy,MAP_NUM_CHUNKS*2-(ox+lx),MAP_NUM_CHUNKS*2-(oy+ly),1+ox,1+oy);

	surf->Fill32(0xFFFFFF00,1+ax-2,1+ay+0,2,1);
	surf->Fill32(0xFFFFFF00,1+ax+0,1+ay-2,1,2);
	surf->Fill32(0xFFFFFF00,1+ax+1,1+ay+0,2,1);
	surf->Fill32(0xFFFFFF00,1+ax+0,1+ay+1,1,2);
}

uint32 MiniMapGump::sampleAtPoint(int x, int y, CurrentMap *currentmap)
{
	Item *item = currentmap->traceTopItem(x,y,1<<15,-1,0,ShapeInfo::SI_ROOF|ShapeInfo::SI_OCCL|ShapeInfo::SI_LAND|ShapeInfo::SI_SEA);

	if (item) 
	{
		sint32 ix,iy,iz,idx,idy,idz;
		item->getLocation(ix,iy,iz);
		item->getFootpadWorld(idx,idy,idz);

		ix -= x;
		iy -= y;

		Shape *sh = item->getShapeObject();
		if (!sh) return 0;

		ShapeFrame *frame = sh->getFrame(item->getFrame());
		if (!frame) return 0;

		const Pentagram::Palette *pal = sh->getPalette();
		if (!pal) return 0;

		// Screenspace bounding box bottom x coord (RNB x coord)
		int sx = (ix - iy)/4;
		// Screenspace bounding box bottom extent  (RNB y coord)
		int sy = (ix + iy)/8 + idz;

		uint16 r=0, g=0, b=0, c=0;

		for (int j = 0; j < 2; j++) for (int i = 0; i < 2; i++)
		{
			if (!frame->hasPoint(i-sx,j-sy)) continue;

			uint16 r2, g2, b2;
			UNPACK_RGB8(pal->native_untransformed[frame->getPixelAtPoint(i-sx,j-sy)], r2, g2, b2);
			r += RenderSurface::Gamma22toGamma10[r2]; g += RenderSurface::Gamma22toGamma10[g2]; b += RenderSurface::Gamma22toGamma10[b2];
			c++;
		}
		if (!c) return 0;

		return PACK_RGB8(RenderSurface::Gamma10toGamma22[r/c],RenderSurface::Gamma10toGamma22[g/c],RenderSurface::Gamma10toGamma22[b/c]);
	}
	else return 0;
}

void MiniMapGump::ConCmd_toggle(const Console::ArgvType &argv)
{
	GUIApp *app = GUIApp::get_instance();
	Gump *desktop = app->getDesktopGump();
	Gump *mmg = desktop->FindGump(MiniMapGump::ClassType);

	if (!mmg) {
		mmg = new MiniMapGump(4,4);
		mmg->InitGump(0);
		mmg->setRelativePosition(TOP_LEFT, 4, 4);

	}
	else {
		mmg->Close();
	}
}

void MiniMapGump::ConCmd_generateWholeMap(const Console::ArgvType &argv)
{
	World *world = World::get_instance();
	CurrentMap *currentmap = world->getCurrentMap();
	currentmap->setWholeMapFast();
}

uint16 MiniMapGump::TraceObjId(int mx, int my)
{
	uint16 objid = Gump::TraceObjId(mx,my);

	if (!objid || objid == 65535)
		if (PointOnGump(mx,my))
			objid = getObjId();

	return objid;
}

void MiniMapGump::saveData(ODataSource* ods)
{
	Gump::saveData(ods);
}

bool MiniMapGump::loadData(IDataSource* ids, uint32 version)
{
	if (!Gump::loadData(ids, version)) return false;

	lastMapNum = 0;
	minimap.format = TEX_FMT_NATIVE;
	minimap.width = minimap.height = MAP_NUM_CHUNKS*MINMAPGUMP_SCALE;
	minimap.buffer = texbuffer[0];

	con.AddConsoleCommand("MiniMapGump::generateWholeMap",
						  MiniMapGump::ConCmd_generateWholeMap);
	return true;
}
