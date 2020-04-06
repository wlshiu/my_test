/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file time_event.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/04/06
 * @license
 * @description
 */

#ifndef __time_event_H_wYDrJaMu_lNbZ_Hb3K_sRLV_uBAaD99rFZnc__
#define __time_event_H_wYDrJaMu_lNbZ_Hb3K_sRLV_uBAaD99rFZnc__

#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>
//=============================================================================
//                  Constant Definition
//=============================================================================
typedef enum time_ev_mode
{
    TIME_EV_MODE_INTERVAL   = 0x123,
    TIME_EV_MODE_PERIODIC,

} time_ev_mode_t;

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct time_ev
{
    struct time_ev  *next;
    uint32_t        timeout;

    // parameters
    uint32_t        interval;
    time_ev_mode_t  mode;
    void            *pExtra_data;

    void (*ev_handler)(struct time_ev *pTime_ev);

} time_ev_t;

//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
void
time_ev_init(void);


void
time_ev_start(
    time_ev_t   *pTime_ev);


void
time_ev_stop(
    time_ev_t   *pTime_ev);


void
time_ev_routine(void);



#ifdef __cplusplus
}
#endif

#endif
