/**
 * Copyright (c) 2023 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file usr_overlay_a.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2023/06/21
 * @license
 * @description
 */


#include "log.h"
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
#if defined ( __CC_ARM )
#pragma arm section code = "overlay_b"
#endif

int overlay_b_exec(void)
{
    printf("#");
    return 0;
}

#if defined ( __CC_ARM )
#pragma arm section
#endif
