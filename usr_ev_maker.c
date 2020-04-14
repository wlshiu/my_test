/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file usr_ev_maker.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/04/13
 * @license
 * @description
 */


#include <stdio.h>
#include <string.h>
#include "usr_ev_maker.h"

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
int
usr_ev_roll_call(
    usr_ev_script_t *pEv_script,
    uint8_t         *pBuf,
    int             *pBuf_len)
{
    int     rval = 0;
    do {
        usr_ev_base_t   *pEv_base = (usr_ev_base_t*)pBuf;

        if( !pBuf || !pBuf_len || *pBuf_len == 0 )
        {
            rval = -1;
            break;
        }

        pEv_base->type = USR_EV_TYPE_ROLL_CALL;

        snprintf((char*)pEv_base->pData, *pBuf_len - sizeof(usr_ev_base_t), "%s", "discovery");

        *pBuf_len = sizeof(usr_ev_base_t) + strlen((const char*)pEv_base->pData);

        pEv_base->length = *pBuf_len;

    } while(0);
    return rval;
}

