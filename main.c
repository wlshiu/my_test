
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "elf.h"

int main(int argc, char **argv)
{
    FILE        *fin = 0;
    uint8_t     *pbuf;

    do {
        Fhdr        fhdr;
        uint64_t    len;

        printf("%s\n", argv[1]);

        if( !(fin = fopen(argv[1], "rb")) )
        {
            printf("open %s fail\n", argv[1]);
            break;
        }

        pbuf = readelfsection(fin, ".text", &len, &fhdr);

        freeelf(&fhdr);

    } while(0);

    if( pbuf )  free(pbuf);
    if( fin )   fclose(fin);

    system("pause");
    return 0;
}
