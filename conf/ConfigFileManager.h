/*
Copyright (C) 2002-2004 The Pentagram team

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

#ifndef CONFIGFILEMANAGER_H
#define CONFIGFILEMANAGER_H

#include <string>
#include "istring.h"
#include <vector>

class INIFile;

class ConfigFileManager
{
public:
	ConfigFileManager();
	~ConfigFileManager();

	static ConfigFileManager* get_instance() { return configfilemanager; }

	//! read a config file. Multiple files may be read. Order is important.
	//! \param fname The file to read
	//! \param root The name of the root node in the file
	//! \param readonly If true, don't write to this file's tree (or the file)
	//! \return true if succesful
	bool readConfigFile(std::string fname, Pentagram::istring root,
						bool readonly=false);
	bool readConfigString(std::string config, Pentagram::istring root,
						  bool readonly=false);

	//! write all (writable) config files in the given root
	//! \param root The root to write, or empty string to write everything
	void write(Pentagram::istring root = c_empty_string);

	//! clear everything
	void clear();

	//! clear everything in a root
	void clearRoot(Pentagram::istring root);

	//! does the key exist?
	bool exists(Pentagram::istring key);

	//! get value
	bool get(Pentagram::istring key,std::string &ret);
	//! get value
	bool get(Pentagram::istring key, int &ret);
	//! get value
	bool get(Pentagram::istring, bool &ret);

	//! set value
	void set(Pentagram::istring key, std::string value);
	//! set value
	void set(Pentagram::istring key, const char* value);
	//! set value
	void set(Pentagram::istring key, int value);
	//! set value
	void set(Pentagram::istring key, bool value);

	//! remove key
	void unset(Pentagram::istring key);

	//! list all keys in a section
	//! \param section The section to return setkeys of
	//! \param longformat If true, return the full key name, instead of
	//!                   just the last part
	//! \return the keys. They have no guaranteed order.
	std::vector<Pentagram::istring> listKeys(Pentagram::istring section,
										  bool longformat = false);

	//! list all sections
	//! \param root The config root to list all sections in
	//! \param longformat If true, return the full key name (including section)
	//! \return the sections. They have no guaranteed order.
	std::vector<Pentagram::istring> listSections(Pentagram::istring root,
											  bool longformat = false);

	//! list all key-value pairs in the given section.
	//! \param section The section to list
	//! \param longformat If true, return the full key name (including section)
	//! \return the key-value pairs. They have no guaranteed order.
	std::map<Pentagram::istring,std::string> listKeyValues(
		Pentagram::istring section, bool longformat = false);

private:

	INIFile* findKeyINI(Pentagram::istring key);
	INIFile* findWriteINI(Pentagram::istring key);

	std::vector<INIFile*> inifiles;

	static ConfigFileManager* configfilemanager;
};

#endif
