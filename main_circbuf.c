#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>

#include "circbuf_template.h"
#include "pthread.h"


DECLARE_CBUF(int, cbuf_t);

static cbuf_t       g_cbuf = {0};
static FILE         *g_fdump_push = 0;
static FILE         *g_fdump_pop = 0;

static void*
_task_push(void *argv)
{
    int     *pIs_blocking = (int*)argv;
    while( *pIs_blocking )
    {
        int     rst = 0;
        int     data = rand();
        cbuf_push(&g_cbuf, int, data, &rst);
        if( !rst )
            fprintf(g_fdump_push, "%d\n", data);

        Sleep(rand() & 0x3);
    }

    pthread_exit(0);
    return 0;
}


static void*
_task_pop(void *argv)
{
    int     *pIs_blocking = (int*)argv;
    while( *pIs_blocking )
    {
        int     rst = 0;
        int     data = 0;
        cbuf_pop(&g_cbuf, int, &data, &rst);
        if( !rst )
            fprintf(g_fdump_pop, "%d\n", data);

        Sleep(rand() & 0x3);
    }

    pthread_exit(0);
    return 0;
}

int main()
{
    int         buf[3] = {0};
    int         is_blocking = 1;
    pthread_t   t1, t2;

    srand(time(0));

    if( !(g_fdump_push = fopen("z_push.dmp", "wb")) )
    {
        printf("open push dump fail\n");
        while(1);
    }

    if( !(g_fdump_pop = fopen("z_pop.dmp", "wb")) )
    {
        printf("open pop dump fail\n");
        while(1);
    }

    cbuf_init(&g_cbuf, buf, 3);

    pthread_create(&t1, 0, _task_push, &is_blocking);
    pthread_create(&t2, 0, _task_pop, &is_blocking);

    Sleep(2000);

    is_blocking = 0;

    pthread_join(t1, 0);
    pthread_join(t2, 0);

    if( g_fdump_push )  fclose(g_fdump_push);
    if( g_fdump_pop )   fclose(g_fdump_pop);

    printf("=============== done\n");

    while(1)
        __asm("nop");

    return 0;
}
