/*
Copyright (C) 2003  The Pentagram Team

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

// Windows Stuff
#ifdef WIN32

// These will prevent inclusion of mmsystem sections
#define MMNODRV         // No Installable driver support
#define MMNOWAVE        // No Waveform support
#define MMNOAUX         // No Auxiliary audio support
#define MMNOMIXER       // No Mixer support
#define MMNOTIMER       // No Timer support
#define MMNOJOY         // No Joystick support
#define MMNOMMIO        // No Multimedia file I/O support

#include "WindowsMidiDriver.h"

const MidiDriver::MidiDriverDesc WindowsMidiDriver::desc = 
		MidiDriver::MidiDriverDesc ("Windows", createInstance);

using std::endl;
#include "Q_strcasecmp.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <mmsystem.h>
#include <winbase.h>

WindowsMidiDriver::WindowsMidiDriver() : 
	LowLevelMidiDriver(), dev_num(-1), midi_port(0), _streamEvent(0)
{
#ifdef WIN32_USE_DUAL_MIDIDRIVERS
	midi_port2 = 0;
#endif
}

bool WindowsMidiDriver::doMCIError(MMRESULT mmsys_err)
{
	if (mmsys_err != MMSYSERR_NOERROR)
	{
		char buf[512];
		midiOutGetErrorText(mmsys_err, buf, 512);
		perr << buf << endl;
		return true;
	}
	return false;
}

int WindowsMidiDriver::open()
{
	int i;
	// Get Win32 Midi Device num
	//config->value("config/audio/midi/win32_device", dev_num, -1);
	dev_num = -1;
#ifdef WIN32_USE_DUAL_MIDIDRIVERS
	int dev_num2 = -2;
#endif

	// List all the midi devices.
	MIDIOUTCAPS caps;
	signed long dev_count = (signed long) midiOutGetNumDevs(); 
	pout << dev_count << " Midi Devices Detected" << endl;
	pout << "Listing midi devices:" << endl;

	for (i = -1; i < dev_count; i++)
	{
		midiOutGetDevCaps ((UINT) i, &caps, sizeof(caps));
		pout << i << ": " << caps.szPname << endl;
#ifdef WIN32_USE_DUAL_MIDIDRIVERS
		if (!Pentagram::Q_strncasecmp(caps.szPname, "SB Live! Synth A", 16)) dev_num = i;
		else if (!Pentagram::Q_strncasecmp(caps.szPname, "SB Live! Synth B", 16)) dev_num2 = i;
#endif
	}

	if (dev_num < -1 || dev_num >= dev_count)
	{
		perr << "Warning Midi device in config is out of range." << endl;
		dev_num = -1;
	}

	midiOutGetDevCaps ((UINT) dev_num, &caps, sizeof(caps));
	pout << "Using device " << dev_num << ": "<< caps.szPname << endl;

	_streamEvent = CreateEvent (NULL, true, true, NULL);
	UINT mmsys_err = midiOutOpen (&midi_port, dev_num, (unsigned long) _streamEvent, 0, CALLBACK_EVENT);

#ifdef WIN32_USE_DUAL_MIDIDRIVERS
	if (dev_num2 != -2 && mmsys_err != MMSYSERR_NOERROR)
	{
		midiOutGetDevCaps ((UINT) dev_num2, &caps, sizeof(caps));
		if (dev_num2 != -2) pout << "Using device " << dev_num2 << ": "<< caps.szPname << endl;
		mmsys_err = midiOutOpen (&midi_port2, dev_num2, 0, 0, 0);
	}
#endif

	if (doMCIError(mmsys_err))
	{
		perr << "Error: Unable to open win32 midi device" << endl;
		CloseHandle(_streamEvent);
		_streamEvent = 0;
		return 1;
	}

	// Set Win32 Midi Device num
	//config->set("config/audio/midi/win32_device", dev_num, true);
	
	return 0;
}

void WindowsMidiDriver::close()
{
#ifdef WIN32_USE_DUAL_MIDIDRIVERS
	if (midi_port2 != 0) midiOutClose (midi_port2);
	midi_port2 = 0;
#endif
	midiOutClose (midi_port);
	midi_port = 0;
	CloseHandle(_streamEvent);
	_streamEvent = 0;
}

void WindowsMidiDriver::send(uint32 message)
{
#ifdef WIN32_USE_DUAL_MIDIDRIVERS
	if (message & 0x1 && midi_port2 != 0) 
		midiOutShortMsg (midi_port2,  message);
	else
		midiOutShortMsg (midi_port,  message);
#else
	midiOutShortMsg (midi_port,  message);
#endif
}

void WindowsMidiDriver::send_sysex (uint8 status, const uint8 *msg, uint16 length)
{
#ifdef WIN32_USE_DUAL_MIDIDRIVERS
	// Hack for multiple devices. Not exactly 'fast'
	if (midi_port2 != 0) {
		HMIDIOUT			orig_midi_port = midi_port;
		HMIDIOUT			orig_midi_port2 = midi_port2;

		// Send to port 1
		midi_port2 = 0;
		send_sysex(status, msg, length);

		// Send to port 2
		midi_port = orig_midi_port2;
		send_sysex(status, msg, length);

		// Return the ports to normal
		midi_port = orig_midi_port;
		midi_port2 = orig_midi_port2;
	}
#endif

	if (WaitForSingleObject (_streamEvent, 2000) == WAIT_TIMEOUT) {
		perr << "Error: Could not send SysEx - MMSYSTEM is still trying to send data after 2 seconds." << std::endl;
		return;
	}

	MMRESULT result = midiOutUnprepareHeader (midi_port, &_streamHeader, sizeof (_streamHeader));
	if (doMCIError(result)) {
		//check_error (result);
		perr << "Error: Could not send SysEx - midiOutUnprepareHeader failed." << std::endl;
		return;
	}

	_streamBuffer [0] = status;
	memcpy(&_streamBuffer[1], msg, length);

	_streamHeader.lpData = (char *) _streamBuffer;
	_streamHeader.dwBufferLength = length + 1;
	_streamHeader.dwBytesRecorded = length + 1;
	_streamHeader.dwUser = 0;
	_streamHeader.dwFlags = 0;

	result = midiOutPrepareHeader (midi_port, &_streamHeader, sizeof (_streamHeader));
	if (doMCIError(result)) {
		//check_error (result);
		perr << "Error: Could not send SysEx - midiOutPrepareHeader failed." << std::endl;
		return;
	}

	ResetEvent(_streamEvent);
	result = midiOutLongMsg (midi_port, &_streamHeader, sizeof (_streamHeader));
	if (doMCIError(result)) {
		//check_error(result);
		perr << "Error: Could not send SysEx - midiOutLongMsg failed." << std::endl;
		SetEvent(_streamEvent);
		return;
	}
}

void WindowsMidiDriver::increaseThreadPriority()
{
	SetThreadPriority (GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
}

void WindowsMidiDriver::yield()
{
	Sleep(1);
}

#endif //WIN32
