/**
 * Copyright (c) 2023 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file sys_sim.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2023/07/02
 * @license
 * @description
 */

#ifndef __sys_sim_H_wdYTiEYf_l93F_HmxR_sotb_uzSflROmKhpm__
#define __sys_sim_H_wdYTiEYf_l93F_HmxR_sotb_uzSflROmKhpm__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/timeb.h>
#include <windows.h>
//=============================================================================
//                  Constant Definition
//=============================================================================
typedef enum sys_umsg
{
    SYS_UMSG_HOST_TX        = (WM_USER + 100),
    SYS_UMSG_HOST_RX        = (WM_USER + 200),
} sys_msg_t;


//=============================================================================
//                  Macro Definition
//=============================================================================
#define err(str, ...)       do{ printf("[%s:%d][Error] " str, __func__, __LINE__, ##__VA_ARGS__); while(1); }while(0)
//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct sys_msg_info
{
    uint8_t     *pMsg_buf;
    int         msg_len;
} sys_msg_info_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================

void sys_log(char *pStr);

int sys_wait_msg(uint8_t *pBuf, int *pBuf_size);

int sys_send_msg(DWORD th_id, uint8_t *pBuf);

#ifdef __cplusplus
}
#endif

#endif
