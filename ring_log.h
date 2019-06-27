/**
 * Copyright (c) 2015 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file ring_log.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2015/10/22
 * @license
 * @description
 */

#ifndef __ring_log_H_wk9HTZwA_lUh7_Ht4D_stB2_ulVwgQNIgYjO__
#define __ring_log_H_wk9HTZwA_lUh7_Ht4D_stB2_ulVwgQNIgYjO__

#ifdef __cplusplus
extern "C" {
#endif


//=============================================================================
//                Constant Definition
//=============================================================================

//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================
/**
 *  ring log init parameters
 */
typedef struct rlog_init_info
{
    unsigned int        buffer_size;
} rlog_init_info_t;


/**
 *  handle of ring log
 */
typedef void*   HRLOG;
//=============================================================================
//                Global Data Definition
//=============================================================================

//=============================================================================
//                Private Function Definition
//=============================================================================

//=============================================================================
//                Public Function Definition
//=============================================================================
int
rlog_create(
    HRLOG               *pHRlog,
    rlog_init_info_t    *pInit_info);


int
rlog_destroy(
    HRLOG   *pHRlog);


int
rlog_post(
    HRLOG       hRlog,
    const char  *pStr_format, ...);


int
rlog_dump(
    HRLOG   hRlog,
    char    *pOut_path);


#ifdef __cplusplus
}
#endif

#endif