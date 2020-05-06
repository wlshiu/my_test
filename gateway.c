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
typedef enum gw_role
{
    GW_ROLE_SELF            = 1,
    GW_ROLE_DOWN_STREAM,

} gw_role_t;
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
_gateway_fill_req(
    upg_pkt_info_t  *pInfo)
{
    gw_role_t       *pRole = (gw_role_t*)pInfo->pExtra;
    upg_pkt_hdr_t   *pPkt_hdr = pInfo->pPkt_hdr;

    switch( pInfo->opcode )
    {
        default:    break;
        case UPG_OPCODE_DISCOVERY_RESP:
            if( *pRole == GW_ROLE_SELF )
            {
                pPkt_hdr->short_data = 0x333;
                break;
            }
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
        upg_pkt_info_t      pkt_info = {0};

        do { // receive from controller
            g_gw_opr.port   = CONFIG_CONTROLLER_SINK_PORT;
            g_gw_opr.length = sizeof(g_gw_buf);
            rval = upg_recv(&g_gw_opr);
            if( g_gw_opr.length <= 0 )
                break;

            // TODO: assemble data slices in difference packets

            pkt_info.pBuf_pkt     = g_gw_buf;
            pkt_info.buf_pkt_len  = sizeof(g_gw_buf);
            rval = upg_pkt_unpack(&pkt_info);
            if( rval )
            {
                log_msg("unpack fail \n");
                break;
            }

            // response received message
            if( pkt_info.pPkt_hdr )
            {
                gw_role_t       role = GW_ROLE_SELF;
                upg_pkt_hdr_t   *pPkt_hdr = pkt_info.pPkt_hdr;

                switch( pPkt_hdr->cmd_opcode )
                {
                    case UPG_OPCODE_DISCOVERY_REQ:
                        log_msg("(gw rx ctlr) disc req\n");

                        //------------------------------
                        // forward to leaf end
                        g_gw_opr.length = sizeof(g_gw_buf);
                        g_gw_opr.port   = CONFIG_LEAF_SOURCE_PORT;
                        upg_send(&g_gw_opr);

                    case UPG_OPCODE_DISCOVERY_RESP:
                        //------------------------------
                        // response to controller
                        pkt_info.opcode       = UPG_OPCODE_DISCOVERY_RESP;
                        pkt_info.buf_pkt_len  = sizeof(g_gw_buf);
                        pkt_info.cb_fill_data = _gateway_fill_req;
                        pkt_info.pExtra       = &role;
                        rval = upg_pkt_pack(&pkt_info);
                        if( rval )
                        {
                            printf("pack fail %d\n", rval);
                            break;
                        }

                        log_msg("(gw resp disc to ctlr)\n");

                        g_gw_opr.length = sizeof(g_gw_buf);
                        g_gw_opr.port   = CONFIG_CONTROLLER_SOURCE_PORT;
                        upg_send(&g_gw_opr);
                        break;

                    default:
                        log_msg("(gw rx) drop opcode= %02x\n", pPkt_hdr->cmd_opcode);
                        break;
                }
            }
        } while(0);


        do { // receive from leaf end
            g_gw_opr.port     = CONFIG_LEAF_SINK_PORT;
            g_gw_opr.length   = sizeof(g_gw_buf);
            rval = upg_recv(&g_gw_opr);
            if( g_gw_opr.length <= 0 )
                break;

            // TODO: assemble data slices in difference packets

            pkt_info.pBuf_pkt     = g_gw_buf;
            pkt_info.buf_pkt_len  = sizeof(g_gw_buf);
            rval = upg_pkt_unpack(&pkt_info);
            if( rval )
            {
                log_msg("unpack fail \n");
                break;
            }

            // response received message
            if( pkt_info.pPkt_hdr )
            {
                gw_role_t       role = GW_ROLE_DOWN_STREAM;
                upg_pkt_hdr_t   *pPkt_hdr = pkt_info.pPkt_hdr;

                // TODO: send response to leaf end and forward to controller

                switch( pPkt_hdr->cmd_opcode )
                {
                    case UPG_OPCODE_DISCOVERY_RESP:
                        log_msg("(gw rx leaf) disc resp\n");

                        //------------------------------
                        // forward to controller
                        pkt_info.opcode       = UPG_OPCODE_DISCOVERY_RESP;
                        pkt_info.buf_pkt_len  = sizeof(g_gw_buf);
                        pkt_info.cb_fill_data = _gateway_fill_req;
                        pkt_info.pExtra       = &role;
                        rval = upg_pkt_pack(&pkt_info);
                        if( rval )
                        {
                            printf("pack fail %d\n", rval);
                            break;
                        }

                        log_msg("(gw forward leaf disc to ctlr)\n");

                        g_gw_opr.length = sizeof(g_gw_buf);
                        g_gw_opr.port   = CONFIG_CONTROLLER_SOURCE_PORT;
                        upg_send(&g_gw_opr);
                        break;

                    case UPG_OPCODE_DISCOVERY_REQ:
                    default:
                        log_msg("(rx gw) drop opcode= %02x\n", pPkt_hdr->cmd_opcode);
                        break;
                }
            }
        } while(0);

    } while(0);
    return rval;
}
