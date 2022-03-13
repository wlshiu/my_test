/****************************************************************/
/* 802.11i HMAC-SHA-1 Test Code                                 */
/* Copyright (c) 2002, David Johnston                           */
/* Author: David Johnston                                       */
/* Email (home): dj@deadhat.com                                 */
/* Email (general): david.johnston@ieee.org                     */
/* Version 0.1                                                  */
/*
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
/*                                                              */
/* This code implements the NIST HMAC-SHA-1 algorithm as used   */
/* the IEEE 802.11i security spec.                              */
/*                                                              */
/* Supported message length is limited to 4096 characters       */
/* ToDo:                                                        */
/*   Sort out endian tolerance. Currently little endian.        */
/****************************************************************/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "hmac_sha1.h"

#define ELEM_COUNT(_array)			((uint32_t)(sizeof(_array)/sizeof(_array[0])))

#define MAX_MESSAGE_LENGTH          4096


void sha1(const uint8_t *pBuffer, uint32_t nSize, uint8_t *sha1)
{
    uint32_t SHA1_tmp;

#define SHA1_ROTL(a, b) \
            (SHA1_tmp = (a), ((SHA1_tmp >> (32 - b)) & (0x7fffffff >> (31 - b))) | (SHA1_tmp << b))

#define SHA1_F(B, C, D, t) \
            ((t < 40) ? \
            ((t < 20) ? ((B & C) | ((~B) & D)) : (B ^ C ^ D)) :\
            ((t < 60) ? ((B & C) | (B & D) | (C & D)) : (B ^ C ^ D)))

#define MAX_STACK_ALLOC_SIZE    2048

    uint32_t i, K[80], W[80];
    uint32_t nH0 = 0x67452301;
    uint32_t nH1 = 0xEFCDAB89;
    uint32_t nH2 = 0x98BADCFE;
    uint32_t nH3 = 0x10325476;
    uint32_t nH4 = 0xC3D2E1F0;

    for(i = 0; i < 20; i++)
        K[i] = 0x5A827999;
    for(i = 20; i < 40; i++)
        K[i] = 0x6ED9EBA1;
    for(i = 40; i < 60; i++)
        K[i] = 0x8F1BBCDC;
    for(i = 60; i < 80; i++)
        K[i] = 0xCA62C1D6;

    uint32_t nTemp = nSize + ((nSize % 64 > 56)
                 ? (128 - nSize % 64) : (64 - nSize % 64));

    uint8_t *pTemp = 0;
    pTemp = (uint8_t *)malloc(nTemp);

    for(i = 0; i < nSize; i++)
        pTemp[i + 3 - 2 * (i % 4)] = pBuffer[i];

    for(pTemp[i + 3 - 2 * (i % 4)] = 128, i++; i < nTemp; i++)
        pTemp[i + 3 - 2 * (i % 4)] = 0;

    *((uint32_t *)(pTemp + nTemp - 4)) = nSize << 3;
    *((uint32_t *)(pTemp + nTemp - 8)) = nSize >> 29;

    for(uint8_t *ppend = pTemp + nTemp; pTemp < ppend; pTemp += 64)
    {
        for(i = 0; i < 16; i++)
            W[i] = ((int *)pTemp)[i];

        for(i = 16; i < 80; i++)
            W[i] = SHA1_ROTL((W[i - 3] ^ W[i - 8] ^ W[i - 14] ^ W[i - 16]), 1);

        uint32_t A = nH0, B = nH1, C = nH2, D = nH3, E = nH4;
        for(i = 0; i < 80; i++)
        {
            uint32_t TEMP = SHA1_ROTL(A, 5) + SHA1_F(B, C, D, i) + E + W[i] + K[i];
            E = D, D = C, C = SHA1_ROTL(B, 30), B = A, A = TEMP;
        }
        nH0 += A, nH1 += B, nH2 += C, nH3 += D, nH4 += E;
    }

    uint32_t aryResult[] = { nH0, nH1, nH2, nH3, nH4 };
    for(i = 0; i < ELEM_COUNT(aryResult); i++)
    {
        sha1[i * 4 + 0] = (uint8_t)(aryResult[i] >> 24);
        sha1[i * 4 + 1] = (uint8_t)(aryResult[i] >> 16);
        sha1[i * 4 + 2] = (uint8_t)(aryResult[i] >> 8);
        sha1[i * 4 + 3] = (uint8_t)(aryResult[i]);
    }

    free(pTemp);
    return;
}

/******************************************************/
/* hmac-sha1()                                        */
/* Performs the hmac-sha1 keyed secure hash algorithm */
/******************************************************/
void hmac_sha1(const uint8_t *key, const uint32_t key_length,
               const uint8_t *data, const uint32_t data_length, uint8_t *digest)
{
    const int nBlockSize = 64; /* blocksize */
    uint8_t ipad = 0x36;
    uint8_t opad = 0x5c;

    uint8_t k0[64];
    uint8_t k0xorIpad[64];
    uint8_t step7data[64];
    uint8_t step5data[MAX_MESSAGE_LENGTH + 128];
    uint8_t step8data[nBlockSize + 20];

    memset(k0, 0, sizeof(k0));
    memset(k0xorIpad, 0, sizeof(k0xorIpad));
    memset(step7data, 0, sizeof(step7data));
    memset(step5data, 0, sizeof(step5data));
    memset(step8data, 0, sizeof(step8data));

    if (key_length != nBlockSize)    /* Step 1 */
    {
        /* Step 2 */
        if (key_length > nBlockSize)
        {
            sha1(key, key_length, digest);
            for(uint32_t i = 0; i < HAMC_SHA1_DIGEST_SIZE; ++i)
            {
                k0[i] = digest[i];
            }
        }
        else if (key_length < nBlockSize)  /* Step 3 */
        {
            memcpy(k0, key, key_length);
        }
    }
    else
    {
        memcpy(k0, key, nBlockSize);
//        for(uint32_t i = 0; i < nBlockSize; ++i)
//        {
//            k0[i] = key[i];
//        }
    }

    /* Step 4 */
    for(uint32_t i = 0; i < 64; ++i)
    {
        k0xorIpad[i] = k0[i] ^ ipad;
    }

    /* Step 5 */
#if 0
    for(uint32_t i = 0; i < 64; ++i)
    {
        step5data[i] = k0xorIpad[i];
    }

    for(uint32_t i = 0; i < data_length; ++i)
    {
        step5data[i + 64] = data[i];
    }
#else
    memcpy(&step5data[0], &k0xorIpad[0], 64);
    memcpy(&step5data[64], &data[0], data_length);
#endif

    /* Step 6 */
    sha1(step5data, data_length + nBlockSize, digest);

    /* Step 7 */
    for(uint32_t i = 0; i < 64; ++i)
    {
        step7data[i] = k0[i] ^ opad;
    }

    /* Step 8 */
#if 0
    for(uint32_t i = 0; i < nBlockSize; ++i)
    {
        step8data[i] = step7data[i];
    }

    for(uint32_t i = 0; i < HAMC_SHA1_DIGEST_SIZE; ++i)
    {
        step8data[i + nBlockSize] = digest[i];
    }
#else
    memcpy(&step8data[0], &step7data[0], nBlockSize);
    memcpy(&step8data[nBlockSize], &digest[0], HAMC_SHA1_DIGEST_SIZE);
#endif

    /* Step 9 */
    sha1(step8data, sizeof(step8data), digest);
}
