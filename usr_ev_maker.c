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
usr_ev_discovery(
    usr_ev_script_t *pEv_script,
    uint8_t         *pBuf,
    int             *pBuf_len)
{
    int     rval = 0;
    do {
        if( !pBuf || !pBuf_len || *pBuf_len == 0 )
        {
            rval = -1;
            break;
        }
        snprintf(pBuf, *pBuf_len, "%s", "discovery");

        *pBuf_len = strlen((const char*)pBuf);

    } while(0);
    return rval;
}

