//----------------------------------------------------------------------------
//
//  QuokkADB ADB keyboard and mouse adapter
//     Copyright (C) 2011 Circuits At Home, LTD. All rights reserved.
//     Copyright (C) 2022 Rabbit Hole Computing LLC
//
//  This file is part of QuokkADB.
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
//  with this file. If not, see <https://www.gnu.org/licenses/>.
//
//  A majority of this code were originally released under a 
//  General Public License version 2 (GPL2) License.
//  See LICENSE in the root of this repository for more info.
//
//---------------------------------------------------------------------------

#include "keyboardrptparser.h"
#include <tusb.h>

#define VALUE_WITHIN(v,l,h) (((v)>=(l)) && ((v)<=(h)))
#define pgm_read_byte(addr) (*(const unsigned char *)(addr))

extern uint16_t modifierkeys;

uint8_t inline findModifierKey(hid_keyboard_report_t const *report, const hid_keyboard_modifier_bm_t mod ) {
        return (mod & report->modifier) ? 1 : 0;
}
void KeyboardReportParser::AddKeyboard(uint8_t dev_addr, uint8_t instance) {
        for(size_t i = 0; i < MAX_KEYBOARDS; i++)
        {
                if (!keyboards_list[i].in_use)
                {
                        keyboards_list[i].in_use = true;
                        keyboards_list[i].device_addr = dev_addr;
                        keyboards_list[i].instance = instance;
                        SetUSBkeyboardLEDs(kbdLockingKeys.kbdLeds.bmCapsLock, 
                                kbdLockingKeys.kbdLeds.bmNumLock,
                                kbdLockingKeys.kbdLeds.bmScrollLock
                        );
                        break;
                }
               
        }

}
void KeyboardReportParser::RemoveKeyboard(uint8_t dev_addr, uint8_t instance) {
  for(size_t i = 0; i < MAX_KEYBOARDS; i++)
  {
    if (keyboards_list[i].in_use && keyboards_list[i].device_addr == dev_addr && keyboards_list[i].instance == instance)
    {
      keyboards_list[i].in_use = false;
      break;
    }
  }
}

void KeyboardReportParser::Parse(uint8_t dev_addr, uint8_t instance, hid_keyboard_report_t const *report) {
        static bool caps_on = false;
        union {
                KBDINFO kbdInfo;
                uint8_t bInfo[sizeof (KBDINFO)];
        } current_state;
        
        KBDINFO *cur_kbd_info  = &(current_state.kbdInfo); 

        cur_kbd_info->bmLeftCtrl =   findModifierKey(report, KEYBOARD_MODIFIER_LEFTCTRL);
        cur_kbd_info->bmLeftShift =  findModifierKey(report, KEYBOARD_MODIFIER_LEFTSHIFT);
        cur_kbd_info->bmLeftAlt =    findModifierKey(report, KEYBOARD_MODIFIER_LEFTALT);
        cur_kbd_info->bmLeftGUI =    findModifierKey(report, KEYBOARD_MODIFIER_LEFTGUI);
        cur_kbd_info->bmRightCtrl =  findModifierKey(report, KEYBOARD_MODIFIER_RIGHTCTRL);
        cur_kbd_info->bmRightShift = findModifierKey(report, KEYBOARD_MODIFIER_RIGHTSHIFT);
        cur_kbd_info->bmRightAlt =   findModifierKey(report, KEYBOARD_MODIFIER_RIGHTALT);
        cur_kbd_info->bmRightGUI =   findModifierKey(report, KEYBOARD_MODIFIER_RIGHTGUI);
        memcpy(cur_kbd_info->Keys, report->keycode, 6);
        cur_kbd_info->bReserved =  report->reserved;
        
        // provide event for changed control key state
        if (prevState.bInfo[0x00] != current_state.bInfo[0x00]) {
                OnModifierKeysChanged(prevState.bInfo[0x00], current_state.bInfo[0x00]);
        }

        for (uint8_t i = 2; i < 8; i++) {
                bool down = false;
                bool up = false;

                for (uint8_t j = 2; j < 8; j++) {
                        if (current_state.bInfo[i] == prevState.bInfo[j] && current_state.bInfo[i] != 1)
                                down = true;
                        if (current_state.bInfo[j] == prevState.bInfo[i] && prevState.bInfo[i] != 1)
                                up = true;
                }
                if (!down) {
                        HandleLockingKeys(dev_addr, instance, current_state.bInfo[i]); 
                        // This if statement is handling locking/unlocking the caps lock key
                        if (current_state.bInfo[i] == UHS_HID_BOOT_KEY_CAPS_LOCK) {
                                if (kbdLockingKeys.kbdLeds.bmCapsLock == 1) {
                                        OnKeyDown(current_state.bInfo[0], current_state.bInfo[i]);
                                }
                                else {
                                        OnKeyUp(current_state.bInfo[0], current_state.bInfo[i]);
                                }
                        }
                        else {
                                OnKeyDown(current_state.bInfo[0], current_state.bInfo[i]);
                        }
                }
                if (!up) {
                        // Ignore key up on caps lock
                        if (prevState.bInfo[i] != UHS_HID_BOOT_KEY_CAPS_LOCK) {
                                OnKeyUp(current_state.bInfo[0], prevState.bInfo[i]);
                        }
                }
        }
        for (uint8_t i = 0; i < 8; i++)
                prevState.bInfo[i] = current_state.bInfo[i];

}
bool tuh_hid_set_report(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, void* report, uint16_t len);

void KeyboardReportParser::SetUSBkeyboardLEDs(bool capslock, bool numlock, bool scrolllock){
        // Send LEDs statuses to USB keyboard
        kbdLockingKeys.kbdLeds.bmCapsLock = capslock ? 1 : 0;
        kbdLockingKeys.kbdLeds.bmNumLock = numlock ? 1 : 0;
        kbdLockingKeys.kbdLeds.bmScrollLock = scrolllock ? 1 : 0;
        usb_set_leds = true;

}

void KeyboardReportParser::ChangeUSBKeyboardLEDs(void)
{
        if (usb_set_leds == false) 
                return;

        static size_t i = 0;
        static uint8_t usb_kbd_leds = 0;
        // USB HID Keyboard LED bit location 0x1 - numlock, 0x2 - capslock, 0x4 - scrollock
        usb_kbd_leds = kbdLockingKeys.kbdLeds.bmNumLock ? 0x1 : 0;
        usb_kbd_leds |=  kbdLockingKeys.kbdLeds.bmCapsLock ? 0x2 : 0;
        usb_kbd_leds |= kbdLockingKeys.kbdLeds.bmScrollLock ? 0x4 : 0;

        if (keyboards_list[i].in_use) {
                if (!tuh_hid_set_report(
                        keyboards_list[i].device_addr, 
                        keyboards_list[i].instance,  
                        0, 
                        HID_REPORT_TYPE_OUTPUT, 
                        &(usb_kbd_leds), 
                        sizeof(usb_kbd_leds)
                )) {
                     printf("KBD: tuh_hid_set_report failed, dev addr: %hhx, instance: %hhx\n",
                        keyboards_list[i].device_addr, 
                        keyboards_list[i].instance);   
                }        
        }
        i++;

        if (i >= MAX_KEYBOARDS) {
                usb_set_leds = false;
                i = 0;
        }
}

const uint8_t KeyboardReportParser::numKeys[10]  = {'!', '@', '#', '$', '%', '^', '&', '*', '(', ')'};
const uint8_t KeyboardReportParser::symKeysUp[12]  = {'_', '+', '{', '}', '|', '~', ':', '"', '~', '<', '>', '?'};
const uint8_t KeyboardReportParser::symKeysLo[12]  = {'-', '=', '[', ']', '\\', ' ', ';', '\'', '`', ',', '.', '/'};
const uint8_t KeyboardReportParser::padKeys[5]  = {'/', '*', '-', '+', '\r'};

uint8_t KeyboardReportParser::OemToAscii(uint8_t mod, uint8_t key) {
        uint8_t shift = (mod & 0x22);

        // [a-z]
        if (VALUE_WITHIN(key, 0x04, 0x1d)) {
                // Upper case letters
                if ((kbdLockingKeys.kbdLeds.bmCapsLock == 0 && shift) ||
                        (kbdLockingKeys.kbdLeds.bmCapsLock == 1 && shift == 0))
                        return (key - 4 + 'A');

                        // Lower case letters
                else
                        return (key - 4 + 'a');
        }// Numbers
        else if (VALUE_WITHIN(key, 0x1e, 0x27)) {
                if (shift)
                        return ((uint8_t)pgm_read_byte(&getNumKeys()[key - 0x1e])); 
                else
                        return ((key == UHS_HID_BOOT_KEY_ZERO) ? '0' : key - 0x1e + '1');
        }// Keypad Numbers
        else if(VALUE_WITHIN(key, 0x59, 0x61)) {
                if(kbdLockingKeys.kbdLeds.bmNumLock == 1)
                        return (key - 0x59 + '1');
        } else if(VALUE_WITHIN(key, 0x2d, 0x38))
                return  ((shift) ? (uint8_t)pgm_read_byte(&getSymKeysUp()[key - 0x2d]) : (uint8_t)pgm_read_byte(&getSymKeysLo()[key - 0x2d]));
        else if(VALUE_WITHIN(key, 0x54, 0x58))
                return (uint8_t)pgm_read_byte(&getPadKeys()[key - 0x54]);
        else {
                switch(key) {
                        case UHS_HID_BOOT_KEY_SPACE: return (0x20);
                        case UHS_HID_BOOT_KEY_ENTER: return ('\r'); // Carriage return (0x0D)
                        case UHS_HID_BOOT_KEY_ZERO2: return ((kbdLockingKeys.kbdLeds.bmNumLock == 1) ? '0': 0);
                        case UHS_HID_BOOT_KEY_PERIOD: return ((kbdLockingKeys.kbdLeds.bmNumLock == 1) ? '.': 0);
                }
        }
        return ( 0);
}
