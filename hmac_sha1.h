/**
 * Copyright (c) 2022 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file hmac_sha1.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2022/03/04
 * @license
 * @description
 */

#ifndef __hmac_sha1_H_w607GMN9_loMX_HB2l_sWIO_u9wYdvvUS8wY__
#define __hmac_sha1_H_w607GMN9_loMX_HB2l_sWIO_u9wYdvvUS8wY__

#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>
//=============================================================================
//                  Constant Definition
//=============================================================================
#define HAMC_SHA1_DIGEST_SIZE           20 /* bytes */
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
void hmac_sha1(const uint8_t *key, const uint32_t key_length,
               const uint8_t *data, const uint32_t data_length, uint8_t *digest);


#ifdef __cplusplus
}
#endif

#endif
