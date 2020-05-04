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


#include "upgrade.h"
#include "gateway.h"

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
        sock_info_t         *pSock_info = (sock_info_t*)pOp->pTunnel_info;
        struct sockaddr_in  *pSock_in = 0;
        SOCKET              sd;

        if( pOp->port == CONFIG_CONTROLLER_SOURCE_PORT )
        {
            sd       = pSock_info[SOCK_CTLR_SOURCE].sd;
            pSock_in = &pSock_info[SOCK_CTLR_SOURCE].sock_in;
        }
        else if( pOp->port == CONFIG_LEAF_SOURCE_PORT  )
        {
            sd       = pSock_info[SOCK_LEAF_SOURCE].sd;
            pSock_in = &pSock_info[SOCK_LEAF_SOURCE].sock_in;
        }
        else
        {
            rval = -1;
            break;
        }

        rval = sendto(sd, (const char*)pOp->pData, pOp->length,
                      0, (struct sockaddr*)pSock_in, sizeof(struct sockaddr_in));
    } while(0);
    return rval;
}

static int
_gateway_recv(upg_operator_t *pOp)
{
    int     rval = 0;
    do {
        sock_info_t         *pSock_info = (sock_info_t*)pOp->pTunnel_info;
        SOCKET              sd;
        struct sockaddr_in  remote_addr;
        int                 addr_len = sizeof(remote_addr);

        if( pOp->port == CONFIG_CONTROLLER_SINK_PORT )
        {
            sd = pSock_info[SOCK_CTLR_SINK].sd;
        }
        else if ( pOp->port == CONFIG_LEAF_SINK_PORT )
        {
            sd = pSock_info[SOCK_LEAF_SINK].sd;
        }
        else
        {
            rval = -1;
            break;
        }

        rval = recvfrom(sd, (char*)pOp->pData, pOp->length,
                        0, (struct sockaddr*)&remote_addr, &addr_len);

        pOp->length = (rval > 0) ? rval : 0;
        if( pOp->length )
        {
            pOp->pData[pOp->length - 1] = 0x00;
            log_msg("[GW:%d] from(%s) '%s' \n",
                    pOp->port, inet_ntoa(remote_addr.sin_addr), pOp->pData);
        }
    } while(0);
    return rval;
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

        //-----------------------
        // setup BSD sockets
        for(int i = 0; i < SOCK_TOTAL; i++)
        {
            u_long  no_block = 1;

            g_gw_sock_info[i].sd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            if( g_gw_sock_info[i].sd == INVALID_SOCKET )
            {
                rval = -1;
                log_msg("socket error !\n");
                break;
            }

            ioctlsocket(g_gw_sock_info[i].sd, FIONBIO, &no_block);
        }

        if( rval ) break;

        for(int i = 0; i < SOCK_TOTAL; i++)
        {
            switch( i )
            {
                // socket server
                case SOCK_CTLR_SINK:
                case SOCK_LEAF_SINK:
                    g_gw_sock_info[i].sock_in.sin_family           = AF_INET;
                    g_gw_sock_info[i].sock_in.sin_port             = htons(g_gw_sock_info[i].port);
                    g_gw_sock_info[i].sock_in.sin_addr.S_un.S_addr = INADDR_ANY;
                    if( bind(g_gw_sock_info[i].sd, (struct sockaddr*)&g_gw_sock_info[i].sock_in, sizeof(struct sockaddr_in))
                            == SOCKET_ERROR )
                    {
                        rval = -1;
                        log_msg("bind error %d !\n", i);
                        break;
                    }
                    break;

                // socket client
                case SOCK_CTLR_SOURCE:
                case SOCK_LEAF_SOURCE:
                    g_gw_sock_info[i].sock_in.sin_family           = AF_INET;
                    g_gw_sock_info[i].sock_in.sin_port             = htons(g_gw_sock_info[i].port);
                    g_gw_sock_info[i].sock_in.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
                    break;
            }

            if( rval ) break;
        }
    } while(0);
    return rval;
}

int
gateway_deinit(void)
{
    int     rval = 0;

    //-----------------------
    // destroy BSD sockets
    for(int i = 0; i < SOCK_TOTAL; i++)
    {
        if( g_gw_sock_info[i].sd < 0 )
            continue;

        closesocket(g_gw_sock_info[i].sd);
    }

    return rval;
}

int
gateway_routine(void)
{
    int     rval = 0;
    do {
        // receive from controller
        g_gw_opr.port = CONFIG_CONTROLLER_SINK_PORT;
        g_gw_opr.length   = sizeof(g_gw_buf);
        rval = upg_recv(&g_gw_opr);
        if( rval > 0 )
        {
            // TODO: response received message
            #if 0
            g_gw_opr.length = sizeof(g_gw_buf);
            g_gw_opr.port   = CONFIG_CONTROLLER_SOURCE_PORT;

            upg_send(&g_gw_opr);
            #endif
        }

        // receive from leaf end
        g_gw_opr.port     = CONFIG_LEAF_SINK_PORT;
        g_gw_opr.length   = sizeof(g_gw_buf);
        rval = upg_recv(&g_gw_opr);
        if( rval > 0 )
        {
            // TODO: response received message
            #if 0
            g_gw_opr.length = sizeof(g_gw_buf);
            g_gw_opr.port   = CONFIG_LEAF_SOURCE_PORT;

            upg_send(&g_gw_opr);
            #endif
        }
    } while(0);
    return rval;
}
