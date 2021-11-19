
#include <stdint.h>
#include "kdf.h"

static uint8_t      g_AES128key[16] =
{
    0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
    0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
};


int main(int argc, const char *argv[])
{
    //	Simple "string" dependent of the aplication, and diferent from each one
    unsigned char context[] = { 0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70,
                                0x80, 0x90, 0xa0, 0xb0, 0xc0, 0xd0, 0xe0, 0x00
                              };

    /* The number of keys to derive (each one with of 128 bits) */
    int numKeys = 2;

    unsigned char devKey[BLOCK_SIZE * numKeys];

    deriveKey(g_AES128key, context, numKeys, devKey);

    int e;
    printf("\n DEV_KEY \n");
    for(e = 0; e < BLOCK_SIZE * numKeys; e++)
    {
        if( e && !(e & 0xF) )
            printf("\n");
        printf("%02X, ", devKey[e] & 0xff);
    }

    printf("\n");

    system("pause");
    return 0;
}
