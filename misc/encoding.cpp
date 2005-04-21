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

uint16 encoding[256] = {
	0x0000,0x0001,0x0002,0x0003, 0x0004,0x0005,0x0006,0x0007, //0x00
	0x0008,0x0009,0x000A,0x000B, 0x000C,0x000D,0x000E,0x000F,
	0x0010,0x0011,0x0012,0x0013, 0x0014,0x0015,0x0016,0x0017, //0x10
	0x0018,0x0019,0x001A,0x001B, 0x001C,0x001D,0x001E,0x001F,
	0x0020,0x0021,0x0022,0x0023, 0x0024,0x0025,0x0026,0x0027, //0x20
	0x0028,0x0029,0x002A,0x002B, 0x002C,0x002D,0x002E,0x002F,
	0x0030,0x0031,0x0032,0x0033, 0x0034,0x0035,0x0036,0x0037, //0x30
	0x0038,0x0039,0x003A,0x003B, 0x003C,0x003D,0x003E,0x003F,
	0x2022,0x0041,0x0042,0x0043, 0x0044,0x0045,0x0046,0x0047, //0x40
	0x0048,0x0049,0x004A,0x004B, 0x004C,0x004D,0x004E,0x004F,
	0x0050,0x0051,0x0052,0x0053, 0x0054,0x0055,0x0056,0x0057, //0x50
	0x0058,0x0059,0x005A,0x005B, 0x005C,0x005D,0x005E,0x005F,
	0x0060,0x0061,0x0062,0x0063, 0x0064,0x0065,0x0066,0x0067, //0x60
	0x0068,0x0069,0x006A,0x006B, 0x006C,0x006D,0x006E,0x006F,
	0x0070,0x0071,0x0072,0x0073, 0x0074,0x0075,0x0076,0x0077, //0x70
	0x0078,0x0079,0x007A,0x007B, 0x007C,0x007D,0x007E,0x007F,

	0x00C7,0x00FC,0x00E9,0x00E2, 0x00E4,0x00E0,0x00E5,0x00E7, //0x80
	0x00EA,0x00EB,0x00E8,0x00EF, 0x00EE,0x00EC,0x00C4,0x00C5,
	0x00C9,0x00E6,0x00C6,0x00F4, 0x00F6,0x00F2,0x00FB,0x00F9, //0x90
	0x00FF,0x00D6,0x00DC,0x00F8, 0x00A3,0x00D8,0x00D7,0x0192,
	0x00E1,0x00ED,0x00F3,0x00FA, 0x00F1,0x00D1,0x00AA,0x00BA, //0xA0
	0x00BF,0x00AE,0x00AC,0x00BD, 0x00BC,0x00A1,0x00AB,0x00BB,
	0x2591,0x2592,0x2593,0x2502, 0x2524,0x00C1,0x00C2,0x00C0, //0xB0
	0x00A9,0x2563,0x2551,0x2557, 0x255D,0x00A2,0x00A5,0x2510,
	0x2514,0x2534,0x252C,0x251C, 0x2500,0x253C,0x00E3,0x00C3, //0xC0
	0x255A,0x2554,0x2569,0x2566, 0x2560,0x2550,0x256C,0x00A4,
	0x00F0,0x00D0,0x00CA,0x00CB, 0x00C8,0x0131,0x00CD,0x00CE, //0xD0
	0x00CF,0x2518,0x250C,0x2588, 0x2584,0x00A6,0x00CC,0x2580,
	0x00D3,0x00DF,0x00D4,0x00D2, 0x00F5,0x00D5,0x00B5,0x00FE, //0xE0
	0x00DE,0x00DA,0x00DB,0x00D9, 0x00FD,0x00DD,0x00AF,0x00B4,
	0x00AD,0x00B1,0x2017,0x00BE, 0x00B6,0x00A7,0x00F7,0x00B8, //0xF0
	0x00B0,0x00A8,0x00B7,0x00B9, 0x00B3,0x00B2,0x25A0,0x00A0
};


char reverse_encoding[256] = {
	0x00,0x01,0x02,0x03, 0x04,0x05,0x06,0x07,
	0x08,0x09,0x0A,0x0B, 0x0C,0x0D,0x0E,0x0F,
	0x10,0x11,0x12,0x13, 0x14,0x15,0x16,0x17,
	0x18,0x19,0x1A,0x1B, 0x1C,0x1D,0x1E,0x1F,
	0x20,0x21,0x22,0x23, 0x24,0x25,0x26,0x27,
	0x28,0x29,0x2A,0x2B, 0x2C,0x2D,0x2E,0x2F,
	0x30,0x31,0x32,0x33, 0x34,0x35,0x36,0x37,
	0x38,0x39,0x3A,0x3B, 0x3C,0x3D,0x3E,0x3F,
	0x40,0x41,0x42,0x43, 0x44,0x45,0x46,0x47,
	0x48,0x49,0x4A,0x4B, 0x4C,0x4D,0x4E,0x4F,
	0x50,0x51,0x52,0x53, 0x54,0x55,0x56,0x57,
	0x58,0x59,0x5A,0x5B, 0x5C,0x5D,0x5E,0x5F,
	0x60,0x61,0x62,0x63, 0x64,0x65,0x66,0x67,
	0x68,0x69,0x6A,0x6B, 0x6C,0x6D,0x6E,0x6F,
	0x70,0x71,0x72,0x73, 0x74,0x75,0x76,0x77,
	0x78,0x79,0x7A,0x7B, 0x7C,0x7D,0x7E,0x00,

	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, //0x80
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, //0x90
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0xFF,0xAD,0xBD,0x9C, 0xCF,0xBE,0xDD,0xF5, //0xA0
	0xF9,0xB8,0xA6,0xAE, 0xAA,0xF0,0xA9,0xEE,
	0xF8,0xF1,0xFD,0xFC, 0xEF,0xD6,0xF4,0xFA, //0xB0
	0xF7,0xFB,0xA7,0xAF, 0xAC,0xAB,0xF3,0xA8,
	0xB7,0xB5,0xB6,0xC7, 0x8E,0x8F,0x92,0x80, //0xC0
	0xD4,0x91,0xD2,0xD3, 0xDE,0xD6,0xD7,0xD8,
	0xD1,0xA5,0xE3,0xE0, 0xE2,0xD5,0x99,0x9E, //0xD0
	0x9D,0xEB,0xE9,0xEA, 0x9A,0xED,0xE8,0xE1,
	0x85,0xA0,0x83,0xC6, 0x84,0x86,0x91,0x87, //0xE0
	0x8A,0x82,0x88,0x89, 0x8D,0xA1,0x8C,0x8B,
	0xD0,0xA4,0x95,0xA2, 0x93,0xD4,0x94,0xF6, //0xF0
	0x9B,0x97,0xA3,0x96, 0x81,0xEC,0xD7,0x98
};
