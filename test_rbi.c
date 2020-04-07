#include <windows.h>
#include <stdio.h>
#include <pthread.h>

#include "log.h"
#include "rbi.h"

static pthread_cond_t   g_usr_cond;
static pthread_mutex_t  g_usr_mtx;

static rbi_t    g_hRbi;

static void*
_task_node_1(void *argv)
{
    int         rval = 0;
    int         cnt = 0;
    uint32_t    *pIs_running = (uint32_t*)argv;

    while( *pIs_running )
    {
        char    msg[16] = {0};

        snprintf(msg, sizeof(msg), "push %03d\n", cnt);
        rval = rbi_push(g_hRbi, (uint8_t*)msg, strlen(msg) + 1);
        if( rval )
        {
            pthread_mutex_lock(&g_usr_mtx);
            fprintf(stdout, "[%s] push %03d fail\n", __func__, cnt);
            fflush(stdout);
            pthread_mutex_unlock(&g_usr_mtx);
            continue;
        }

        cnt++;
    }

    pthread_exit(0);
    return 0;
}

static void*
_task_node_2(void *argv)
{
    int         rval = 0;
    uint32_t    *pIs_running = (uint32_t*)argv;

    while( *pIs_running )
    {
        char    msg[16] = {0};

        rval = rbi_pop(g_hRbi, (uint8_t*)msg, sizeof(msg));
        if( !rval )
            continue;

        pthread_mutex_lock(&g_usr_mtx);
        fprintf(stdout, "[%s] %s\n", __func__, msg);
        fflush(stdout);
        pthread_mutex_unlock(&g_usr_mtx);
    }

    pthread_exit(0);
    return 0;
}

int main(int argc, char **argv)
{
    do {
        pthread_t   t1, t2;
        uint32_t    is_running = 1;

        pthread_mutex_init(&g_usr_mtx, 0);

        g_hRbi = rbi_init(4, 8);
        if( !g_hRbi ) break;

        pthread_create(&t1, 0, _task_node_1, &is_running);
        pthread_create(&t2, 0, _task_node_2, &is_running);

        Sleep(100);

        is_running = 0;

        pthread_join(t1, 0);
        pthread_join(t2, 0);

        rbi_deinit(g_hRbi);
    } while(0);

    system("pause");
    return 0;
}
