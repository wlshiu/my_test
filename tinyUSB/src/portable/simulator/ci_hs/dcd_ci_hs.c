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

#include "device/dcd.h"

#include "sim_common.h"
//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================
queue_handle_t          g_hQ_D2H; // device tx -> host rx
DWORD                   g_th_dcd_id;

static HANDLE           g_hTh_DCDListener;
static HANDLE           g_hEventListenerReady;


static uint32_t              g_dcd_tx_buf[CONFIG_BUFFER_SIZE >> 4] = {0};
static uint32_t              g_dcd_rx_buf[CONFIG_BUFFER_SIZE >> 4] = {0};
//=============================================================================
//                  Private Function Definition
//=============================================================================
static uint32_t __stdcall
_thread_dcd_listener(PVOID pM)
{
    printf(" @ %s\n", __func__);

    if( !SetEvent(g_hEventListenerReady) ) //set thread start event
    {
        err("set event fail \n");
    }

    while(1)
    {
        int     buf_size = sizeof(g_dcd_rx_buf);

        _wait_msg((uint8_t*)g_dcd_rx_buf, &buf_size);

        #if 0
        printf("===== %s RX ====\n", __func__);
        for(int i = 0; i < buf_size; i++)
        {
            printf("%c", buf[i]);
        }

        printf("\n");

        snprintf(g_mst_rx_buf, sizeof(g_mst_rx_buf), "  from %s", __func__);
        #endif

//        _send_msg(g_th_hcd_id, (uint8_t*)g_dcd_tx_buf, sizeof(g_dcd_tx_buf));
    }

    return 0;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
void dcd_init(uint8_t rhport)
{
    int     has_exit = false;

    trace("\n");
    queue_init(&g_hQ_D2H);

    g_hEventListenerReady = CreateEvent(0, FALSE, FALSE, 0); //create thread start event
    if( g_hEventListenerReady == 0 )
    {
        err("create event fail \n");
    }

    g_hTh_DCDListener = CreateThread(NULL, 0 /* use default stack size */,
                       (LPTHREAD_START_ROUTINE)_thread_dcd_listener,
                       (void*)&has_exit, // argument to thread function
                       0,               // use default creation flags
                       &g_th_dcd_id);    // returns the th
    if( g_hTh_DCDListener == NULL )
    {
        err("CreateThread fail\n");
    }

    WaitForSingleObject(g_hEventListenerReady, INFINITE);

    return;
}

void dcd_int_enable(uint8_t rhport)
{
    trace("\n");
    (void) rhport;
    NVIC_EnableIRQ(USB_IRQn);
    return;
}

void dcd_int_disable(uint8_t rhport)
{
    trace("\n");
    (void) rhport;
    NVIC_DisableIRQ(USB_IRQn);
    return;
}


void dcd_set_address(uint8_t rhport, uint8_t dev_addr)
{
    trace("\n");
    // Response with status first before changing device address
    return;
}

void dcd_remote_wakeup(uint8_t rhport)
{
    trace("\n");
    return;
}

void dcd_connect(uint8_t rhport)
{
    return;
}

void dcd_disconnect(uint8_t rhport)
{
    trace("\n");
    return;
}

void dcd_sof_enable(uint8_t rhport, bool en)
{
    trace("\n");
    (void) rhport;
    (void) en;

    // TODO implement later
    return;
}

/**
 *  DCD Endpoint Port
 */
void dcd_edpt_stall(uint8_t rhport, uint8_t ep_addr)
{
    trace("\n");
    return;
}

void dcd_edpt_clear_stall(uint8_t rhport, uint8_t ep_addr)
{
    trace("\n");
    return;
}

bool dcd_edpt_open(uint8_t rhport, tusb_desc_endpoint_t const *pEndpoint_desc)
{
    trace("\n");
    return true;
}

void dcd_edpt_close_all(uint8_t rhport)
{
    trace("\n");
    return;
}

void dcd_edpt_close(uint8_t rhport, uint8_t ep_addr)
{
    trace("\n");
    return;
}

bool dcd_edpt_xfer(uint8_t rhport, uint8_t ep_addr, uint8_t *pBuffer, uint16_t total_bytes)
{
    trace("\n");
//    _send_msg(g_th_dcd_id, pBuffer, total_bytes);
    return true;
}

// fifo has to be aligned to 4k boundary
bool dcd_edpt_xfer_fifo(uint8_t rhport, uint8_t ep_addr, tu_fifo_t *pFifo, uint16_t total_bytes)
{
    trace("\n");
    return true;
}


#endif
