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

#include "pent_include.h"

#include <SDL.h>
#include "GUIApp.h"
#include "version.h"

int main(int argc, char* argv[])
{
	GUIApp app(argc, argv);

#ifdef SAFE_CONSOLE_STREAMS
	console_streambuf<char> fb;
	ppout = new console_ostream<char>(&fb);

	console_err_streambuf<char> err_fb;
	pperr = new console_err_ostream<char>(&err_fb);

#endif

	pout << "Pentagram version " << PentagramVersion::version << std::endl;
	pout << "Built: " << PentagramVersion::buildtime << std::endl;
	pout << "Optional features: " << PentagramVersion::features << std::endl;
	pout << std::endl;

	app.startup();
	app.run();

	return 0;
}
