/* RC5REF.C -- Reference implementation of RC5-32/12/16 in C.        */
/* Copyright (C) 1995 RSA Data Security, Inc.                        */


#include "rc5.h"

static uint32_t S[TABLE_SIZE];                      /* expanded key table          */

/* Rotation operators. x must be unsigned, to get logical right shift*/
#define ROTL(x, y)      (((x)<<((y)&((WORD_BITS)-1))) | ((x)>>((WORD_BITS)-((y)&((WORD_BITS)-1)))))
#define ROTR(x, y)      (((x)>>((y)&((WORD_BITS)-1))) | ((x)<<((WORD_BITS)-((y)&((WORD_BITS)-1)))))

void rc5_encrypt(uint32_t *pt, uint32_t *ct) /* 2 WORD input pt/output ct    */
{
    uint32_t  i;
    uint32_t  A = pt[0] + S[0];
    uint32_t  B = pt[1] + S[1];
    for (i = 1; i <= ROUNDS_TIMES; i++)
    {
        A = ROTL(A ^ B, B) + S[2 * i];
        B = ROTL(B ^ A, A) + S[2 * i + 1];
    }
    ct[0] = A;
    ct[1] = B;
    return;
}

void rc5_decrypt(uint32_t *ct, uint32_t *pt) /* 2 WORD input ct/output pt    */
{
    uint32_t i;
    uint32_t B = ct[1];
    uint32_t A = ct[0];
    for (i = ROUNDS_TIMES; i > 0; i--)
    {
        B = ROTR(B - S[2 * i + 1], A) ^ A;
        A = ROTR(A - S[2 * i], B) ^ B;
    }
    pt[1] = B - S[1];
    pt[0] = A - S[0];
    return;
}

void rc5_setup(uint8_t *K) /* secret input key K[0...b-1]      */
{
    uint32_t P = 0xb7e15163, Q = 0x9e3779b9;     /* magic constants             */

    uint32_t i, j, k, u = WORD_BITS / 8, A, B, L[KEY_WORDS];

    /* Initialize L, then S, then mix key into S */
    for (i = KEY_BYTES - 1, L[KEY_WORDS - 1] = 0; i != -1; i--)
        L[i / u] = (L[i / u] << 8) + K[i];

    for (S[0] = P, i = 1; i < TABLE_SIZE; i++)
        S[i] = S[i - 1] + Q;

    for (A = B = i = j = k = 0;
         k < 3 * TABLE_SIZE;
         k++, i = (i + 1) % TABLE_SIZE, j = (j + 1) % KEY_WORDS) /* 3*t > 3*c */
    {
        A = S[i] = ROTL(S[i] + (A + B), 3);
        B = L[j] = ROTL(L[j] + (A + B), (A + B));
    }
    return;
}
