/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file main.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/04/30
 * @license
 * @description
 */


#include <stdint.h>
#include "common.h"

#include "upgrade.h"
#include "gateway.h"
#include "leaf_end.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
#define CONFIG_DATA_SLICE_SIZE      (1 << 10)
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct ctlr_info
{
    FILE        *fp;
    uint32_t    *pIs_running;
} ctlr_info_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================
pthread_mutex_t             g_log_mtx;

static pthread_cond_t       g_usr_cond;
static pthread_mutex_t      g_usr_mtx;

static uint8_t              g_ctlr_buf[2048] = {0};
static upg_operator_t       g_ctlr_opr = {0};

static sock_info_t          g_ctlr_sock_info[SOCK_TOTAL] =
{
    [SOCK_CTLR_SOURCE] = { .port = CONFIG_CONTROLLER_SOURCE_PORT, },
    [SOCK_CTLR_SINK]   = { .port = CONFIG_CONTROLLER_SINK_PORT, },
    [SOCK_LEAF_SOURCE] = { .port = CONFIG_LEAF_SOURCE_PORT, },
    [SOCK_LEAF_SINK]   = { .port = CONFIG_LEAF_SINK_PORT, },
};
//=============================================================================
//                  Private Function Definition
//=============================================================================
static void*
_task_gateway(void *argv)
{
    int         rval = 0;
    uint32_t    *pIs_running = (uint32_t*)argv;

    pthread_mutex_lock(&g_usr_mtx);
    pthread_cond_signal(&g_usr_cond);
    pthread_mutex_unlock(&g_usr_mtx);

    do {
        rval = gateway_init();
        if( rval ) break;

        while( *pIs_running )
        {
            gateway_routine();

            Sleep(4);
        }
    } while(0);

    gateway_deinit();

    pthread_exit(0);
    return 0;
}

static void*
_task_leaf_end(void *argv)
{
    int         rval = 0;
    uint32_t    *pIs_running = (uint32_t*)argv;

    pthread_mutex_lock(&g_usr_mtx);
    pthread_cond_signal(&g_usr_cond);
    pthread_mutex_unlock(&g_usr_mtx);

    do {
        rval = leaf_init();
        if( rval ) break;

        while( *pIs_running )
        {
            leaf_routine();

            Sleep(30);
        }

    } while(0);

    leaf_deinit();

    pthread_exit(0);
    return 0;
}

static int
_ctlr_send(upg_operator_t *pOp)
{
    int     rval = 0;
    do {
        sock_info_t         *pSock_info = (sock_info_t*)pOp->pTunnel_info;
        struct sockaddr_in  *pSock_in = 0;
        SOCKET              sd;

        if( pOp->port == CONFIG_CONTROLLER_SINK_PORT )
        {
            sd       = pSock_info[SOCK_CTLR_SINK].sd;
            pSock_in = &pSock_info[SOCK_CTLR_SINK].sock_in;
        }
        else if( pOp->port == CONFIG_CONTROLLER_SOURCE_PORT )
        {
            // test
            sd       = pSock_info[SOCK_CTLR_SOURCE].sd;
            pSock_in = &pSock_info[SOCK_CTLR_SOURCE].sock_in;
        }
        else
        {
            rval = -1;
            break;
        }

        rval = sendto(sd, (const char*)pOp->pData, pOp->length,
                      0, (struct sockaddr*)&pSock_info[SOCK_CTLR_SOURCE].sock_in, sizeof(struct sockaddr_in));
        printf("rval = %d\n", rval);
        __asm("nop");
    } while(0);
    return rval;
}

static int
_ctlr_recv(upg_operator_t *pOp)
{
    int     rval = 0;
    do {
        sock_info_t         *pSock_info = (sock_info_t*)pOp->pTunnel_info;
        SOCKET              sd;
        struct sockaddr_in  remote_addr;
        int                 addr_len = sizeof(remote_addr);

        if( pOp->port == CONFIG_CONTROLLER_SOURCE_PORT )
        {
//            sd = pSock_info[SOCK_CTLR_SOURCE].sd;
            rval = recvfrom(pSock_info[SOCK_CTLR_SOURCE].sd, (char*)pOp->pData, pOp->length,
                            0, (struct sockaddr*)&remote_addr, &addr_len);
            log_msg("1.rx rval = %d\n", rval);

            rval = recvfrom(pSock_info[SOCK_CTLR_SINK].sd, (char*)pOp->pData, pOp->length,
                            0, (struct sockaddr*)&remote_addr, &addr_len);
            log_msg("2.rx rval = %d\n", rval);
        }
        else
        {
            rval = -1;
            break;
        }

//        rval = recvfrom(sd, (char*)pOp->pData, pOp->length,
//                        0, (struct sockaddr*)&remote_addr, &addr_len);

        pOp->length = (rval > 0) ? rval : 0;
        if( pOp->length )
        {
            pOp->pData[pOp->length - 1] = 0x00;
            log_msg("[ctlr:%d] from(%s) '%s' \n",
                    pOp->port, inet_ntoa(remote_addr.sin_addr), pOp->pData);
        }
    } while(0);
    return rval;
}

static int
_ctlr_init(void)
{
    int     rval = 0;
    do {
        g_ctlr_opr.pData        = g_ctlr_buf;
        g_ctlr_opr.length       = sizeof(g_ctlr_buf);
        g_ctlr_opr.pTunnel_info = &g_ctlr_sock_info;
        g_ctlr_opr.cb_ll_recv   = _ctlr_recv;
        g_ctlr_opr.cb_ll_send   = _ctlr_send;

        //-----------------------
        // setup BSD sockets
        for(int i = 0; i < SOCK_TOTAL; i++)
        {
            u_long  no_block = 1;

            if( i == SOCK_LEAF_SOURCE ||
                i == SOCK_LEAF_SINK )
            {
//                g_ctlr_sock_info[i].sd = -1;
                continue;
            }

            g_ctlr_sock_info[i].sd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            if( g_ctlr_sock_info[i].sd == INVALID_SOCKET )
            {
                rval = -1;
                log_msg("socket error !");
                break;
            }

            ioctlsocket(g_ctlr_sock_info[i].sd, FIONBIO, &no_block);
        }

        if( rval ) break;

        // socket server
        g_ctlr_sock_info[SOCK_CTLR_SOURCE].sock_in.sin_family           = AF_INET;
        g_ctlr_sock_info[SOCK_CTLR_SOURCE].sock_in.sin_port             = htons(g_ctlr_sock_info[SOCK_CTLR_SOURCE].port);
        g_ctlr_sock_info[SOCK_CTLR_SOURCE].sock_in.sin_addr.S_un.S_addr = INADDR_ANY;
        if( bind(g_ctlr_sock_info[SOCK_CTLR_SOURCE].sd,
                 (struct sockaddr*)&g_ctlr_sock_info[SOCK_CTLR_SOURCE].sock_in,
                 sizeof(struct sockaddr_in)) == SOCKET_ERROR )
        {
            rval = -1;
            log_msg("bind error %d !\n");
            break;
        }

        // socket client
        g_ctlr_sock_info[SOCK_CTLR_SINK].sock_in.sin_family           = AF_INET;
        g_ctlr_sock_info[SOCK_CTLR_SINK].sock_in.sin_port             = htons(g_ctlr_sock_info[SOCK_CTLR_SINK].port);
        g_ctlr_sock_info[SOCK_CTLR_SINK].sock_in.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
    } while(0);
    return rval;
}

static int
_ctlr_deinit(void)
{
    int     rval = -1;
    for(int i = 0; i < SOCK_TOTAL; i++)
    {
        if( g_ctlr_sock_info[i].sd < 0 )
            continue;

        closesocket(g_ctlr_sock_info[i].sd);
    }
    return rval;
}


static int
_task_controller(void *argv)
{
    int             rval = 0;
    ctlr_info_t     *pInfo = (ctlr_info_t*)argv;

    do {
        FILE     *fin = pInfo->fp;

        rval = _ctlr_init();
        if( rval ) break;

        //-----------------
        // controller routine
        while( *pInfo->pIs_running )
        {
            // receive from gateway
            g_ctlr_opr.port     = CONFIG_CONTROLLER_SOURCE_PORT;
            g_ctlr_opr.length   = sizeof(g_ctlr_buf);
            rval = upg_recv(&g_ctlr_opr);
            if( rval > 0 )
            {
                // TODO: response received message
                #if 0
                g_ctlr_opr.length = sizeof(g_ctlr_buf);
                g_ctlr_opr.port   = CONFIG_CONTROLLER_SOURCE_PORT;

                upg_send(&g_ctlr_opr);
                #endif
            }

            //--------------------
            // active
        #if 1
            static int cnt = 0;
            if( cnt++ < 3 )
            {
                snprintf(g_ctlr_buf, sizeof(g_ctlr_buf), "hi %d ~~~~", cnt);
                g_ctlr_opr.length = strlen(g_ctlr_buf);
//                g_ctlr_opr.port   = CONFIG_CONTROLLER_SINK_PORT;
                g_ctlr_opr.port   = CONFIG_CONTROLLER_SOURCE_PORT;
                upg_send(&g_ctlr_opr);
            }

        #else
            if( fin )
            {
                int     slice_size = CONFIG_DATA_SLICE_SIZE;

                // read data
                slice_size = (CONFIG_DATA_SLICE_SIZE < sizeof(g_ctlr_buf))
                           ? CONFIG_DATA_SLICE_SIZE
                           : sizeof(g_ctlr_buf);

                g_ctlr_opr.length = fread(g_ctlr_buf, 1, slice_size, fin);
                if( g_ctlr_opr.length )
                {
                    g_ctlr_opr.port = CONFIG_CONTROLLER_SINK_PORT;
                    upg_send(&g_ctlr_opr);
                }
            }
        #endif // 1
            Sleep(100);
        }
    } while(0);

    _ctlr_deinit();

    *pInfo->pIs_running = 0;
    return rval;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
int main(int argc, char **argv)
{
    int         rval = 0;
    pthread_t   t_srv, t_clnt;
    do {
        uint32_t    is_running = 1;

        WSADATA     w;
        WORD        sockVersion = MAKEWORD(2,2);
        if( WSAStartup(sockVersion, &w) != 0 )
        {
            log_msg("Could not open Windows connection.\n");
            break;;
        }

        rval = pthread_mutex_init(&g_log_mtx, 0);
        if( rval )   break;

        rval = pthread_mutex_init(&g_usr_mtx, 0);
        if( rval )   break;

        rval = pthread_cond_init(&g_usr_cond, 0);
        if( rval )   break;

//        pthread_create(&t_srv, 0, _task_gateway, &is_running);
//
//        pthread_mutex_lock(&g_usr_mtx);
//        pthread_cond_wait(&g_usr_cond, &g_usr_mtx);
//        pthread_mutex_unlock(&g_usr_mtx);

//        pthread_create(&t_clnt, 0, _task_leaf_end, &is_running);
//
//        pthread_mutex_lock(&g_usr_mtx);
//        pthread_cond_wait(&g_usr_cond, &g_usr_mtx);
//        pthread_mutex_unlock(&g_usr_mtx);

        //-----------------------
        // controller
        {
            FILE            *fin = 0;
            ctlr_info_t     info = {0};

            if( !(fin = fopen(argv[1], "rb")) )
            {
                break;
            }

            info.fp             = fin;
            info.pIs_running    = &is_running;
            _task_controller(&info);

            fclose(fin);
        }

        pthread_join(t_srv, 0);
        pthread_join(t_clnt, 0);
    } while(0);

    WSACleanup();

    system("pause");
    return 0;
}
