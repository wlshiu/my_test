/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file virtual_phy.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/04/06
 * @license
 * @description
 */

#ifndef __virtual_phy_H_wqu4VmmZ_l7FO_Hqdj_s7BG_uV5srYGjmxuA__
#define __virtual_phy_H_wqu4VmmZ_l7FO_Hqdj_s7BG_uV5srYGjmxuA__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <pthread.h>
//#include "mqueue.h"
#include "rbi.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
#define CONFIG_VPHY_NODE_MAX            10
#define CONFIG_VPHY_MSG_SIZE            256
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
struct vphy_dataq;

typedef void (*cb_vphy_isr_t)(struct vphy_dataq *pDataq);

typedef struct vphy_package
{
    pthread_t   src_tid;
    uint32_t    data_len;
    uint8_t     data[];
} vphy_package_t;

typedef struct vphy_dataq
{
    pthread_t       tid;
#if 0
    char            dataq_name[16];
    mqd_t           dataq;
#else
    rbi_t           dataq;
#endif
    cb_vphy_isr_t   cb_isr;
    void            *pUser_data;

} vphy_dataq_t;


//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
int
vphy_init(void);


int
vphy_deinit(void);


int
vphy_register_event_callback(
    cb_vphy_isr_t   cb_isr,
    void            *pUser_data);


int
vphy_send(
    uint8_t     *pData,
    uint32_t    length);


#ifdef __cplusplus
}
#endif

#endif
