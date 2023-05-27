/**
 * Copyright (c) 2023 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file main.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2023/05/27
 * @license
 * @description
 */


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "smm.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
#define CONFIG_TEST_TIMES       10
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================
static unsigned char        g_buf_pool[16 << 10];
//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
int main()
{
    smm_handle_t    hSmm = {0};

    srand(time(0));

    hSmm.pool_addr = (unsigned long)&g_buf_pool;
    hSmm.pool_size = sizeof(g_buf_pool);

    smm_init(&hSmm);

    do {
        unsigned char   *pBuf[CONFIG_TEST_TIMES] = {0};

        for(int i = 0; i < CONFIG_TEST_TIMES; i++)
        {
            int     buf_size = rand() % 200;

            buf_size = (buf_size) ? buf_size : 10;

            pBuf[i] = smm_malloc(&hSmm, buf_size);
            if( (long)pBuf[i] <= 0 )
            {
                if( (long)pBuf[i] == SMM_ERR_OVER_POOL)
                    printf("memory overflow (%d)!\n", (int)pBuf[i]);
                else
                    printf("something wrong (%d)!\n", (int)pBuf[i]);

                system("pause");
                while(1);
            }

            if( rand() & 0x1 )
            {
                smm_free(&hSmm, (void*)pBuf[i]);
            }

        }

        printf("after free\n");
        smm_usage(&hSmm);

    } while(0);

    smm_deinit(&hSmm);

    return 0;
}
