/*
Copyright (C) 2005 The Pentagram team

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

#include "Archive.h"
#include "IDataSource.h"
#include "ArchiveFile.h"

#include "ZipFile.h"
#include "FlexFile.h"
#include "U8SaveFile.h"
#include "DirFile.h"

namespace Pentagram {

DEFINE_RUNTIME_CLASSTYPE_CODE_BASE_CLASS(Archive);

Archive::Archive()
{
	count = 0;
}

Archive::~Archive()
{
	for (unsigned int i = 0; i < sources.size(); ++i)
		delete sources[i];
	sources.clear();
}


Archive::Archive(ArchiveFile* af)
{
	count = 0;
	addSource(af);
}

Archive::Archive(IDataSource* ids)
{
	count = 0;
	addSource(ids);
}

Archive::Archive(const std::string& path)
{
	count = 0;
	addSource(path);
}

bool Archive::addSource(ArchiveFile* af)
{
	sources.push_back(af);

	uint32 indexcount = af->getIndexCount();
	if (indexcount > count) count = indexcount;

	return true;
}

bool Archive::addSource(IDataSource* ids)
{
	ArchiveFile* s = 0;

	if (FlexFile::isFlexFile(ids)) {
		s = new FlexFile(ids);
	} else if (U8SaveFile::isU8SaveFile(ids)) {
		s = new U8SaveFile(ids);
	} else if (ZipFile::isZipFile(ids)) {
		s = new ZipFile(ids);
	}

	if (!s) return false;
	if (!s->isValid()) {
		delete s;
		return false;
	}

	return addSource(s);
}

bool Archive::addSource(const std::string& path)
{
	ArchiveFile* s = new DirFile(path);
	if (!s->isValid()) {
		delete s;
		return false;
	}

	return addSource(s);
}

void Archive::cache()
{
	for (unsigned int i = 0; i < count; ++i)
		cache(i);
}

void Archive::uncache()
{
	for (unsigned int i = 0; i < count; ++i)
		uncache(i);
}

uint8* Archive::getRawObject(uint32 index, uint32* sizep)
{
	ArchiveFile* f = findArchiveFile(index);
	if (!f) return 0;

	return f->getObject(index, sizep);
}

uint32 Archive::getRawSize(uint32 index)
{
	ArchiveFile* f = findArchiveFile(index);
	if (!f) return 0;

	return f->getSize(index);
}

ArchiveFile* Archive::findArchiveFile(uint32 index)
{
	unsigned int n = sources.size();
	for (unsigned int i = 1; i <= n; ++i) {
		if (sources[n-i]->exists(index))
			return sources[n-i];
	}

	return 0;
}

}
