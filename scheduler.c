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
//=============================================================================
//                  Constant Definition
//=============================================================================

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
    uint32_t            timeout;
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

        if( !pJob )
        {
            rval = -1;
            break;
        }

        len = sizeof(scheduler_job_list_t) + sizeof(scheduler_job_t) + sizeof(uint32_t) * pJob->destination_cnt;
        if( !(pNew = malloc(len)) )
        {
            rval = -2;
            break;
        }

        memset(pNew, 0x0, len);

        *pNew->pJob = *pJob;

        pNew->timeout = g_schedualer_tick + pJob->wait_time;
        pNew->uid     = job_uid;

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
                    pPrev->next = pNew;
                    pNew->next = pTail->next;
                    break;
                }

                pPrev = pTail;
                pTail = pTail->next;
            }

            if( !pTail )
                pPrev->next = pNew;
        }
        else
        {
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
scheduler_proc(void)
{
    int     rval = 0;
    do {
        scheduler_job_list_t    *pJob_list_act = 0;
        scheduler_job_list_t    *pJob_list_act_head = 0;
        scheduler_job_list_t    *pJob_list_cur = g_pScheduler_jobs_head;

        if( !g_pScheduler_jobs_head )
            break;

        // get active jobs
        pthread_mutex_lock(&g_schedualer_job_mtx);
        while( pJob_list_cur )
        {
            if( pJob_list_cur->timeout > g_schedualer_tick )
                break;

            if( pJob_list_act_head )
            {
                pJob_list_act->next = pJob_list_cur;
                pJob_list_act       = pJob_list_act->next;
            }
            else
            {
                pJob_list_act_head = pJob_list_cur;
                pJob_list_act      = pJob_list_cur;
            }

            pJob_list_cur = pJob_list_cur->next;
        }

        g_pScheduler_jobs_head = pJob_list_cur;

        pthread_mutex_unlock(&g_schedualer_job_mtx);

        // process jobs
        pJob_list_act = pJob_list_act_head;
        while( pJob_list_act )
        {
            scheduler_watcher_list_t    *pWatcher_cur = 0;
            scheduler_job_t             *pJob = 0;
            uint8_t                     *pJob_ctxt = 0;
            int                         job_ctxt_len = 256;

            pJob_list_cur = pJob_list_act;
            pJob_list_act = pJob_list_act->next;

            pJob = pJob_list_cur->pJob;

            rval = 0;

            // search target destination UID
            pthread_mutex_lock(&g_schedualer_mtx);

            pWatcher_cur = g_pScheduler_watchers_head;
            while( pWatcher_cur )
            {
                for(int i = 0; i < pJob->destination_cnt; i++)
                {
                    if( pWatcher_cur->pWatcher->watcher_uid != pJob->pDest_uid[i] )
                        continue;

                    if( !pJob_ctxt )
                    {
                        if( !(pJob_ctxt = malloc(job_ctxt_len)) )
                            break;

                        memset(pJob_ctxt, 0x0, job_ctxt_len);

                        if( pJob->cb_create_job_ctxt )
                        {
                            rval = pJob->cb_create_job_ctxt(pJob, pJob_ctxt, &job_ctxt_len);
                            if( rval < 0 ) break;
                        }
                    }

                    rval = rbi_push(pWatcher_cur->pWatcher->msgq, pJob_ctxt, job_ctxt_len);

                    break;
                }

                // drop this job
                if( rval ) break;

                pWatcher_cur = pWatcher_cur->next;
            }

            pthread_mutex_unlock(&g_schedualer_mtx);

            if( pJob_ctxt )
            {
                if( pJob->cb_destroy_job_ctxt )
                {
                    pJob->cb_destroy_job_ctxt(pJob, pJob_ctxt, &job_ctxt_len);
                }

                free(pJob_ctxt);
            }

            free(pJob_list_cur);
        }


    } while(0);
    return rval;
}
