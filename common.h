/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file common.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/04/30
 * @license
 * @description
 */

#ifndef __common_H_w4GUPSzJ_lAuA_Hu9D_sbSj_ubVlZYPoDoXs__
#define __common_H_w4GUPSzJ_lAuA_Hu9D_sbSj_ubVlZYPoDoXs__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include <pthread.h>
//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================
#define log_msg(str, ...)                   \
    do {                                    \
        pthread_mutex_lock(&g_log_mtx);     \
        printf(str, ##__VA_ARGS__ );        \
        pthread_mutex_unlock(&g_log_mtx);   \
    } while(0)
//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================
extern pthread_mutex_t      g_log_mtx;
//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================

#ifdef __cplusplus
}
#endif

#endif
