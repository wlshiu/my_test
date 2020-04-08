/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file sys_time.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/04/08
 * @license
 * @description
 */


#include "sys_time.h"

#if !defined(WIN32)
#error "system time only works on windows"
#endif


/**
 *  this code only works on win2000, XP, Vista, 7 and up
 *  win95, win98 and ME are not supported
 *  WINVER must have value 0x500 or higher
 */
#ifndef WINVER
#define WINVER      0x500
#endif

#if WINVER < 0x500
#error "WINVER is < 0x500, cannot compile for old windows versions"
#endif

#include <windows.h>
#include <stdio.h>
//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================
static cb_sys_tmr_alarm_t       g_cb_sys_tmr_isr = 0;
static void                     *g_pUser_data = 0;
static HANDLE                   g_win_tmr;
//=============================================================================
//                  Private Function Definition
//=============================================================================
static VOID CALLBACK
_sys_tmr_isr(PVOID lpParameter, BOOLEAN TimerOrWaitFired)
{
    if( g_cb_sys_tmr_isr )
        g_cb_sys_tmr_isr(g_pUser_data);
}

//=============================================================================
//                  Public Function Definition
//=============================================================================
int
sys_tmr_start(
    uint32_t            time_interval_ms,
    cb_sys_tmr_alarm_t  cb_alarm,
    void                *pUser_data)
{
    g_cb_sys_tmr_isr = cb_alarm;
    g_pUser_data     = pUser_data;

    /**
     *  BOOL WINAPI CreateTimerQueueTimer(
     *          _Out_    PHANDLE             phNewTimer,
     *          _In_opt_ HANDLE              TimerQueue,
     *          _In_     WAITORTIMERCALLBACK Callback,
     *          _In_opt_ PVOID               Parameter,
     *          _In_     DWORD               DueTime,
     *          _In_     DWORD               Period,
     *          _In_     ULONG               Flags
     *          );
     */
    if( CreateTimerQueueTimer(&g_win_tmr,
                              NULL,
                              (WAITORTIMERCALLBACK)_sys_tmr_isr,
                              NULL,
                              time_interval_ms,
                              time_interval_ms,
                              WT_EXECUTEINTIMERTHREAD) == 0 )
    {
        printf("\nCreateTimerQueueTimer() error !!!\n");
        return -1;
    }

    return 0;
}

void
sys_tmr_stop(void)
{
    DeleteTimerQueueTimer(NULL, g_win_tmr, NULL);
    return;
}

sys_tmr_t
sys_tmr_get_time(void)
{
    static LARGE_INTEGER   start;
    QueryPerformanceCounter(&start);
    return (sys_tmr_t)&start;
}

uint32_t
sys_tmr_get_duration(sys_tmr_t hTmr)
{
    LARGE_INTEGER   *pStart = (LARGE_INTEGER*)(hTmr);
    LARGE_INTEGER   stop, freq;
    double          time_gap = 0.0f;

    if( !hTmr ) return 0;

    QueryPerformanceCounter(&stop);
    QueryPerformanceFrequency(&freq);

    time_gap = (stop.QuadPart - pStart->QuadPart) / (double) freq.QuadPart;
    return (uint32_t)(time_gap * 1000);
}

