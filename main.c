#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>
#include <windows.h>
#include "stdint.h"

#define uintx_t     uint64_t

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
    uintx_t d;
    for (d = 2; d < r; d++)
    {
        uintx_t     ed = _mul(e, d); // re = (e*d) % r;
        uintx_t     re = _mod(ed, r);
        if (re == 1)
        {
            printf("e=%" PRIu64 ", d=%" PRIu64 ", r=%" PRIu64 ", (e*d) _mod r=%" PRIu64 "\n", e, d, r, re);
            return d;
        }
    }
    assert(0);
}

/*
uintx_t power(uintx_t a, uintx_t k, uintx_t N)
{
  uintx_t p=1, i;
  for (i=1; i<=k; i++) {
    p = _mul(p, a); // p = (p * a) % N;
    p = _mod(p, N);
  }
  return p;
}
*/

#if 1
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

#else
uintx_t power(uintx_t a, uintx_t k, uintx_t N)
{
    if (k < 0)          assert(0);
    if (k == 0)         return 1;
    else if (k == 1)    return a;

    // k >=2
    uintx_t k2 = k >> 1;           // k2 = k / 2;
    uintx_t re = k & 0x1;           // re = k % 2;
    uintx_t ak2 = power(a, k2, N); // ak2 = a^(k/2);
    uintx_t ak = ak2 * ak2;       // ak  = ak2*ak2 = a^((k/2)*2)
    uintx_t akN = ak % N;         // akN = ak % N
    if (re == 1)                 // if k is odd
    {
        akN = akN * a;             //   ak = ak*a;
        return akN % N;            //   return ak * k;
    }
    else                         // else
        return akN;
}
#endif


int main(void)
{
    uintx_t     public_key = 0;
    uintx_t     private_key = 0;
    uintx_t     N = 0;

    {   // generate key-pair
        #if 0
        uintx_t p = newBigInt("2213"), q = newBigInt("2663");
        #else
        uintx_t     p = prime_num_generator(2500);
        uintx_t     q = prime_num_generator(5000);
        #endif
        uintx_t     r = _mul(p - 1, q - 1);

        printf("p= %" PRIu64 ", q= %" PRIu64 "\n", p, q);

        N = _mul(p, q);
        printf("N= %s, r= %s\n", big2str(N), big2str(r));

        public_key = newBigInt("4723");
        private_key = inv(public_key, r);

        printf("\n\n");
        printf("Public key  = %" PRIu64 "\n", public_key);
        printf("Private key = %" PRIu64 "\n\n", private_key);
    }

    do {
        uintx_t     plaintext_msg = 0;
        uintx_t     ciphertext = 0;
        uintx_t     plaintext_msg2 = 0;

        // generate data
//        plaintext_msg = newBigInt("3320");
        plaintext_msg = newBigInt("9999999"); // support max: 9999999
        printf("plaintext_msg= '%s'\n", big2str(plaintext_msg));

        // encrypt
        ciphertext = power(plaintext_msg, public_key, N);
        printf("ciphertext= '%s'\n", big2str(ciphertext));

        // decrypt
        plaintext_msg2 = power(ciphertext, private_key, N);
        printf("plaintext_msg2= '%s'\n", big2str(plaintext_msg2));
    } while(0);


    while(1)
        __asm("nop");
    return 0;
}

