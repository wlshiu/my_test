#include <stdio.h>
#include <stdlib.h>
#include "time_event.h"



static void
_app_time_ev_handle(time_ev_t *pTime_ev)
{
    printf("time ev: %d\n", (int)pTime_ev->pExtra_data);

    if( pTime_ev->mode == TIME_EV_MODE_INTERVAL)
        time_ev_stop(pTime_ev);
    return;
}

static time_ev_t    g_tm_ev[] =
{
    [0] = { .interval = 1, .mode = TIME_EV_MODE_PERIODIC, .ev_handler = _app_time_ev_handle, .pExtra_data = (void*)0x0 },
    [1] = { .interval = 5, .mode = TIME_EV_MODE_INTERVAL, .ev_handler = _app_time_ev_handle, .pExtra_data = (void*)0x1 },
    [2] = { .interval = 3, .mode = TIME_EV_MODE_INTERVAL, .ev_handler = _app_time_ev_handle, .pExtra_data = (void*)0x2 },
    [3] = { .interval = 4, .mode = TIME_EV_MODE_INTERVAL, .ev_handler = _app_time_ev_handle, .pExtra_data = (void*)0x3 },
};

int main()
{
    time_ev_init();

    time_ev_start(&g_tm_ev[0]);
    time_ev_start(&g_tm_ev[1]);
    time_ev_start(&g_tm_ev[2]);
    time_ev_start(&g_tm_ev[3]);

    while(1)
    {
        time_ev_routine();
    }
    return 0;
}
