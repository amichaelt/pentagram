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

#ifndef UCPROCESS_H
#define UCPROCESS_H

#include <list>
#include "Process.h"
#include "UCStack.h"

class Usecode;
class IDataSource;
class ODataSource;


// probably won't inherit from Process directly in the future
class UCProcess : public Process
{
	friend class UCMachine;
	friend class Kernel;
public:
	UCProcess(Usecode* usecode=0);
    ~UCProcess();

	// p_dynamic_cast stuff
	ENABLE_RUNTIME_CLASSTYPE();

	void load(uint16 classid_, uint16 offset_, uint32 this_ptr = 0,
			  int thissize = 0, const uint8* args = 0, int argsize = 0);
	virtual bool run(const uint32 framenum);

	virtual void terminate();

	void freeOnTerminate(uint16 index, int type);

	bool loadData(IDataSource* ids);
protected:
	virtual void saveData(ODataSource* ods);

	void call(uint16 classid_, uint16 offset_);
	bool ret();

	// stack base pointer
	uint16 bp;

	Usecode* usecode;

	uint16 classid;
	uint16 ip;

	uint32 temp32;

	// data stack
	UCStack stack;

	// "Free Me" list
	std::list<std::pair<uint16, int> > freeonterminate;
};

#endif
