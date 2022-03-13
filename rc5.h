/**
 * Copyright (c) 2021 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file rc5.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2022/03/04
 * @license
 * @description
 */

#ifndef __rc5_H_wYuWLLL1_l90y_HCL0_sI96_u1FfwHNgfcP5__
#define __rc5_H_wYuWLLL1_l90y_HCL0_sI96_u1FfwHNgfcP5__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
//=============================================================================
//                  Constant Definition
//=============================================================================
#define WORD_BITS        32     /* word size in bits                                  */

#define KEY_BYTES        16     /* number of bytes in key                             */
#define KEY_WORDS         4     /* number words in key = ceil(8*KEY_BYTES/WORD_BITS)  */

#if 0
// algorithm default
#define ROUNDS_TIMES     12     /* number of rounds                                   */
#define TABLE_SIZE       26     /* size of table S = 2*(ROUNDS_TIMES+1) words         */
#else
#define ROUNDS_TIMES     7     /* number of rounds                                   */
#define TABLE_SIZE       16     /* size of table S = 2*(ROUNDS_TIMES+1) words         */
#endif
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
void rc5_encrypt(uint32_t *pt, uint32_t *ct);
void rc5_decrypt(uint32_t *ct, uint32_t *pt);
void rc5_setup(uint8_t *K);


#ifdef __cplusplus
}
#endif

#endif
