/*
***************************************************************************
*
* Author: Teunis van Beelen
*
* Copyright (C) 2010, 2011, 2012 Teunis van Beelen
*
* teuniz@gmail.com
*
***************************************************************************
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation version 2 of the License.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along
* with this program; if not, write to the Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*
***************************************************************************
*
* This version of GPL is at http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
*
***************************************************************************
*/



#include "timer.h"



void (*timer_func_handler_pntr)(void);

HANDLE  win_timer;

static VOID CALLBACK timer_sig_handler(PVOID lpParameter, BOOLEAN TimerOrWaitFired)
{
    timer_func_handler_pntr();
}


void*
timer_get_time(void)
{
    static LARGE_INTEGER   start;
    QueryPerformanceCounter(&start);
    return (void*)&start;
}

uint32_t
timer_get_duration(
    void    *pT)
{
    LARGE_INTEGER   *pStart = (LARGE_INTEGER*)(pT);
    LARGE_INTEGER   stop, freq;

    if( !pT )   return 0;

    QueryPerformanceCounter(&stop);
    QueryPerformanceFrequency(&freq);

    double      time_gap = (stop.QuadPart - pStart->QuadPart) / (double) freq.QuadPart;
    return (unsigned long)(time_gap * 1000);
}

/**
 *  \brief  timer_start
 *
 *  \param [in] mSec                    Period (micro-seconds)
 *  \param [in] timer_func_handler      callback handler
 *  \return
 *      error code
 */
int timer_start(int mSec, void (*timer_func_handler)(void))
{
    timer_func_handler_pntr = timer_func_handler;

    if(CreateTimerQueueTimer(&win_timer, NULL,
                             (WAITORTIMERCALLBACK)timer_sig_handler, NULL,
                             mSec, mSec, WT_EXECUTEINTIMERTHREAD) == 0)
    {
        printf("\nCreateTimerQueueTimer() error\n");
        return(1);
    }

    return(0);
}


void timer_stop(void)
{
//    CloseHandle(win_timer);
    DeleteTimerQueueTimer(NULL, win_timer, NULL);
}

