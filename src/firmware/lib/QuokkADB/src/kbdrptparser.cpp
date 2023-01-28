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

    if (!m_keyboard_events.enqueue(new KeyEvent(key, KeyEvent::KeyDown)))
    {
        Serial.println("Warning! unable to enqueue new KeyDown");
    }
    // If power button replacement queue key twice
    else if (key == USB_KEY_PAUSE || key == USB_KEY_F15)
    {
        if (!m_keyboard_events.enqueue(new KeyEvent(key, KeyEvent::KeyDown)))
        {
            Serial.println("Warning! unable to enqueue new Power Button KeyDown");
        }
    }

    // Special handling of Caps Lock because of its held down nature
    if (key == USB_KEY_CAPSLOCK)
    {
        if (kbdLockingKeys.kbdLeds.bmCapsLock == 1)
        {
            B_UNSET(m_custom_mod_keys, CapsLockFlag);
            B_UNSET(m_custom_mod_keys, Led2CapsLockFlag);
        }
        else
        {
            B_SET(m_custom_mod_keys, CapsLockFlag);
            B_SET(m_custom_mod_keys, Led2CapsLockFlag);
        }        
    }

    /* @DEBUG original key status
    if (key == USB_KEY_SCROLLLOCK)
    {
        B_UNSET(m_custom_mod_keys, ScrollLockFlag);
        if (B_IS_SET(m_custom_mod_keys, Led3ScrollLockFlag))
        {
            B_TOGGLE(m_custom_mod_keys, Led3ScrollLockFlag);
        }
    }
    if (key == USB_KEY_NUMLOCK)
    {
        B_UNSET(m_custom_mod_keys, NumLockFlag);
        if (B_IS_SET(m_custom_mod_keys, Led1NumLockFlag))
        {
            B_UNSET(m_custom_mod_keys, Led1NumLockFlag);
        }
    }
    */
    if (key == USB_KEY_NUMLOCK)
    {
        if (kbdLockingKeys.kbdLeds.bmNumLock == 1)
        {
            B_SET(m_custom_mod_keys, NumLockFlag);
            B_SET(m_custom_mod_keys, Led1NumLockFlag);
        }
        else
        {
            B_UNSET(m_custom_mod_keys, NumLockFlag);
            B_UNSET(m_custom_mod_keys, Led1NumLockFlag);
        }        
    }
    if (key == USB_KEY_SCROLLLOCK)
    {
        if (kbdLockingKeys.kbdLeds.bmScrollLock == 1)
        {
            B_SET(m_custom_mod_keys, ScrollLockFlag);
            B_SET(m_custom_mod_keys, Led3ScrollLockFlag);
        }
        else
        {
            B_UNSET(m_custom_mod_keys, ScrollLockFlag);
            B_UNSET(m_custom_mod_keys, Led3ScrollLockFlag);
        }        
    }

    if (key == USB_KEY_BACKSPACE)
    {
        B_SET(m_custom_mod_keys, DeleteFlag);
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
    if (!m_keyboard_events.enqueue(new KeyEvent(key, KeyEvent::KeyUp)))
    {
        Serial.println("Warning! unable to enqueue new KeyDown");
    }
    // If power button replacement queue key twice
    else if (key == USB_KEY_PAUSE || key == USB_KEY_F15)
    {
        if (!m_keyboard_events.enqueue(new KeyEvent(key, KeyEvent::KeyUp)))
        {
            Serial.println("Warning! unable to enqueue new Power Button KeyUp");
        }
    }

/* @DEBUG Num and scroll lock keys 

    if (key == USB_KEY_SCROLLLOCK)
    {
        B_SET(m_custom_mod_keys, ScrollLockFlag);
        if (!B_IS_SET(m_custom_mod_keys, Led3ScrollLockFlag))
        {
            B_SET(m_custom_mod_keys, Led3ScrollLockFlag);
        }
    }
    if (key == USB_KEY_NUMLOCK)
    {
      B_SET(m_custom_mod_keys, NumLockFlag);
       if (!B_IS_SET(m_custom_mod_keys, Led1NumLockFlag))
        {
            B_SET(m_custom_mod_keys, Led1NumLockFlag);
        }
    }
 */

    if (key == USB_KEY_BACKSPACE)
    {
        B_UNSET(m_custom_mod_keys, DeleteFlag);
    }
}
