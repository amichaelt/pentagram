/*
Copyright (C) 2003 The Pentagram team

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

#ifndef IDMAN_H
#define IDMAN_H

class IDataSource;
class ODataSource;

#include <vector>

//
// idMan. Used to allocate and keep track of unused ids.
// Supposed to be used by Kernel and World for pID and ObjID
//
// The idMan itself uses a nifty linked list that is also an array.
// As such it has the advantages of both. Adds and removals are fast,
// As is checking to see if an ID is used.
//
// idMan works as a LILO (last in last out) for id recycling. So an id that has
// been cleared will not be used until all other currently unused ids have been
// allocated.
//

class idMan
{
	uint16 		begin;			//!< start of the available range of IDs
	uint16 		end;			//!< current end of the range
	uint16 		max_end;		//!< end of the available range
	uint16 		startcount;		//!< number of IDs to make available initially

	uint16 		usedcount;		//!< number of IDs currently in use

	std::vector<uint16> ids;	//!< the 'next' field in a list of free IDs
	uint16		first;			//!< the first ID in the free list
	uint16		last;			//!< the last ID in the last list
public:
	//! \param begin start of the range of available IDs
	//! \param max_end end of the range of available IDs
	//! \param startcount number of IDs to make available initially (0 = all)
	idMan(uint16 begin, uint16 max_end, uint16 startcount=0);
	~idMan();

	//! clear all IDs, reset size to the startcount
	void		clearAll();

	//! get a free ID
	//! \return a free ID, or 0 if none are available
	uint16		getNewID();

	//! mark a given ID as used, expanding if necessary.
	//! Note: reserveID is O(n), so don't use too often.
	//! \return false if the ID was already used or is out of range
	bool		reserveID(uint16 id);

	//! release an id
	void		clearID(uint16 id);

	//! check if an ID is in use
	bool		isIDUsed(uint16 id)
		{ return id <= end && ids[id] == 0 && id != last; }

	void save(ODataSource* ods);
	bool load(IDataSource* ids);

private:
	//! double the amount of available IDs (up to the maximum passed 
	//! to the constructor)
	void expand();
};

#endif //IDMAN_H
