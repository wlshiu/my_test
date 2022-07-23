/**
 * Copyright (c) 2022 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file phy_sim.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2022/07/23
 * @license
 * @description
 */


#include "phy_sim.h"

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

D_ERR
d_CreateHandle(
    D_HANDLE    **ppHD,
    void        *extraData)
{
    D_ERR        result = D_ERR_OK;

    do{
        if( *pHD != 0 )
        {
            printf("error, Exist d handle !!");
            break;
        }

        //----------------------
        (*pD) = 0;

    }while(0);

    if( result != D_ERR_OK )
    {
        printf("%s, err 0x%x !", __FUNCTION__, result);
    }

    return result;
}

D_ERR
d_DestroyHandle(
    D_HANDLE    **ppHD,
    void        *extraData)
{
    D_ERR        result = D_ERR_OK;


    return result;
}

/*
D_ERR
d_tamplete(
    D_HANDLE    *pHD,
    void        *extraData)
{
    D_ERR        result = D_ERR_OK;


    if( result != D_ERR_OK )
    {
        printf("%s, err 0x%x !", __FUNCTION__, result);
    }

    return result;
}
*/