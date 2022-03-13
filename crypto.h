/**
 * Copyright (c) 2022 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file crypto.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2022/03/13
 * @license
 * @description
 */

#ifndef __crypto_H_whCyf1Cc_lIvG_HqYQ_smn8_u0RnLw5qXeJq__
#define __crypto_H_whCyf1Cc_lIvG_HqYQ_smn8_u0RnLw5qXeJq__

#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>
//=============================================================================
//                  Constant Definition
//=============================================================================
typedef enum crypto_err
{
    CRYPTO_ERR_OK       = 0,
    CRYPTO_ERR_AUTH_FAIL,
    CRYPTO_ERR_FAIL,

} crypto_err_t;
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct crypto_param
{
    uint32_t    *pCiphertext;
    int         ciphertext_nbytes; // It must be 8-bytes align
    uint32_t    *pPlaintext;
    int         plaintext_nbytes;  // It must be 8-bytes align
    int         packetno;          // the no. of packet, start from '0'
} crypto_param_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
crypto_err_t
crypto_init(char *pUser_key, int user_key_nbyts);

crypto_err_t
crypto_decrypt(crypto_param_t *pParam);

#ifdef __cplusplus
}
#endif

#endif
