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

#ifndef ANIMACTION_H
#define ANIMACTION_H

#include <vector>
class Actor;

struct AnimFrame
{
	int frame;
	int deltaz;
	int deltadir;
	int sfx;
	uint32 flags;

	inline bool is_flipped() { return (flags & 32) != 0; }
};

struct AnimAction {
	uint32 shapenum;
	uint32 action;

	std::vector<AnimFrame> frames[8]; // 8 directions
	unsigned int size;
	int framerepeat;
	uint32 flags;

	//! return the range of the animation to play
	//! \param actor The actor to play the animation for
	//! \param dir The direction
	//! \param startframe The first frame to play
	//! \param endframe The frame after the last frame to play
	void getAnimRange(Actor* actor, int dir,
					  unsigned int& startframe, unsigned int& endframe);

	enum {
		AAF_TWOSTEP = 0x01,
		AAF_LOOPING = 0x04,
		AAF_HANGING = 0x80
	} AnimActionFlags;
};


#endif
