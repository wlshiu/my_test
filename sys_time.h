/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file sys_time.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/04/08
 * @license
 * @description
 */

#ifndef __sys_time_H_wu4srbD6_lHQG_Htim_sfQm_u3mu4KKgeWAb__
#define __sys_time_H_wu4srbD6_lHQG_Htim_sfQm_u3mu4KKgeWAb__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef void*   sys_tmr_t;
typedef void    (*cb_sys_tmr_alarm_t)(void *pUser_data);
//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
int
sys_tmr_start(
    uint32_t            time_interval_ms,
    cb_sys_tmr_alarm_t  cb_alarm,
    void                *pUser_data);


void
sys_tmr_stop(void);


sys_tmr_t
sys_tmr_get_time(void);


uint32_t
sys_tmr_get_duration(
    sys_tmr_t   hTmr);


#ifdef __cplusplus
}
#endif

#endif
