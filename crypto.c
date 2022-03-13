/**
 * Copyright (c) 2022 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file crypto.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2022/03/13
 * @license
 * @description
 */


#include <string.h>
#include "crypto.h"

#include "hmac_sha1.h"
#include "rc5.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
#define CONFIG_INTERNAL_MESSAGE         "2022WL03"
#define INTERNAL_KEY                    20220308
//=============================================================================
//                  Macro Definition
//=============================================================================
#define FOURCC(a, b, c, d)      ((((a) & 0xFF) << 24) | (((b) & 0xFF) << 16) | (((c) & 0xFF) << 8) | ((d) & 0xFF))
//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct cry_head
{
    uint32_t    tag[2];
    uint32_t    version;
    uint32_t    kdf[5];
    uint32_t    bin_size;
    uint32_t    crc32;

} cry_head_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================
static char         *g_pInternal_msg = CONFIG_INTERNAL_MESSAGE;
static uint8_t      g_key_extend[HAMC_SHA1_DIGEST_SIZE] = {0};

static cry_head_t   g_header = {0};
//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
crypto_err_t
crypto_init(char *pUser_key, int user_key_nbyts)
{
    crypto_err_t    rval = CRYPTO_ERR_OK;

    hmac_sha1((uint8_t*)pUser_key, user_key_nbyts,
              (uint8_t*)g_pInternal_msg, strlen(g_pInternal_msg),
              g_key_extend);

    rc5_setup((uint8_t*)&g_key_extend);
    return rval;
}

crypto_err_t
crypto_decrypt(crypto_param_t *pParam)
{
    crypto_err_t    rval = CRYPTO_ERR_OK;
    uint32_t        *pPlaintext = 0;
    uint32_t        tmp_plaintext[2] = {0};

    if( pParam->packetno == 0 )
    {
        // first packet
        uint32_t    cur_word_idx = 0;
        int         dec_words = 0;

        rc5_decrypt((uint32_t*)&pParam->pCiphertext[cur_word_idx], (uint32_t*)&tmp_plaintext);

        if( tmp_plaintext[0] != FOURCC('Z', 'B', 'i', 't') ||
            tmp_plaintext[1] != FOURCC('S', 'e', 'm', 'i') )
        {
            rval = CRYPTO_ERR_AUTH_FAIL;
            return rval;
        }

        cur_word_idx = sizeof(g_header.tag)/sizeof(g_header.tag[0]);
        pPlaintext = (uint32_t*)((uint32_t)&g_header + sizeof(g_header.tag));
        for(int i = 0; i < (sizeof(g_header) - sizeof(g_header.tag)) >> 2; i++)
        {
            pPlaintext[i] = pParam->pCiphertext[cur_word_idx++] ^ (uint32_t)INTERNAL_KEY;
        }

        pPlaintext = pParam->pPlaintext;
        dec_words  = (pParam->ciphertext_nbytes >> 2) - cur_word_idx;

        pParam->plaintext_nbytes = dec_words << 2;

        for(int i = 0; i < dec_words; i += 2)
        {
            rc5_decrypt(&pParam->pCiphertext[cur_word_idx], (uint32_t*)&tmp_plaintext);
            cur_word_idx += 2;

            pPlaintext[i]     = tmp_plaintext[0];
            pPlaintext[i + 1] = tmp_plaintext[1];
        }
    }
    else
    {
        pPlaintext = pParam->pPlaintext;
        for(int i = 0; i < (pParam->ciphertext_nbytes >> 2); i += 2)
        {
            rc5_decrypt((uint32_t*)&pParam->pCiphertext[i], (uint32_t*)&tmp_plaintext);
            pPlaintext[i]     = tmp_plaintext[0];
            pPlaintext[i + 1] = tmp_plaintext[1];
        }
    }
    return rval;
}






