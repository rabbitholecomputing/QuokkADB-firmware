//---------------------------------------------------------------------------
//
//	ADBuino & QuokkaADB ADB keyboard and mouse adapter
//
//     Copyright 2011 Jun WAKO <wakojun@gmail.com>
//     Copyright 2013 Shay Green <gblargg@gmail.com>
//	   Copyright (C) 2017 bbraun
//	   Copyright (C) 2020 difegue
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
//  with the file. If not, see <https://www.gnu.org/licenses/>.
//
//  Portions of this code were originally released under a Modified BSD 
//  License. See LICENSE in the root of this repository for more info.
//
//----------------------------------------------------------------------------
#include <Arduino.h>
#include "adb.h"
#include "bithacks.h"
#include "math.h"
#include <platform_logmsg.h>
#include <log_cache.h>
#include "adbkeytostring.h"

uint8_t mouse_addr = MOUSE_DEFAULT_ADDR;
uint8_t kbd_addr = KBD_DEFAULT_ADDR;
uint8_t mouse_handler_id = MOUSE_DEFAULT_HANDLER_ID;
uint8_t kbd_handler_id = KBD_DEFAULT_HANDLER_ID;
uint8_t mousepending = 0;
uint8_t kbdpending = 0;
uint8_t kbdskip = 0;
uint16_t kbdprev0 = 0;
uint16_t mousereg0 = 0;
uint16_t kbdreg0 = 0;
uint16_t kbdreg2 = 0xFFFF;
uint8_t kbdsrq = 0;
uint8_t mousesrq = 0;
uint16_t modifierkeys = 0xFFFF;
uint64_t kbskiptimer = 0;
bool adb_reset = false;
volatile bool adb_collision = false; 
volatile bool collision_detection = false;
bool mouse_skip_next_listen_reg3 = false;
bool kbd_skip_next_listen_reg3 = false;

extern bool global_debug;
// The original data_lo code would just set the bit as an output
// That works for a host, since the host is doing the pullup on the ADB line,
// but for a device, it won't reliably pull the line low.  We need to actually
// set it.


inline bool AdbInterface::place_bit0(void)
{
  data_lo();
  adb_delay_us(65);
  data_hi();
  return adb_delay_us(35);
}
inline bool AdbInterface::place_bit1(void)
{
  data_lo();
  adb_delay_us(35);
  data_hi();
  return adb_delay_us(65);
}
inline bool AdbInterface::send_byte(uint8_t data)
{
  for (int i = 0; i < 8; i++)
  {
    if (data & (0x80 >> i))
    {
      if(!place_bit1()) return false;
    }
    else
    {
      if(!place_bit0()) return false;
    }
  }
  return true;
}
int16_t AdbInterface::ReceiveCommand(uint8_t srq)
{
  uint8_t bits; 
  uint16_t lo, hi;
  int16_t data = 0;
  // find attention & start bit
  hi = wait_data_lo(ADB_START_BIT_DELAY); 
  if (!hi)
    return -1;
  do 
  {
    lo = wait_data_hi(4000);
    if (!lo || lo > 820 || lo < 780)
    {
      if (lo > 2950) 
      {
        adb_reset = true;
        logmsg("ALL: Global reset detected");
        if (global_debug)
        {
          Logmsg.print("ALL: Global reset detected, wait time was ");
          Logmsg.print(lo, fmtDEC);
          Logmsg.println("us");
        }
        return -100;
      }
      else {
        if (global_debug)
        {
          Logmsg.print("ALL: Error in attention low time,  wait time was ");
          Logmsg.print(lo, fmtDEC);
          Logmsg.println("us");

        }
      }
      return -1;

    }
    else
    {
      // Attention was ~800us like it should be
      break;
    }
  }
  while(true);

  hi = wait_data_lo(100);
  if (!hi && hi > 70 && hi < 40)
  {
    if (global_debug)
    {
      Logmsg.print("Start bit not found, wait time was ");
      Logmsg.print(hi, fmtDEC);
      Logmsg.println("us");
    }
    return -3;
  }

  for (bits = 0; bits < 8; bits++)
  {
    lo = wait_data_hi(130);
    if (!lo)
    {
      goto out;
    }
    hi = wait_data_lo(100);
    if (!hi)
    {
      goto out;
    }
    if (120 < lo + hi )
    {
      goto out;
    }

    data <<= 1;
    if (lo < 40)
    {
      data |= 1;
    }
  }

  // Stop bit normal low time is 70uS + can have an SRQ time of 300uS
  hi = wait_data_hi(400);
  if (hi > 0 && hi <= 80)
  {
    logmsg("Receiving CMD: ", (uint8_t) data ," -- No SRQ");
  }
  else if (hi <= 380)
  {
    logmsg("Receiving CMD: ", (uint8_t) data ," -- SRQ");
  }
  else 
  {
    logmsg("Receiving CMD: ", (uint8_t) data ,"-- SRQ/StopBit timeout");
  }

  return data;
out:
  logmsg("-- Error reading CMD bits");
  if (global_debug)
  {
    Logmsg.print("ALL: Error reading CMD bits, low time ");
    Logmsg.print(lo, fmtDEC);
    Logmsg.print(", high time ");
    Logmsg.print(hi, fmtDEC);
    Logmsg.print(" at bit ");
    Logmsg.println(bits, fmtHEX);
  }
  return -4;
}


void AdbInterface::ProcessCommand(int16_t cmd)
{
  uint8_t  listen_addr, listen_handler_id;
  uint16_t mousereg3, kbdreg3;
  int32_t listen_register;

  if (cmd < 0)
  {
    // -1 is waiting for a signal
    // -100 is a 3ms ADB reset
    if (-1 == cmd || -100 == cmd)
    {
      if (cmd == -100) 
      {
        logmsg("ALL: Global 3ms reset signal ---------------------------------------------------------");
      }
      return;
    }

    if (global_debug)
    {
      Logmsg.print("ALL: CMD code error, cmd: ");
      Logmsg.println(cmd, fmtHEX);
    }
    return;
  }

  if ((0x0F & cmd) == 0x00) 
  {
    adb_reset = true;
    logmsg("ALL: Cmd for reset all devices");
    if (global_debug)
    {
      Logmsg.println("ALL: Cmd for reset all devices");
    }
    return;
  }
  // see if it is addressed to us
  uint8_t address_nibble =  (cmd >> 4) & 0x0F;
  uint8_t command_nibble = cmd & 0x0F; 
 
  adb_cmd_t command_type;

  const char* info_string;

  const char info_reset[] = "Reset";
  const char info_flush[] = "Flush";
  const char info_listen[] = "Listen";
  const char info_talk[] = "Talk";

  const char *info_reg_string; 
  const char *reg_strings[] = {"Reg 0", "Reg 1", "Reg 2", "Reg 3"};
  
  if (command_nibble == 0)
  {
    command_type = adb_cmd_t::reset;
    info_string = info_reset;
  }
  else if (command_nibble == 1)
  {
    command_type = adb_cmd_t::flush;
    info_string = info_flush;
  }
  else if (0x2 == ((command_nibble >> 2) & 0x3))
  {
    command_type = adb_cmd_t::listen;
    info_string = info_listen;
    info_reg_string = reg_strings[cmd & 0x3];
  }
  else if (0x3 == ((command_nibble >> 2) & 0x3))
  {
    command_type = adb_cmd_t::talk;
    info_string = info_talk;
    info_reg_string = reg_strings[cmd & 0x3];
  }
  else
  {
    command_type = adb_cmd_t::illegal;
    logmsg("-- ERROR: Address: ", address_nibble, " [illegal cmd] raw cmd: ", command_nibble );
    return;
  }

  if (command_type == adb_cmd_t::listen || command_type == adb_cmd_t::talk)
  {
    logmsg("-- Address: ", address_nibble, " ", info_string, " ", info_reg_string ," raw cmd: ", command_nibble );
  }
  else
  {
    logmsg("-- Address: ", address_nibble, " ",  info_string, " raw cmd: ", command_nibble);
  }

  bool left_btn, right_btn;
  int8_t y_mv, x_mv;
  bool key1up, key2up;
  uint8_t key1code, key2code;
  bool srq_enabled;
  uint8_t unused_address , handler_id;
  int32_t talk_register;
  switch (command_nibble)
  {
  case 0x1:
    logmsg("-- Got FLUSH request");
    break;
  case 0x8:
    Logmsg.println("MOUSE: Got LISTEN request for register 0");
    break;
  case 0x9:
    Logmsg.println("MOUSE: Got LISTEN request for register 1");
    break;
  case 0xA:
    Logmsg.println("MOUSE: Got LISTEN request for register 2");
    break;
  case 0xB:
    listen_register = Receive16bitRegister();
    logmsg("-- Register 3 data: ", (uint16_t) listen_register);

    if (global_debug)
    {
      Logmsg.print("MOUSE: Got LISTEN request for register 3 at address 0x");
      Logmsg.println( mouse_addr, fmtHEX);
    }

    if (listen_register >= 0)
    {
      listen_addr = (listen_register >> 8) & 0x0F;
      listen_handler_id = listen_register & 0xFF;
      logmsg("-- Listen addr: ", listen_addr, " handler id: ", listen_handler_id);
      if (global_debug)
      {
        Logmsg.print("MOUSE: Listen Register 3 value is 0x");
        Logmsg.println(listen_register, fmtHEX);
      }
      // self-test
      if (0xFF == listen_handler_id)
      {
        logmsg("-- Self Test, handler id 0xFF");
        break;
      }
      // Change of address 
      if (0xFE == listen_handler_id )
      {
        logmsg("-- Change of address: ",address_nibble,", new address: ", listen_addr, " move handler id 0xFE");
        if (mouse_skip_next_listen_reg3)
        {
          mouse_skip_next_listen_reg3 = false;
          logmsg("-- talk reg 3 had a collision at address: ", address_nibble);
          if (global_debug)
          {
            Logmsg.print("MOUSE: TALK reg 3 had a collision at 0x");
            Logmsg.println(mouse_addr, fmtHEX);
          }
          break;
        }
      }
      else
      {
        
        // Don't change address for mouse, handler id values can be 1, 2, or 4.
        //   1 - standard mouse 
        //   2 - standard mouse with extra sensitivity
        //   4 - extended mouse 
        /*  Don't change mouse type from handler id 1 (default)
        if (listen_handler_id == 1 || listen_handler_id == 2)
        {
          mouse_handler_id  = listen_handler_id;
        }
        */
        if (global_debug)
        {
          Logmsg.print("MOUSE: LSTN Reg3 val is 0x");
          Logmsg.print(listen_register, fmtHEX);
          Logmsg.print("@0x");
          Logmsg.println(mouse_addr, fmtHEX);
          
          Logmsg.print("MOUSE: handler id change to  0x");
          Logmsg.println( mouse_handler_id, fmtHEX);
        }
      }
    } 
    else
    {
      logmsg("-- Listen Register 3 errored with code ", listen_register);

      if (global_debug)
      {
        
        Logmsg.print("MOUSE: Listen Register 3 errored with code ");
        Logmsg.println( listen_register, fmtDEC);
      }
    }
    break;
  case 0xC: // talk register 0
    talk_register = Snoop16bitRegister();
    if (talk_register < 0)
    {
      if (talk_register == -100)
        logmsg(" -- No Data");
      else
        logmsg("-- Error: talk register 0 error num, ", talk_register);
      break;
    }
    logmsg("-- Talk register 0, register value: ", (uint16_t) talk_register);
    left_btn = !((1<<15) & talk_register);
    right_btn = !((1<<7) & talk_register);
    y_mv = (talk_register >> 8) & 0x7F;
    if (y_mv & (0x40))
    {
      // extend 7bit negative number to 8bit negative number
      y_mv |= 0x80; 
    }
    x_mv = (talk_register) & 0x7F;
    if (x_mv & (0x40))
    {
      // extend 7bit negative number to 8bit negative number
      x_mv |= 0x80; 
    }
    logmsg("-- If mouse LB: ", left_btn, " RB: ", right_btn, " y: ", y_mv, " x: ", x_mv );
    key1up = (talk_register >> 15) & 0x1;
    key2up = (talk_register >> 7) & 0x1;
    key1code = (talk_register >> 8 ) & 0x7F;
    key2code = talk_register & 0x7F;
    if (key1code == 0x7F && key2code == 0x7F)
    {
      logmsg("-- If keyboard, power button key 1 and 2 codes = 0x7F key1:" , key1up ? " up " : " down ","key2:", key2up ? " up" : " down");
    }
    else
    {  
      logmsg("-- If keyboard, key1: ", adb_key_to_string(key1code), key1up ? " up " : " down ", "code: ", key1code,
                            " key2: ", adb_key_to_string(key2code), key2up ? " up " : " down ", "code: ", key2code);
    }
    break;
  case 0xD: // talk register 1
    logmsg("-- Got TALK request for register 1, 0xD");
    break;
  case 0xE: // talk register 2
    logmsg("-- Got TALK request for register 2, 0xE");
    break;
  case 0xF: // talk register 3
    // sets device address
    talk_register = Receive16bitRegister();
    logmsg("-- Got TALK request for register 3, 0xF");
    if (talk_register == -100)
    {
      logmsg("-- no response from device address: ",address_nibble ," queried");
      break;
    }
    else if (talk_register < 0)
    {
      logmsg("-- Talk register read error: ", talk_register);
      break;
    }
    logmsg("-- talk reg 3: ", (uint16_t) talk_register);
    srq_enabled = !!(talk_register & (1 << 14));
    unused_address = (talk_register >> 8) & 0xF;
    handler_id = talk_register & 0xF;
    logmsg("-- Reg 3 values Srq_en: ", srq_enabled, " device addr: ", unused_address, " handler id: ", handler_id );
    break;
  default:
    logmsg("Unknown cmd: ", command_nibble);
    break;
  }
}

uint16_t AdbInterface::GetAdbRegister3Keyboard()
{
  uint16_t kbdreg3 = 0;
  // Bit 15 Reserved; must be 0
  B_UNSET(kbdreg3, 15);
  // 14      Exceptional event, device specific; always 1 if not used
  B_SET(kbdreg3, 14);
  // 13      Service Request enable; 1 = enabled
  B_UNSET(kbdreg3, 13);
  // 12      Reserved; must be 0
  B_UNSET(kbdreg3, 12);
  // 11-8      Device address
  // "ADB - The Untold Story: Space Aliens Ate My Mouse"
  // specifies that a random value should be returned as the address for register 3
  static uint8_t random_address = 0xFF;
  if (random_address == 0xFF)
  {
    random_address = rand() & 0xF;
  }

  kbdreg3 |=  random_address << 8;
  // 7-0       Device Handler ID
  kbdreg3 |= kbd_handler_id;
  
  return kbdreg3;
}
uint16_t AdbInterface::GetAdbRegister3Mouse()
{
  uint16_t mousereg3 = 0;
  // Bit 15 Reserved; must be 0
  B_UNSET(mousereg3, 15);
  // 14      Exceptional event, device specific; always 1 if not used
  B_SET(mousereg3, 14);
  // 13      Service Request enable; 1 = enabled
  B_UNSET(mousereg3, 13);
  // 12      Reserved; must be 0
  B_UNSET(mousereg3, 12);
  // 11-8      Device address 
  // "ADB - The Untold Story: Space Aliens Ate My Mouse"
  // specifies that a random value should be returned as the address for register 3
  static uint8_t random_address = 0xFF;
  if (random_address == 0xFF)
  {
    random_address = rand() & 0xF;
  }
  mousereg3 |= random_address << 8;  
  // 7-0       Device Handler ID
  mousereg3 |= mouse_handler_id;

  return mousereg3;
}

void AdbInterface::Reset(void)
{
  mouse_addr = MOUSE_DEFAULT_ADDR;
  kbd_addr = KBD_DEFAULT_ADDR;
  mouse_handler_id = MOUSE_DEFAULT_HANDLER_ID;
  kbd_handler_id = KBD_DEFAULT_HANDLER_ID;
  kbdreg2 = 0xFFFF;
}
