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

#include "pent_include.h"

#include "XMLTree.h"
#include "XMLNode.h"

#include "IDataSource.h"
#include "FileSystem.h"

XMLTree::XMLTree()
	: tree(new XMLNode("config")), root("config"), is_file(false),
	  readonly(false)
{

}

XMLTree::XMLTree(std::string fname, std::string root_)
	: tree (new XMLNode(root_)), root(root_), is_file(true),
	  readonly(false)
{
	readConfigFile(fname);
}

XMLTree::~XMLTree()
{
	delete tree;
}

void XMLTree::clear(std::string root_)
{
	delete tree;
	tree = new XMLNode(root_);
	root = root_;
	is_file = false;
	readonly = false;
}

bool XMLTree::readConfigFile(std::string fname)
{
#warning fixme: non-FileSystem file access!
#if 0
	IFileDataSource* ds = filesystem->ReadFile(fname, true); // open as text

	if (!ds)
		return false;

	std::string sbuf, line;
	while (ds->getline(line))
	{
		sbuf += line + "\n";
	}

	delete ds;

	if (!readConfigString(sbuf))
		return false;

#else
	std::ifstream f(fname.c_str());
	std::string sbuf, line;
	while (f.good()) {
		std::getline(f, line);
		sbuf += line;
	}
	if (!readConfigString(sbuf))
		return false;

#endif

	is_file = true; // readConfigString sets is_file = false
	filename = fname;
	return true;
}

bool XMLTree::readConfigString(std::string s)
{
	is_file = false;

	std::string sbuf(s);
	std::size_t nn=0;
	while(isspace(s[nn])) ++nn;

	if (s[nn] != '<') {
		PERR("Error reading config file");
		return false;
	}
	++nn;
	
	tree->xmlparse(sbuf,nn);

	return true;
}

std::string XMLTree::dump()
{
	return tree->dump();
}

void XMLTree::write()
{
	if (!is_file || readonly)
		return;
#warning fixme: non-FileSystem file access!
#if 0
	OFileDataSource ds = filesystem->WriteFile(filename);
	if (!ds) {
		PERR("Error writing config file")
		return;
	}

	ds->write(dump());
	delete ds;
#else
	std::ofstream f(filename.c_str());
	f << dump();
#endif
}

bool XMLTree::hasNode(std::string key) const
{
	const XMLNode *sub = tree->subtree(key);
	if (sub)
		return true;
	else
		return false;
}

bool XMLTree::checkRoot(std::string key) const
{
	std::string k = key.substr(0, key.find('/'));
	return (k == root);
}

void XMLTree::value(std::string key, std::string &ret,
					const char *defaultvalue) const
{
	const XMLNode *sub = tree->subtree(key);
	if (sub)
		ret = sub->value();
	else
		ret = defaultvalue;
}

void XMLTree::value(std::string key, int &ret,
					int defaultvalue) const
{
	const XMLNode *sub = tree->subtree(key);
	if (sub)
		ret = atoi(sub->value().c_str());
	else
		ret = defaultvalue;
}

void XMLTree::value(std::string key, bool &ret,
					bool defaultvalue) const
{
#warning fixme: get to_uppercase function!
#if 0
	const XMLNode *sub = tree->subtree(key);
	if (sub)
		ret = (to_uppercase(sub->value()) == "YES");
	else
		ret = defaultvalue;
#endif
}

void XMLTree::set(std::string key, std::string value)
{
	tree->xmlassign(key, value);
}

void XMLTree::set(std::string key, const char* value)
{
	tree->xmlassign(key, value);
}

void XMLTree::set(std::string key, int value)
{
	char buf[32];
	snprintf(buf, 32, "%d", value);
	set(key, value);
}

void XMLTree::set(std::string key, bool value)
{
	if (value)
		set(key, "yes");
	else
		set(key, "no");
}

std::vector<std::string> XMLTree::listKeys(std::string key, bool longformat)
{
	std::vector<std::string> keys;
	const XMLNode *sub = tree->subtree(key);
	if (sub)
		sub->listkeys(key, keys, longformat);

	return keys;
}