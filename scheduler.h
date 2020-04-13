/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file scheduler.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/04/08
 * @license
 * @description
 */

#ifndef __scheduler_H_woP5WlUm_ljsb_HvVu_sBda_ujX9QbJ5hXGQ__
#define __scheduler_H_woP5WlUm_ljsb_HvVu_sBda_ujX9QbJ5hXGQ__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "rbi.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
typedef enum scheduler_ev
{
    SCHEDULER_EV_MODE_INTERVAL   = 0x12, // once
    SCHEDULER_EV_MODE_PERIODIC,
} scheduler_ev_t;
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct scheduler_job
{
    scheduler_ev_t      ev_type;
    uint32_t            wait_time;  // waiting time (depend on time_quantum) to execute a job

    uint32_t            job_serial_number; // for debug

    /**
     *  @brief  cb_create_job_ctxt
     *              create the context of this job to send to watchers
     *
     *  @param [in] pJob            job info
     *  @param [in] ppCtxt          the buffer of the context of a job
     *  @param [in] pCtxt_len       the buffer length of the context of a job
     *  @return
     *      0: ok, others: fail
     */
    int (*cb_create_job_ctxt)(struct scheduler_job *pJob, uint8_t **ppCtxt, int *pCtxt_len);

    /**
     *  @brief  cb_destroy_job_ctxt
     *              destroy the context of this job
     *
     *  @param [in] pJob            job info
     *  @param [in] ppCtxt          the buffer of the context of a job
     *  @param [in] pCtxt_len       the buffer length of the context of a job
     *  @return
     *      0: ok, others: fail
     */
    int (*cb_destroy_job_ctxt)(struct scheduler_job *pJob, uint8_t **ppCtxt, int *pCtxt_len);

    void                *pExtra_data;

    uint32_t            src_uid;

    /**
     *  when destination_cnt == SCHEDULER_DESTINATION_ALL,
     *  scheduler will broadcast this job to all watchers
     */
#define SCHEDULER_DESTINATION_ALL       -1
    uint32_t            destination_cnt;
    uint32_t            pDest_uid[];
} scheduler_job_t;

typedef struct scheduler_watcher
{
    uint32_t    watcher_uid;

    rbi_t       msgq; // It MUST be initialized before register to the scheduler

    void        *pUsr_data;

} scheduler_watcher_t;

typedef void (*cb_watcher_routine_t)(scheduler_watcher_t *pWatcher);
//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
/**
 *  @brief  scheduler_init
 *
 *  @param [in] time_quantum        time quantum for time scaling (unit: ms)
 *  @return
 *      0: ok, others: fail
 */
int
scheduler_init(
    uint32_t    time_quantum);


void
scheduler_deinit(void);


/**
 *  @brief  scheduler_get_tick
 *              the ticks depend on  time_quantum of scheduler_init
 *  @return
 *      the ticks of scheduler
 */
uint32_t
scheduler_get_tick(void);


/**
 *  @brief  scheduler_add_job
 *              this job will be executed after (now + pJob->wait_time)
 *
 *  @param [in] pJob        target job info
 *  @param [in] pJob_uid    return the uid of a job in this scheduler
 *  @return
 *      0: ok, others: fail
 */
int
scheduler_add_job(
    scheduler_job_t     *pJob,
    uint32_t            *pJob_uid);


int
scheduler_del_job(
    uint32_t    job_uid);


/**
 *  @brief  scheduler_register_watcher
 *              Register a watcher to scheduler
 *
 *  @param [in] pWatcher        target watcher info
 *  @return
 *      error code
 */
int
scheduler_register_watcher(
    scheduler_watcher_t     *pWatcher);


int
scheduler_unregister_watcher(
    uint32_t    watcher_uid);


/**
 *  @brief  scheduler_proc
 *              main process flow of scheduler
 *  @param [in] cb_watcher_routine      the routine of watchers
 *                                      - This callback is used at one thread case.
 *                                        The routine is immediately called with watcher info after scheduler processing
 *  @return
 *      error code
 */
int
scheduler_proc(
    cb_watcher_routine_t  cb_watcher_routine);


#ifdef __cplusplus
}
#endif

#endif
