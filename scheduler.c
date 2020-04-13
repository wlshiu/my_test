/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file scheduler.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/04/08
 * @license
 * @description
 */


#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "scheduler.h"
#include "sys_time.h"

#include "log.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
#define CONFIG_SCHEDULER_JOB_MAX_NUM        10
#define CONFIG_WATCHER_SHUFFLE_SIZE         100
//=============================================================================
//                  Macro Definition
//=============================================================================
#define ADD_NODE(struct_type, pHead, pCur, node_size, node_uid) \
    do {struct_type    *pNew = malloc(node_size);               \
        memset(pNew, 0x0, node_size);                           \
        *pNew->pJob   = *pCur;                                  \
        pNew->timeout = g_schedualer_tick + pCur->wait_time;    \
        pNew->uid     = node_uid;                               \
        if( pHead ) {                                           \
            struct_type    *pTail = pHead;                      \
            while( pTail ) {                                    \
                if( !pTail->next ) {                            \
                    pTail->next = pNew;                         \
                    break;                                      \
                }                                               \
                pTail = pTail->next;                            \
            }                                                   \
        }                                                       \
        else pHead = pNew;                                      \
    } while(0)


//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct scheduler_job_list
{
    struct scheduler_job_list   *next;
    uint32_t            uid;
    uint64_t            timeout;
    scheduler_job_t     pJob[];
} scheduler_job_list_t;

typedef struct scheduler_watcher_list
{
    struct scheduler_watcher_list   *next;
    scheduler_watcher_t     pWatcher[];
} scheduler_watcher_list_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================
static scheduler_job_list_t         *g_pScheduler_jobs_head = 0;
static scheduler_watcher_list_t     *g_pScheduler_watchers_head = 0;

static scheduler_watcher_t          **g_ppScheduler_watcher_shuffle = 0;

static uint32_t                     g_job_cnt = 0;

static uint64_t                     g_schedualer_tick = 0;
static pthread_mutex_t              g_schedualer_job_mtx;
static pthread_mutex_t              g_schedualer_mtx;

//=============================================================================
//                  Private Function Definition
//=============================================================================
static void
_time_observer(void *pInfo)
{
    g_schedualer_tick++;
    return;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
int
scheduler_init(
    uint32_t    time_quantum)
{
    int     rval = 0;

    do {
        g_pScheduler_jobs_head     = 0;
        g_pScheduler_watchers_head = 0;

        if( !(g_ppScheduler_watcher_shuffle
                = malloc(sizeof(scheduler_watcher_t*) * CONFIG_WATCHER_SHUFFLE_SIZE)) )
        {
            rval = -1;
            break;
        }

        memset(g_ppScheduler_watcher_shuffle, 0x0, sizeof(scheduler_watcher_t*) * CONFIG_WATCHER_SHUFFLE_SIZE);

        // scaling system time
        g_schedualer_tick = 0ull;
        sys_tmr_start(time_quantum, _time_observer, 0);

        pthread_mutex_init(&g_schedualer_job_mtx, 0);
        pthread_mutex_init(&g_schedualer_mtx, 0);
    } while(0);

    return rval;
}

void
scheduler_deinit(void)
{
    sys_tmr_stop();

    if( g_ppScheduler_watcher_shuffle )
        free(g_ppScheduler_watcher_shuffle);
    g_ppScheduler_watcher_shuffle = 0;

    pthread_mutex_destroy(&g_schedualer_job_mtx);
    pthread_mutex_destroy(&g_schedualer_mtx);
    return;
}


int
scheduler_add_job(
    scheduler_job_t     *pJob,
    uint32_t            *pJob_uid)
{
    static uint32_t     job_uid = 0;
    int                 rval = 0;

    /**
     *  the trigger time of jobs are relative with now
     */
    do {
        scheduler_job_list_t    *pNew = 0;
        uint32_t                len = 0;

        if( !pJob || g_job_cnt == CONFIG_SCHEDULER_JOB_MAX_NUM )
        {
            rval = -1;
            break;
        }

        len = (pJob->destination_cnt == SCHEDULER_DESTINATION_ALL)
            ? sizeof(scheduler_job_list_t) + sizeof(scheduler_job_t)
            : sizeof(scheduler_job_list_t) + sizeof(scheduler_job_t) + sizeof(uint32_t) * pJob->destination_cnt;
        if( !(pNew = malloc(len)) )
        {
            rval = -2;
            break;
        }

        memset(pNew, 0x0, len);

        memcpy(pNew->pJob, pJob,
               (pJob->destination_cnt == SCHEDULER_DESTINATION_ALL)
                ? sizeof(scheduler_job_t)
                : sizeof(scheduler_job_t) + sizeof(uint32_t) * pJob->destination_cnt);

        pNew->timeout = g_schedualer_tick + pJob->wait_time;
        pNew->uid     = job_uid;

        pNew->pJob->job_serial_number = pNew->uid;

        pthread_mutex_lock(&g_schedualer_job_mtx);

        if( g_pScheduler_jobs_head )
        {
            scheduler_job_list_t    *pTail = g_pScheduler_jobs_head;
            scheduler_job_list_t    *pPrev = 0;

            while( pTail )
            {
                // sort by timeout
                if( pTail->timeout > pNew->timeout )
                {
                    g_job_cnt++;

                    if( pPrev )
                    {
                        pPrev->next = pNew;
                        pNew->next  = pTail;
                    }
                    else
                    {
                        pNew->next = pTail;
                        g_pScheduler_jobs_head = pNew;
                    }
                    break;
                }

                pPrev = pTail;
                pTail = pTail->next;
            }

            if( !pTail )
            {
                g_job_cnt++;
                pPrev->next = pNew;
            }
        }
        else
        {
            g_job_cnt++;
            g_pScheduler_jobs_head = pNew;
        }

        pthread_mutex_unlock(&g_schedualer_job_mtx);
    } while(0);

    if( pJob_uid ) *pJob_uid = job_uid;

    job_uid++;

    return rval;
}

int
scheduler_del_job(
    uint32_t    job_uid)
{
    int     rval = 0;

    pthread_mutex_unlock(&g_schedualer_job_mtx);

    do {
        scheduler_job_list_t    *pTail = g_pScheduler_jobs_head;
        scheduler_job_list_t    *pPrev = 0;

        if( !g_pScheduler_jobs_head )
            break;

        while( pTail )
        {
            if( pTail->uid == job_uid )
            {
                if( !pPrev )
                    g_pScheduler_jobs_head = 0;
                else
                    pPrev->next = pTail->next;

                free(pTail);
                break;
            }
            pPrev = pTail;
            pTail = pTail->next;
        }

    } while(0);

    pthread_mutex_unlock(&g_schedualer_job_mtx);
    return rval;
}

int
scheduler_register_watcher(
    scheduler_watcher_t     *pWatcher)
{
    int     rval = 0;

    pthread_mutex_lock(&g_schedualer_mtx);
    do {
        scheduler_watcher_list_t    *pNew = 0;

        if( !(pNew = malloc(sizeof(scheduler_watcher_list_t) + sizeof(scheduler_watcher_t))) )
        {
            rval = -1;
            break;
        }
        memset(pNew, 0x0, sizeof(scheduler_watcher_list_t) + sizeof(scheduler_watcher_t));
        *pNew->pWatcher = *pWatcher;

        if( g_pScheduler_watchers_head )
        {
            scheduler_watcher_list_t    *pTail = g_pScheduler_watchers_head;
            while( pTail )
            {
                if( !pTail->next )
                {
                    pTail->next = pNew;
                    break;
                }
                pTail = pTail->next;
            }
        }
        else
        {
            g_pScheduler_watchers_head = pNew;
        }
    } while(0);

    pthread_mutex_unlock(&g_schedualer_mtx);
    return rval;
}

int
scheduler_unregister_watcher(
    uint32_t    watcher_uid)
{
    int     rval = 0;

    pthread_mutex_lock(&g_schedualer_mtx);

    do {
        scheduler_watcher_list_t    *pTail = g_pScheduler_watchers_head;
        scheduler_watcher_list_t    *pPrev = 0;

        if( !g_pScheduler_watchers_head )
            break;

        while( pTail )
        {
            if( pTail->pWatcher->watcher_uid == watcher_uid )
            {
                if( !pPrev )
                    g_pScheduler_watchers_head = 0;
                else
                    pPrev->next = pTail->next;

                free(pTail);
                break;
            }

            pPrev = pTail;
            pTail = pTail->next;
        }
    } while(0);

    pthread_mutex_unlock(&g_schedualer_mtx);
    return rval;
}

int
scheduler_proc(
    cb_watcher_routine_t  cb_watcher_routine)
{
    int     rval = 0;
    do {
        scheduler_job_list_t    *pJob_list_act = 0;
        scheduler_job_list_t    *pJob_list_act_head = 0;
        scheduler_job_list_t    *pJob_list_cur = 0;

        // get active jobs
        pthread_mutex_lock(&g_schedualer_job_mtx);

        if( !g_pScheduler_jobs_head )
        {
            pthread_mutex_unlock(&g_schedualer_job_mtx);
            break;
        }

        pJob_list_cur = g_pScheduler_jobs_head;

        while( pJob_list_cur )
        {
            if( pJob_list_cur->timeout > g_schedualer_tick )
                break;

            if( pJob_list_act_head )
            {
                g_job_cnt--;

                pJob_list_act->next = pJob_list_cur;
                pJob_list_act       = pJob_list_act->next;
            }
            else
            {
                g_job_cnt--;

                pJob_list_act_head = pJob_list_cur;
                pJob_list_act      = pJob_list_cur;
            }

            pJob_list_cur = pJob_list_cur->next;

            pJob_list_act->next = 0;
        }

        g_pScheduler_jobs_head = pJob_list_cur;

        pthread_mutex_unlock(&g_schedualer_job_mtx);

        /**
         *  process jobs
         *  One job SHOULD send to multi-watchers (job order issue)
         */

        memset(g_ppScheduler_watcher_shuffle, 0x0, sizeof(scheduler_watcher_t*) * CONFIG_WATCHER_SHUFFLE_SIZE);

        pJob_list_act = pJob_list_act_head;
        while( pJob_list_act )
        {
            scheduler_job_t     *pJob = 0;
            uint8_t             *pJob_ctxt = 0;
            int                 job_ctxt_len = 0;
            int                 total_watcher = 0;

            pJob_list_cur = pJob_list_act;
            pJob_list_act = pJob_list_act->next;

            pJob = pJob_list_cur->pJob;

            if( pJob->destination_cnt == -1 )
            {
                scheduler_watcher_list_t    *pWatcher_cur = 0;

                pthread_mutex_lock(&g_schedualer_mtx);

                pWatcher_cur = g_pScheduler_watchers_head;
                while( pWatcher_cur )
                {
                    if( !pJob_ctxt && pJob->cb_create_job_ctxt )
                    {
                        rval = pJob->cb_create_job_ctxt(pJob, &pJob_ctxt, &job_ctxt_len);
                        if( rval < 0 ) break;
                    }

                    rbi_push(pWatcher_cur->pWatcher->msgq, pJob_ctxt, job_ctxt_len);

                    g_ppScheduler_watcher_shuffle[total_watcher++] = pWatcher_cur->pWatcher;
                    if( total_watcher == CONFIG_WATCHER_SHUFFLE_SIZE )
                    {
                        rval = -3;
                        break;
                    }

                    pWatcher_cur = pWatcher_cur->next;
                }
                pthread_mutex_unlock(&g_schedualer_mtx);
            }
            else
            {
                for(int i = 0; i < pJob->destination_cnt; i++)
                {
                    scheduler_watcher_list_t    *pWatcher_cur = 0;

                    // search target destination UID
                    pthread_mutex_lock(&g_schedualer_mtx);

                    pWatcher_cur = g_pScheduler_watchers_head;
                    while( pWatcher_cur )
                    {
                        if( pWatcher_cur->pWatcher->watcher_uid == pJob->pDest_uid[i] )
                        {
                            if( !pJob_ctxt && pJob->cb_create_job_ctxt )
                            {
                                rval = pJob->cb_create_job_ctxt(pJob, &pJob_ctxt, &job_ctxt_len);
                                if( rval < 0 ) break;
                            }

                            rbi_push(pWatcher_cur->pWatcher->msgq, pJob_ctxt, job_ctxt_len);

                            g_ppScheduler_watcher_shuffle[total_watcher++] = pWatcher_cur->pWatcher;
                            if( total_watcher == CONFIG_WATCHER_SHUFFLE_SIZE )
                            {
                                rval = -3;
                                break;
                            }

                            break;
                        }

                        pWatcher_cur = pWatcher_cur->next;
                    }

                    pthread_mutex_unlock(&g_schedualer_mtx);

                    // drop this job
                    if( rval ) break;
                }
            }

            if( pJob_ctxt && pJob->cb_destroy_job_ctxt )
            {
                pJob->cb_destroy_job_ctxt(pJob, &pJob_ctxt, &job_ctxt_len);
            }

            free(pJob_list_cur);

            /**
             *  All watchers process the current job with one thread
             */
            if( cb_watcher_routine )
            {
                // shuffle order
                srand(time(NULL));
                for(int i = 0; i < total_watcher; i++)
                {
                    int                     j = rand() % total_watcher;
                    scheduler_watcher_t     *a = g_ppScheduler_watcher_shuffle[j];

                    g_ppScheduler_watcher_shuffle[j] = g_ppScheduler_watcher_shuffle[i];
                    g_ppScheduler_watcher_shuffle[i] = (scheduler_watcher_t*)a;
                }

                // trigger watcher routine
                for(int i = 0; i < CONFIG_WATCHER_SHUFFLE_SIZE; i++)
                {
                    if( !g_ppScheduler_watcher_shuffle[i] )
                        continue;

                    cb_watcher_routine(g_ppScheduler_watcher_shuffle[i]);
                }
            }
        }

    } while(0);
    return rval;
}
