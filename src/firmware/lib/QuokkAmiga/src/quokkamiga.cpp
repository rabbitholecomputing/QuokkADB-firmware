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

#include "quokkamiga.h"

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/bootrom.h"

#include <tusb.h>
#include "printf/printf.h"
#include "quokkamiga_gpio.h"
#include "amigakbdparser.h"
#include "amigakeyboard.h"
#include "flashsettings.h"



using amiga_keyboard::AmigaKeyboard;

bool global_debug = false;
extern FlashSettings setting_storage;

// core1: handle host events
void core1_main() {
  sleep_ms(10);
  tuh_init(0);
  /*------------ Core1 main loop ------------*/
  while (true) {
    tuh_task(); // tinyusb host task
  }
}

AmigaKbdRptParser kbdparser;
AmigaKeyboard keyboard;

int quokkamiga(void)
{
  set_sys_clock_khz(125000, true);

  uart_gpio_init();
  led_gpio_init();
  amiga_kbd_gpio_init();
  setting_storage.init();
  sleep_ms(10);
  
  multicore_reset_core1();
  // all USB task run in core1
  multicore_launch_core1(core1_main);

  led_blink(1);


  /*------------ Core0 main loop ------------*/
  while (true) 
  { 
    AmigaKey* key;
    if (kbdparser.isKeyQueued())
    {
      key = kbdparser.getAmigaKeyCode();
      keyboard.start_send_key(key);
      // if the key is soft reset
      if (key->rotatedKeyCode == (AMIGA_AMIGA_CTRL << 1) && key->isKeyDown) {
        // @TODO implement soft reset
        // if(!keyboard->startSoftRest())
        {
          // hard reset
          amiga_hard_reset();
        }
      }
      else {
        
      }
      free(key);
    }
  }
  return 0;
}

bool amiga_hard_reset()
{
    uint64_t timer_start = time_us_64();
    uint64_t time;
    AmigaKey* reset_key;
    bool reset_keyboard = false;
    keyboard.start_hard_reset();
    // Check if hard reset buttons are held down long enough
    while (true)
    {
      if (kbdparser.isKeyQueued())
      {
          reset_key = kbdparser.getAmigaKeyCode();
          if (reset_key->rotatedKeyCode == (AMIGA_AMIGA_CTRL << 1) && !reset_key->isKeyDown)
          {
            keyboard.stop_hard_reset();
            time = time_us_64();
            if (timer_start - time > AMIGA_HARD_RESET_TIME_US){
              reset_keyboard = true;
            }
            free(reset_key);
            break;
          }
          free(reset_key);
      }
    }
  return reset_keyboard;
}