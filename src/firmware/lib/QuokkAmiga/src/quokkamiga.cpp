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



using amiga_keyboard::AmigaKeyboard;

bool global_debug = false;


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
      keyboard.send_key(key);
      free(key);
    }
  }
  return 0;
}