/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file usr_ev_maker.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/04/13
 * @license
 * @description
 */

#ifndef __usr_ev_maker_H_wSEQWI2R_lmdq_HAR9_s2BO_ukz0Fn8Uxxq5__
#define __usr_ev_maker_H_wSEQWI2R_lmdq_HAR9_s2BO_ukz0Fn8Uxxq5__

#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>
//=============================================================================
//                  Constant Definition
//=============================================================================
typedef enum usr_ev_type
{
    USR_EV_TYPE_NONE    = 0,
    USR_EV_TYPE_,

} usr_ev_type_t;
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct usr_ev_script
{
    uint32_t    time_offset;    // msec
    uint32_t    is_acted;

    int (*pf_make_event)(struct ev_script *pEv_script, uint8_t *pBuf, int *pBuf_len);

    void        *pUsr_info;

} usr_ev_script_t;
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
usr_ev_discovery(
    usr_ev_script_t *pEv_script,
    uint8_t         *pBuf,
    int             *pBuf_len);


#ifdef __cplusplus
}
#endif

#endif
