#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "stdint.h"

#define N           100000
#define TEST_TIMES  10


static uint64_t
random(uint64_t low, uint64_t up)
{
    uint64_t x = ( (uint64_t)rand() & 0xFFF0000000000000ULL >> 0 ) |
               ( (uint64_t)rand() & 0xFFF1000000000000ULL >> 12 ) |
               ( (uint64_t)rand() & 0xFFF1000000000000ULL >> 25 ) |
               ( (uint64_t)rand() & 0xFFF1000000000000ULL >> 38 ) |
               ( (uint64_t)rand() & 0xFFF1000000000000ULL >> 41 ) ;

    return x % (up - low + 1) + low;
}

static uint64_t
Montgomery(uint64_t n, uint64_t p, uint64_t m)
{
    uint64_t k = 1;
    n %= m;
    while(p != 1)
    {
        if((p & 1) != 0)
            k = (k * n) % m;
        n = (n * n % m);
        p >>= 1;
    }
    return (n * k) % m;
}


static int
MillerTest(uint64_t n, uint64_t times)
{
    uint64_t x, s, d; // N-1 = d*2^s
    uint64_t a, r, k;
//    uint64_t flag = 0;

    if(n == 2 || n == 3)
        return 1;
    if(!(n & 1))
        return 0; /* filt multi. of 2 */
    /* n-1 = 2^s*d */
    d = n - 1, s = 0;
    while(!(d & 1))
        ++s, d >>= 1;
    for(k = 0; k != times; ++k)
    {
        a = random(1, n - 1); /* a = rand_between(2, n-2) */
        x = Montgomery(a, d, n); /* x=a^d mod n */
        if(x == 1 || x == n - 1)
            continue;
        for(r = 0; r != s; ++r)
        {
            if(x == n - 1)
                break;
            // x = Montgomery(x, 2, n);
            x = (x * x) % n;
        }
        if(r < s)
            continue;
        return 0;
    }
    return 1;
}

/**
 *  @brief  prime_num_generator()
 *              Get the prime number and more than start_num
 *
 *  @param [in] start_num       the start number, range: start_num ~ N
 *  @return
 *
 */
uint64_t prime_num_generator(uint64_t start_num)
{
    uint64_t  prime_num = 0;
    do {
        if( start_num >= N )
            break;

        // between 2 ~ N
        for(prime_num = start_num; prime_num != N; ++prime_num)
        {
            int     is_prime = MillerTest(prime_num, TEST_TIMES);

            if( is_prime )  break;
        }
    } while(0);

    return prime_num;
}
