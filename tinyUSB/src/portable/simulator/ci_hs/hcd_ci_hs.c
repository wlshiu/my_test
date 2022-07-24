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

// Highspeed USB IP implement EHCI for host functionality

#if CFG_TUH_ENABLED && defined(TUP_SIM_HS)

#include "common/tusb_common.h"
#include "host/hcd.h"
#include "portable/ehci/ehci_api.h"

#include "sim_common.h"
//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================
#define err(str, ...)       do{ printf("[%s:%d][Error] " str, __func__, __LINE__, ##__VA_ARGS__); while(1); }while(0)

//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================
queue_handle_t          g_hQ_H2D; // host tx -> device rx
DWORD                   g_th_hcd_id;

static HANDLE           g_hTh_HCDListener;
static HANDLE           g_hEventListenerReady;


static uint32_t              g_hcd_tx_buf[CONFIG_BUFFER_SIZE >> 4] = {0};
static uint32_t              g_hcd_rx_buf[CONFIG_BUFFER_SIZE >> 4] = {0};
//=============================================================================
//                  Private Function Definition
//=============================================================================
static uint32_t __stdcall
_thread_hcd_listener(PVOID pM)
{
    printf(" @ %s\n", __func__);

    if( !SetEvent(g_hEventListenerReady) ) //set thread start event
    {
        err("set event fail \n");
    }

    while(1)
    {
        int     buf_size = sizeof(g_hcd_rx_buf);

        _wait_msg((uint8_t*)g_hcd_rx_buf, &buf_size);

        #if 0
        printf("===== %s RX ====\n", __func__);
        for(int i = 0; i < buf_size; i++)
        {
            printf("%c", buf[i]);
        }

        printf("\n");

        snprintf(g_mst_rx_buf, sizeof(g_mst_rx_buf), "  from %s", __func__);
        #endif

//        _send_msg(g_th_dcd_id, (uint8_t*)g_hcd_tx_buf, sizeof(g_hcd_rx_buf));
    }

    return 0;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
bool hcd_init(uint8_t rhport)
{
    int     has_exit = false;

    trace("\n");
    queue_init(&g_hQ_H2D);

    g_hEventListenerReady = CreateEvent(0, FALSE, FALSE, 0); //create thread start event
    if( g_hEventListenerReady == 0 )
    {
        err("create event fail \n");
    }

    g_hTh_HCDListener = CreateThread(NULL, 0 /* use default stack size */,
                       (LPTHREAD_START_ROUTINE)_thread_hcd_listener,
                       (void*)&has_exit, // argument to thread function
                       0,               // use default creation flags
                       &g_th_hcd_id);    // returns the th
    if( g_hTh_HCDListener == NULL )
    {
        err("CreateThread fail\n");
    }

    WaitForSingleObject(g_hEventListenerReady, INFINITE);

    return true;
}

void hcd_port_reset(uint8_t rhport)
{
    trace("\n");
    assert(rhport == 0);
    // TODO: Nothing to do here yet. Perhaps need to reset some state?
    return;
}

bool hcd_port_connect_status(uint8_t rhport)
{
    uint32_t    status = USB_SIE_STATUS_SPEED_BITS;
    trace("\n");
    assert(rhport == 0);
    return status & USB_SIE_STATUS_SPEED_BITS;
}

tusb_speed_t hcd_port_speed_get(uint8_t rhport)
{
    trace("\n");
    assert(rhport == 0);
    // TODO: Should enumval this register
    return TUSB_SPEED_FULL;
}

// Close all opened endpoint belong to this device
void hcd_device_close(uint8_t rhport, uint8_t dev_addr)
{
    trace("\n");
    (void) rhport;

    if (dev_addr == 0)
        return;

    return;
}

uint32_t hcd_frame_number(uint8_t rhport)
{
    trace("\n");
    (void) rhport;
    return 0;
}

void hcd_int_enable(uint8_t rhport)
{
    trace("\n");
    (void)rhport;
    NVIC_EnableIRQ(USB_IRQn);
    return;
}

void hcd_int_disable(uint8_t rhport)
{
    trace("\n");
    (void)rhport;
    NVIC_DisableIRQ(USB_IRQn);
    return;
}

bool hcd_edpt_open(uint8_t rhport, uint8_t dev_addr, tusb_desc_endpoint_t const * ep_desc)
{
    trace("\n");
    (void) rhport;

    printf("hcd_edpt_open dev_addr %d, ep_addr %d\n", dev_addr, ep_desc->bEndpointAddress);

    return true;
}

bool hcd_edpt_xfer(uint8_t rhport, uint8_t dev_addr, uint8_t ep_addr, uint8_t *pBuffer, uint16_t buflen)
{
    trace("\n");
    (void) rhport;

    printf("hcd_edpt_xfer dev_addr %d, ep_addr 0x%x, len %d\n", dev_addr, ep_addr, buflen);

//   hw_endpoint_xfer_start(ep, pBuffer, buflen);
//    _send_msg(g_th_dcd_id, pBuffer, buflen);
    return true;
}

bool hcd_setup_send(uint8_t rhport, uint8_t dev_addr, uint8_t const setup_packet[8])
{
    trace("\n");
    (void) rhport;

    return true;
}


bool hcd_edpt_clear_stall(uint8_t dev_addr, uint8_t ep_addr)
{
    trace("\n");
    (void) dev_addr;
    (void) ep_addr;
    return true;
}

void hcd_port_reset_end(uint8_t rhport)
{
    trace("\n");
    (void) rhport;
    return;
}
#endif
