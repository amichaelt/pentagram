/*
 *  Copyright (C) 2002 The Pentagram Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef CONVERTUSECODECRUSADER_H
#define CONVERTUSECODECRUSADER_H

#include "Convert.h"

/* Needs to be split into CrusaderRemorse and CrusaderRegret classes */

class ConvertUsecodeCrusader : public ConvertUsecode
{
	public:
		const char* const *intrinsics()  { return _intrinsics;  };
		const char* const *event_names() { return _event_names; };
		void readheader(IFileDataSource *ucfile, UsecodeHeader &uch, uint32 &curOffset);
		void readevents(IFileDataSource *ucfile, const UsecodeHeader &uch)
		{ 
			int num_crusader_routines = uch.offset / 6;
			for (int i=0; i < num_crusader_routines; i++) {
				uint32 size = read2(ucfile);
				uint32 offset = read4(ucfile);
				EventMap[offset] = i;
				#ifdef DISASM_DEBUG
				pout << "Crusader Routine: " << i << ": " << std::hex << std::setw(4) << offset << std::dec << " size " << size << endl;
				#endif
			}
		};
		
	private:
		static const char* const _intrinsics[];
		static const char* const _event_names[];
};

const char* const ConvertUsecodeCrusader::_intrinsics[] = {
	// 0000
	"Intrinsic0000()",
	"Intrinsic0001()",
	"Intrinsic0002()",
	"Intrinsic0003()",
	"Intrinsic0004()",
	"Intrinsic0005()",
	"Intrinsic0006()",
	"Intrinsic0007()",
	"Intrinsic0008()",
	"Intrinsic0009()",
	"Intrinsic000A()",
	"Intrinsic000B()",
	"Intrinsic000C()",
	"Intrinsic000D()",
	"Intrinsic000E()",
	"Intrinsic000F()",
	// 0010
	"Intrinsic0010()",
	"Intrinsic0011()",
	"Intrinsic0012()",
	"Intrinsic0013()",
	"Intrinsic0014()",
	"Intrinsic0015()",
	"Intrinsic0016()",
	"Intrinsic0017()",
	"Intrinsic0018()",
	"Intrinsic0019()",
	"Intrinsic001A()",
	"Intrinsic001B()",
	"Intrinsic001C()",
	"Intrinsic001D()",
	"Intrinsic001E()",
	"Intrinsic001F()",
	// 0020
	"Intrinsic0020()",
	"Intrinsic0021()",
	"Intrinsic0022()",
	"Intrinsic0023()",
	"Intrinsic0024()",
	"Intrinsic0025()",
	"Intrinsic0026()",
	"Intrinsic0027()",
	"Intrinsic0028()",
	"Intrinsic0029()",
	"Intrinsic002A()",
	"Intrinsic002B()",
	"Intrinsic002C()",
	"Intrinsic002D()",
	"Intrinsic002E()",
	"Intrinsic002F()",
	// 0030
	"Intrinsic0030()",
	"Intrinsic0031()",
	"Intrinsic0032()",
	"Intrinsic0033()",
	"Intrinsic0034()",
	"Intrinsic0035()",
	"Intrinsic0036()",
	"Intrinsic0037()",
	"Intrinsic0038()",
	"Intrinsic0039()",
	"Intrinsic003A()",
	"Intrinsic003B()",
	"Intrinsic003C()",
	"Intrinsic003D()",
	"Intrinsic003E()",
	"Intrinsic003F()",
	// 0040
	"Intrinsic0040()",
	"Intrinsic0041()",
	"Intrinsic0042()",
	"Intrinsic0043()",
	"Intrinsic0044()",
	"Intrinsic0045()",
	"Intrinsic0046()",
	"Intrinsic0047()",
	"Intrinsic0048()",
	"Intrinsic0049()",
	"Intrinsic004A()",
	"Intrinsic004B()",
	"Intrinsic004C()",
	"Intrinsic004D()",
	"Intrinsic004E()",
	"Intrinsic004F()",
	// 0050
	"Intrinsic0050()",
	"Intrinsic0051()",
	"Intrinsic0052()",
	"Intrinsic0053()",
	"Intrinsic0054()",
	"Intrinsic0055()",
	"Intrinsic0056()",
	"Intrinsic0057()",
	"Intrinsic0058()",
	"Intrinsic0059()",
	"Intrinsic005A()",
	"Intrinsic005B()",
	"Intrinsic005C()",
	"Intrinsic005D()",
	"Intrinsic005E()",
	"Intrinsic005F()",
	// 0060
	"Intrinsic0060()",
	"Intrinsic0061()",
	"Intrinsic0062()",
	"Intrinsic0063()",
	"Intrinsic0064()",
	"Intrinsic0065()",
	"Intrinsic0066()",
	"Intrinsic0067()",
	"Intrinsic0068()",
	"Intrinsic0069()",
	"Intrinsic006A()",
	"Intrinsic006B()",
	"Intrinsic006C()",
	"Intrinsic006D()",
	"Intrinsic006E()",
	"Intrinsic006F()",
	// 0070
	"Intrinsic0070()",
	"Intrinsic0071()",
	"Intrinsic0072()",
	"Intrinsic0073()",
	"Intrinsic0074()",
	"Intrinsic0075()",
	"Intrinsic0076()",
	"Intrinsic0077()",
	"Intrinsic0078()",
	"Intrinsic0079()",
	"Intrinsic007A()",
	"Intrinsic007B()",
	"Intrinsic007C()",
	"Intrinsic007D()",
	"Intrinsic007E()",
	"Intrinsic007F()",
	// 0080
	"Intrinsic0080()",
	"Intrinsic0081()",
	"Intrinsic0082()",
	"Intrinsic0083()",
	"Intrinsic0084()",
	"Intrinsic0085()",
	"Intrinsic0086()",
	"Intrinsic0087()",
	"Intrinsic0088()",
	"Intrinsic0089()",
	"Intrinsic008A()",
	"Intrinsic008B()",
	"Intrinsic008C()",
	"Intrinsic008D()",
	"Intrinsic008E()",
	"Intrinsic008F()",
	// 0090
	"Intrinsic0090()",
	"Intrinsic0091()",
	"Intrinsic0092()",
	"Intrinsic0093()",
	"Intrinsic0094()",
	"Intrinsic0095()",
	"Intrinsic0096()",
	"Intrinsic0097()",
	"Intrinsic0098()",
	"Intrinsic0099()",
	"Intrinsic009A()",
	"Intrinsic009B()",
	"Intrinsic009C()",
	"Intrinsic009D()",
	"Intrinsic009E()",
	"Intrinsic009F()",
	// 00A0
	"Intrinsic00A0()",
	"Intrinsic00A1()",
	"Intrinsic00A2()",
	"Intrinsic00A3()",
	"Intrinsic00A4()",
	"Intrinsic00A5()",
	"Intrinsic00A6()",
	"Intrinsic00A7()",
	"Intrinsic00A8()",
	"Intrinsic00A9()",
	"Intrinsic00AA()",
	"Intrinsic00AB()",
	"Intrinsic00AC()",
	"Intrinsic00AD()",
	"Intrinsic00AE()",
	"Intrinsic00AF()",
	// 00B0
	"Intrinsic00B0()",
	"Intrinsic00B1()",
	"Intrinsic00B2()",
	"Intrinsic00B3()",
	"Intrinsic00B4()",
	"Intrinsic00B5()",
	"Intrinsic00B6()",
	"Intrinsic00B7()",
	"Intrinsic00B8()",
	"Intrinsic00B9()",
	"Intrinsic00BA()",
	"Intrinsic00BB()",
	"Intrinsic00BC()",
	"Intrinsic00BD()",
	"Intrinsic00BE()",
	"Intrinsic00BF()",
	// 00C0
	"Intrinsic00C0()",
	"Intrinsic00C1()",
	"Intrinsic00C2()",
	"Intrinsic00C3()",
	"Intrinsic00C4()",
	"Intrinsic00C5()",
	"Intrinsic00C6()",
	"Intrinsic00C7()",
	"Intrinsic00C8()",
	"Intrinsic00C9()",
	"Intrinsic00CA()",
	"Intrinsic00CB()",
	"Intrinsic00CC()",
	"Intrinsic00CD()",
	"Intrinsic00CE()",
	"Intrinsic00CF()",
	// 00D0
	"Intrinsic00D0()",
	"Intrinsic00D1()",
	"Intrinsic00D2()",
	"Intrinsic00D3()",
	"Intrinsic00D4()",
	"Intrinsic00D5()",
	"Intrinsic00D6()",
	"Intrinsic00D7()",
	"Intrinsic00D8()",
	"Intrinsic00D9()",
	"Intrinsic00DA()",
	"Intrinsic00DB()",
	"Intrinsic00DC()",
	"Intrinsic00DD()",
	"Intrinsic00DE()",
	"Intrinsic00DF()",
	// 00E0
	"Intrinsic00E0()",
	"Intrinsic00E1()",
	"Intrinsic00E2()",
	"Intrinsic00E3()",
	"Intrinsic00E4()",
	"Intrinsic00E5()",
	"Intrinsic00E6()",
	"Intrinsic00E7()",
	"Intrinsic00E8()",
	"Intrinsic00E9()",
	"Intrinsic00EA()",
	"Intrinsic00EB()",
	"Intrinsic00EC()",
	"Intrinsic00ED()",
	"Intrinsic00EE()",
	"Intrinsic00EF()",
	// 00F0
	"Intrinsic00F0()",
	"Intrinsic00F1()",
	"Intrinsic00F2()",
	"Intrinsic00F3()",
	"Intrinsic00F4()",
	"Intrinsic00F5()",
	"Intrinsic00F6()",
	"Intrinsic00F7()",
	"Intrinsic00F8()",
	"Intrinsic00F9()",
	"Intrinsic00FA()",
	"Intrinsic00FB()",
	"Intrinsic00FC()",
	"Intrinsic00FD()",
	"Intrinsic00FE()",
	"Intrinsic00FF()",
	// 0100
	"Intrinsic0100()",
	"Intrinsic0101()",
	"Intrinsic0102()",
	"Intrinsic0103()",
	"Intrinsic0104()",
	"Intrinsic0105()",
	"Intrinsic0106()",
	"Intrinsic0107()",
	"Intrinsic0108()",
	"Intrinsic0109()",
	"Intrinsic010A()",
	"Intrinsic010B()",
	"Intrinsic010C()",
	"Intrinsic010D()",
	"Intrinsic010E()",
	"Intrinsic010F()",
	// 0110
	"Intrinsic0110()",
	"Intrinsic0111()",
	"Intrinsic0112()",
	"Intrinsic0113()",
	"Intrinsic0114()",
	"Intrinsic0115()",
	"Intrinsic0116()",
	"Intrinsic0117()",
	"Intrinsic0118()",
	"Intrinsic0119()",
	"Intrinsic011A()",
	"Intrinsic011B()",
	"Intrinsic011C()",
	"Intrinsic011D()",
	"Intrinsic011E()",
	"Intrinsic011F()",
	// 0120
	"Intrinsic0120()",
	"Intrinsic0121()",
	"Intrinsic0122()",
	"Intrinsic0123()",
	"Intrinsic0124()",
	"Intrinsic0125()",
	"Intrinsic0126()",
	"Intrinsic0127()",
	"Intrinsic0128()",
	"Intrinsic0129()",
	"Intrinsic012A()",
	"Intrinsic012B()",
	"Intrinsic012C()",
	"Intrinsic012D()",
	"Intrinsic012E()",
	"Intrinsic012F()",
	// 0130
	"Intrinsic0130()",
	"Intrinsic0131()",
	"Intrinsic0132()",
	"Intrinsic0133()",
	"Intrinsic0134()",
	"Intrinsic0135()",
	"Intrinsic0136()",
	"Intrinsic0137()",
	"Intrinsic0138()",
	"Intrinsic0139()",
	"Intrinsic013A()",
	"Intrinsic013B()",
	"Intrinsic013C()",
	"Intrinsic013D()",
	"Intrinsic013E()",
	"Intrinsic013F()",
	0
};

const char * const ConvertUsecodeCrusader::_event_names[] = {
	"look()",						// 0x00
	"use()",						// 0x01
	"anim()",						// 0x02
	"setActivity()",				// 0x03
	"cachein()",					// 0x04
	"hit(ushort,short)",			// 0x05
	"gotHit(ushort,short)",			// 0x06
	"hatch()",						// 0x07
	"schedule()",					// 0x08
	"release()",					// 0x09
	"equip()",						// 0x0A
	"unequip()",					// 0x0B
	"combine()",					// 0x0C
	"func0D",						// 0x0D
	"calledFromAnim()",				// 0x0E
	"enterFastArea()",				// 0x0F

	"leaveFastArea()",				// 0x10
	"cast(ushort)",					// 0x11
	"justMoved()",					// 0x12
	"AvatarStoleSomething(ushort)",	// 0x13
	"animGetHit()",					// 0x14
	"guardianBark(int)",			// 0x15
	"func16",						// 0x16
	"func17",						// 0x17
	"func18",						// 0x18
	"func19",						// 0x19
	"func1A",						// 0x1A
	"func1B",						// 0x1B
	"func1C",						// 0x1C
	"func1D",						// 0x1D
	"func1E",						// 0x1E
	"func1F",						// 0x1F
	0
};

void ConvertUsecodeCrusader::readheader(IFileDataSource *ucfile, UsecodeHeader &uch, uint32 &curOffset)
{
	uch.routines = read4(ucfile);     // routines
	uch.maxOffset = read4(ucfile);           // total code size,
	uch.offset = read4(ucfile)-20;    // code offset,
	uch.externTable = read4(ucfile); // extern symbol table offset
	uch.fixupTable = read4(ucfile);  // fixup table offset
	#ifdef DISASM_DEBUG
	con.Printf("Routines:\t%04X\n", uch.routines);
	con.Printf("MaxOffset:\t%04X\nOffset:\t\t%04X\n", uch.maxOffset, uch.offset);
	con.Printf("ExternTable:\t%04X\nFixupTable:\t%04X\n", uch.externTable, uch.fixupTable);
	#endif
	uch.maxOffset += 1;
	#ifdef DISASM_DEBUG
	con.Printf("Adjusted MaxOffset:\t%04X\n", uch.maxOffset);
	#endif
	curOffset = 1-uch.offset;
};

#endif
