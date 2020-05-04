/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file test_rbi.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/05/04
 * @license
 * @description
 */


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include "rbi.h"
#include "pthread.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
#define CONFIG_SLOT_SIZE        5
#define CONFIG_UNIT_SIZE        1024

#define CONFIG_INPUT_FILE_NAME  "glass.jpg"
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================
pthread_mutex_t      g_log_mtx;

static rbi_t        g_rbi = {0};
static uint32_t     g_eos = 0;
static uint32_t     g_cache[CONFIG_SLOT_SIZE] __attribute__ ((aligned (4)));
//=============================================================================
//                  Private Function Definition
//=============================================================================
static void*
_task_push(void *argv)
{
    rbi_t   *pRBI = (rbi_t*)argv;
    FILE    *fin = 0;

    fin = fopen(CONFIG_INPUT_FILE_NAME, "rb");
    if( !fin )
    {
        printf("open %s fail \n", CONFIG_INPUT_FILE_NAME);
        while(1);
    }

    while(1)
    {
        int         slice_size = ((rand() >> 5) & 0xFF) + 5;
        uint8_t     *pCur = 0;

        if( !(pCur = malloc(slice_size)) )
        {
            Sleep(5);
            continue;
        }

        slice_size = fread(pCur + 4, 1, slice_size - 4, fin);
        if( slice_size )
        {
            int     rval = 0;

            printf("push %p\n", pCur);

            *((uint32_t*)pCur) = slice_size;

            while( (rval = rbi_push(pRBI, (uint32_t)pCur)) )
            {
                Sleep(3);
            }
        }
        else
        {
            g_eos = 1;
            break;
        }
    }
    pthread_exit(0);
    return 0;
}

static void*
_task_pop(void *argv)
{
    rbi_t   *pRBI = (rbi_t*)argv;
    FILE    *fout = 0;

    fout = fopen("dump.bin", "wb");
    if( !fout )
    {
        printf("open dump.bin fail \n");
        while(1);
    }

    while(1)
    {
        uint8_t     *pCur = 0;

        if( rbi_pick(pRBI) )
        {
            int     timeout = 0;

            do {
                pCur = rbi_pop(pRBI);
                if( pCur )
                {
                    fwrite(pCur + 4, 1, *(uint32_t*)pCur, fout);
                    printf("pop: %p\n", pCur);
                    free(pCur);
                }
            } while( g_eos && timeout++ < 1000 );

            if( g_eos )
            {
                fclose(fout);
                break;
            }
        }
    }
    pthread_exit(0);
    return 0;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
int main(void)
{
    int     is_running = 1;
    do {
        pthread_t   t1, t2;

        srand(time(0));

        rbi_init(&g_rbi, (uint32_t*)&g_cache, CONFIG_SLOT_SIZE);

        pthread_create(&t1, 0, _task_push, &g_rbi);
        pthread_create(&t2, 0, _task_pop, &g_rbi);

        pthread_join(t1, 0);
        pthread_join(t2, 0);
    } while(0);

    system("pause");
    return 0;
}
