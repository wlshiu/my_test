/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file msg_box.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/04/08
 * @license
 * @description
 */

#ifndef __msg_box_H_wQWK0sCh_lVZc_HVL5_sbQE_uX0pc7Iiv6sf__
#define __msg_box_H_wQWK0sCh_lVZc_HVL5_sbQE_uX0pc7Iiv6sf__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct mbox_argv;
{
    union {
        uint32_t    value;
        float       fval;
        void        *pAddr;
    };
} mbox_argv_t;


typedef struct mbox
{
    struct mbox     *next;

    int (*pf_handler)(struct *pMBox);
    
    union {
        struct {
            mbox_argv_t     argv[4];
        } def;
    } argv;
} mbox_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================

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
