/*
Copyright (C) 2006 The Pentagram team

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
#include "JPFont.h"
#include "ShapeFont.h"
#include "Shape.h"
#include "ShapeFrame.h"
#include "JPRenderedText.h"
#include "encoding.h"

DEFINE_RUNTIME_CLASSTYPE_CODE(JPFont,Pentagram::Font);


JPFont::JPFont(ShapeFont* jpfont, unsigned int fontnum_)
	: fontnum(fontnum_), shapefont(jpfont)
{
	assert(shapefont->frameCount() > 256);
}


JPFont::~JPFont()
{

}

int JPFont::getWidth(int c)
{
	return shapefont->getFrame(c)->width;
}

int JPFont::getHeight()
{
	return shapefont->getHeight();
}

int JPFont::getBaseline()
{
	return shapefont->getBaseline();
}

int JPFont::getBaselineSkip()
{
	return shapefont->getBaselineSkip();
}


void JPFont::getStringSize(const std::string& text, int& width, int& height)
{
	int hlead = shapefont->getHlead();
	width = hlead;
	height = getHeight();

	for (unsigned int i = 0; i < text.size(); ++i)
	{
		if (text[i] == '\n' || text[i] == '\r') {
			// ignore
		} else {
			uint16 sjis = text[i] & 0xFF;
			if (sjis >= 0x80) {
				uint16 t = text[++i] & 0xFF;
				sjis += (t << 8);
			}
			width += getWidth(Pentagram::shiftjis_to_ultima8(sjis))-hlead;
		}
	}
}


static inline bool isSpace(char c, bool u8specials)
{
	return (c == ' ' || c == '\t' || c == '\n' || c == '\r' ||
			(u8specials && (c == '%' || c == '~' || c == '*')));
}

static inline bool isTab(char c, bool u8specials)
{
	return (c == '\t' || (u8specials && c == '%'));
}

static inline bool isBreak(char c, bool u8specials)
{
	return (c == '\n' || (u8specials && (c == '~' || c == '*')));
}


static unsigned int findWordEnd(const std::string& text, unsigned int start,
								bool u8specials)
{
	unsigned int index = start;
	while (index < text.size()) {
		if (isSpace(text[index], u8specials))
			return index; 

		index++;
	}
	return index;
}

static unsigned int passSpace(const std::string& text, unsigned int start,
							  bool u8specials)
{
	unsigned int index = start;
	while (index < text.size()) {
		if (!isSpace(text[index], u8specials))
			return index; 

		index++;
	}
	return index;
}


std::list<PositionedText> JPFont::typesetText(const std::string& text,
											  unsigned int& remaining,
											  int width, int height,
											  TextAlign align, bool u8specials,
											  int& resultwidth,
											  int& resultheight,
											  std::string::size_type cursor)
{
#if 0
	pout << "typeset (" << width << "," << height << ") : "
		 << text << std::endl;
#endif

	// be optimistic and assume everything will fit
	remaining = text.size();

	unsigned int curlinestart = 0;
	std::string curline;

	int totalwidth = 0;
	int totalheight = 0;

	std::list<PositionedText> lines;
	PositionedText line;

	unsigned int i = 0;

	bool breakhere = false;
	while (true)
	{
		if (i >= text.size() || breakhere || isBreak(text[i],u8specials))
		{
			// break here
			int stringwidth = 0, stringheight = 0;
			getStringSize(curline, stringwidth, stringheight);
			line.dims.x = 0; line.dims.y = totalheight;
			line.dims.w = stringwidth;
			line.dims.h = stringheight;
			line.text = curline;
			line.cursor = std::string::npos;
			if (cursor != std::string::npos && cursor >= curlinestart &&
				(cursor < i || (!breakhere && cursor == i)))
			{
				line.cursor = cursor - curlinestart;
				if (line.dims.w == 0) {
					stringwidth = line.dims.w = 2;
				}
			}
			lines.push_back(line);

			if (stringwidth > totalwidth) totalwidth = stringwidth;
			totalheight += getBaselineSkip();

			curline = "";

			if (i >= text.size())
				break; // done

			if (breakhere) {
				breakhere = false;
				curlinestart = i;
			} else {
				i = curlinestart = i+1; // FIXME: CR/LF?
			}

			if (height != 0 && totalheight + getHeight() > height) {
				// next line won't fit
				remaining = curlinestart;
				break;
			}

		} else {

			// see if next word still fits on the current line
			unsigned int nextword = passSpace(text, i, u8specials);

			// process spaces
			bool foundLF = false;
			std::string spaces;
			for (; i < nextword; ++i) {
				if (isBreak(text[i],u8specials)) {
					foundLF = true;
					break;
				} else if (isTab(text[i],u8specials)) {
					spaces.append("    ");
				} else if (!curline.empty()){
					spaces.append(" ");
				}
			}
			if (foundLF) continue;

			// process word
			unsigned int endofnextword = findWordEnd(text,nextword,u8specials);
			int stringwidth = 0, stringheight = 0;
			std::string newline = curline + spaces +
				text.substr(nextword,endofnextword-nextword);
			getStringSize(newline, stringwidth, stringheight);

			// if not, break line before this word
			if (width != 0 && stringwidth > width) {
				if (!curline.empty()) {
					i = nextword;
				} else {
					// word is longer than the line; have to break in mid-word
					// FIXME: broken with Shift-JIS!
					i = nextword;
					newline.clear();
					do {
						newline += text[i];
						if (text[i] & 0x80)
							newline += text[i+1];
						getStringSize(newline, stringwidth, stringheight);
						if (stringwidth <= width) {
							curline += text[i];
							if (text[i] & 0x80)
								curline += text[++i];
							i++;
						}
					} while (stringwidth <= width);
				}
				breakhere = true;
				continue;
			} else {
				// copy next word into curline
				curline = newline;
				i = endofnextword;
			}
		}
	}

	if (lines.size() == 1 && align == TEXT_LEFT) {
		// only one line, so use the actual text width
	    width = totalwidth;
	}

	if (width != 0) totalwidth = width;

	// adjust total height
	totalheight -= getBaselineSkip();
	totalheight += getHeight();

	// fixup x coordinates of lines
	std::list<PositionedText>::iterator iter;
	for (iter = lines.begin(); iter != lines.end(); ++iter) {
		switch (align) {
		case TEXT_LEFT:
			break;
		case TEXT_RIGHT:
			iter->dims.x = totalwidth - iter->dims.w;
			break;
		case TEXT_CENTER:
			iter->dims.x = (totalwidth - iter->dims.w) / 2;
			break;
		}
#if 0
		pout << iter->dims.x << "," << iter->dims.y << " "
			 << iter->dims.w << "," << iter->dims.h << ": "
			 << iter->text << std::endl;
#endif
	}

	resultwidth = totalwidth;
	resultheight = totalheight;

	return lines;
}

RenderedText* JPFont::renderText(const std::string& text,
								 unsigned int& remaining,
								 int width, int height, TextAlign align,
								 bool u8specials,
								 std::string::size_type cursor)
{
	int resultwidth, resultheight;
	std::list<PositionedText> lines = typesetText(text, remaining,
												  width, height,
												  align, false,
												  resultwidth, resultheight,
												  cursor);

	return new JPRenderedText(lines, resultwidth, resultheight,
							  shapefont->getVlead(), shapefont, fontnum);
}


