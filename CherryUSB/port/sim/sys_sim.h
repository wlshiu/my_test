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

#include "usb_musb_reg.h"
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
#define trace(str, ...)     do{ printf("[%s:%d] " str, __func__, __LINE__, ##__VA_ARGS__); }while(0)
//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct sys_msg_info
{
    uint8_t     *pMsg_buf;
    int         msg_len;
} sys_msg_info_t;

typedef struct sys_packet_hdr
{
    uint32_t    len;
    uint32_t    raw[];
} sys_packet_hdr_t;

typedef struct sys_vmsg_node
{
    USB0_Type       usb_regs;
    uint32_t        raw[512 >> 2];
} sys_vmsg_node_t;

typedef struct sys_vmsgq
{
#define CONFIG_SYS_VMSGQ_CNT_MAX        32
    volatile uint32_t   tail;
    volatile uint32_t   head;

    sys_vmsg_node_t     *pMsg[CONFIG_SYS_VMSGQ_CNT_MAX];
} sys_vmsgq_t;

//=============================================================================
//                  Global Data Definition
//=============================================================================
extern uint32_t      g_udev_rxbuf[1024 >> 2];
extern uint32_t      g_udev_txbuf[1024 >> 2];

extern uint32_t      g_uhost_tx_buf[1024 >> 2];
extern uint32_t      g_uhost_rx_buf[1024 >> 2];

extern HANDLE  g_hMutexHC;
extern HANDLE  g_hMutexDC;

extern DWORD   g_th_uhost_id;
extern DWORD   g_th_udev_id;


extern USB0_Type    g_USBD;
extern USB0_Type    g_USBH;

extern bool     g_is_hc_end;
extern bool     g_is_dc_end;

extern sys_vmsgq_t     g_dc_vmsgq;
extern sys_vmsgq_t     g_hc_vmsgq;
//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================

void sys_log(char *pStr);

int sys_wait_msg(uint8_t *pBuf, int *pBuf_size);

int sys_send_msg(DWORD th_id, uint8_t *pBuf);

void sys_mutex_lock(void *hMtx);
void sys_mutex_unlock(void *hMtx);


int sys_vmsgq_send(sys_vmsgq_t *pVMsgq, sys_vmsg_node_t *pNode);
int sys_vmsgq_recv(sys_vmsgq_t *pVMsgq, sys_vmsg_node_t **ppNode);

#ifdef __cplusplus
}
#endif

#endif
