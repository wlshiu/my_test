/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the uIP TCP/IP stack
 *
 * $Id: clock-arch.c,v 1.2 2006/06/12 08:00:31 adam Exp $
 */

/**
 * \file
 *         Implementation of architecture-specific clock functionality
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include <stdio.h>
#include "clock-arch.h"

#if !defined(WIN32)
#error "this verion is for windows platform !!"
#endif

#ifndef WINVER
#define WINVER 0x500
#endif

#include <windows.h>

#if 0
static int              g_is_initialized = 0;
static clock_time_t     g_time_cnt = 0;
/*---------------------------------------------------------------------------*/
static VOID CALLBACK
timer_sig_handler(PVOID lpParameter, BOOLEAN TimerOrWaitFired)
{
    g_time_cnt++;
}

clock_time_t
clock_time(void)
{

    if( g_is_initialized == 0 )
    {
        HANDLE      win_timer;
        if( CreateTimerQueueTimer(&win_timer, NULL,
                                  (WAITORTIMERCALLBACK)timer_sig_handler,
                                  NULL, 1, 1,
                                  WT_EXECUTEINTIMERTHREAD) == 0 )
        {
            printf("\nCreateTimerQueueTimer() error\n");
            while(1);
        }
        g_is_initialized = 1;
    }
    return (clock_time_t)g_time_cnt;
}
#else
clock_time_t
clock_time(void)
{
    return (clock_time_t)GetTickCount();
}
#endif
/*---------------------------------------------------------------------------*/
