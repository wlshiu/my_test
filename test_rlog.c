#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>
#include <time.h>

#include <pthread.h>

#include "ring_log.h"
#include <malloc.h>

#include "mleak_check.h"
///////////////////////////////////////////////////
static pthread_mutex_t  g_event_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t   g_event_cond = PTHREAD_COND_INITIALIZER;

static HRLOG               g_hRlog = 0;
///////////////////////////////////////////////////

///////////////////////////////////////////////////
static void*
_test_thread(void* a)
{
    int     *pBExit_thread = (int*)a;
    int     cnt = 0;

    while( !(*pBExit_thread) )
    {
        rlog_post(g_hRlog, "test %d\n", cnt++);
        usleep(20000);
    }

    pthread_exit(0);
    return 0;
}

int main(void)
{
    int                 j, bExit_thread = 0;
    int                 thread_num = 2;
    pthread_t           tid[2];
    struct timespec     time_info;
    rlog_init_info_t    init_info = {0};

    init_info.buffer_size = 10 << 10;
    rlog_create(&g_hRlog, &init_info);

    for(j = 0; j < thread_num; j++)
        pthread_create(&tid[j], NULL, &_test_thread, &bExit_thread);

    pthread_mutex_lock(&g_event_mutex);
    time_info.tv_sec  = time(NULL) + 2;
    time_info.tv_nsec = 0;
    pthread_cond_timedwait(&g_event_cond, &g_event_mutex, &time_info);
    pthread_mutex_unlock(&g_event_mutex);

    bExit_thread = 1;
    for(j = 0; j < thread_num; j++)
        pthread_join(tid[j], NULL);

    rlog_dump(g_hRlog, "./dump_log.txt");
    rlog_destroy(&g_hRlog);

    mlead_dump();
    return 0;
}