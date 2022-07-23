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
 * This file is part of the TinyUSB stack.
 */

#include "tusb_option.h"

#if CFG_TUD_ENABLED && defined(TUP_SIM_HS)

//--------------------------------------------------------------------+
// INCLUDE
//--------------------------------------------------------------------+
#include "device/dcd.h"


//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF
//--------------------------------------------------------------------+
void dcd_init(uint8_t rhport)
{
    return;
}

void dcd_int_enable(uint8_t rhport)
{
    return;
}

void dcd_int_disable(uint8_t rhport)
{
    return;
}

void dcd_set_address(uint8_t rhport, uint8_t dev_addr)
{
    // Response with status first before changing device address
    return;
}

void dcd_remote_wakeup(uint8_t rhport)
{
    return;
}

void dcd_connect(uint8_t rhport)
{
    return;
}

void dcd_disconnect(uint8_t rhport)
{
    return;
}

void dcd_sof_enable(uint8_t rhport, bool en)
{
  (void) rhport;
  (void) en;

  // TODO implement later
}

//--------------------------------------------------------------------+
// DCD Endpoint Port
//--------------------------------------------------------------------+
void dcd_edpt_stall(uint8_t rhport, uint8_t ep_addr)
{
    return;
}

void dcd_edpt_clear_stall(uint8_t rhport, uint8_t ep_addr)
{
    return;
}

bool dcd_edpt_open(uint8_t rhport, tusb_desc_endpoint_t const * p_endpoint_desc)
{
  return true;
}

void dcd_edpt_close_all (uint8_t rhport)
{
    return;
}

void dcd_edpt_close(uint8_t rhport, uint8_t ep_addr)
{
    return;
}

bool dcd_edpt_xfer(uint8_t rhport, uint8_t ep_addr, uint8_t * buffer, uint16_t total_bytes)
{
    return true;
}

// fifo has to be aligned to 4k boundary
bool dcd_edpt_xfer_fifo (uint8_t rhport, uint8_t ep_addr, tu_fifo_t * ff, uint16_t total_bytes)
{
    return true;
}


#endif
