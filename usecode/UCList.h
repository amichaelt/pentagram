/*
Copyright (C) 2002 The Pentagram team

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

#ifndef UCLIST_H
#define UCLIST_H

#include <vector>
#include <string>

// stringlists: elementsize = 2, each element is actually a stringref
// see for example the 0x0E opcode: there is no way to see if the
// created list is a stringlist or not
// the opcodes which do need a distinction have a operand for this.

// Question: how are unionList/substractList supposed to know what to do?
// their behaviour differs if this is a stringlist
// (stringlists not implemented yet)

// Question: does substractList remove _all_ occurences of elements or only 1?

// the exact behaviour of string push/pop/list opcodes (wrt memory allocation)
// also needs to be investigated

// Also, do lists use refcounting? (for passing lists to different processes)

class UCList
{
	std::vector<uint8> elements;
	unsigned int elementsize;
	unsigned int size;

 public:
	UCList(unsigned int elementsize_, unsigned int capacity=0) : 
		elementsize(elementsize_)
	{
		if (capacity > 0)
			elements.reserve(elementsize * capacity);
	}

	~UCList() {
		// slight problem: we don't know if we're a stringlist
		free();
	}

	const uint8* operator[](uint32 index) {
		// check that index isn't out of bounds...
		return &(elements[index * elementsize]);
	}

	void append(const uint8* e) {
		elements.reserve((size + 1) * elementsize);
		for (unsigned int i = 0; i < elementsize; i++)
			elements[size*elementsize + i] = e[i];
		size++;
	}

	void remove(const uint8* e) {
		// do we need to erase all occurences of e or just the first one?
		// (deleting all, currently)
		for (unsigned int i = 0; i < size; i++) {
			bool equal = true;
			for (unsigned int j = 0; j < elementsize && equal; j++)
				equal = (elements[i*elementsize + j] == e[j]);
			if (!equal) {
				elements.erase(elements.begin()+i*elementsize,
							   elements.begin()+(i+1)*elementsize);
				size--;
				i--; // back up a bit
			}
		}
	}

	bool inList(const uint8* e) {
		for (unsigned int i = 0; i < size; i++) {
			bool equal = true;
			for (unsigned int j = 0; j < elementsize && equal; j++)
				equal = (elements[i*elementsize + j] == e[j]);
			if (equal)
				return true;
		}
		return false;
	}

	void appendList(UCList& l) {
		// need to check if elementsizes match...
		elements.reserve(elementsize * (size + l.size));
		for (unsigned int i = 0; i < l.size; i++)
			append(l[i]);
		l.free(); // NB: if l is a stringlist, do _not_ free the strings in l,
		          // since all strings are now in this
	}
	void unionList(UCList& l) { // like append, but remove duplicates
		// need to check if elementsizes match...
		for (unsigned int i = 0; i < l.size; i++)
			if (!inList(l[i]))
				append(l[i]);
	}
	void substractList(UCList& l) {
		for (unsigned int i = 0; i < l.size; i++)
			remove(l[i]);
	}

	void free() { elements.clear(); size = 0; }
	uint32 getSize() const { return size; }

	void assign(uint32 index, const uint8* e) {
		// need to check that index isn't out-of-bounds? (or grow list?)
		for (unsigned int i = 0; i < elementsize; i++)
			elements[index*elementsize+i] = e[i];
	}

	void copyList(UCList& l) { // deep copy for list
		free();
		appendList(l);
	}

	void freeStrings();
	void copyStringList(UCList& l);
	void unionStringList(UCList& l);
	void substractStringList(UCList& l);
	bool stringInList(uint16 str);
	void assignString(uint32 index, uint16 str);
	void removeString(uint16 str);

	uint16 getStringIndex(uint32 index);
 private:
	std::string& getString(uint32 index);
};

#endif
