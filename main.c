/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file main.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/04/07
 * @license
 * @description
 */


#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "virtual_phy.h"
#include "log.h"
#include "rbi.h"
#include "scheduler.h"

//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct buf
{
    uint32_t    *pBuf;
    uint32_t    buf_len;

} buf_t;

typedef struct node_args
{
    int         node_id;
    buf_t       buf_rx;
    buf_t       buf_tx;

    void        *pRoute_table;
} node_args_t;

typedef struct node_attr
{
    pthread_t   tid;
    uint32_t    node_id;

    uint32_t    is_running;

} node_attr_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================
static node_attr_t      *g_pNode_attr = 0;
static pthread_cond_t   g_usr_cond;
static pthread_mutex_t  g_usr_mtx;
//=============================================================================
//                  Private Function Definition
//=============================================================================
static void
_phy_recv(
    vphy_dataq_t    *pDataq)
{
    int     bytes = 0;
    do {
        node_args_t     *pNode_info = (node_args_t*)pDataq->pUser_data;
        vphy_package_t  *pPackage = 0;

        bytes = rbi_pop(pDataq->dataq,
                       (uint8_t*)pNode_info->buf_rx.pBuf,
                       (int)pNode_info->buf_rx.buf_len);
        if( !bytes )  break;

        pPackage = (vphy_package_t*)pNode_info->buf_rx.pBuf;

        log_out("tid= %08x: rx= %s\n", pPackage->src_tid, pPackage->data);

    } while(0);

    return;
}

static void*
_task_node(void *argv)
{
#define CONFIG_EV_MSG_SIZE      128
    int             rval = 0;
    uint8_t         *pEv_msg = 0;
    node_args_t     node_info = {0};

    do {
        int                     node_id = *((int*)argv);
        scheduler_watcher_t     watcher = { .watcher_uid = pthread_self(), };

        if( !(node_info.buf_rx.pBuf = malloc(CONFIG_VPHY_MSG_SIZE)) )
            break;

        if( !(node_info.buf_tx.pBuf = malloc((CONFIG_VPHY_MSG_SIZE - 32))) )
            break;

        if( !(pEv_msg = malloc(CONFIG_EV_MSG_SIZE)) )
            break;

        watcher.msgq = rbi_init(4, CONFIG_VPHY_MSG_SIZE);
        if( !watcher.msgq )
        {
            break;
        }


        pthread_mutex_lock(&g_usr_mtx);
        pthread_cond_signal(&g_usr_cond);
        pthread_mutex_unlock(&g_usr_mtx);

        node_info.node_id        = node_id;
        node_info.buf_rx.buf_len = CONFIG_VPHY_MSG_SIZE;
        node_info.buf_tx.buf_len = CONFIG_VPHY_MSG_SIZE - 32;

        vphy_register_event_callback(_phy_recv, &node_info);

        log_out("node id %d (tid= x%08x)\n", node_id, pthread_self());

        while(1)
        {
            int     bytes = 0;

            bytes = rbi_pop(watcher.msgq, pEv_msg, CONFIG_EV_MSG_SIZE);
            if( bytes )
            {
                memset((void*)node_info.buf_tx.pBuf, 0x0, node_info.buf_tx.buf_len);
                snprintf((char*)node_info.buf_tx.pBuf, node_info.buf_tx.buf_len, "src node=%d", node_id);

                rval = vphy_send((uint8_t*)node_info.buf_tx.pBuf, strlen((char*)node_info.buf_tx.pBuf) + 1);
                if( rval )
                {
                    log_out("=> node id %d (tid= x%08x) sent fail (%d)\n", node_id, pthread_self(), rval);
                }
            }

            // TODO: handle receive data

        }

    } while(0);

    if( node_info.buf_rx.pBuf )    free(node_info.buf_rx.pBuf);
    if( node_info.buf_tx.pBuf )    free(node_info.buf_tx.pBuf);
    if( pEv_msg )                  free(pEv_msg);

    pthread_exit(0);
    return 0;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
static void
_usage(char *pProgram)
{
    printf("%s usage:\n"
           "    -n      node number\n"
           , pProgram);
    system("pause");
    exit(-1);
}

int main(int argc, char **argv)
{
    int     node_number = 0;

    /*
     * Read through command-line arguments for options.
     */
    for(int i = 1; i < argc; i++)
    {
        if( argv[i][0] == '-' )
        {
            if (argv[i][1] == 'n')
            {
                node_number = atoi(argv[i + 1]);
                i++;
            }
            else
            {
                printf("Invalid option.\n");
                _usage(argv[0]);
                return 2;
            }
        }
    }

    do {
        int     rval = 0;
        if( !(g_pNode_attr = malloc(sizeof(node_attr_t) * node_number)) )
        {
            break;
        }
        memset(g_pNode_attr, 0x0, sizeof(node_attr_t) * node_number);

        vphy_init();

        rval = pthread_mutex_init(&g_usr_mtx, 0);
        if( rval )   break;

        rval = pthread_cond_init(&g_usr_cond, 0);
        if( rval )   break;

        for(int i = 0; i < node_number; i++)
        {
            g_pNode_attr[i].is_running = 1;
            g_pNode_attr[i].node_id    = i;
            pthread_create(&g_pNode_attr[i].tid, 0, _task_node, &g_pNode_attr[i]);

            pthread_mutex_lock(&g_usr_mtx);
            pthread_cond_wait(&g_usr_cond, &g_usr_mtx);
            pthread_mutex_unlock(&g_usr_mtx);
        }

        while(1)
            __asm("nop");

        for(int i = 0; i < node_number; i++)
        {
            pthread_join(g_pNode_attr[i].tid, 0);
        }

        vphy_deinit();

    } while(0);

    return 0;
}