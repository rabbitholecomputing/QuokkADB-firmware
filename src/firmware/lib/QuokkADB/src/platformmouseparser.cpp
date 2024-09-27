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

#include <algorithm>

#include "platformmouseparser.h"
#include <platform_logmsg.h>
#include <adb_platform.h>
#include "tusb.h"
#include <pico/mutex.h>
#include "flashsettings.h"
#include <usb_hid_keys.h>

#define ENQUEUE_RETRY_TIMEOUT 3000
extern FlashSettings setting_storage;

int8_t PlatformMouseParser::AdjustMovement(int32_t& axis)
{
    // axis is the current sum of all usb movement
    auto sensitivity_divisor = setting_storage.settings()->sensitivity_divisor;

    int32_t scaled = 0;

    // take whatever we can fit into this report (after scaling); leave the
    // rest for the next report. Limits are a signed 7 bit number
    if (axis > 0) {
        scaled = std::min(axis / sensitivity_divisor, 63L);
    } else if (axis < 0) {
        scaled = std::max(axis / sensitivity_divisor, -64L);
    }
    axis -= scaled * sensitivity_divisor;

    return static_cast<int8_t>(scaled & 0x7F);
}

void PlatformMouseParser::Parse(const hid_mouse_report_t *report){
    auto sensitivity_divisor = setting_storage.settings()->sensitivity_divisor;
        
    MOUSEINFO mouse_info = {0};

    if (m_processed)
    {
        m_processed = false;
    }

    mouse_info.bmLeftButton   = !!(report->buttons & MOUSE_BUTTON_LEFT);
    mouse_info.bmRightButton  = !!(report->buttons & MOUSE_BUTTON_RIGHT);
    mouse_info.bmMiddleButton = !!(report->buttons & MOUSE_BUTTON_MIDDLE);
    mouse_info.dX = report->x;
    mouse_info.dY = report->y;

    m_x += mouse_info.dX;
    m_y += mouse_info.dY;

    mouse_info.dWheel = report->wheel;

    auto significant_motion = (m_x >= sensitivity_divisor || m_x <= -sensitivity_divisor || m_y >= sensitivity_divisor || m_y <= -sensitivity_divisor);

    if(mouse_info.dX != 0 || mouse_info.dY != 0) {
        OnMouseMove(&mouse_info);
    }

    bool left_change = false;
    bool right_change = false;
    // change to mouse left button down
    if (!prevState.mouseInfo.bmLeftButton && mouse_info.bmLeftButton) {
        OnLeftButtonDown(&mouse_info);
        left_change = true;
    }

    // change to mouse left button up
    if (prevState.mouseInfo.bmLeftButton && !mouse_info.bmLeftButton) {
        OnLeftButtonUp(&mouse_info);
        left_change = true;
    }

    if (!prevState.mouseInfo.bmRightButton && mouse_info.bmRightButton) {
        OnRightButtonDown(&mouse_info);
        right_change = true;
    }

    if (prevState.mouseInfo.bmRightButton && !mouse_info.bmRightButton) {
        OnRightButtonUp(&mouse_info);
        right_change = true;
    }

    if (!prevState.mouseInfo.bmMiddleButton && mouse_info.bmMiddleButton) {
        OnMiddleButtonDown(&mouse_info);
        setting_storage.settings()->ctrl_lmb ^= 1;
    }

    if (!prevState.mouseInfo.bmMiddleButton && mouse_info.bmMiddleButton) {
        OnMiddleButtonUp(&mouse_info);
    }

    MOUSE_CLICK* click;
    bool ctrl_click = setting_storage.settings()->ctrl_lmb && right_change;
    bool ctrl_click_down = mouse_info.bmRightButton == 1;
    if (left_change || right_change || mouse_info.dWheel != 0)
    {
        click = new MOUSE_CLICK;
        if (right_change)
        {
            if (mouse_info.bmRightButton)
            {
                // right mouse button down
                if (ctrl_click)
                {
                    click->bmLeftButton = true;
                    click->bmMiddleButton = mouse_info.bmMiddleButton;
                    click->bmRightButton = false;
                }
                else
                {
                    click->bmRightButton = true;
                    click->bmMiddleButton = mouse_info.bmMiddleButton;
                    click->bmLeftButton = mouse_info.bmLeftButton;
                }
            }
            else
            {
                // right mouse button up
                if (ctrl_click)
                {
                    click->bmLeftButton = false;
                    click->bmMiddleButton = mouse_info.bmMiddleButton;
                    click->bmRightButton = false;
                }
                else
                {
                    click->bmRightButton = false;
                    click->bmLeftButton = mouse_info.bmLeftButton;
                }
            }
        }

        if (left_change)
        {
            if (!ctrl_click)
                click->bmLeftButton = mouse_info.bmLeftButton;
            click->bmMiddleButton = mouse_info.bmMiddleButton;
            click->bmRightButton = mouse_info.bmRightButton;
        }

        click->dWheel = mouse_info.dWheel;

        uint32_t start = millis();
        bool success = false;
        enum class ctrl_key_logic {WAIT_EMPTY_CLICK_Q, WAIT_EMPTY_KB_Q, CTRL_PRESSED, CTRL_PRESSED_EXECUTED, Q_CLICK,
                                    CLICK_QED, WAIT_TO_RELEASE, CTRL_RELEASED} ctrl_key_state_machine;
        ctrl_key_state_machine = ctrl_key_logic::WAIT_EMPTY_CLICK_Q;
        while((uint32_t)(millis() - start) < ENQUEUE_RETRY_TIMEOUT)
        {
            // Right click to set control + left click down state-machine
            if (ctrl_click && ctrl_click_down && success == false)
            {
                switch (ctrl_key_state_machine)
                {
                    case ctrl_key_logic::WAIT_EMPTY_CLICK_Q :
                        if (m_click_events.isEmpty())
                        {
                            ctrl_key_state_machine = ctrl_key_logic::WAIT_EMPTY_KB_Q;
                            start = millis();
                        }
                        break;
                    case ctrl_key_logic::WAIT_EMPTY_KB_Q :
                        if (!m_keyboard->PendingKeyboardEvent())
                        {
                            m_keyboard->OnKeyDown(0, USB_KEY_LEFTCTRL);
                            ctrl_key_state_machine = ctrl_key_logic::CTRL_PRESSED;
                            start = millis();
                        }
                        break;
                    case ctrl_key_logic::CTRL_PRESSED:
                        if (!m_keyboard->PendingKeyboardEvent())
                        {
                            ctrl_key_state_machine = ctrl_key_logic::CTRL_PRESSED_EXECUTED;
                            start = millis();
                        }
                        break;
                    case ctrl_key_logic::CTRL_PRESSED_EXECUTED:
                        // wait 50ms before clicking left mouse button while CTRL is pressed
                        if ((uint32_t)(millis() - start) > 50)
                        {
                            ctrl_key_state_machine = ctrl_key_logic::Q_CLICK;
                            start = millis();
                        }
                        break;
                    case ctrl_key_logic::Q_CLICK:
                        if (m_click_events.enqueue(click))
                        {
                            ctrl_key_state_machine = ctrl_key_logic::CLICK_QED;
                            start = millis();
                        }
                        break;
                    case ctrl_key_logic::CLICK_QED:
                        if (m_click_events.isEmpty())
                        {
                            ctrl_key_state_machine = ctrl_key_logic::WAIT_TO_RELEASE;
                            start = millis();
                        }
                        break;
                    case ctrl_key_logic::WAIT_TO_RELEASE:
                        // wait 50 ms before release the CTRL key
                        if ((uint32_t)(millis() - start) > 50)
                        {
                            m_keyboard->OnKeyUp(0, USB_KEY_LEFTCTRL);
                            ctrl_key_state_machine = ctrl_key_logic::CTRL_RELEASED;
                            start = millis();
                        }
                        break;
                    case ctrl_key_logic::CTRL_RELEASED:
                        // wait for the key up to be sent to the host
                        if (!m_keyboard->PendingKeyboardEvent())
                            success = true; // exit state-machine
                        break;
                }
            } 
            else  if (m_click_events.enqueue(click))
            {
                // Normal click
                success = true;
            }
            
            if (success == true)
                break;
        }

        if (!success)
        {
            Logmsg.println("Timeout Warning! Unable to enqueue click event after retrying");
        }

    }


    bool axis = 0 == setting_storage.settings()->swap_mouse_wheel_axis;
    int8_t times = setting_storage.settings()->mouse_wheel_count;
    int8_t scroll = mouse_info.dWheel;

    // If `times` is positive, for each scroll movement increment, press the arrow key `times` number of times
    // If `times` is negative, divide the scroll movement by times
    // If `times` is zero or one, use the scroll value as is
    if (times > 1)
    {
        scroll *= times;
    }

    if (times < 0)
    {
        static int16_t acc = 0;
        acc += scroll;
        if (abs(acc / times) > 0)
        {
            scroll = acc / -times;
            acc -= scroll * -times;
        }
        else
            scroll = 0;
    }

    if (scroll > 0)
    {
        for (int dw = 0; dw < scroll; dw++)
        {
                m_keyboard->OnKeyDown(0, axis ? USB_KEY_UP : USB_KEY_DOWN);
                m_keyboard->OnKeyUp(0, axis ? USB_KEY_UP : USB_KEY_DOWN);
        }
    }

    if (scroll < 0)
    {
        for (int dw = 0; dw > scroll; dw--)
        {
            m_keyboard->OnKeyDown(0, axis ? USB_KEY_DOWN : USB_KEY_UP);
            m_keyboard->OnKeyUp(0, axis ? USB_KEY_DOWN : USB_KEY_UP);
        }
    }

    memcpy(prevState.bInfo, &mouse_info, sizeof(prevState.bInfo));
    if (significant_motion) {
        m_ready = true;
    }
}
