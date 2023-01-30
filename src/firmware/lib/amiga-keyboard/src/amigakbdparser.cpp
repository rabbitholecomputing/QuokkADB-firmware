//---------------------------------------------------------------------------
//
//	ADBuino & QuokkaADB ADB keyboard and mouse adapter
//
//	   Copyright (C) 2021-2022 akuker
//     Copyright (C) 2022 Rabbit Hole Computing LLC
//
//  This file is part of the ADBuino and the QuokkaADB projects.
//
//  This file is free software: you can redistribute it and/or modify it under 
//  the terms of the GNU General Public License as published by the Free 
//  Software Foundation, either version 3 of the License, or (at your option) 
//  any later version.
//
//  This file is distributed in the hope that it will be useful, but WITHOUT ANY 
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
//  FOR A PARTICULAR PURPOSE. See the GNU General Public License for more 
//  details.
//
//  You should have received a copy of the GNU General Public License along 
//  with file. If not, see <https://www.gnu.org/licenses/>.
//
//----------------------------------------------------------------------------

#include "amigakbdparser.h"
#include "usb_hid_keys.h"
#include "usbtoamiga.h"
#ifdef RP2040
#include "rp2040_serial.h"
using rp2040_serial::Serial;
#endif

extern bool global_debug;

bool AmigaKbdRptParser::IsKeyQueued(void)
{
    return !m_keyboard_events.isEmpty();
}

AmigaKey* AmigaKbdRptParser::GetAmigaKeyCode()
{
    KeyEvent *event;
    AmigaKey* key = new AmigaKey();
    // Pack the first key event
    if (!m_keyboard_events.isEmpty())
    {
        event = m_keyboard_events.dequeue();
        key->rotatedKeyCode = usb_keycode_to_amiga_code(event->GetKeycode()) << 1;
        key->isKeyDown = event->IsKeyDown();
        free(event);
        // MSB bit is low (logic 1) for key down
        if (!key->isKeyDown)
        {
            key->rotatedKeyCode |= 1;
        }
    }
    return key;
}
