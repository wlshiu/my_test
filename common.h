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

#include <windows.h>
#include <winsock2.h>

#include "rbi.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
#define CONFIG_CONTROLLER_SINK_PORT     1234
#define CONFIG_CONTROLLER_SOURCE_PORT   4321

#define CONFIG_LEAF_SINK_PORT           5678
#define CONFIG_LEAF_SOURCE_PORT         8765


#define SOCK_CTLR_SOURCE     0
#define SOCK_CTLR_SINK       1
#define SOCK_LEAF_SOURCE     2
#define SOCK_LEAF_SINK       3
#define SOCK_TOTAL           4


typedef enum upg_opcode
{
    UPG_OPCODE_BASE         = 0x00,
    UPG_OPCODE_DISCOVERY    = 0x01,


    UPG_OPCODE_DATA         = 0xd0,
    UPG_OPCODE_DATA_WR      = 0xd1,
    UPG_OPCODE_DATA_REQ     = 0xd2,

} upg_opcode_t;
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
typedef struct sock_info
{
    SOCKET              sd;
    uint32_t            port;

    union {
        struct sockaddr_in  sock_in;
        struct sockaddr     sock_addr;
    };
} sock_info_t;
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
int
common_init(void);


int
common_get_rbi(
    uint32_t    uid,
    rbi_t       **ppHRBI);



#ifdef __cplusplus
}
#endif

#endif
