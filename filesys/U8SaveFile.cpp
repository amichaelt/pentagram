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

#include "pent_include.h"

#include "filesys/U8SaveFile.h"
#include "filesys/IDataSource.h"

DEFINE_RUNTIME_CLASSTYPE_CODE(U8SaveFile,NamedArchiveFile);


U8SaveFile::U8SaveFile(IDataSource* ds_)
{
	ds = ds_;
	count = 0;
	valid = isU8SaveFile(ds);

	if (valid)
		valid = readMetadata();
}

U8SaveFile::~U8SaveFile()
{
	delete ds;
}

//static
bool U8SaveFile::isU8SaveFile(IDataSource* ds)
{
	ds->seek(0);
	char buf[24];
	ds->read(buf, 23);
	buf[23] = '\0';

	return (std::strncmp(buf, "Ultima 8 SaveGame File.", 23) == 0);
}

bool U8SaveFile::readMetadata()
{
	ds->seek(0x18);
	count = ds->read2();

	offsets.resize(count);
	sizes.resize(count);

	for (unsigned int i = 0; i < count; ++i)
	{
		uint32 namelen = ds->read4();
		char* buf = new char[namelen];
		ds->read(buf, static_cast<sint32>(namelen));
		std::string filename = buf;
		indices[filename] = i;
		storeIndexedName(filename);
		delete[] buf;
		sizes[i] = ds->read4();
		offsets[i] = ds->getPos();
		ds->skip(sizes[i]); // skip data
	}

	return true;
}

bool U8SaveFile::findIndex(const std::string& name, uint32& index)
{
	std::map<std::string, uint32>::iterator iter;
	iter = indices.find(name);
	if (iter == indices.end()) return false;
	index = iter->second;
	return true;
}

bool U8SaveFile::exists(const std::string& name)
{
	uint32 index;
	return findIndex(name, index);
}

uint8* U8SaveFile::getObject(const std::string& name, uint32* sizep)
{
	uint32 index;
	if (!findIndex(name, index)) return 0;

	uint32 size = sizes[index];
	if (size == 0) return 0;

	uint8 *object = new uint8[size];
	uint32 offset = offsets[index];

	ds->seek(offset);
	ds->read(object, size);

	if (sizep) *sizep = size;

	return object;
}


uint32 U8SaveFile::getSize(const std::string& name)
{
	uint32 index;
	if (!findIndex(name, index)) return 0;

	return sizes[index];
}
