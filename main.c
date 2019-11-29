
#include <windows.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "pthread.h"
#include "ipc_user.h"

static uint8_t              g_buf[32 << 10] = {0};
static nodeq_header_t       *pNodeq_hdr0 = 0;
static nodeq_header_t       *pNodeq_hdr1 = 0;

static void*
_task_a(void *argv)
{
    uint32_t    *pIs_running = (uint32_t*)argv;
    int         cnt = 0;

    while( *pIs_running )
    {
        node_t  *pNode_cur = 0;

        // push to queue
        GET_NEXT_WR_NODE(&g_buf, 0, &pNode_cur);
        if( pNode_cur )
        {
            pNode_cur->is_used = true;
            snprintf(pNode_cur->data, sizeof(pNode_cur->data), "a -> b (%02d)\n", cnt++);
        }

        // pop from queue
        GET_NEXT_RD_NODE(&g_buf, 1, &pNode_cur);
        if( pNode_cur  )
        {
            if( pNode_cur->is_used )
            {
                printf("%s\n", pNode_cur->data);
                pNode_cur->is_used = false;
            }
        }

        Sleep(1);
    }

    pthread_exit(0);
    return 0;
}

static void*
_task_b(void *argv)
{
    uint32_t    *pIs_running = (uint32_t*)argv;
    int         cnt = 100;

    while( *pIs_running )
    {
        node_t  *pNode_cur = 0;

        // push to queue
        GET_NEXT_WR_NODE(&g_buf, 1, &pNode_cur);
        if( pNode_cur )
        {
            pNode_cur->is_used = true;
            snprintf(pNode_cur->data, sizeof(pNode_cur->data), "b -> a (%02d)\n", cnt++);
        }

        // pop from queue
        GET_NEXT_RD_NODE(&g_buf, 0, &pNode_cur);
        if( pNode_cur )
        {
            if( pNode_cur->is_used )
            {
                printf("%s\n", pNode_cur->data);
                pNode_cur->is_used = false;
            }
        }

        Sleep(1);
    }

    pthread_exit(0);
    return 0;
}


int main()
{
    pthread_t   t1, t2;
    uint32_t    is_running = 1;

    memset(g_buf, 0x0, sizeof(g_buf));

    pNodeq_hdr0 = GET_NODEQ_HDR(&g_buf, 0);
    pNodeq_hdr1 = GET_NODEQ_HDR(&g_buf, 1);

//    UPDATE_NODE_INDEX(&g_buf, 0, wr);
//    UPDATE_NODE_INDEX(&g_buf, 1, wr);

    pthread_create(&t1, 0, _task_a, &is_running);
    pthread_create(&t2, 0, _task_b, &is_running);

    Sleep(20);
//    is_running = 0;
    pthread_join(t1, 0);
    pthread_join(t2, 0);

    while(1)
        __asm("nop");
    return 0;
}
