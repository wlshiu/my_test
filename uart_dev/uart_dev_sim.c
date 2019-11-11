/**
 * Copyright (c) 2019 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file uart_dev_sim.c.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2019/11/11
 * @license
 * @description
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "uart_dev.h"
#include "pthread.h"
#include "log.h"

//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct sim_dev
{
    FILE                *fd_uart;
    volatile uint16_t   rd_idx;
    volatile uint16_t   wr_idx;

    int                 is_rx_running;
    uint8_t             *pRx_buf;
    long                rx_buf_len;
    uart_dev_type_t     dev_type;
} sim_dev_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================
static sim_dev_t   g_sim_dev = {.fd_uart = 0, };
//=============================================================================
//                  Private Function Definition
//=============================================================================
static uart_handle_t
_sim_uart_init(uart_cfg_t *pCfg)
{
    sim_dev_t       *pHDev = 0;
    do {
        memset(&g_sim_dev, 0x0, sizeof(g_sim_dev));

        if( !(g_sim_dev.fd_uart = fopen(pCfg->pDev_name, "rb")) )
        {
            err("open %s fail \n", pCfg->pDev_name);
            break;
        }

        pHDev = &g_sim_dev;
    } while(0);

    return (uart_handle_t)pHDev;
}

static int
_sim_uart_deinit(uart_handle_t pHandle)
{
    int     rval = 0;
    do {
        sim_dev_t     *pHDev = (sim_dev_t*)pHandle;

        if( pHDev->fd_uart )    fclose(pHDev->fd_uart);
        pHDev->fd_uart = 0;

    } while(0);

    return rval;
}

static int
_sim_uart_send_bytes(
    uart_handle_t   pHandle,
    uint8_t         *pData,
    int             data_len)
{
    int     rval = 0;
    do {
//        sim_dev_t     *pHDev = (sim_dev_t*)pHandle;
    } while(0);
    return rval;
}

static int
_sim_uart_recv_bytes(
    uart_handle_t   pHandle,
    uint8_t         *pData,
    int             *pData_len)
{
    int     rval = -1;
    do {
        sim_dev_t       *pHDev = (sim_dev_t*)pHandle;
        int             buf_len = *pData_len;

        if( !pHDev || !pHDev->fd_uart )
            break;

        *pData_len = fread(pData, 1, buf_len, pHDev->fd_uart);

        rval = 0;
    } while(0);
    return rval;
}

static int
_sim_uart_get_state(
    uart_handle_t   pHandle,
    uart_state_t    state)
{
    int             rval = 0;
    do {
        sim_dev_t     *pHDev = (sim_dev_t*)pHandle;
        if( !pHDev )   break;

        if( state == UART_STATE_GET_RX_EVENT )
        {
            rval = 1;
        }
    } while(0);
    return rval;
}

static int
_sim_uart_reset_buf(
    uart_handle_t   pHandle,
    uint8_t         *pBuf,
    uint32_t        buf_len)
{
    int             rval = 0;
    do {
    } while(0);
    return rval;
}

//=============================================================================
//                  Public Function Definition
//=============================================================================
uart_ops_t      g_sim_uart_sim_ops =
{
    .dev_type      = UART_DEV_TYPE_SIM,
    .init          = _sim_uart_init,
    .deinit        = _sim_uart_deinit,
    .send_bytes    = _sim_uart_send_bytes,
    .recv_bytes    = _sim_uart_recv_bytes,
    .get_state     = _sim_uart_get_state,
    .reset_buf     = _sim_uart_reset_buf,

};

