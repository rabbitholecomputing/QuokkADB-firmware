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
    inline bool IsAmigaAmigaCtrl() {return amiga_amiga_ctrl;}
    inline bool IsSoftReset() {return _soft_rest;}
    inline void SetSoftReset(bool isReset){ _soft_rest = isReset;}
protected:
    bool AmigaAmigaCtrlTest();
    bool left_amiga_down = false;
    bool right_amiga_down = false;
    bool left_ctrl_down = false;
    bool right_ctrl_down = false;
    bool amiga_amiga_ctrl = false;
    bool _soft_rest = false;
};