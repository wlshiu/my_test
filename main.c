#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>
#include <time.h>
#include <windows.h>
#include "stdint.h"

#define uintx_t     uint64_t

#define TWOCC(a, b)     ((((a) & 0xFF) << 8) | ((b) & 0xFF))

typedef struct key
{
    uintx_t         N;
    uintx_t         public_key;
    uintx_t         private_key;

} key_t;

typedef struct cypher
{
    uint32_t    data[10];

} cypher_t;

extern uint64_t prime_num_generator(uint64_t start_num);

uintx_t newBigInt(char *str)
{
    return atoi(str);
}

char tempStr[1025];

char *big2str(uintx_t a)
{
    sprintf(tempStr, "%" PRIu64, a);
    return tempStr;
}

static uintx_t
_mul(uintx_t a, uintx_t b)
{
    return a * b;
}

static uintx_t
_div(uintx_t a, uintx_t b)
{
    return a / b;
}

static uintx_t
_mod(uintx_t a, uintx_t n)
{
    return a % n;
}

uintx_t inv(uintx_t e, uintx_t r)
{
    uintx_t     d;
    for(d = 2; d < r; d++)
    {
        uintx_t     ed = _mul(e, d); // re = (e*d) % r;
        uintx_t     re = _mod(ed, r);
        if (re == 1)
        {
            // printf("e=%" PRIu64 ", d=%" PRIu64 ", r=%" PRIu64 ", (e*d) _mod r=%" PRIu64 "\n", e, d, r, re);
            break;
        }
    }
    return d;
}

uintx_t power(uintx_t a, uintx_t k, uintx_t N)
{
    uintx_t     l_out = 1;

    while( k != 0 )
    {
        if( k & 0x01 )
        {
            l_out *= a;

            l_out = _mod(l_out, N);
        }
        a *= a;
        a = _mod(a, N);
        k = k >> 1;
    }
    return l_out;
}

int key_generator(key_t *pKey)
{
    int         rval = 0;
    uintx_t     r;

    memset(pKey, 0x0, sizeof(key_t));

    do {
        do {
            uintx_t     p = prime_num_generator(2500);
            uintx_t     q = prime_num_generator(5000);

            r = _mul(p - 1, q - 1);
            pKey->N = _mul(p, q);

        } while( !pKey->N );

        pKey->public_key = 0;

        while( pKey->public_key == 0 )
            pKey->public_key = rand() & 0xFFF;

        pKey->private_key = inv(pKey->public_key, r);

    } while( pKey->private_key == r );

    return rval;
}

int encrypt(char *pMsg, key_t *pKey, cypher_t *pCiphertext)
{
    int     rval = 0;
    char    *pBuf = 0;
    do {
        int     len = 0;
        if( !pMsg || ((strlen(pMsg) + 1 ) >> 1) > (sizeof(cypher_t) >> 2) )
        {
            rval = -1;
            break;
        }

        len = (sizeof(cypher_t) >> 1);

        if( !(pBuf = malloc(len)) )
        {
            rval = -1;
            break;
        }

        memset(pBuf, 0x0, len);

        len = (strlen(pMsg) + 1) & ~0x1;

        for(int i = 0; i < len; i += 2)
        {
            uintx_t     plaintext = (*(pMsg + i) << 8) | (*(pMsg + i + 1));
            pCiphertext->data[i >> 1] = power(plaintext, pKey->public_key, pKey->N);
        }

    } while(0);

    if( pBuf )  free(pBuf);
    return rval;
}

int
decrypt(key_t *pKey, cypher_t *pCiphertext, char *pMsg_buf, int msg_len)
{
    int     rval = 0;
    do {
        int     pos = 0;
        memset(pMsg_buf, 0x0, msg_len);

        for(int i = 0; i < sizeof(cypher_t) >> 2; i++)
        {
            uintx_t     plaintext = 0;

            if( !pCiphertext->data[i] )
                continue;

            plaintext = power(pCiphertext->data[i], pKey->private_key, pKey->N);

            pMsg_buf[pos]     = (plaintext >> 8) & 0xFF;
            pMsg_buf[pos + 1] = plaintext & 0xFF;
            pos += 2;
        }

    } while(0);
    return rval;
}

int main(void)
{
    int     cnt = 0;

    srand((unsigned int)time(NULL));

    do {
        int         rval = 0;
        key_t       key = {.N = 0,};
        cypher_t    cypher = {0};
        char        *pMsg = "test1234";
        char        msg_buf[128] = {0};

        // generate key-pair
        rval = key_generator(&key);
        if( rval )
        {
            printf("key generate fail \n");
            break;
        }

        printf("\n\n");
        printf("Public key  = %" PRIu64 "\n", key.public_key);
        printf("Private key = %" PRIu64 "\n\n", key.private_key);

        // encrypt
        memset(&cypher, 0x0, sizeof(cypher));
        rval = encrypt(pMsg, &key, &cypher);
        if( rval )
        {
            printf("encrypt fail \n");
            break;
        }

        // decrypt
        rval = decrypt(&key, &cypher, msg_buf, sizeof(msg_buf));
        if( rval )
        {
            printf("decrypt fail \n");
            break;
        }

        printf("%02d-th: plaintext_msg: %s\n", cnt, msg_buf);
    } while( cnt++ < 10 );


    while(1)
        __asm("nop");
    return 0;
}

