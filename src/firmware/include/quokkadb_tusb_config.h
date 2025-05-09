/**
 * QuokkADB
 * Copyright (C) 2022 Rabbit Hole Computing LLC
 * This file is derived from hathach's (tinyusb.org) TinyUSB
 * 
 * 4 October 2022 - Modified by Rabbit Hole Computing LLC
 */

/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#ifndef _QUOKKADB_TUSB_CONFIG_H_
#define _QUOKKADB_TUSB_CONFIG_H_

#ifdef __cplusplus
 extern "C" {
#endif
// #include "hardware/uart.h"
//--------------------------------------------------------------------
// COMMON CONFIGURATION
//--------------------------------------------------------------------

#define CFG_TUSB_OS               OPT_OS_PICO
#define CFG_TUSB_MCU              OPT_MCU_RP2040 

// Enable host stack 
#define CFG_TUH_ENABLED     1

// CFG_TUSB_DEBUG is defined by compiler in DEBUG build
#define CFG_TUSB_DEBUG 0
//#define CFG_TUSB_DEBUG 3


/* USB DMA on some MCUs can only access a specific SRAM region with restriction on alignment.
 * Tinyusb use follows macros to declare transferring memory so that they can be put
 * into those specific section.
 * e.g
 * - CFG_TUSB_MEM SECTION : __attribute__ (( section(".usb_ram") ))
 * - CFG_TUSB_MEM_ALIGN   : __attribute__ ((aligned(4)))
 */
#ifndef CFG_TUSB_MEM_SECTION
#define CFG_TUSB_MEM_SECTION
#endif

#ifndef CFG_TUSB_MEM_ALIGN
#define CFG_TUSB_MEM_ALIGN          __attribute__ ((aligned(4)))
#endif

//--------------------------------------------------------------------
// HOST CONFIGURATION
//--------------------------------------------------------------------

// Size of buffer to hold descriptors and other data used for enumeration
#define CFG_TUH_ENUMERATION_BUFSIZE 1024

#define CFG_TUH_HUB                 7
// max device support (excluding hub device)
#define CFG_TUH_DEVICE_MAX          14 // hub typically has 4 ports
#define CFG_TUH_HID                 8 // typical keyboard + mouse device can have 3-4 HID interfaces
#define CFG_TUH_MSC                 0

#define CFG_TUH_HID_EPIN_BUFSIZE    64
#define CFG_TUH_HID_EPOUT_BUFSIZE   64




#ifdef __cplusplus
 }
#endif

#endif /* _QUOKKADB_TUSB_CONFIG_H_ */
