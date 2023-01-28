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

#ifdef ADBUINO
#include <hidboot.h>
#elif RP2040
#include "keyboardrptparser.h"
#endif

#include "scqueue.h"

using simple_circular_queue::SCQueue;

#define KEYBOARD_QUEUE_CAPACITY (20)

class KeyEvent
{
public:
    static const uint8_t NoKey = 0xFF;
    static const uint8_t KeyDown = 0x01;
    static const uint8_t KeyUp = 0x02;
    inline uint8_t GetKeycode() { return m_keycode; }
    inline bool IsKeyUp() { return m_key_updown == KeyUp; }
    inline bool IsKeyDown() { return m_key_updown == KeyDown; }
    KeyEvent(uint8_t KeyCode, uint8_t KeyUpDown)
    {
        m_key_updown = KeyUpDown;
        m_keycode = KeyCode;
    }

protected:
    uint8_t m_keycode;
    uint8_t m_key_updown;
};

class UsbKbdRptParser : public KeyboardReportParser
{
public:
    void PrintKey(uint8_t mod, uint8_t key);

    KeyEvent GetKeyEvent();
    bool PendingKeyboardEvent();
    void Reset(void);
    

protected:
    void OnModifierKeysChanged(uint8_t before, uint8_t after);


    void OnKeyPressed(uint8_t key);

    uint8_t m_last_key_pressed;
    uint8_t m_last_key_up_or_down;

    uint8_t m_previous_key_pressed;
    MODIFIERKEYS m_modifier_keys;


    SCQueue<KeyEvent*, KEYBOARD_QUEUE_CAPACITY> m_keyboard_events;

};
