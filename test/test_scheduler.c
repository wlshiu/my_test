/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file test_scheduler.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/04/08
 * @license
 * @description
 */


#include <windows.h>
#include <stdio.h>
#include <pthread.h>

#include "sys_time.h"
#include "log.h"
#include "scheduler.h"

//=============================================================================
//                  Constant Definition
//=============================================================================
#define CONFIG_WATCHER_NUM                  2
#define CONFIG_CMD_MSG_SIZE                 64
#define CONFIG_TEST_DURATION_MS             5000
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct watcher_attr
{
    pthread_t       tid;
    int             is_running;

    uint32_t        watcher_id;

} watcher_attr_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================
static watcher_attr_t       g_watcher_attr[CONFIG_WATCHER_NUM] = {{.is_running = 0,},};
static pthread_cond_t       g_usr_cond;
static pthread_mutex_t      g_usr_mtx;
//=============================================================================
//                  Private Function Definition
//=============================================================================

static void*
_task_schedule(void *argv)
{
    int                     *pIs_running = (int*)argv;

    pthread_mutex_lock(&g_usr_mtx);
    pthread_cond_signal(&g_usr_cond);
    pthread_mutex_unlock(&g_usr_mtx);

    while( *pIs_running )
    {
        scheduler_proc();
    }

    pthread_exit(0);
    return 0;
}

static void*
_task_watcher(void *argv)
{
    watcher_attr_t          *pAttr = (watcher_attr_t*)argv;
    scheduler_watcher_t     watcher = { .watcher_uid = (uint32_t)-1, };
    uint8_t                 *pCmd_msg = 0;
    int                     cmd_msg_len = CONFIG_CMD_MSG_SIZE;

    watcher.watcher_uid = pAttr->watcher_id;

    if( !(pCmd_msg = malloc(cmd_msg_len)) )
    {
        pthread_exit(0);
        return 0;
    }

    do {
        watcher.msgq = rbi_init(4, CONFIG_CMD_MSG_SIZE);
        if( !watcher.msgq ) break;

        scheduler_register_watcher(&watcher);

        pthread_mutex_lock(&g_usr_mtx);
        pthread_cond_signal(&g_usr_cond);
        pthread_mutex_unlock(&g_usr_mtx);

        while( pAttr->is_running )
        {
            uint32_t    bytes = 0;

            bytes = rbi_pop(watcher.msgq, pCmd_msg, cmd_msg_len);
            if( !bytes )
            {
                Sleep(5);
                continue;
            }

            pCmd_msg[cmd_msg_len - 1] = '\0';
            printf("[watcher_%d] rx: -%s-\n", watcher.watcher_uid, pCmd_msg);
        }
    } while(0);

    if( pCmd_msg )  free(pCmd_msg);

    pthread_exit(0);
    return 0;
}

static int
_create_job_ctxt(scheduler_job_t *pJob, uint8_t **ppCtxt, int *pCtxt_len)
{
    do {
        int     ctxt_len = CONFIG_CMD_MSG_SIZE;
        char    *pCtxt = 0;

        if( !(pCtxt = malloc(ctxt_len)) )
        {
            break;
        }
        memset(pCtxt, 0x0, ctxt_len);

        snprintf((char*)pCtxt, ctxt_len, "src=0x%x, dest=watcher_%d\n", pJob->src_uid, *pJob->pDest_uid);

        *ppCtxt    = (uint8_t*)pCtxt;
        *pCtxt_len = ctxt_len;
    } while(0);

    return 0;
}

static int
_destroy_job_ctxt(scheduler_job_t *pJob, uint8_t **ppCtxt, int *pCtxt_len)
{
    char    *pCtxt = (char*)(*ppCtxt);

    if( pCtxt )     free(pCtxt);

    *ppCtxt = 0;
    return 0;
}

static void
_log_job(scheduler_job_t *pJob)
{
    printf("job->ev         = x%x\n", pJob->ev_type);
    printf("job->wait_time  = %d\n", pJob->wait_time);
    printf("job->src_uid    = %x\n", pJob->src_uid);
    printf("job->destination_cnt = %x\n", pJob->destination_cnt);
    for(int i = 0; i < pJob->destination_cnt; i++)
        printf("job->dest   = %d\n", pJob->pDest_uid[i]);
    return;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
int main(int argc, char **argv)
{
    int     rval = 0;
    do {
        pthread_t   tscheduler;
        int         is_running = 1;
        uint32_t    time_scaling = 100;
        sys_tmr_t   hSys_tm = 0;

        scheduler_init(time_scaling);

        hSys_tm = sys_tmr_get_time();

        rval = pthread_mutex_init(&g_usr_mtx, 0);
        if( rval )   break;

        rval = pthread_cond_init(&g_usr_cond, 0);
        if( rval )   break;

        pthread_create(&tscheduler, 0, _task_schedule, &is_running);
        pthread_mutex_lock(&g_usr_mtx);
        pthread_cond_wait(&g_usr_cond, &g_usr_mtx);
        pthread_mutex_unlock(&g_usr_mtx);

        for(int i = 0; i < CONFIG_WATCHER_NUM; i++)
        {
            g_watcher_attr[i].is_running = 1;
            g_watcher_attr[i].watcher_id = i;
            pthread_create(&g_watcher_attr[i].tid, 0, _task_watcher, &g_watcher_attr[i]);

            pthread_mutex_lock(&g_usr_mtx);
            pthread_cond_wait(&g_usr_cond, &g_usr_mtx);
            pthread_mutex_unlock(&g_usr_mtx);
        }

        while(1)
        {
            uint32_t        duration = 0ul;

            duration = sys_tmr_get_duration(hSys_tm);
        #if 0
            if( duration > CONFIG_TEST_DURATION_MS )
            {
                is_running = 0;
                break;
            }
            else if( duration > (CONFIG_TEST_DURATION_MS >> 1) )
            {
                static int      is_once = 0;

                if( is_once ) continue;

                // TODO: add a job
            }
            else if( duration > (CONFIG_TEST_DURATION_MS >> 2) )
        #endif
            {
                static int          is_once = 0;
                uint32_t            certificate = (uint32_t)-1;
                uint32_t            dest_cnt = CONFIG_WATCHER_NUM;
                scheduler_job_t     *pJob = 0;
                uint32_t            len = 0;

                if( is_once ) continue;

//                is_once = 1;

                // add a job
                len = sizeof(scheduler_job_t) + sizeof(uint32_t) * dest_cnt;
                if( !(pJob = malloc(len)) )
                {
                    printf("[%s:%d] fail\n", __func__, __LINE__);
                    break;
                }
                memset(pJob, 0x0, len);

                pJob->ev_type               = SCHEDULER_EV_MODE_INTERVAL;
                pJob->wait_time             = 2; // real waiting time = 2 * time_scaling (ms)
                pJob->src_uid               = pthread_self();
                pJob->cb_create_job_ctxt    = _create_job_ctxt;
                pJob->cb_destroy_job_ctxt   = _destroy_job_ctxt;
                pJob->destination_cnt       = dest_cnt;
                for(int j = 0; j < dest_cnt; j++)
                {
                    pJob->pDest_uid[j] = g_watcher_attr[j].watcher_id;
                }

                printf("tid(0x%x) add job\n", pJob->src_uid);

                scheduler_add_job(pJob, &certificate);

                free(pJob);
            }
        }

        pthread_join(tscheduler, 0);
        pthread_join(g_watcher_attr[0].tid, 0);
        pthread_join(g_watcher_attr[1].tid, 0);
    } while(0);

    scheduler_deinit();

    system("pause");
    return 0;
}
