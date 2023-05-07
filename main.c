#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

/* Based on http://home.thep.lu.se/~bjorn/crc/crc32_simple.c */
static uint32_t     g_table[0x100];


static uint32_t _gen_rand_value(int base)
{
#define FOURCC(a, b, c, d)      (((a) << 24) | ((b) << 16) | ((c) << 8) | (d))

    static uint32_t     seed = FOURCC('u', 'n', 'k', 'n');
    uint32_t    r = 0;

    if( seed == FOURCC('u', 'n', 'k', 'n') )
    {
        srand((base < 0) ? time(0) : base);
        seed = base;
    }

    r = (base < 0) ? rand() : base;

    for (int j = 0; j < 8; ++j)
        r = (r & 1 ? 0 : (uint32_t)0xEDB88320L) ^ r >> 1;

    r = r ^ (uint32_t)0xFF000000L;
    return r;
}

void mac_addr(const void *data, size_t n_bytes, uint32_t* mac_high, uint32_t* mac_low)
{
    for (size_t i = 0; i < sizeof(g_table) >> 2; ++i)
    {
    #if 0
        uint32_t    r = i;

        r = rand();

        for (int j = 0; j < 8; ++j)
            r = (r & 1 ? 0 : (uint32_t)0xEDB88320L) ^ r >> 1;

        g_table[i] = r ^ (uint32_t)0xFF000000L;
    #else
        g_table[i] = _gen_rand_value(-1);
    #endif
    }

    *mac_low = 0;
    for (size_t i = 0; i < n_bytes; ++i)
        *mac_low = g_table[(uint8_t) * mac_low ^ ((uint8_t*)data)[i]] ^ *mac_low >> 8;

    *mac_high = 0;
    for (size_t i = 0; i < n_bytes; ++i)
        *mac_high = g_table[(uint8_t) * mac_high ^ ((uint8_t*)data)[n_bytes - i - 1]] ^ *mac_high >> 8;

    *mac_high = *mac_high & 0xFFFF;
    return;
}

int main(int ac, char** av)
{
    const uint8_t   chip_id1[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    const uint8_t   chip_id2[8] = {1, 2, 3, 4, 5, 6, 7, 9};
    const uint8_t   chip_id3[8] = {0, 2, 3, 4, 5, 6, 7, 8};
    const uint8_t   chip_id4[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    const uint8_t   chip_id5[8] = {1, 1, 1, 1, 1, 1, 1, 1};

    uint32_t    mac_high;
    uint32_t    mac_low;

    srand(time(0));

    /* test 1 */
    mac_addr(chip_id1, 8, &mac_high, &mac_low);
    printf("%02X-%02X-%02X-%02X-%02X-%02X\n",
           (mac_high >> 8) & 0xFF,
           (mac_high) & 0xFF,
           (mac_low >> 24) & 0xFF,
           (mac_low >> 16) & 0xFF,
           (mac_low >> 8) & 0xFF,
           (mac_low) & 0xFF);

    /* test 2 */
    mac_addr(chip_id2, 8, &mac_high, &mac_low);
//    printf("%04x%08x\n", mac_high, mac_low);
    printf("%02X-%02X-%02X-%02X-%02X-%02X\n",
           (mac_high >> 8) & 0xFF,
           (mac_high) & 0xFF,
           (mac_low >> 24) & 0xFF,
           (mac_low >> 16) & 0xFF,
           (mac_low >> 8) & 0xFF,
           (mac_low) & 0xFF);

    /* test 3 */
    mac_addr(chip_id3, 8, &mac_high, &mac_low);
//    printf("%04x%08x\n", mac_high, mac_low);
    printf("%02X-%02X-%02X-%02X-%02X-%02X\n",
           (mac_high >> 8) & 0xFF,
           (mac_high) & 0xFF,
           (mac_low >> 24) & 0xFF,
           (mac_low >> 16) & 0xFF,
           (mac_low >> 8) & 0xFF,
           (mac_low) & 0xFF);

    /* test 4 */
    mac_addr(chip_id4, 8, &mac_high, &mac_low);
//    printf("%04x%08x\n", mac_high, mac_low);
    printf("%02X-%02X-%02X-%02X-%02X-%02X\n",
           (mac_high >> 8) & 0xFF,
           (mac_high) & 0xFF,
           (mac_low >> 24) & 0xFF,
           (mac_low >> 16) & 0xFF,
           (mac_low >> 8) & 0xFF,
           (mac_low) & 0xFF);

    /* test 5 */
    mac_addr(chip_id5, 8, &mac_high, &mac_low);
//    printf("%04x%08x\n", mac_high, mac_low);
    printf("%02X-%02X-%02X-%02X-%02X-%02X\n",
           (mac_high >> 8) & 0xFF,
           (mac_high) & 0xFF,
           (mac_low >> 24) & 0xFF,
           (mac_low >> 16) & 0xFF,
           (mac_low >> 8) & 0xFF,
           (mac_low) & 0xFF);

    system("pause");
    return 0;
}
