/**
 * Copyright (c) 2018 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file log2mem.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2018/07/18
 * @license
 * @description
 */

#ifndef __log2mem_H_wan6PCkZ_lXzU_HHPA_sEXD_uPfNiEgYCPVD__
#define __log2mem_H_wan6PCkZ_lXzU_HHPA_sEXD_uPfNiEgYCPVD__

#ifdef __cplusplus
extern "C" {
#endif


//=============================================================================
//                  Constant Definition
//=============================================================================
#define LOG_BUF_MAX_SIZE        100
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct log_node
{
    unsigned int        time_stamp;
    unsigned int        argv_0;
    unsigned int        argv_1;
    unsigned int        argv_2;
} log_node_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================
extern unsigned int        g_log_idx;
extern log_node_t          g_log_buf[LOG_BUF_MAX_SIZE];
//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
int
log2mem_init(void);


int
log2mem_push(
    unsigned int    argv_0,
    unsigned int    argv_1,
    unsigned int    argv_2);


int
log2mem_show(void);


#ifdef __cplusplus
}
#endif

#endif
