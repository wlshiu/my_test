/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file upgrade.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/04/30
 * @license
 * @description
 */


#include "common.h"
#include "upgrade.h"
#include "upgrade_packages.h"
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
upg_init()
{
    int     rval = 0;
    return rval;
}

int
upg_deinit()
{
    int     rval = 0;
    return rval;
}

int
upg_send(
    upg_operator_t  *pOp)
{
    int     rval = 0;
    do {
        if( !pOp || !pOp->cb_ll_send )
        {
            rval = -__LINE__;
            break;
        }

        rval = pOp->cb_ll_send(pOp);
        if( rval ) break;


        // TODO: xxx

    } while(0);
    return rval;
}

int
upg_recv(
    upg_operator_t  *pOp)
{
    int     rval = 0;
    do {
        if( !pOp || !pOp->cb_ll_recv )
        {
            rval = -__LINE__;
            break;
        }

        rval = pOp->cb_ll_recv(pOp);
        if( rval )
        {
            break;
        }

        // TODO: pre-parsing data

    } while(0);
    return rval;
}
