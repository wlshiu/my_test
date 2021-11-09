#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define CRC16POLY1      0x1021U
#define CRC16POLY2      0x8408U  /* left-right reversal */

#define CRC16_SART_VALUE    0xFFFFu

#define CONFIG_BUF_SIZE     512

static uint32_t     g_words_buf[CONFIG_BUF_SIZE >> 2] = {0};

static void usage(char *prog)
{
    printf("usage: %s <file name>\n", prog);
    exit(-1);
}


static uint16_t calc_crc16(uint8_t c[], int n)
{
	int i, j;
	unsigned long r;

	r = 0xFFFFU;
	for (i = 0; i < n; i++)
    {
		r ^= c[i];
		for (j = 0; j < 8; j++)
		{
			if (r & 1) r = (r >> 1) ^ CRC16POLY2;
			else       r >>= 1;
		}
	}
	return r ^ 0xFFFFU;
}

static uint16_t calc_crc16_ex(uint8_t c[], int n, uint16_t crc16)
{
	int i, j;

	for (i = 0; i < n; i++)
    {
		crc16 ^= c[i];
		for (j = 0; j < 8; j++)
		{
			if (crc16 & 1)
                crc16 = (crc16 >> 1) ^ CRC16POLY2;
			else
                crc16 >>= 1;
		}
	}
	return crc16 ^ 0xFFFFU;
}

int main(int argc, char **argv)
{
    FILE        *fin = 0;
    uint8_t     *pBuf = 0;

    do {
        uint32_t    filesize = 0;
        uint16_t    crc16_value = 0;

        if( argc < 2 )
        {
            usage(argv[0]);
            break;
        }

        if( !(fin = fopen(argv[1], "rb")) )
        {
            printf("can't open %s\n", argv[1]);
            break;
        }

        fseek(fin, 0x0, SEEK_END);
        filesize = ftell(fin);
        fseek(fin, 0x0, SEEK_SET);

        if( !(pBuf = malloc(filesize)) )
        {
            printf("malloc %d fail \n", filesize);
            break;
        }

        fread(pBuf, 1, filesize, fin);

        crc16_value = calc_crc16(pBuf, filesize);
        printf("1. crc16= 0x%04X\n", crc16_value);
#if 1
        crc16_value = calc_crc16_ex(pBuf, filesize, CRC16_SART_VALUE);
        printf("2. crc16= 0x%04X\n", crc16_value);

        {
            uint8_t     *pCur = pBuf;
            uint32_t    remain = filesize;

            crc16_value = CRC16_SART_VALUE;

            while( remain )
            {
                uint32_t    length = 0;

                length = (remain < sizeof(g_words_buf))
                       ? remain : sizeof(g_words_buf);

                memcpy(g_words_buf, pCur, length);
                pCur   += length;
                remain -= length;

                crc16_value = calc_crc16_ex((uint8_t*)&g_words_buf, length, crc16_value);
            }

            printf("3. crc16= 0x%04X\n", crc16_value);
        }
#endif
    } while(0);

    if( fin )       fclose(fin);
    if( pBuf )      free(pBuf);

    system("pause");
    return 0;
}
