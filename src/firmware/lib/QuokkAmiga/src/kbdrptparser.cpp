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
#include "kbdrptparser.h"
#include "bithacks.h"
#include "usb_hid_keys.h"

#ifdef RP2040
#include "rp2040_serial.h"
using rp2040_serial::Serial;
#endif

extern bool global_debug;

void KbdRptParser::OnKeyDown(uint8_t mod, uint8_t key)
{

    if (key == USB_KEY_NONE || key == USB_KEY_ERR_OVF)
    return;

    if (global_debug)
    {
        Serial.print("DN ");
        PrintKey(mod, key);
    }
    uint8_t c = OemToAscii(mod, key);
    // m_last_key_pressed = key;
    // m_last_key_up_or_down = KeyEvent::KeyDown;

    if (c)
        OnKeyPressed(c);

    if (m_keyboard_events.enqueue(new KeyEvent(key, KeyEvent::KeyDown)))
    {
        switch(key)
        {
            case USB_KEY_LEFTMETA :
                left_amiga_down = true;
                amiga_amiga_ctrl = AmigaAmigaCtrlTest();
            break;
            case USB_KEY_RIGHTMETA :
                right_amiga_down = true;
                amiga_amiga_ctrl = AmigaAmigaCtrlTest();
            break;
            case USB_KEY_LEFTCTRL :
                left_ctrl_down = true;
                amiga_amiga_ctrl = AmigaAmigaCtrlTest();
            break;        
            case USB_KEY_RIGHTCTRL :
                right_ctrl_down = true;
                amiga_amiga_ctrl = AmigaAmigaCtrlTest();
            break;
        }
        // Use F12 as AMIGA AMIGA CTRL short cut
        if (amiga_amiga_ctrl) 
        {
            !m_keyboard_events.enqueue(new KeyEvent(USB_KEY_F12, KeyEvent::KeyDown));
        }
    }
    else
    {
        Serial.println("Warning! unable to enqueue new KeyDown");
    }
}


void KbdRptParser::OnKeyUp(uint8_t mod, uint8_t key)
{
    if (key == USB_KEY_NONE || key == USB_KEY_ERR_OVF)
        return;

    if (global_debug)
    {
        Serial.print("UP ");
        PrintKey(mod, key);
    }

    if (m_keyboard_events.enqueue(new KeyEvent(key, KeyEvent::KeyUp)))
    {
        if(amiga_amiga_ctrl)
        {
            switch(key)
            {
                case USB_KEY_LEFTMETA :
                    left_amiga_down = false;
                    amiga_amiga_ctrl = AmigaAmigaCtrlTest();
                break;
                case USB_KEY_RIGHTMETA :
                    right_amiga_down = false;
                    amiga_amiga_ctrl = AmigaAmigaCtrlTest();
                break;
                case USB_KEY_LEFTCTRL :
                    left_ctrl_down = false;
                    amiga_amiga_ctrl = AmigaAmigaCtrlTest();
                break;        
                case USB_KEY_RIGHTCTRL :
                    right_ctrl_down = false;
                    amiga_amiga_ctrl = AmigaAmigaCtrlTest();
                break;
            }

            if (!amiga_amiga_ctrl)
            {
                // Use F12 as amiga amiga ctrl shortcut
                m_keyboard_events.enqueue(new KeyEvent(USB_KEY_F12, KeyEvent::KeyUp));
            }
        }
    }
    else
    {
        Serial.println("Warning! unable to enqueue new KeyDown");
    }
}


bool KbdRptParser::AmigaAmigaCtrlTest()
{
    return left_amiga_down && right_amiga_down && (left_ctrl_down || right_ctrl_down);
}