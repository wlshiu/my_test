/**
 * Copyright (c) 2023 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file sys_sim.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2023/07/02
 * @license
 * @description
 */


#include "sys_sim.h"

//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================

void sys_log(char *pStr)
{
    struct tm       *now;
    struct timeb    tb;

    char    date_str[16] = {0};
    char    time_str[16] = {0};

    ftime(&tb);

    now = localtime(&tb.time);
    snprintf(date_str, sizeof(date_str), "%04d-%02d-%02d",
             now->tm_year + 1900,
             now->tm_mon + 1,
             now->tm_mday);

    snprintf(time_str, sizeof(time_str), "%02d:%02d:%02d",
             now->tm_hour, now->tm_min, now->tm_sec);

    printf("[%s %s.%s] %s", date_str, time_str, tb.millitm, pStr);
    return;
}

int sys_wait_msg(uint8_t *pBuf, int *pBuf_size)
{
    MSG     msg;

    while(true)
    {
        if( !GetMessage(&msg, 0, 0, 0) ) //get msg from message queue
        {
            printf(" th %d wait\n", GetCurrentThreadId());
            Sleep(1);
            continue;
        }

        switch( msg.message )
        {
            case SYS_UMSG_HOST_TX:
            case SYS_UMSG_HOST_RX:
                {
                    int             len = *pBuf_size;
                    sys_msg_info_t  *pMsg_info = (sys_msg_info_t*)msg.wParam;

                    len = (pMsg_info->msg_len > len) ? len : pMsg_info->msg_len;
                    memcpy(pBuf, pMsg_info->pMsg_buf, len);
                    *pBuf_size = len;
                }
                return 0;

            default:
                break;
        }
    }
    return 0;
}

int sys_send_msg(DWORD th_id, uint8_t *pBuf)
{
    if( !PostThreadMessage(th_id, SYS_UMSG_HOST_TX, (WPARAM)pBuf, 0) ) //post thread msg
    {
        // fail
    }
    return 0;
}

