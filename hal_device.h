/**
 * Copyright (c) 2024 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file hal_device.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2024/09/10
 * @license
 * @description
 */

#ifndef __hal_device_H_wh21It4q_lc5o_HN4v_sYN9_uMvM6ZVOvOoN__
#define __hal_device_H_wh21It4q_lc5o_HN4v_sYN9_uMvM6ZVOvOoN__

#ifdef __cplusplus
extern "C" {
#endif

#include "hal_sim.h"
//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================
/**
 *  \brief  Count leading zeros
 *
 *  \param [in] x       Value to count the leading zeros
 *  \return
 *      Number of leading zeros in value
 */
__STATIC_FORCEINLINE uint8_t HAL_CLZ(uint32_t x)
{
    uint8_t     n = 0;

    if( x == 0 )    return 32;

    if( x <= 0x0000FFFFul ) { n += 16; x <<= 16; }
    if( x <= 0x00FFFFFFul ) { n += 8; x <<= 8; }
    if( x <= 0x0FFFFFFFul ) { n += 4; x <<= 4; }
    if( x <= 0x3FFFFFFFul ) { n += 2; x <<= 2; }
    if( x <= 0x7FFFFFFFul ) { n += 1; x <<= 1; }
    return n;
}


/**
 *  \brief  Count the amount of bit-1
 *
 *  \param [in] x       Value to count the amount of bit-1
 *  \return
 *      The amount of bit-1
 */
__STATIC_FORCEINLINE int HAL_PopCount(unsigned int x)
{
    x = (x & 0x55555555) + ((x & 0xaaaaaaaa) >> 1);
    x = (x & 0x33333333) + ((x & 0xcccccccc) >> 2);
    x = (x & 0x0f0f0f0f) + ((x & 0xf0f0f0f0) >> 4);
    x = (x & 0x00ff00ff) + ((x & 0xff00ff00) >> 8);
    x = (x & 0x0000ffff) + ((x & 0xffff0000) >> 16);
    return x;
}


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


#ifdef __cplusplus
}
#endif

#endif
