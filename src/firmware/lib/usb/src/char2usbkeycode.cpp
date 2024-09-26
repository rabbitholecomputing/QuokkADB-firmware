//---------------------------------------------------------------------------
//
//	ADBuino & QuokkADB ADB keyboard and mouse adapter
//
//	   Copyright (C) 2021-2022 akuker
//     Copyright (C) 2022 Rabbit Hole Computing LLC
//
//  This file is part of the ADBuino and the QuokkADB projects.
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
//  with the file. If not, see <https://www.gnu.org/licenses/>.
//
//----------------------------------------------------------------------------

#include "usb_hid_keys.h"
#include "char2usbkeycode.h"
#include <ctype.h>
static uint8_t alpha_azerty_usb_keycode(const char letter)
{
    switch (tolower(letter))
    {
        case 'a': return USB_KEY_Q;
        case 'z': return USB_KEY_W;
        case 'q': return USB_KEY_A;
        case 'm': return USB_KEY_SEMICOLON;
        case 'w': return USB_KEY_Z;
        default: // do nothing
            break;
    }

    return USB_KEY_A + tolower(letter) - 'a';
}

usbkey_t char_to_usb_keycode(char character, Region region)
{
    usbkey_t key;
    if (character >= 'A' && character <= 'Z')
    {
        if (region == RegionUS)
        {
            key.keycode = USB_KEY_A + (character - 'A');
        }
        if (region == RegionFR)
        {
            key.keycode = alpha_azerty_usb_keycode(character);
        }
        key.shift_down = true;
        return key;
    }

    if (character >= 'a' && character <= 'z')
    {
        if (region == RegionUS)
            key.keycode = USB_KEY_A + (character - 'a');
        if (region == RegionFR)
            key.keycode = alpha_azerty_usb_keycode(character);

        key.shift_down = false;
        return key;
    }

    if (character == '.')
    {
        if (region == RegionUS)
        {
            key.keycode = USB_KEY_DOT;
            key.shift_down = false;
        }
        if (region == RegionFR)
        {
            key.keycode = USB_KEY_COMMA;
            key.shift_down = false;
        }
        return key;
    }

    if (character == '0')
    {
        key.keycode = USB_KEY_0;
        if (region == RegionUS)
            key.shift_down = false;
        if (region == RegionFR)
            key.shift_down = true;

        return key;
    }

    if (character >= '1' && character <= '9')
    {
        key.keycode = USB_KEY_1 + (character - '1');
        if (region == RegionUS)
            key.shift_down = false;
        if (region == RegionFR)
            key.shift_down = true;
        return key;
    }

    if (character == ':')
    {
        if (region == RegionUS)
        {
            key.keycode = USB_KEY_SEMICOLON;
            key.shift_down = true;
        }
        if (region == RegionFR)
        {
            key.keycode = USB_KEY_DOT;
            key.shift_down = false;
        }
        return key;
    }

    if (character == ' ')
    {
        key.keycode = USB_KEY_SPACE;
        key.shift_down = false;
        return key;
    }

    if (character == '-')
    {
        if (region == RegionUS)
            key.keycode = USB_KEY_MINUS;
        if (region == RegionFR)
            key.keycode = USB_KEY_EQUAL;
        key.shift_down = false;
        return key;
    }

    if (character == '*')
    {
        if (region == RegionUS)
            key.keycode = USB_KEY_8;
        if (region == RegionFR)
            key.keycode = USB_KEY_RIGHTBRACE;
        key.shift_down = true;
        return key;
    }

    if (character == '\n')
    {
        key.keycode = USB_KEY_ENTER;
        key.shift_down = true;
        return key;
    }

    if (character == '[')
    {
        if (region == RegionUS)
            key.keycode = USB_KEY_LEFTBRACE;
        if (region == RegionFR)
            key.keycode = USB_KEY_102ND;
        key.shift_down = false;
        return key;
    }

    if (character == ']')
    {
        if (region == RegionUS)
        {
            key.keycode = USB_KEY_RIGHTBRACE;
            key.shift_down = false;
        }
        if (region == RegionFR)
        {
            key.keycode = USB_KEY_102ND;
            key.shift_down = true;
        }
        return key;
    }

    if (character == '=')
    {
        if (region == RegionUS)
            key.keycode = USB_KEY_EQUAL;
        if (region == RegionFR)
            key.keycode = USB_KEY_SLASH;
        key.shift_down = false;
        return key;
    }

    if (character == '+')
    {
        if (region == RegionUS)
            key.keycode = USB_KEY_EQUAL;
        if (region == RegionFR)
            key.keycode = USB_KEY_SLASH;
        key.shift_down = true;
        return key;
    }

    if (character == '(')
    {
        if (region == RegionUS)
        {
            key.keycode = USB_KEY_9;
            key.shift_down = true;
        }
        if (region == RegionFR)
        {
            key.keycode = USB_KEY_5;
            key.shift_down = false;
        }
        return key;
    }

    if (character == ')')
    {
        if (region == RegionUS)
        {
            key.keycode = USB_KEY_0;
            key.shift_down = true;
        }
        if (region == RegionFR)
        {
            key.keycode = USB_KEY_MINUS;
            key.shift_down = false;
        }

        return key;
    }

    if (character == '/')
    {
        if (region == RegionUS)
        {
            key.keycode = USB_KEY_SLASH;
            key.shift_down = false;
        }
        if (region == RegionFR)
        {
            key.keycode = USB_KEY_DOT;
            key.shift_down = true;
        }
        return key;
    }

    if (character == '<')
    {
        if (region == RegionUS)
        {
            key.keycode = USB_KEY_COMMA;
            key.shift_down = true;
        }
        if (region == RegionFR)
        {
            key.keycode = USB_KEY_102ND;
            key.shift_down = false;
        }
        return key;
    }

    if (character == '>')
    {
        if (region == RegionUS)
            key.keycode = USB_KEY_DOT;
        if (region == RegionFR)
            key.keycode = USB_KEY_102ND;

        key.shift_down = true;
        return key;
    }

    key.keycode = 0;
    key.shift_down = false;
    return key;
}
