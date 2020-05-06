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

#include "upgrade_packets.h"
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
        rbi_t       *pRBI = 0;
        uint8_t     *pCur = 0;

        if( pOp->port == CONFIG_CONTROLLER_SINK_PORT )
        {
            rval = common_get_rbi(SOCK_CTLR_SINK, &pRBI);
            if( rval ) break;
        }
        else if( pOp->port == CONFIG_CONTROLLER_SOURCE_PORT )
        {
            rval = common_get_rbi(SOCK_CTLR_SOURCE, &pRBI);
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
_ctlr_recv(upg_operator_t *pOp)
{
    int     rval = 0;
    do {
        rbi_t       *pRBI = 0;
        uint8_t     *pCur = 0;
        int         len = 0;

        if( pOp->port == CONFIG_CONTROLLER_SOURCE_PORT )
        {
            rval = common_get_rbi(SOCK_CTLR_SOURCE, &pRBI);
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
_ctlr_init(void)
{
    int     rval = 0;
    do {
        g_ctlr_opr.pData        = g_ctlr_buf;
        g_ctlr_opr.length       = sizeof(g_ctlr_buf);
        g_ctlr_opr.pTunnel_info = &g_ctlr_sock_info;
        g_ctlr_opr.cb_ll_recv   = _ctlr_recv;
        g_ctlr_opr.cb_ll_send   = _ctlr_send;

    } while(0);
    return rval;
}

static int
_ctlr_deinit(void)
{
    int     rval = -1;
    return rval;
}

static int
_ctlr_fill_req(
    upg_pkt_info_t  *pInfo)
{
    upg_pkt_hdr_t   *pPkt_hdr = pInfo->pPkt_hdr;

    switch( pInfo->opcode )
    {
        default:    break;

        case UPG_OPCODE_DATA_WR:
            pPkt_hdr->short_data = 0x3333;

            // payload is appended to packet header
            snprintf((char*)pPkt_hdr->pPayload, pInfo->buf_pkt_len - pInfo->cur_pkt_len, "%s", "ctlr discovery");

            pInfo->cur_pkt_len += (strlen((const char*)pPkt_hdr->pPayload) + 1);

            if( pInfo->cur_pkt_len & 0x1 )
            {
                pPkt_hdr->pPayload[pInfo->cur_pkt_len] = 0;

                // packet length MUST be 2-bytes alignment
                pInfo->cur_pkt_len++;
            }
            break;
    }
    return 0;
}

static int
_task_controller(void *argv)
{
    int             rval = 0;
    ctlr_info_t     *pInfo = (ctlr_info_t*)argv;

    do {
        FILE     *fin = pInfo->fp;
        int      step = 0;

        rval = _ctlr_init();
        if( rval ) break;

        //-----------------
        // controller routine
        while( *pInfo->pIs_running )
        {
            upg_pkt_info_t      pkt_info = {0};

            pkt_info.cur_pkt_len  = 0;
            pkt_info.pPkt_hdr     = (upg_pkt_hdr_t*)g_ctlr_buf;
            pkt_info.pBuf_pkt     = g_ctlr_buf;
            pkt_info.buf_pkt_len  = sizeof(g_ctlr_buf);
            pkt_info.cb_fill_data = _ctlr_fill_req;

            do { // receive from gateway
                g_ctlr_opr.port     = CONFIG_CONTROLLER_SOURCE_PORT;
                g_ctlr_opr.length   = sizeof(g_ctlr_buf);
                rval = upg_recv(&g_ctlr_opr);
                if( g_ctlr_opr.length <= 0 )
                    break;

                // TODO: assemble data slices in difference packets

                pkt_info.pBuf_pkt     = g_ctlr_buf;
                pkt_info.buf_pkt_len  = sizeof(g_ctlr_buf);
                rval = upg_pkt_unpack(&pkt_info);
                if( rval )
                {
                    log_msg("unpack fail \n");
                    break;
                }

                // response received message
                if( pkt_info.pPkt_hdr )
                {
                    upg_pkt_hdr_t   *pPkt_hdr = pkt_info.pPkt_hdr;

                    switch( pPkt_hdr->cmd_opcode )
                    {
                        case UPG_OPCODE_DISCOVERY_RESP:
                            log_msg("(ctlr get resp) 0x%x\n", pPkt_hdr->short_data);
                            break;
                        default:
                            log_msg("(ctlr rx) drop opcode= %02x\n", pPkt_hdr->cmd_opcode);
                            break;
                    }
                }

                // TODO: response received message
                #if 0
                g_ctlr_opr.length = sizeof(g_ctlr_buf);
                g_ctlr_opr.port   = CONFIG_CONTROLLER_SOURCE_PORT;

                upg_send(&g_ctlr_opr);
                #endif
            } while(0);



            //--------------------
            // active
            switch( step )
            {
                case 0:
                    pkt_info.opcode = UPG_OPCODE_DISCOVERY_REQ;
                    step++;
                    break;
//                case 1:
//                    step++;
//                    break;
                default:
                    step = -1;
                    static int   is_done = 0;
                    if( is_done == 0 )
                    {
                        log_msg("ctlr idle\n");
                        is_done = 1;
                    }
                    break;
            }

            if( step > 0 )
            {
                rval = upg_pkt_pack(&pkt_info);
                if( rval )
                {
                    printf("pack fail %d\n", rval);
                    step--; // retry
                    continue;
                }

                g_ctlr_opr.length = pkt_info.cur_pkt_len;
                g_ctlr_opr.port   = CONFIG_CONTROLLER_SINK_PORT;
                upg_send(&g_ctlr_opr);
            }
        #if 0
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
    pthread_t   t_gw, t_leaf;
    do {
        uint32_t    is_running = 1;

        rval = pthread_mutex_init(&g_log_mtx, 0);
        if( rval )   break;

        rval = pthread_mutex_init(&g_usr_mtx, 0);
        if( rval )   break;

        rval = pthread_cond_init(&g_usr_cond, 0);
        if( rval )   break;

        pthread_create(&t_gw, 0, _task_gateway, &is_running);

        pthread_mutex_lock(&g_usr_mtx);
        pthread_cond_wait(&g_usr_cond, &g_usr_mtx);
        pthread_mutex_unlock(&g_usr_mtx);

        pthread_create(&t_leaf, 0, _task_leaf_end, &is_running);

        pthread_mutex_lock(&g_usr_mtx);
        pthread_cond_wait(&g_usr_cond, &g_usr_mtx);
        pthread_mutex_unlock(&g_usr_mtx);

        common_init();
        //-----------------------
        {   // controller
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

        pthread_join(t_gw, 0);
        pthread_join(t_leaf, 0);
    } while(0);

    system("pause");
    return 0;
}
