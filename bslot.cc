/**
 * Copyright (c) 2019 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file flag_buf.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2019/01/22
 * @license
 * @description
 */


#include "bslot.h"

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
/**
 * get_first_zero_bit
 *
 * Provides position of first 0 bit in a 32 bit value
 *
 * @param value - given value
 *
 * @return - 0th bit position
 */
static unsigned int
_bslot_get_first_zero_bit(uint32_t value)
{
    unsigned int    idx;
    unsigned int    tmp32;

    /* Invert value */
    value = ~value;

    /* (~value) & (2's complement of value) */
    value = (value & (-value)) - 1;

    /* log2(value) */

    tmp32 = value - ((value >> 1) & 033333333333)
            - ((value >> 2) & 011111111111);

    idx = ((tmp32 + (tmp32 >> 3)) & 030707070707) % 63;

    return idx;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================

/**
 * bslot_get_buffer
 *
 * This function provides unique 32 bit buffer slot index.
 *
 * @param bitmap        - bit map for buffer
 * @param bitmap_len    - length of bitmap
 *
 * return - a unique buffer
 */
int
bslot_get_buffer(
    unsigned long   *bitmap,
    int             bitmap_len)
{
    int     idx = -1;
    int     i, tmp32;

    /* Find first available buffer */
    for (i = 0; i < bitmap_len; i++)
    {
        tmp32 = _bslot_get_first_zero_bit(bitmap[i]);

        if (tmp32 < 32)
        {
            idx = tmp32 + (i << 5);
            bitmap[i] |= (1 << tmp32);
            break;
        }
    }

    return idx;
}

/**
 * bslot_release_buffer
 *
 * Frees the given buffer.
 *
 * @param bitmap        - bit map for buffers
 * @param bitmap_len    - size of bitmap
 * @param idx           - free the buffer slot index
 *
 * return - none
 */
int
bslot_release_buffer(
    unsigned long   *bitmap,
    int             bitmap_len,
    int             idx)
{
    unsigned int    i, j;
    unsigned long   mask = 1;

    if (idx >= (bitmap_len << 5))
        return -1;

    /* Mark the idx as available */
    i = idx >> 5;
    j = idx & 31;

    mask = mask << j;
    bitmap[i] = bitmap[i] & (~mask);

    return 0;
}

/**
 * bslot_is_buffer_set
 *
 * Checks whether buffer is used or free.
 *
 * @param bitmap        - bit map for buffers
 * @param bitmap_len    - size of bitmap
 * @param idx           - buffer slot index
 *
 * return - TRUE/FALSE
 */
int
bslot_is_buffer_set(
    unsigned long   *bitmap,
    int             bitmap_len,
    int             idx)
{
    int i, j;
    unsigned long mask = 1;

    if (idx >= (bitmap_len << 5) )
        return -1;

    /* Mark the id as available */
    i = idx >> 5;
    j = idx & 31;
    mask = mask << j;

    return (bitmap[i] & mask);
}

/**
 * bslot_set_buffer
 *
 * Marks the buffer as consumed.
 *
 * @param bitmap        - bit map for buffer
 * @param bitmap_len    - size of bitmap
 * @param idx           - buffer slot index
 *
 * return - none
 */
int
bslot_set_buffer(
    unsigned long   *bitmap,
    int             bitmap_len,
    int             idx)
{
    int             i, j;
    unsigned long   mask = 1;

    if (idx >= (bitmap_len << 5))
        return -1;

    /* Mark the id as available */
    i = idx >> 5;
    j = idx & 31;
    mask = mask << j;
    bitmap[i] |= mask;

    return 0;
}
