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
#include <Arduino.h>
#include "platformkbdparser.h"
#include "usb_hid_keys.h"
#include "platform_config.h"
#include "char2usbkeycode.h"
#include "flashsettings.h"
#include <tusb.h>
#include "platform_logmsg.h"
#include "blink.h"

#define VALUE_WITHIN(v, l, h) (((v) >= (l)) && ((v) <= (h)))
#define pgm_read_byte(addr) (*(const unsigned char *)(addr))

extern uint16_t modifierkeys;
extern bool set_hid_report_ready;
extern FlashSettings setting_storage;
extern bool global_debug;

uint8_t inline findModifierKey(hid_keyboard_report_t const *report, const hid_keyboard_modifier_bm_t mod)
{
    return (mod & report->modifier) ? 1 : 0;
}

PlatformKbdParser::PlatformKbdParser()
{
    kbdLockingKeys.bLeds = 0;
    region = RegionUS;
}
PlatformKbdParser::~PlatformKbdParser()
{
}

void PlatformKbdParser::AddKeyboard(uint8_t dev_addr, uint8_t instance)
{
    for (size_t i = 0; i < MAX_KEYBOARDS; i++)
    {
        if (!keyboards_list[i].in_use)
        {
            keyboards_list[i].in_use = true;
            keyboards_list[i].device_addr = dev_addr;
            keyboards_list[i].instance = instance;
            SetUSBkeyboardLEDs(kbdLockingKeys.kbdLeds.bmCapsLock,
                               kbdLockingKeys.kbdLeds.bmNumLock,
                               kbdLockingKeys.kbdLeds.bmScrollLock);
            break;
        }
    }
}
void PlatformKbdParser::RemoveKeyboard(uint8_t dev_addr, uint8_t instance)
{
    for (size_t i = 0; i < MAX_KEYBOARDS; i++)
    {
        if (keyboards_list[i].in_use && keyboards_list[i].device_addr == dev_addr && keyboards_list[i].instance == instance)
        {
            keyboards_list[i].in_use = false;
            break;
        }
    }
}

void PlatformKbdParser::Parse(uint8_t dev_addr, uint8_t instance, hid_keyboard_report_t const *report)
{
    union
    {
        KBDINFO kbdInfo;
        uint8_t bInfo[sizeof(KBDINFO)];
    } current_state;

    KBDINFO *cur_kbd_info = &(current_state.kbdInfo);

    cur_kbd_info->bmLeftCtrl = findModifierKey(report, KEYBOARD_MODIFIER_LEFTCTRL);
    cur_kbd_info->bmLeftShift = findModifierKey(report, KEYBOARD_MODIFIER_LEFTSHIFT);
    cur_kbd_info->bmRightCtrl = findModifierKey(report, KEYBOARD_MODIFIER_RIGHTCTRL);
    cur_kbd_info->bmRightShift = findModifierKey(report, KEYBOARD_MODIFIER_RIGHTSHIFT);

    if (setting_storage.settings()->swap_modifiers)
    {
        cur_kbd_info->bmLeftGUI = findModifierKey(report, KEYBOARD_MODIFIER_LEFTALT);
        cur_kbd_info->bmLeftAlt = findModifierKey(report, KEYBOARD_MODIFIER_LEFTGUI);
        cur_kbd_info->bmRightGUI = findModifierKey(report, KEYBOARD_MODIFIER_RIGHTGUI);
        cur_kbd_info->bmRightAlt = findModifierKey(report, KEYBOARD_MODIFIER_RIGHTALT);
    }
    else
    {
        cur_kbd_info->bmLeftAlt = findModifierKey(report, KEYBOARD_MODIFIER_LEFTALT);
        cur_kbd_info->bmLeftGUI = findModifierKey(report, KEYBOARD_MODIFIER_LEFTGUI);
        cur_kbd_info->bmRightAlt = findModifierKey(report, KEYBOARD_MODIFIER_RIGHTALT);
        cur_kbd_info->bmRightGUI = findModifierKey(report, KEYBOARD_MODIFIER_RIGHTGUI);
    }
    memcpy(cur_kbd_info->Keys, report->keycode, 6);
    cur_kbd_info->bReserved = report->reserved;

    if (PlatformKbdParser::SpecialKeyCombo(cur_kbd_info))
    {
        return;
    }

    // provide event for changed control key state
    if (prevState.bInfo[0x00] != current_state.bInfo[0x00])
    {
        OnControlKeysChanged(prevState.bInfo[0x00], current_state.bInfo[0x00]);
    }

    for (uint8_t i = 2; i < 8; i++)
    {
        bool down = false;
        bool up = false;

        for (uint8_t j = 2; j < 8; j++)
        {
            if (current_state.bInfo[i] == prevState.bInfo[j] && current_state.bInfo[i] != 1)
                down = true;
            if (current_state.bInfo[j] == prevState.bInfo[i] && prevState.bInfo[i] != 1)
                up = true;
        }
        if (!down)
        {
            HandleLockingKeys(dev_addr, instance, current_state.bInfo[i]);
            OnKeyDown(current_state.bInfo[0], current_state.bInfo[i]);
        }
        if (!up)
        {
            // Ignore key up on caps lock
            if (prevState.bInfo[i] != UHS_HID_BOOT_KEY_CAPS_LOCK)
            {
                OnKeyUp(current_state.bInfo[0], prevState.bInfo[i]);
            }
        }
    }

    // store current buttons to test against next keyboard action
    for (uint8_t i = 0; i < 8; i++)
        prevState.bInfo[i] = current_state.bInfo[i];
}
// bool tuh_hid_set_report(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, void* report, uint16_t len);

void PlatformKbdParser::SetUSBkeyboardLEDs(bool capslock, bool numlock, bool scrollock)
{
    // Send LEDs statuses to USB keyboard
    kbdLockingKeys.kbdLeds.bmCapsLock = capslock ? 1 : 0;
    kbdLockingKeys.kbdLeds.bmNumLock = numlock ? 1 : 0;
    kbdLockingKeys.kbdLeds.bmScrollLock = scrollock ? 1 : 0;
    usb_set_leds = true;
}


static const char ON_STRING[] = "On";
static const char OFF_STRING[] = "Off";
static const char REGION_US_STRING[] = "USA";
static const char REGION_FR_STRING[] = "Belgium/France";
static const char REGION_DE_STRING[] = "German";
static const char REGION_CH_STRING[] = "Swiss-DE/FR";
static const char REGION_DK_STRING[] = "Denmark";
static const char REGION_UK_STRING[] = "Ireland/UK";
static const char REGION_IT_QZ_STRING[] = "Italy-QZERTY";
static const char REGION_IT_QW_STRING[] = "Italy-QWERTY";

static void region_selection_string(char* print_buf, size_t len, Region region)
{
    snprintf(print_buf, len,
        "Regions:\n"
        " %c %s\n"
        " %c %s\n"
        " %c %s\n"
        " %c %s\n"
        " %c %s\n"
        " %c %s\n"
        " %c %s\n"
        " %c %s\n",
        region == RegionUS ? '*' : '-', REGION_US_STRING,
        region == RegionFR ? '*' : '-', REGION_FR_STRING,
        region == RegionDE ? '*' : '-', REGION_DE_STRING,
        region == RegionCH ? '*' : '-', REGION_CH_STRING,
        region == RegionDK ? '*' : '-', REGION_DK_STRING,
        region == RegionUK ? '*' : '-', REGION_UK_STRING,
        region == RegionITqz ? '*' : '-', REGION_IT_QZ_STRING,
        region == RegionITqw ? '*' : '-', REGION_IT_QW_STRING

    );
}

bool PlatformKbdParser::SpecialKeyCombo(KBDINFO *cur_kbd_info)
{
    // Special keycombo actions
    uint8_t special_key_count = 0;
    uint8_t special_key = 0;
    uint8_t special_keys[] = {USB_KEY_V, USB_KEY_P, USB_KEY_H, USB_KEY_G, USB_KEY_S, USB_KEY_R, USB_KEY_T,
                              USB_KEY_K, USB_KEY_L, USB_KEY_KPPLUS, USB_KEY_EQUAL, USB_KEY_SLASH, USB_KEY_1, USB_KEY_RIGHTBRACE,
                              USB_KEY_KPMINUS, USB_KEY_MINUS, USB_KEY_C, USB_KEY_D, USB_KEY_U,
                              };
    uint8_t caps_lock_down = false;
    int16_t region_num;
    char print_buf[1536];


    for (uint8_t i = 0; i < 6; i++)
    {
        if (cur_kbd_info->Keys[i] == USB_KEY_CAPSLOCK)
        {
            caps_lock_down = true;
        }
        for (size_t j = 0; j < sizeof(special_keys); j++)
        {
            if (special_keys[j] == cur_kbd_info->Keys[i])
            {
                special_key_count++;
                special_key = cur_kbd_info->Keys[i];
            }
        }
    }

    bool enter_shortcut1 = caps_lock_down &&
            (cur_kbd_info->bmLeftCtrl  || cur_kbd_info->bmRightCtrl) &&
            (cur_kbd_info->bmLeftShift   || cur_kbd_info->bmRightShift);

    bool enter_shortcut2 =
             (cur_kbd_info->bmLeftCtrl  || cur_kbd_info->bmRightCtrl) &&
             (cur_kbd_info->bmLeftGUI || cur_kbd_info->bmRightGUI)&&
             (cur_kbd_info->bmLeftAlt   || cur_kbd_info->bmRightAlt);


    if (special_key_count == 1 && (enter_shortcut1 || enter_shortcut2))
    {
        
        if (special_key == USB_KEY_V)
            SendString(PLATFORM_FW_VER_STRING);
        else if (special_key == USB_KEY_P)
        {
            snprintf(print_buf, sizeof(print_buf),
                    "Current Settings\n"
                    "================\n"
                    "Command <-> Option key swap: %s\n"
                    "Layout: %s\n"
                    "LED: %s\n"
                    "Mouse Sensitivity Divisor: %u\n"
                    "(higher = less sensitive)\n"
                    "Right Mouse Button: %s\n"
                    "Mouse wheel count: %d\n"
                    "Flip mouse wheel axis: %s\n"
                    "\n"
                    "Special Keys = CAPS + Ctrl + Shift + (Key)\n"
                    "Alternate Keys = Ctrl + Cmd + Option + (Key)\n"
                    "------------------------------------------\n"
                    "(V): print firmware version\n"
                    "(P): print current settings (this message)\n"
                    "(H): select next region layout\n"
                    "(G): select previous region layout\n"
                    "(S): save settings to flash - LED blinks %d times\n"
                    "(R): remove settings from flash - LED blinks %d times\n"
                    "(K): swap option and command key positions - LED blinks thrice\n"
                    "(L): toggle status LED On/Off\n"
                    "(+): increase sensitivity - LED blinks twice\n"
                    "(-): decrease sensitivity - LED blink once\n"
                    "(T): swap right mouse button (RMB) function between Ctrl + LMB and ADB RMB\n"
                    "Blinks twice for Ctrl + LMB and blinks once for ADB RMB\n"
                    "Note: In MacOS 8 and 9 you will want Ctrl + LMB - ADB RMB might work in NeXTSTEP\n"
                    "\n"
                    "Change mouse wheel count 'x' by one with 'C' or 'D'\n"
                    "If positive press the up/down arrow 'x' times for each wheel movement\n"
                    "If negative divide the mouse wheel movement by 'abs(x)'\n"
                    "(D): increase the mouse wheel count - LED blinks twice\n"
                    "(C): decrease the mouse wheel count - LED blink once\n"
                    "(U): flip mouse wheel axis - LED blinks thrice\n"
                    "Note: not all mice support the mouse wheel in HID boot protocol\n"
                    ,
                    setting_storage.settings()->swap_modifiers ? ON_STRING : OFF_STRING,
                    region == RegionFR   ? REGION_FR_STRING :
                    region == RegionDE   ? REGION_DE_STRING :
                    region == RegionCH   ? REGION_CH_STRING :
                    region == RegionDK   ? REGION_DK_STRING :
                    region == RegionUK   ? REGION_UK_STRING :
                    region == RegionITqz ? REGION_IT_QZ_STRING :
                    region == RegionITqw ? REGION_IT_QW_STRING :
                                           REGION_US_STRING,
                    setting_storage.settings()->led_on ? ON_STRING : OFF_STRING,
                    setting_storage.settings()->sensitivity_divisor,
                    setting_storage.settings()->ctrl_lmb ? "Ctrl+LBM" : "ADB RMB",
                    setting_storage.settings()->mouse_wheel_count,
                    setting_storage.settings()->swap_mouse_wheel_axis ? ON_STRING : OFF_STRING,
                    SAVE_TO_FLASH_BLINK_COUNT,
                    CLEAR_FLASH_BLINK_COUNT);
            SendString(print_buf);
        }
        else if (special_key == USB_KEY_S)
        {
            setting_storage.save();
            blink_led.blink(SAVE_TO_FLASH_BLINK_COUNT);
        }
        else if (special_key == USB_KEY_R)
        {
            setting_storage.clear();
            blink_led.blink(CLEAR_FLASH_BLINK_COUNT);
        }
        else if (special_key == USB_KEY_K)
        {
            setting_storage.settings()->swap_modifiers ^= 1;
            blink_led.blink(3);
        }
        else if (special_key == USB_KEY_L)
            setting_storage.settings()->led_on ^= 1;
        else if (  ((region == RegionUS || region == RegionUK) 
                        && (special_key == USB_KEY_KPPLUS || special_key == USB_KEY_EQUAL))
                || (region == RegionFR && (special_key == USB_KEY_KPPLUS || special_key == USB_KEY_SLASH))
                || (region == RegionDE && (special_key == USB_KEY_KPPLUS || special_key == USB_KEY_RIGHTBRACE))
                || (region == RegionCH && (special_key == USB_KEY_KPPLUS || special_key == USB_KEY_1))
                || (region == RegionDK && (special_key == USB_KEY_KPPLUS || special_key == USB_KEY_MINUS))
        )
        {
            if (setting_storage.settings()->sensitivity_divisor <= 1)
                setting_storage.settings()->sensitivity_divisor = 1;
            else
                setting_storage.settings()->sensitivity_divisor--;
            blink_led.blink(2);
        }
        else if (  ((region == RegionUS || region == RegionUK)
                        && (special_key == USB_KEY_KPMINUS || special_key == USB_KEY_MINUS))
                || (region == RegionFR && (special_key == USB_KEY_KPMINUS || special_key == USB_KEY_EQUAL))
                || ((region == RegionDE || region == RegionCH || region == RegionDK)
                        && (special_key == USB_KEY_KPMINUS || special_key == USB_KEY_SLASH))
        )
        {
            if (setting_storage.settings()->sensitivity_divisor >= 16)
                setting_storage.settings()->sensitivity_divisor = 16;
            else
                setting_storage.settings()->sensitivity_divisor++;
            blink_led.blink(1);
        }
        else if (special_key == USB_KEY_H)
        {
            region_num = setting_storage.settings()->region;
            region_num++;
            if (region_num > LAST_REGION)
                region_num = 0;

            setting_storage.settings()->region = region_num;
            region = (Region) region_num;
            region_selection_string(print_buf, sizeof(print_buf), region);
            SendString(print_buf);
        }
        else if (special_key == USB_KEY_G)
        {
            region_num = setting_storage.settings()->region;
            region_num--;
            if (region_num < 0)
                region_num = LAST_REGION;

            setting_storage.settings()->region = region_num;
            region = (Region) region_num;
            region_selection_string(print_buf, sizeof(print_buf), region);
            SendString(print_buf);
        }
        else if (special_key == USB_KEY_T)
        {
            setting_storage.settings()->ctrl_lmb ^= 1;
            if (setting_storage.settings()->ctrl_lmb == 1)
                blink_led.blink(2);
            else
                blink_led.blink(1);
        }
        else if (special_key == USB_KEY_C)
        {
            if (setting_storage.settings()->mouse_wheel_count <= -8)
            {
                setting_storage.settings()->mouse_wheel_count = -8;
            }
            else
            {
                setting_storage.settings()->mouse_wheel_count--;
                blink_led.blink(1);
            }
        }
        else if (special_key == USB_KEY_D)
        {
            if (setting_storage.settings()->mouse_wheel_count >= 10)
            {
                setting_storage.settings()->mouse_wheel_count = 10;
            }
            else
            {
                setting_storage.settings()->mouse_wheel_count++;
                blink_led.blink(2);
            }
        }
        else if (special_key == USB_KEY_U)
        {
            setting_storage.settings()->swap_mouse_wheel_axis ^= 1;
            blink_led.blink(3);
        }
        else
            return false;
        return true;
    }
    return false;
}

void PlatformKbdParser::SendString(const char *message)
{
    int i = 0;
    usbkey_t key;

    // force key up on modifier keys
    while (PendingKeyboardEvent());
    OnKeyUp(0, USB_KEY_LEFTSHIFT);
    OnKeyUp(0, USB_KEY_RIGHTSHIFT);
    OnKeyUp(0, USB_KEY_LEFTCTRL);
    OnKeyUp(0, USB_KEY_RIGHTCTRL);
    OnKeyUp(0, USB_KEY_LEFTALT);
    OnKeyUp(0, USB_KEY_RIGHTALT);
    OnKeyUp(0, USB_KEY_CAPSLOCK);
    OnKeyUp(0, USB_KEY_LEFTMETA);
    OnKeyUp(0, USB_KEY_RIGHTMETA);
    if (m_keyboard_events.enqueue(new KeyEvent(USB_KEY_CAPSLOCK, KeyEvent::KeyUp, 0)))
    {
        // as HandleLocking keys simply toggles the keyboard LEDs, setting it to 1
        // forces it to toggle off.
        kbdLockingKeys.kbdLeds.bmCapsLock = 0;
        SetUSBkeyboardLEDs(kbdLockingKeys.kbdLeds.bmCapsLock, kbdLockingKeys.kbdLeds.bmNumLock, kbdLockingKeys.kbdLeds.bmScrollLock);
    }
    else
    {
        if (global_debug)
        {
            Logmsg.println("Warning! unable to queue CAPSLOCK key up");
        }
    }

    while (message[i] != '\0')
    {
        while (PendingKeyboardEvent())
            ;

        key = char_to_usb_keycode(message[i++], region);

        if (key.shift_down)
        {
            OnKeyDown(0, USB_KEY_LEFTSHIFT);
        }

        OnKeyDown(0, key.keycode);

        OnKeyUp(0, key.keycode);

        if (key.shift_down)
        {
            OnKeyUp(0, USB_KEY_LEFTSHIFT);
        }
    }
}

void PlatformKbdParser::ChangeUSBKeyboardLEDs(void)
{
    if (usb_set_leds == false)
        return;

    static size_t i = 0;
    static uint8_t usb_kbd_leds = 0;
    // USB HID Keyboard LED bit location 0x1 - numlock, 0x2 - capslock, 0x4 - scrollock
    usb_kbd_leds = kbdLockingKeys.kbdLeds.bmNumLock ? 0x1 : 0;
    usb_kbd_leds |= kbdLockingKeys.kbdLeds.bmCapsLock ? 0x2 : 0;
    usb_kbd_leds |= kbdLockingKeys.kbdLeds.bmScrollLock ? 0x4 : 0;

    bool try_again = true;
    if (set_hid_report_ready && keyboards_list[i].in_use)
    {
        set_hid_report_ready = false;
        try_again = false;
        if (!tuh_hid_set_report(
                keyboards_list[i].device_addr,
                keyboards_list[i].instance,
                0,
                HID_REPORT_TYPE_OUTPUT,
                &(usb_kbd_leds),
                sizeof(usb_kbd_leds)))
        {
            set_hid_report_ready = true;
            printf("KBD: tuh_hid_set_report failed, dev addr: %hhx, instance: %hhx\n",
                   keyboards_list[i].device_addr,
                   keyboards_list[i].instance);
        }
    }

    if (!keyboards_list[i].in_use || !try_again)
    {
        i++;
    }

    if (i >= MAX_KEYBOARDS)
    {
        usb_set_leds = false;
        i = 0;
    }
}

const uint8_t PlatformKbdParser::numKeys[10] = {'!', '@', '#', '$', '%', '^', '&', '*', '(', ')'};
const uint8_t PlatformKbdParser::symKeysUp[12] = {'_', '+', '{', '}', '|', '~', ':', '"', '~', '<', '>', '?'};
const uint8_t PlatformKbdParser::symKeysLo[12] = {'-', '=', '[', ']', '\\', ' ', ';', '\'', '`', ',', '.', '/'};
const uint8_t PlatformKbdParser::padKeys[5] = {'/', '*', '-', '+', '\r'};

uint8_t PlatformKbdParser::OemToAscii(uint8_t mod, uint8_t key)
{
    uint8_t shift = (mod & 0x22);

    // [a-z]
    if (VALUE_WITHIN(key, 0x04, 0x1d))
    {
        // Upper case letters
        if ((kbdLockingKeys.kbdLeds.bmCapsLock == 0 && shift) ||
            (kbdLockingKeys.kbdLeds.bmCapsLock == 1 && shift == 0))
            return (key - 4 + 'A');

        // Lower case letters
        else
            return (key - 4 + 'a');
    } // Numbers
    else if (VALUE_WITHIN(key, 0x1e, 0x27))
    {
        if (shift)
            return ((uint8_t)pgm_read_byte(&getNumKeys()[key - 0x1e]));
        else
            return ((key == UHS_HID_BOOT_KEY_ZERO) ? '0' : key - 0x1e + '1');
    } // Keypad Numbers
    else if (VALUE_WITHIN(key, 0x59, 0x61))
    {
        if (kbdLockingKeys.kbdLeds.bmNumLock == 1)
            return (key - 0x59 + '1');
    }
    else if (VALUE_WITHIN(key, 0x2d, 0x38))
        return ((shift) ? (uint8_t)pgm_read_byte(&getSymKeysUp()[key - 0x2d]) : (uint8_t)pgm_read_byte(&getSymKeysLo()[key - 0x2d]));
    else if (VALUE_WITHIN(key, 0x54, 0x58))
        return (uint8_t)pgm_read_byte(&getPadKeys()[key - 0x54]);
    else
    {
        switch (key)
        {
        case UHS_HID_BOOT_KEY_SPACE:
            return (0x20);
        case UHS_HID_BOOT_KEY_ENTER:
            return ('\r'); // Carriage return (0x0D)
        case UHS_HID_BOOT_KEY_ZERO2:
            return ((kbdLockingKeys.kbdLeds.bmNumLock == 1) ? '0' : 0);
        case UHS_HID_BOOT_KEY_PERIOD:
            return ((kbdLockingKeys.kbdLeds.bmNumLock == 1) ? '.' : 0);
        }
    }
    return (0);
}