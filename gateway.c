/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file gateway.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/05/04
 * @license
 * @description
 */


#include "common.h"
#include "upgrade.h"
#include "gateway.h"
#include "upgrade_packets.h"
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
static uint8_t              g_gw_buf[2048] = {0};
static upg_operator_t       g_gw_opr = {0};

static sock_info_t          g_gw_sock_info[SOCK_TOTAL] =
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
_gateway_send(upg_operator_t *pOp)
{
    int     rval = 0;
    do {
        rbi_t       *pRBI = 0;
        uint8_t     *pCur = 0;

        if( pOp->port == CONFIG_CONTROLLER_SOURCE_PORT )
        {
            rval = common_get_rbi(SOCK_CTLR_SOURCE, &pRBI);
            if( rval ) break;
        }
        else if( pOp->port == CONFIG_LEAF_SOURCE_PORT  )
        {
            rval = common_get_rbi(SOCK_LEAF_SOURCE, &pRBI);
            if( rval ) break;
        }
        else
        {
            rval = -1;
            break;
        }

        if( !(pCur = malloc(pOp->length + 4)) )
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
_gateway_recv(upg_operator_t *pOp)
{
    int     rval = 0;
    do {
        rbi_t       *pRBI = 0;
        uint8_t     *pCur = 0;
        int         len = 0;

        if( pOp->port == CONFIG_CONTROLLER_SINK_PORT )
        {
            rval = common_get_rbi(SOCK_CTLR_SINK, &pRBI);
            if( rval ) break;
        }
        else if ( pOp->port == CONFIG_LEAF_SINK_PORT )
        {
            rval = common_get_rbi(SOCK_LEAF_SINK, &pRBI);
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


static int
_fill_req(
    upg_pkt_info_t  *pInfo)
{
    upg_pkt_hdr_t   *pPkt_hdr = pInfo->pPkt_hdr;

    switch( pInfo->opcode )
    {
        default:    break;
        case UPG_OPCODE_DISCOVERY_RESP:
            pPkt_hdr->short_data = UPG_DISCOVERY_ACK;
            break;
    }
    return 0;
}

//=============================================================================
//                  Public Function Definition
//=============================================================================
int
gateway_init(void)
{
    int     rval = 0;
    do {
        g_gw_opr.pData        = g_gw_buf;
        g_gw_opr.length       = sizeof(g_gw_buf);
        g_gw_opr.pTunnel_info = &g_gw_sock_info;
        g_gw_opr.cb_ll_recv   = _gateway_recv;
        g_gw_opr.cb_ll_send   = _gateway_send;

    } while(0);
    return rval;
}

int
gateway_deinit(void)
{
    int     rval = 0;
    return rval;
}

int
gateway_routine(void)
{
    int     rval = 0;
    do {
        do {    // receive from controller
            g_gw_opr.port   = CONFIG_CONTROLLER_SINK_PORT;
            g_gw_opr.length = sizeof(g_gw_buf);
            rval = upg_recv(&g_gw_opr);
            if( g_gw_opr.length <= 0 )
                break;

            upg_pkt_info_t      pkt_info = {0};

            pkt_info.pBuf_pkt     = g_gw_buf;
            pkt_info.buf_pkt_len  = sizeof(g_gw_buf);
            rval = upg_pkt_unpack(&pkt_info);
            if( rval )
            {
                log_msg("unpack fail \n");
                break;
            }

            if( pkt_info.pPkt_hdr )
            {
                upg_pkt_hdr_t   *pPkt_hdr = pkt_info.pPkt_hdr;

                switch( pPkt_hdr->cmd_opcode )
                {
                    case UPG_OPCODE_DISCOVERY_REQ:
                        log_msg("(rx gw) get disc req\n");
                        break;
                }
            }

            // TODO: response received message
            #if 0
            g_gw_opr.length = sizeof(g_gw_buf);
            g_gw_opr.port   = CONFIG_CONTROLLER_SOURCE_PORT;

            upg_send(&g_gw_opr);
            #endif
        } while(0);


        do {    // receive from leaf end
            g_gw_opr.port     = CONFIG_LEAF_SINK_PORT;
            g_gw_opr.length   = sizeof(g_gw_buf);
            rval = upg_recv(&g_gw_opr);
            if( g_gw_opr.length > 0 )
            {
                log_msg("(rx leaf) %s\n", g_gw_opr.pData);
                // TODO: response received message
                #if 0
                g_gw_opr.length = sizeof(g_gw_buf);
                g_gw_opr.port   = CONFIG_LEAF_SOURCE_PORT;

                upg_send(&g_gw_opr);
                #endif
            }
        } while(0);

    } while(0);
    return rval;
}
