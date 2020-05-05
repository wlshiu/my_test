/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file leaf_end.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/05/04
 * @license
 * @description
 */



#include "upgrade.h"
#include "leaf_end.h"
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
static uint8_t              g_leaf_buf[2048] = {0};
static upg_operator_t       g_leaf_opr = {0};

static sock_info_t          g_leaf_sock_info[SOCK_TOTAL] =
{
    [SOCK_CTLR_SOURCE] = { .port = CONFIG_CONTROLLER_SOURCE_PORT, },
    [SOCK_CTLR_SINK]   = { .port = CONFIG_CONTROLLER_SINK_PORT, },
    [SOCK_LEAF_SOURCE] = { .port = CONFIG_LEAF_SOURCE_PORT, },
    [SOCK_LEAF_SINK]   = { .port = CONFIG_LEAF_SINK_PORT, },
};
//=============================================================================
//                  Private Function Definition
//=============================================================================
static int
_leaf_send(upg_operator_t *pOp)
{
    int     rval = 0;
    do {
        rbi_t       *pRBI = 0;
        uint8_t     *pCur = 0;

        if( pOp->port == CONFIG_LEAF_SINK_PORT )
        {
            rval = common_get_rbi(SOCK_LEAF_SINK, &pRBI);
            if( rval ) break;
        }
        else
        {
            rval = -1;
            break;
        }

        *((uint32_t*)pCur) = pOp->length;

        memcpy(pCur + 4, pOp->pData, pOp->length);

        while( (rval = rbi_push(pRBI, (uint32_t)pCur)) )
        {
            Sleep(3);
        }
    } while(0);
    return rval;
}

static int
_leaf_recv(upg_operator_t *pOp)
{
    int     rval = 0;
    do {
        rbi_t       *pRBI = 0;
        uint8_t     *pCur = 0;
        int         len = 0;

        if( pOp->port == CONFIG_LEAF_SOURCE_PORT )
        {
            rval = common_get_rbi(SOCK_LEAF_SOURCE, &pRBI);
            if( rval ) break;
        }
        else
        {
            rval = -1;
            break;
        }

        if( rbi_pick(pRBI) )
        {
            uint32_t    raw_size = 0;

            pCur = (uint8_t*)rbi_pop(pRBI);

            raw_size = *((uint32_t*)pCur);
            len = (raw_size < pOp->length) ? raw_size : pOp->length;

            memset(pOp->pData, 0x0, pOp->length);
            memcpy(pOp->pData, pCur + 4, len);
            free(pCur);
        }

        pOp->length = len;
    } while(0);
    return rval;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
int
leaf_init(void)
{
    int     rval = 0;
    do {
        g_leaf_opr.pData        = g_leaf_buf;
        g_leaf_opr.length       = sizeof(g_leaf_buf);
        g_leaf_opr.pTunnel_info = &g_leaf_sock_info;
        g_leaf_opr.cb_ll_recv   = _leaf_recv;
        g_leaf_opr.cb_ll_send   = _leaf_send;

    } while(0);
    return rval;
}

int
leaf_deinit(void)
{
    int     rval = 0;
    return rval;
}

int
leaf_routine(void)
{
    int     rval = 0;
    do {
        // receive from gateway
        g_leaf_opr.port     = CONFIG_LEAF_SOURCE_PORT;
        g_leaf_opr.length   = sizeof(g_leaf_buf);
        rval = upg_recv(&g_leaf_opr);
        if( g_leaf_opr.length > 0 )
        {
            log_msg("(rx gw) %s\n", g_leaf_opr.pData);

            // TODO: response received message
            #if 0
            g_leaf_opr.length = sizeof(g_leaf_buf);
            g_leaf_opr.port   = CONFIG_CONTROLLER_SOURCE_PORT;

            upg_send(&g_leaf_opr);
            #endif
        }
    } while(0);
    return rval;
}
