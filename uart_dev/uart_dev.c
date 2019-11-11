/**
 * Copyright (c) 2019 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file uart_dev.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2019/11/11
 * @license
 * @description
 */


#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "uart_dev.h"
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
typedef struct uart_dev
{
    uart_handle_t   hUart;
    uart_ops_t      *pUart_ops;

} uart_dev_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================
extern uart_ops_t      g_sim_uart_sim_ops;
extern uart_ops_t      g_rasp_pi_uart_pi_ops;

static uart_ops_t*     g_pUart_ops_list[] =
{
    &g_rasp_pi_uart_pi_ops,
    &g_sim_uart_sim_ops,
    0
};
//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
uart_handle_t
uart_dev_init(
    uart_cfg_t          *pCfg,
    cb_set_user_ops_t   cb_set_user_ops)
{
    uart_err_t  rval = UART_ERR_OK;
    uart_dev_t  *pDev = 0;

    assert(pCfg);

    do {
        if( !(pDev = malloc(sizeof(uart_dev_t))) )
        {
            err("%s", "create device fail\n");
            rval = UART_ERR_ALLOCATE_FAIL;
            break;
        }

        memset(pDev, 0x0, sizeof(uart_dev_t));

        // search the active uart_ops
        if( cb_set_user_ops )
            pDev->pUart_ops = cb_set_user_ops(pCfg);
        else
        {
            int         i = 0;

            while( g_pUart_ops_list[i] )
            {
                if( g_pUart_ops_list[i]->dev_type == pCfg->act_dev_type )
                {
                    pDev->pUart_ops = g_pUart_ops_list[i];
                    break;
                }

                i++;
            }
        }

        if( !pDev->pUart_ops )
        {
            rval = UART_ERR_INVALID_PARAM;
            break;
        }

        // call init method
        if( pDev->pUart_ops->init )
        {
            pDev->hUart = pDev->pUart_ops->init(pCfg);
            if( !pDev->hUart )
            {
                err("%s", "device init fail \n");
                rval = UART_ERR_INVALID_PARAM;
                break;
            }
        }
    } while(0);

    if( rval )
    {
        uart_dev_deinit((uart_handle_t)pDev);
        pDev = 0;
    }

    return (uart_handle_t*)pDev;
}

uart_err_t
uart_dev_deinit(
    uart_handle_t   hUart)
{
    int     rval = 0;
    do {
        uart_dev_t  *pDev = (uart_dev_t*)hUart;

        if( !hUart )    break;

        if( pDev->pUart_ops && pDev->pUart_ops->deinit )
            pDev->pUart_ops->deinit(pDev->hUart);

        free(pDev);

    } while(0);

    return rval;
}

uart_err_t
uart_dev_send_bytes(
    uart_handle_t   hUart,
    uint8_t         *pData,
    int             len)
{
    int     rval = 0;
    do {
        uart_dev_t  *pDev = (uart_dev_t*)hUart;

        if( !hUart || !pData || len <= 0 )
        {
            err("invalid parameters: hUart= %p, pData= %p, len= %d\n",
                hUart, pData, len);
            rval = UART_ERR_INVALID_PARAM;
            break;
        }

        if( pDev->pUart_ops && pDev->pUart_ops->send_bytes )
            rval = pDev->pUart_ops->send_bytes(pDev->hUart, pData, len);

    } while(0);
    return rval;
}

uart_err_t
uart_dev_recv_bytes(
    uart_handle_t   hUart,
    uint8_t         *pData,
    int             *pLen)
{
    int     rval = 0;
    do {
        uart_dev_t  *pDev = (uart_dev_t*)hUart;

        if( !hUart || !pData || !pLen )
        {
            err("invalid parameters: hUart= %p, pData= %p, pLen= %p\n",
                hUart, pData, pLen);
            rval = UART_ERR_INVALID_PARAM;
            break;
        }

        if( *pLen == 0 )
        {
            // err("%s", "buffer length can't be 0\n");
            // rval = UART_ERR_INVALID_PARAM;
            break;
        }

        if( pDev->pUart_ops && pDev->pUart_ops->recv_bytes )
            rval = pDev->pUart_ops->recv_bytes(pDev->hUart, pData, pLen);

    } while(0);
    return rval;
}

uart_err_t
uart_dev_reset_buf(
    uart_handle_t   hUart,
    uint8_t         *pBuf,
    uint32_t        buf_len)
{
    int     rval = 0;
    do {
        uart_dev_t  *pDev = (uart_dev_t*)hUart;

        if( !hUart )
        {
            err("invalid parameters: hUart= %p\n", hUart);
            rval = UART_ERR_INVALID_PARAM;
            break;
        }

        if( pDev->pUart_ops && pDev->pUart_ops->reset_buf )
            rval = pDev->pUart_ops->reset_buf(pDev->hUart, pBuf, buf_len);

    } while(0);
    return rval;
}

int
uart_dev_get_state(
    uart_handle_t   hUart,
    uart_state_t    state)
{
    int     rval = 0;
    do {
        uart_dev_t  *pDev = (uart_dev_t*)hUart;

        if( !hUart )
        {
            err("invalid parameters: hUart= %p\n", hUart);
            break;
        }

        if( pDev->pUart_ops && pDev->pUart_ops->get_state )
            rval = pDev->pUart_ops->get_state(pDev->hUart, state);

    } while(0);
    return rval;
}


int
uart_dev_ctrl(
    uart_handle_t   hUart,
    uint32_t        op_code,
    uint32_t        value,
    void            *pExtra)
{
    int     rval = 0;
    do {
        uart_dev_t  *pDev = (uart_dev_t*)hUart;

        if( !hUart )
        {
            err("invalid parameters: hUart= %p\n", hUart);
            break;
        }

        if( pDev->pUart_ops && pDev->pUart_ops->ctrl )
            rval = pDev->pUart_ops->ctrl(pDev->hUart, op_code, value, pExtra);

    } while(0);
    return rval;
}
