//---------------------------------------------------------------------------
//
//	QuokkADB ADB keyboard and mouse adapter
//
//     Copyright (C) 2023 Rabbit Hole Computing LLC
//
//  This file is part of QuokkADB.
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
//
//  Portions of this code were originally released under a Modified BSD 
//  License. See LICENSE in the root of this repository for more info.
//
//----------------------------------------------------------------------------uint8_t usb_keycode_to_adb_code(uint8_t usb_code)
#include "adbkeytostring.h"


const char* adb_key_to_string(uint8_t adb_key_code)
{
    switch (adb_key_code)
    {
    case 0x00:
        return "A";
    case 0x01:
        return "S";
    case 0x02:
        return "D";
    case 0x03:
        return "F";
    case 0x04:
        return "H";
    case 0x05:
        return "G";
    case 0x06:
        return "Z";
    case 0x07:
        return "X";
    case 0x08:
        return "C";
    case 0x09:
        return "V";
    case 0x0B:
        return "B";
    case 0x0C:
        return "Q";
    case 0x0D:
        return "W";
    case 0x0E:
        return "E";
    case 0x0F:
        return "R";
    case 0x10:
        return "Y";
    case 0x11:
        return "T";
    case 0x12:
        return "1" ;
    case 0x13:
        return "2";
    case 0x14:
        return "3";
    case 0x15:
        return "4";
    case 0x16:
        return "6";
    case 0x17:
        return "5";
    case 0x18:
        return "=";
    case 0x19:
        return "9";
    case 0x1A:
        return "7";
    case 0x1B:
        return "-";
    case 0x1C:
        return "8";
    case 0x1D:
        return "0";
    case 0x1E:
        return "}";
    case 0x1F:
        return "O";
    case 0x20:
        return "U";
    case 0x21:
        return "{";
    case 0x22:
        return "I";
    case 0x23:
        return "P";
    case 0x24:
        return "Enter";
    case 0x25:
        return "L";
    case 0x26:
        return "J";
    case 0x27:
        return "'";
    case 0x28:
        return "K";
    case 0x29:
        return ";";
    case 0x2A:
        return "\\";
    case 0x2B:
        return ",";
    case 0x2C:
        return "/";
    case 0x2D:
        return "N";
    case 0x2E:
        return "M";
    case 0x2F:
        return ".";
    case 0x30:
        return "TAB";
    case 0x31:
        return "Space";
    case 0x32:
        return "`";
    case 0x33:
        return "Backspace";
    case 0x35:
        return "Esc";
    case 0x39:
        return "Capslock";
    case 0x3B:
        return "Left";
    case 0x3C:
        return "Right";
    case 0x3D:
        return "Down";
    case 0x3E:
        return "Up";
    case 0x41:
        return "KP.";
    case 0x43:
        return "KP*";
    case 0x45:
        return "KP+";
    case 0x47: 
        return "Numlock";
    case 0x4B:
        return "KP/";
    case 0x4C:
        return "KPEnter";
    case 0x4E:
        return "KP-";
    case 0x51:
        return "KP=";
    case 0x52:
        return "KP0";
    case 0x53:
        return "KP1";
    case 0x54:
        return "KP2";
    case 0x55:
        return "KP3";
    case 0x56:
        return "KP4";
    case 0x57:
        return "KP5";
    case 0x58:
        return "KP6";
    case 0x59:
        return "KP7";
    case 0x5B:
        return "KP8";
    case 0x5C:
        return "KP9";
    case 0x60:
        return "F5";
    case 0x61:
        return "F6";
    case 0x62:
        return "F7";
    case 0x63:
        return "F3";
    case 0x64:
        return "F8";
    case 0x65:
        return "F9";
    case 0x67:
        return "F11";
    case 0x69:
        return "F13";
    case 0x6B: 
        return "F14";
    case 0x6D:
        return "F10";
    case 0x6F:
        return "F12";
    case 0x71:
        return "F15";
    case 0x72:
        return "Help";
    case 0x73:
        return "Home";
    case 0x74:
        return "PageUp";
    case 0x75:
        return "Delete";
    case 0x76:
        return "F4";
    case 0x77:
        return "End";
    case 0x78:
        return "F2";
    case 0x79:
        return "PageDown";
    case 0x7A:
        return "F1";
    case 0x7D:
        return "RightCtrlExt";
    case 0x36:
        return "LeftCtrl";
    case 0x7B:
        return "RightShiftExt";
    case 0x38:
        return "LeftShift";
    case 0x7C:
        return "RightOptionExt";    
    case 0x3A:
        return "LeftOption";
    case 0x37:
        return "Command";
    case 0x7F:
        return "NoKeyPress";
    default:
        return "KeyUnknown";
    }
    return "KeyUnknown";
}

