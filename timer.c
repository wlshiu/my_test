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

#ifdef __linux__


/**
 *  FIXME: Not sure it works or not
 */
#define CFG_ENABLE_SIGINFO      1

struct itimerval    timervalue;
struct sigaction    new_handler, old_handler;

#if defined(CFG_ENABLE_SIGINFO) && (CFG_ENABLE_SIGINFO)
static void
timer_sig_handler(int sig, siginfo_t *si, void *uc)
{
    // ucontext_t      *context = (ucontext_t*)uc;

    timer_func_handler_pntr();
}
#else
static void timer_sig_handler(int arg)
{
    timer_func_handler_pntr();
}
#endif

void*
timer_get_time(void)
{
    static struct timeval   startT;
    gettimeofday(&startT, NULL);
    return (void*)&startT;
}

uint32_t
timer_get_duration(
    void    *pT)
{
    struct timeval  *startT = (struct timeval*)pT;
    struct timeval  currT = {0};
    uint32_t        duration_time = 0;

    gettimeofday(&currT, NULL);
    duration_time = (currT.tv_sec - startT->tv_sec) * 1000;      // sec to ms
    duration_time += ((currT.tv_usec - startT->tv_usec) / 1000); // us to ms
    return (uint32_t)duration_time;
}


int start_timer(int mSec, void (*timer_func_handler)(void))
{
    timer_func_handler_pntr = timer_func_handler;

#if defined(CFG_ENABLE_SIGINFO) && (CFG_ENABLE_SIGINFO)

    printf("Establishing handler for signal %d\n", SIGALRM);
    memset(&new_handler, 0x0, sizeof(new_handler));
    new_handler.sa_flags     = SA_SIGINFO;
    new_handler.sa_sigaction = timer_sig_handler;
    sigemptyset(&new_handler.sa_mask);
    if (sigaction(SIGALRM, &new_handler, NULL) == -1)
    {
        printf("\nsigaction() error\n");
        return(1);
    }
#else

    new_handler.sa_handler = &timer_sig_handler;
    new_handler.sa_flags   = SA_NOMASK;
    if(sigaction(SIGALRM, &new_handler, &old_handler))
    {
        printf("\nsigaction() error\n");
        return(1);
    }
#endif

    timervalue.it_interval.tv_sec = mSec / 1000;
    timervalue.it_interval.tv_usec = (mSec % 1000) * 1000;
    timervalue.it_value.tv_sec = mSec / 1000;
    timervalue.it_value.tv_usec = (mSec % 1000) * 1000;
    if(setitimer(ITIMER_REAL, &timervalue, NULL))
    {
        printf("\nsetitimer() error\n");
        return(1);
    }

    return(0);
}

void stop_timer(void)
{
    timervalue.it_interval.tv_sec = 0;
    timervalue.it_interval.tv_usec = 0;
    timervalue.it_value.tv_sec = 0;
    timervalue.it_value.tv_usec = 0;
    setitimer(ITIMER_REAL, &timervalue, NULL);

    sigaction(SIGALRM, &old_handler, NULL);
}

#else  /* #ifdef __linux__ */

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

int timer_start(int mSec, void (*timer_func_handler)(void))
{
    timer_func_handler_pntr = timer_func_handler;

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

    if(CreateTimerQueueTimer(&win_timer, NULL, (WAITORTIMERCALLBACK)timer_sig_handler, NULL, mSec, mSec, WT_EXECUTEINTIMERTHREAD) == 0)
    {
        printf("\nCreateTimerQueueTimer() error\n");
        return(1);
    }

    return(0);
}


void timer_stop(void)
{
    // CloseHandle(win_timer);
    DeleteTimerQueueTimer(NULL, win_timer, NULL);
}

#endif



