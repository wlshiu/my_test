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
        sock_info_t         *pSock_info = (sock_info_t*)pOp->pTunnel_info;
        struct sockaddr_in  *pSock_in = 0;
        SOCKET              sd;

        if( pOp->port == CONFIG_LEAF_SINK_PORT )
        {
            sd       = pSock_info[SOCK_LEAF_SINK].sd;
            pSock_in = &pSock_info[SOCK_LEAF_SINK].sock_in;
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
_leaf_recv(upg_operator_t *pOp)
{
    int     rval = 0;
    do {
        sock_info_t         *pSock_info = (sock_info_t*)pOp->pTunnel_info;
        SOCKET              sd;
        struct sockaddr_in  remote_addr;
        int                 addr_len = sizeof(remote_addr);

        if( pOp->port == CONFIG_LEAF_SOURCE_PORT )
        {
            sd = pSock_info[SOCK_LEAF_SOURCE].sd;
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
            log_msg("[leaf:%d] from(%s) '%s' \n",
                    pOp->port, inet_ntoa(remote_addr.sin_addr), pOp->pData);
        }
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

        //-----------------------
        // setup BSD sockets
        for(int i = 0; i < SOCK_TOTAL; i++)
        {
            u_long  no_block = 1;

            if( i == SOCK_CTLR_SOURCE ||
                i == SOCK_CTLR_SINK )
            {
                g_leaf_sock_info[i].sd = -1;
                continue;
            }

            g_leaf_sock_info[i].sd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            if( g_leaf_sock_info[i].sd == INVALID_SOCKET )
            {
                rval = -1;
                log_msg("socket error !");
                break;
            }

            ioctlsocket(g_leaf_sock_info[i].sd, FIONBIO, &no_block);
        }

        if( rval ) break;

        // socket server
        g_leaf_sock_info[SOCK_LEAF_SOURCE].sock_in.sin_family           = AF_INET;
        g_leaf_sock_info[SOCK_LEAF_SOURCE].sock_in.sin_port             = htons(g_leaf_sock_info[SOCK_LEAF_SOURCE].port);
        g_leaf_sock_info[SOCK_LEAF_SOURCE].sock_in.sin_addr.S_un.S_addr = INADDR_ANY;
        if( bind(g_leaf_sock_info[SOCK_LEAF_SOURCE].sd,
                 (struct sockaddr*)&g_leaf_sock_info[SOCK_LEAF_SOURCE].sock_in,
                 sizeof(struct sockaddr_in)) == SOCKET_ERROR )
        {
            rval = -1;
            log_msg("bind error %d !\n");
            break;
        }

        // socket client
        g_leaf_sock_info[SOCK_LEAF_SINK].sock_in.sin_family           = AF_INET;
        g_leaf_sock_info[SOCK_LEAF_SINK].sock_in.sin_port             = htons(g_leaf_sock_info[SOCK_LEAF_SINK].port);
        g_leaf_sock_info[SOCK_LEAF_SINK].sock_in.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
    } while(0);
    return rval;
}

int
leaf_deinit(void)
{
    int     rval = 0;
    //-----------------------
    // destroy BSD sockets
    for(int i = 0; i < SOCK_TOTAL; i++)
    {
        if( g_leaf_sock_info[i].sd < 0 )
            continue;

        closesocket(g_leaf_sock_info[i].sd);
    }

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
        if( rval > 0 )
        {
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
