#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "rc5.h"


static uint32_t      g_plaintext[3][2] = {{0},};
static uint32_t      g_decryptedtext[3][2] = {{0},};
static uint32_t      g_ciphertext[3][2] = {{0},};

void main()
{
    unsigned char key[KEY_BYTES] =
    {
        0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
        0x01, 0x12, 0x23, 0x34, 0x45, 0x56, 0x67, 0x78,
    };

    srand(time(0));

    rc5_setup((uint8_t*)&key);

    for(int i = 0; i < 3; i++)
    {
        for(int j = 0; j < 2; j++)
        {
            g_plaintext[i][j] = (uint32_t)rand();
        }
    }

    printf("\n\nPlaintext\n");

    for(int i = 0; i < 3; i++)
    {
        printf("\n");
        for(int j = 0; j < 2; j++)
        {
            printf("0x%08X " , g_plaintext[i][j]);
        }
    }

    printf("\n\nCiphertext\n");
    for(int i = 0; i < 3; i++)
    {
        printf("\n");
        rc5_encrypt(&g_plaintext[i], &g_ciphertext[i]);
        for(int j = 0; j < 2; j++)
        {
            printf("0x%08X " , g_ciphertext[i][j]);
        }
    }

    printf("\n\nDecryption\n");
    memset((void*)&g_decryptedtext, 0xAA, sizeof(g_decryptedtext));
    for(int i = 0; i < 3; i++)
    {
        printf("\n");
        rc5_decrypt(g_ciphertext[i], g_decryptedtext[i]);
        for(int j = 0; j < 2; j++)
        {
            printf("0x%08X " , g_decryptedtext[i][j]);
        }
    }

    printf("\n\n");
    system("pause");
    return;
}
