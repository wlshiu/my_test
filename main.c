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
#include "sys_time.h"
#include "scheduler.h"
#include "usr_ev_maker.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
//#define CONFIG_ENABLE_MULTI_THREAD

#define CONFIG_EV_MSG_SIZE              128
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
static node_attr_t          *g_pNode_attr = 0;
static pthread_cond_t       g_usr_cond;
static pthread_mutex_t      g_usr_mtx;

static usr_ev_script_t      g_ev_scenario[] =
{
    { .time_offset = 123, .pf_make_event = usr_ev_discovery, },
    { .time_offset = 1550, .pf_make_event = usr_ev_discovery, },
    { .time_offset = 3000, .pf_make_event = usr_ev_discovery, },
    { .time_offset = -1, },
};

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

#if defined(CONFIG_ENABLE_MULTI_THREAD)
static void*
_task_node(void *argv)
{
    int             rval = 0;
    uint8_t         *pEv_msg = 0;
    node_args_t     node_info = {0};

    do {
        node_attr_t             *pAttr = (node_attr_t*)argv;
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

        scheduler_register_watcher(&watcher);

        pthread_mutex_lock(&g_usr_mtx);
        pthread_cond_signal(&g_usr_cond);
        pthread_mutex_unlock(&g_usr_mtx);

        node_info.node_id        = pAttr->node_id;
        node_info.buf_rx.buf_len = CONFIG_VPHY_MSG_SIZE;
        node_info.buf_tx.buf_len = CONFIG_VPHY_MSG_SIZE - 32;

        vphy_register_event_callback(_phy_recv, &node_info);

        log_out("node id %d (tid= x%08x)\n", pAttr->node_id, pthread_self());

        while( pAttr->is_running )
        {
            int     bytes = 0;

            bytes = rbi_pop(watcher.msgq, pEv_msg, CONFIG_EV_MSG_SIZE);
            if( bytes )
            {
                #if 1
                log_out("[node %02d-th] cmd msg='%s'\n", pAttr->node_id, pEv_msg);
                #else
                memset((void*)node_info.buf_tx.pBuf, 0x0, node_info.buf_tx.buf_len);
                snprintf((char*)node_info.buf_tx.pBuf, node_info.buf_tx.buf_len, "src node=%d", pAttr->node_id);

                rval = vphy_send((uint8_t*)node_info.buf_tx.pBuf, strlen((char*)node_info.buf_tx.pBuf) + 1);
                if( rval )
                {
                    log_out("=> node id %d (tid= x%08x) sent fail (%d)\n", pAttr->node_id, pthread_self(), rval);
                }
                #endif
            }

            // TODO: handle receive data

            Sleep(10);
        }

    } while(0);

    if( node_info.buf_rx.pBuf )    free(node_info.buf_rx.pBuf);
    if( node_info.buf_tx.pBuf )    free(node_info.buf_tx.pBuf);
    if( pEv_msg )                  free(pEv_msg);

    pthread_exit(0);
    return 0;
}
#else
static void
_node_routine(scheduler_watcher_t *pWatcher)
{
    static uint8_t      cmd_msg[CONFIG_EV_MSG_SIZE] = {0};
    do {
        uint8_t     *pCmd_msg = cmd_msg;
        int         cmd_msg_len = sizeof(cmd_msg);
        uint32_t    bytes = 0;

        memset(pCmd_msg, 0x0, cmd_msg_len);
        bytes = rbi_pop(pWatcher->msgq, pCmd_msg, cmd_msg_len);
        if( !bytes )    break;

        pCmd_msg[cmd_msg_len - 1] = '\0';
        log_out("[watcher_%d] rx: -%s-\n", pWatcher->watcher_uid, pCmd_msg);
    } while(0);

    return;
}
#endif

static void*
_task_schedule(void *argv)
{
    int     *pIs_running = (int*)argv;

    pthread_mutex_lock(&g_usr_mtx);
    pthread_cond_signal(&g_usr_cond);
    pthread_mutex_unlock(&g_usr_mtx);

    while( *pIs_running )
    {
        #if defined(CONFIG_ENABLE_MULTI_THREAD)
        scheduler_proc(0);
        Sleep(3);
        #else
        scheduler_proc(_node_routine);
        #endif
    }

    pthread_exit(0);
    return 0;
}

static int
_create_job_ctxt(scheduler_job_t *pJob, uint8_t **ppCtxt, int *pCtxt_len)
{
    int     rval = 0;
    do {
        usr_ev_script_t     *pEv_script_act = (usr_ev_script_t*)pJob->pExtra_data;
        int                 ctxt_len = CONFIG_EV_MSG_SIZE;
        char                *pCtxt = 0;

        if( !(pCtxt = malloc(ctxt_len)) )
        {
            break;
        }
        memset(pCtxt, 0x0, ctxt_len);

        if( pEv_script_act->pf_make_event )
            pEv_script_act->pf_make_event(pEv_script_act, pCtxt, &ctxt_len);

        *ppCtxt    = (uint8_t*)pCtxt;
        *pCtxt_len = ctxt_len;
    } while(0);

    return rval;
}

static int
_destroy_job_ctxt(scheduler_job_t *pJob, uint8_t **ppCtxt, int *pCtxt_len)
{
    char    *pCtxt = (char*)(*ppCtxt);

    if( pCtxt )     free(pCtxt);

    *ppCtxt = 0;
    return 0;
}

static int
_trigger_event(uint32_t time_offset)
{
    int             rval = 0;
    do {
        usr_ev_script_t     *pEv_script_cur = g_ev_scenario;

        while( pEv_script_cur->time_offset != -1 )
        {
            if( pEv_script_cur->time_offset < time_offset &&
                !pEv_script_cur->is_acted )
            {
                uint32_t            certificate = (uint32_t)-1;
                uint32_t            dest_cnt = SCHEDULER_DESTINATION_ALL;
                scheduler_job_t     *pJob = 0;
                uint32_t            len = 0;

                // add a job
                len = (dest_cnt == SCHEDULER_DESTINATION_ALL)
                    ? sizeof(scheduler_job_t)
                    : sizeof(scheduler_job_t) + sizeof(uint32_t) * dest_cnt;
                if( !(pJob = malloc(len)) )
                {
                    dbg_msg("fail\n");
                    break;
                }
                memset(pJob, 0x0, len);

                // TODO: wait_time and destination list SHULD be assigned
                pJob->ev_type               = SCHEDULER_EV_MODE_INTERVAL;
                pJob->wait_time             = 0; // real waiting time = 2 * time_scaling (ms)
                pJob->src_uid               = pthread_self();
                pJob->cb_create_job_ctxt    = _create_job_ctxt;
                pJob->cb_destroy_job_ctxt   = _destroy_job_ctxt;
                pJob->pExtra_data           = pEv_script_cur;
                pJob->destination_cnt       = dest_cnt;

                if( dest_cnt != -1 )
                {
                    for(int j = 0; j < dest_cnt; j++)
                    {
                        pJob->pDest_uid[j] = g_pNode_attr[j].node_id;
                    }
                }

                rval = scheduler_add_job(pJob, &certificate);
                if( rval )
                    printf("job full...\n");

                free(pJob);

                pEv_script_cur->is_acted = 1;
            }
            pEv_script_cur++;
        }

    } while(0);
    return rval;
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
        int         rval = 0;
        pthread_t   tscheduler;
        int         is_running = 1;
        uint32_t    time_scaling = 10;
        sys_tmr_t   hSys_tm = 0;

        if( !(g_pNode_attr = malloc(sizeof(node_attr_t) * node_number)) )
        {
            break;
        }
        memset(g_pNode_attr, 0x0, sizeof(node_attr_t) * node_number);

        vphy_init();

        scheduler_init(time_scaling);

        rval = pthread_mutex_init(&g_usr_mtx, 0);
        if( rval )   break;

        rval = pthread_cond_init(&g_usr_cond, 0);
        if( rval )   break;

#if defined(CONFIG_ENABLE_MULTI_THREAD)
        for(int i = 0; i < node_number; i++)
        {
            g_pNode_attr[i].is_running = 1;
            g_pNode_attr[i].node_id    = i;
            pthread_create(&g_pNode_attr[i].tid, 0, _task_node, &g_pNode_attr[i]);

            pthread_mutex_lock(&g_usr_mtx);
            pthread_cond_wait(&g_usr_cond, &g_usr_mtx);
            pthread_mutex_unlock(&g_usr_mtx);
        }
#else
        for(int i = 0; i < node_number; i++)
        {
            scheduler_watcher_t     watcher = {0};

            watcher.watcher_uid = i;

            watcher.msgq = rbi_init(4, CONFIG_EV_MSG_SIZE);
            if( !watcher.msgq )
            {
                dbg_msg("fail\n");
                break;
            }

            scheduler_register_watcher(&watcher);
        }
#endif

        {   // thread scheduler
            pthread_attr_t          attr;
            struct sched_param      param;

            // set thread priority
            param.sched_priority = sched_get_priority_max(SCHED_OTHER);

            pthread_attr_init(&attr);
            pthread_attr_setschedparam(&attr, &param);
            pthread_create(&tscheduler, &attr, _task_schedule, &is_running);

            pthread_mutex_lock(&g_usr_mtx);
            pthread_cond_wait(&g_usr_cond, &g_usr_mtx);
            pthread_mutex_unlock(&g_usr_mtx);
        }

        hSys_tm = sys_tmr_get_time();

        while(1)
        {
            uint32_t        duration = 0ul;

            duration = sys_tmr_get_duration(hSys_tm);
            #if 0
            if( duration > 10000 )
            {
                for(int i = 0; i < node_number; i++)
                {
                    g_pNode_attr[i].is_running = 0;
                }

                is_running = 0;
                break;
            }
            #endif

            _trigger_event(duration);

            Sleep(time_scaling);
        }

        for(int i = 0; i < node_number; i++)
        {
            pthread_join(g_pNode_attr[i].tid, 0);
        }

        scheduler_deinit();
        vphy_deinit();

    } while(0);

    return 0;
}
