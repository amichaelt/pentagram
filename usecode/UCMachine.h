/*
Copyright (C) 2002,2003 The Pentagram team

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

#ifndef UCMACHINE_H
#define UCMACHINE_H

#include <map>
#include <string>

#include "UCStack.h"
#include "UCList.h"

#include "intrinsics.h"

class Process;
class UCProcess;
class ConvertUsecode;

class UCMachine
{
public:
	UCMachine(Intrinsic *iset);
	~UCMachine();

	static UCMachine* get_instance() { return ucmachine; }

	bool execProcess(UCProcess* proc);

	std::string& getString(uint16 str) { return stringHeap[str]; }
	UCList* getList(uint16 l) { return listHeap[l]; }

	void freeString(uint16 s);
	void freeStringList(uint16 l);
	void freeList(uint16 l);

	uint16 duplicateString(uint16 str);

	void usecodeStats();

	static uint32 listToPtr(uint16 l);
	static uint32 stringToPtr(uint16 s);
	static uint32 stackToPtr(uint16 pid, uint16 offset);
	static uint32 globalToPtr(uint16 offset);
	static uint32 objectToPtr(uint16 objID);

	static uint16 ptrToObject(uint32 ptr);

	bool assignPointer(uint32 ptr, const uint8* data, uint32 size);
	bool dereferencePointer(uint32 ptr, uint8* data, uint32 size);


	INTRINSIC(I_target);
	INTRINSIC(I_true);
	INTRINSIC(I_dummyProcess);
	INTRINSIC(I_getName);
	INTRINSIC(I_urandom);
	INTRINSIC(I_rndRange);
	INTRINSIC(I_numToStr);
	INTRINSIC(I_getCurrentTimerTick);

protected:
	void loadIntrinsics(Intrinsic *i);

private:

	ConvertUsecode*	convuse;
	Intrinsic* intrinsics;

	// this technically isn't a stack, but UCStack supports the access 
	// functions we need
	UCStack globals;

	std::map<uint16, UCList*> listHeap;
	std::map<uint16, std::string> stringHeap;

	uint16 assignString(const char* str);
	uint16 assignList(UCList* l);

	static UCMachine* ucmachine;
};

#endif
