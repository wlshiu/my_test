#include <iostream>
#include <stdlib.h>

#include "timer.h"

using namespace std;

int var = 0;

LARGE_INTEGER   g_start, g_stop, g_freq;

static void timer_handler(void)
{
    static void     *pT = 0;
    uint32_t        time_gap = 0;
    time_gap = timer_get_duration(pT);
    printf("time interval: %u ms\n", time_gap);

//    QueryPerformanceCounter(&g_stop);
//    QueryPerformanceFrequency(&g_freq);

//    double  time_gap = (g_stop.QuadPart - g_start.QuadPart) / (double) g_freq.QuadPart;
//    printf("Task length: %0.8f seconds.\n", time_gap);

    var++;

//    QueryPerformanceCounter(&g_start);

    pT = timer_get_time();
}


int main(void)
{
    if(timer_start(50, &timer_handler))
    {
        printf("\n timer error\n");
        return(1);
    }

    printf("\npress ctl-c to quit.\n");

    while(1)
    {
        if(var > 100)
        {
            break;
        }
//        Sleep(1000);
    }

    timer_stop();


    return(0);
}




