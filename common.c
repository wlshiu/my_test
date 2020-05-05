/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file common.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/04/30
 * @license
 * @description
 */


#include "common.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
#define CONFIG_SLOT_SIZE        10
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================
pthread_mutex_t             g_log_mtx;

static rbi_t        g_rbi_pool[SOCK_TOTAL] = {0};
static uint32_t     g_cache[SOCK_TOTAL][CONFIG_SLOT_SIZE] = {0};
//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
int
common_init()
{
    int     rval = 0;

    for(int i = 0; i < SOCK_TOTAL; i++)
    {
        rbi_init(&g_rbi_pool[i], (uint32_t*)g_cache[i], CONFIG_SLOT_SIZE);
    }
    return rval;
}


int
common_get_rbi(
    uint32_t    uid,
    rbi_t       **ppHRBI)
{
    int     rval = 0;
    do {
        if( uid >= SOCK_TOTAL )
        {
            rval = -1;
            break;
        }

        *ppHRBI = &g_rbi_pool[uid];

    } while(0);
    return rval;
}
