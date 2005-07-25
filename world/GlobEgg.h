/*
Copyright (C) 2003-2005 The Pentagram team

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

#ifndef GLOBEGG_H
#define GLOBEGG_H

#include "Item.h"

class MapGlob;

class GlobEgg : public Item
{
	friend class ItemFactory;
public:
	GlobEgg();
	virtual ~GlobEgg();

	// p_dynamic_cast stuff
	ENABLE_RUNTIME_CLASSTYPE();

	//! The item has entered the fast area
	virtual void enterFastArea(); 

	bool loadData(IDataSource* ids, uint32 version);
protected:
	virtual void saveData(ODataSource* ods);
};


#endif
