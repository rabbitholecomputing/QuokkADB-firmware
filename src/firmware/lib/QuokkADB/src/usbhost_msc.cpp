
// MSC - mass storage device host
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

#include "tusb.h"
#include "log_cache.h"
#include <class/msc/msc_host.h>
#include "blink.h"

//--------------------------------------------------------------------+
// MACRO TYPEDEF CONSTANT ENUM DECLARATION
//--------------------------------------------------------------------+
static scsi_inquiry_resp_t inquiry_resp;

bool inquiry_complete_cb(uint8_t dev_addr, tuh_msc_complete_data_t const * cb_data)
{
  msc_cbw_t const* cbw = cb_data->cbw;
  msc_csw_t const* csw = cb_data->csw;

  if (csw->status != 0)
  {
    logmsg("Inquiry failed\r\n");
    return false;
  }

  // Print out Vendor ID, Product ID and Rev
  char vendor_id[9] = {0};
  char product_id[17] = {0};
  char product_rev[5] = {0};
  memcpy(vendor_id, inquiry_resp.vendor_id, sizeof(inquiry_resp.vendor_id));
  memcpy(product_id, inquiry_resp.product_id, sizeof(inquiry_resp.product_id));
  memcpy(product_rev,  inquiry_resp.product_rev, sizeof( inquiry_resp.product_rev));
  logmsg(vendor_id, ":", product_id, " rev", product_rev);

  // Get capacity of device
  uint32_t const block_count = tuh_msc_get_block_count(dev_addr, cbw->lun);
  uint32_t const block_size = tuh_msc_get_block_size(dev_addr, cbw->lun);

  logmsg("Disk Size: ",(int)(block_count / ((1024*1024)/block_size)),  "MB");
  logmsg("Block Count = ",(int) block_count," Block Size: ", (int)block_size);

  return true;
}

//------------- IMPLEMENTATION -------------//
void tuh_msc_mount_cb(uint8_t dev_addr)
{
  blink_led.blink(3);
  logmsg("A MassStorage device is mounted");

  uint8_t const lun = 0;
  tuh_msc_inquiry(dev_addr, lun, &inquiry_resp, inquiry_complete_cb, 0);
}

void tuh_msc_umount_cb(uint8_t dev_addr)
{
  blink_led.blink(5);
  (void) dev_addr;
  logmsg("A MassStorage device is unmounted");
}