//---------------------------------------------------------------------------
//
//	QuokkAmiga Amiga keyboard adapter
//
//     Copyright (C) 2022 Rabbit Hole Computing LLC
//
//  This file is part of QuokkAmiga.
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
//----------------------------------------------------------------------------

#include "usbtoamiga.h"
#include "usb_hid_keys.h"
#include <stdint.h>
#include "rp2040_serial.h"
using rp2040_serial::Serial;

uint8_t usb_keycode_to_amiga_code(uint8_t usb_code)
{
    switch (usb_code)
    {
    case USB_KEY_GRAVE:
        return 0x00;
    case USB_KEY_1:
        return 0x01;
    case USB_KEY_2:
        return 0x02;
    case USB_KEY_3:
        return 0x03;
    case USB_KEY_4:
        return 0x04;
    case USB_KEY_5:
        return 0x05;
    case USB_KEY_6:
        return 0x06;
    case USB_KEY_7:
        return 0x07;
    case USB_KEY_8:
        return 0x08;
    case USB_KEY_9:
        return 0x09;
    case USB_KEY_0:
        return 0x0A;
    case USB_KEY_MINUS:
        return 0x0B;
    case USB_KEY_EQUAL:
        return 0x0C;
    case USB_KEY_BACKSLASH:
        return 0x0D;
    case USB_KEY_YEN:
        return 0x0E;
    case USB_KEY_KP0:
        return 0x0F;
    case USB_KEY_Q:
        return 0x10;
    case USB_KEY_W:
        return 0x11;
    case USB_KEY_E:
        return 0x12;
    case USB_KEY_R:
        return 0x13;
    case USB_KEY_T:
        return 0x14;
    case USB_KEY_Y:
        return 0x15;
    case USB_KEY_U:
        return 0x16;
    case USB_KEY_I:
        return 0x17;
    case USB_KEY_O:
        return 0x18;
    case USB_KEY_P:
        return 0x19;
    case USB_KEY_LEFTBRACE:
        return 0x1A;
    case USB_KEY_RIGHTBRACE:
        return 0x1B;
    case USB_KEY_KP1:
        return 0x1D;
    case USB_KEY_KP2:
        return 0x1E;
    case USB_KEY_KP3:
        return 0x1F;
    case USB_KEY_A:
        return 0x20;
    case USB_KEY_S:
        return 0x21;
    case USB_KEY_D:
        return 0x22;
    case USB_KEY_F:
        return 0x23;
    case USB_KEY_G:
        return 0x24;
    case USB_KEY_H:
        return 0x25;
    case USB_KEY_J:
        return 0x26;
    case USB_KEY_K:
        return 0x27;
    case USB_KEY_L:
        return 0x28;
    case USB_KEY_SEMICOLON:
        return 0x29;        
    case USB_KEY_APOSTROPHE:
        return 0x2A;
    case USB_KEY_HASHTILDE:
        return 0x2B;
    case USB_KEY_KP4:
        return 0x2D;        
    case USB_KEY_KP5:
        return 0x2E;        
    case USB_KEY_KP6:
        return 0x2F; 
    case USB_KEY_102ND:
        return 0x30;
    case USB_KEY_Z:
        return 0x31;          
    case USB_KEY_X:
        return 0x32;
    case USB_KEY_C:
        return 0x33;
    case USB_KEY_V:
        return 0x34;
    case USB_KEY_B:
        return 0x35;
    case USB_KEY_N:
        return 0x36;
    case USB_KEY_M:
        return 0x37;
    case USB_KEY_COMMA:
        return 0x38;
    case USB_KEY_DOT:
        return 0x39;
    case USB_KEY_SLASH:
        return 0x3A;
    case USB_KEY_RO:
        return 0x3B;
    case USB_KEY_KPDOT:
        return 0x3C;
    case USB_KEY_KP7:
        return 0x3D;
    case USB_KEY_KP8:
        return 0x3E;
    case USB_KEY_KP9:
        return 0x3F;
    case USB_KEY_SPACE:
        return 0x40;
    case USB_KEY_BACKSPACE:
        return 0x41;
    case USB_KEY_TAB:
        return 0x42;
    case USB_KEY_KPENTER:
        return 0x43;
    case USB_KEY_ENTER:
        return 0x44;
    case USB_KEY_ESC:
        return 0x45;
    case USB_KEY_DELETE:
        return 0x46;
    case USB_KEY_KPMINUS:
        return 0x4A;
    case USB_KEY_F11:
        return 0x4B;
    case USB_KEY_UP:
        return 0x4C;
    case USB_KEY_DOWN:
        return 0x4D;
    case USB_KEY_RIGHT:
        return 0x4E;
    case USB_KEY_LEFT:
        return 0x4F;
    case USB_KEY_F1:
        return 0x50;
    case USB_KEY_F2:
        return 0x51;
    case USB_KEY_F3:
        return 0x52;
    case USB_KEY_F4:
        return 0x53;
    case USB_KEY_F5:
        return 0x54;
    case USB_KEY_F6:
        return 0x55;
    case USB_KEY_F7:
        return 0x56;
    case USB_KEY_F8:
        return 0x57;
    case USB_KEY_F9:
        return 0x58;
    case USB_KEY_F10:
        return 0x59;
    case USB_KEY_KPLEFTPAREN:
        return 0x5A;
    case USB_KEY_KPRIGHTPAREN:
        return 0x5B;
    case USB_KEY_KPSLASH:
        return 0x5C;
    case USB_KEY_KPASTERISK:
        return 0x5D;
    case USB_KEY_KPPLUS:
        return 0x5E;
    // SYSRQ/PRINTSC is mapped to the Amiga help key
    case USB_KEY_SYSRQ:
    case USB_KEY_HELP:
        return 0x5F;
    case USB_KEY_LEFTSHIFT:
        return 0X60;
    case USB_KEY_RIGHTSHIFT:
        return 0x61;
    case USB_KEY_CAPSLOCK:
        return 0x62;
    // Amiga only has one CTRL button
    case USB_KEY_LEFTCTRL:
    case USB_KEY_RIGHTCTRL:
        return 0x63;
    case USB_KEY_LEFTALT:
        return 0x64;
    case USB_KEY_RIGHTALT:
        return 0x65;
    case USB_KEY_LEFTMETA:
        return 0x66;
    case USB_KEY_RIGHTMETA:
        return 0x67;
    case USB_KEY_PAUSE:
        return 0x6E;
    case USB_KEY_F12:
        return 0x6F;

    default:
        Serial.print("Unknown keycode found: ");
        Serial.println(usb_code, HEX);
        return 0xFF;
    }

}


