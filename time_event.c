/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file time_event.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/04/06
 * @license
 * @description
 */


#include "time_event.h"
#include <time.h>
//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================
#define GET_DURATION()      1
//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================
static time_ev_t        *g_pTime_ev_head = 0;
//=============================================================================
//                  Private Function Definition
//=============================================================================
static int
_time_ev_is_started(
    time_ev_t   *pTime_ev)
{
    int         is_started = 0;
    time_ev_t   *pCur = g_pTime_ev_head;

    while( pCur )
    {
        if( pCur == pTime_ev )
        {
            is_started = 1;
            break;
        }
        pCur = pCur->next;
    }

    return is_started;
}

static void
_calc_time(
    time_ev_t   *pTime_ev)
{
    do {
        time_ev_t   *pPrev = 0;
        uint32_t    timeout = pTime_ev->interval;

        if( !g_pTime_ev_head )
        {
            pTime_ev->next    = 0;
            pTime_ev->timeout = pTime_ev->interval;
            g_pTime_ev_head   = pTime_ev;
            break;
        }

        for(time_ev_t *pCur = g_pTime_ev_head; pCur; pCur = pCur->next)
        {
            if( timeout < pCur->timeout )
            {
                pCur->timeout -= timeout;
                break;
            }
            else
                timeout -= pCur->timeout;

            pPrev = pCur;
        }

        pTime_ev->timeout = timeout;

        if( pPrev )
        {
            pTime_ev->next = pPrev->next;
            pPrev->next    = pTime_ev;
            break;
        }

        pTime_ev->next  = g_pTime_ev_head;
        g_pTime_ev_head = pTime_ev;

    } while(0);
    return;

}

//=============================================================================
//                  Public Function Definition
//=============================================================================
void
time_ev_init(void)
{
    g_pTime_ev_head = 0;
    return;
}

void
time_ev_start(
    time_ev_t   *pTime_ev)
{
    if( !_time_ev_is_started(pTime_ev) )
        _calc_time(pTime_ev);

    return;
}

void
time_ev_stop(
    time_ev_t   *pTime_ev)
{
    time_ev_t   *pCur = g_pTime_ev_head;
    time_ev_t   *pPrev = 0;

    while( pCur )
    {
        if( pCur == pTime_ev )
        {
            if( pPrev )
                pPrev->next = pCur->next;
            else
                g_pTime_ev_head = pCur->next;

            if( pCur->next )
                pCur->next->timeout += pTime_ev->timeout;
        }

        pPrev = pCur;
        pCur  = pCur->next;
    }

    return;
}

void
time_ev_routine(void)
{
    uint32_t    elapsed = 0;

    elapsed = GET_DURATION();

    while( g_pTime_ev_head && (g_pTime_ev_head->timeout <= elapsed) )
    {
        time_ev_t   *pCur = g_pTime_ev_head;

        elapsed -= pCur->timeout;

        g_pTime_ev_head = g_pTime_ev_head->next;

        if( TIME_EV_MODE_PERIODIC == pCur->mode )
            _calc_time(pCur);

        pCur->ev_handler(pCur);
    }

    if( g_pTime_ev_head )
        g_pTime_ev_head->timeout -= elapsed;

    return;
}



