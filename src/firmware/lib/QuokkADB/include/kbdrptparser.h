//----------------------------------------------------------------------------
//
//  ADBuino & QuokkADB ADB keyboard and mouse adapter
//     Copyright (C) 2007 Peter H Anderson
//	   Copyright (C) 2021-2022 akuker
//     Copyright (C) 2022 Rabbit Hole Computing LLC
//
//  This file is part of the ADBuino & QuokkADB and projects
//
//  This file is free software: you can redistribute it and/or modify it under 
//  the terms of the GNU General Public License as published by the Free 
//  Software Foundation, either version 3 of the License, or (at your option) 
// any later version.
//
//  This file is distributed in the hope that it will be useful, but WITHOUT ANY 
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
//  FOR A PARTICULAR PURPOSE. See the GNU General Public License for more 
//  details.
//
//  You should have received a copy of the GNU General Public License along 
//  with this file. If not, see <https://www.gnu.org/licenses/>.
//----------------------------------------------------------------------------

#pragma once
#include "usbkbdparser.h"
class KbdRptParser : public UsbKbdRptParser 
{
public:
    void OnKeyDown(uint8_t mod, uint8_t key);
    void OnKeyUp(uint8_t mod, uint8_t key);

protected:
    // Flag to indicated that the key was released
    const uint16_t KeyReleasedFlag = 0x80;

    // Flags for special modifier keys that are used in the ADB
    // protocol, but not handled automatically by the Arduino
    // USB library
    uint16_t m_custom_mod_keys;
    // Flags for the custom ADB-specific modifier keys
    static const int DeleteFlag = 0;
    static const int CapsLockFlag = 1;
    static const int ResetFlag = 2;
    static const int NumLockFlag = 3;
    static const int ScrollLockFlag = 4;
    static const int Led3ScrollLockFlag = 5;
    static const int Led2CapsLockFlag = 6;
    static const int Led1NumLockFlag = 7;


};