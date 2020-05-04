/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file upgrade.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/04/30
 * @license
 * @description
 */

#ifndef __upgrade_H_wMpB4YSc_lodD_HIpv_sR7Z_uOGQLfsFfjLb__
#define __upgrade_H_wMpB4YSc_lodD_HIpv_sR7Z_uOGQLfsFfjLb__

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"
//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct upg_operator
{
    uint32_t    port;
    uint8_t     *pData;
    int         length;

    // low level instance
    int (*cb_ll_send)(struct upg_operator *pOp);
    int (*cb_ll_recv)(struct upg_operator *pOp);

    void        *pTunnel_info;
} upg_operator_t;

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
upg_init();


int
upg_deinit();


int
upg_send(
    upg_operator_t  *pOp);


int
upg_recv(
    upg_operator_t  *pOp);


#ifdef __cplusplus
}
#endif

#endif
