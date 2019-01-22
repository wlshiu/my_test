
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "bslot.h"

#define msg(str, argv...)       do{ printf("%s[%u] " str, __func__, __LINE__, ##argv); }while(0)


// flag of buffer
static unsigned long           g_bitmap[BUF_BMP_SIZE] =
{
    0x7,
};

int main()
{
    int     buf_idx = 0;
    FILE    *fout = 0;
    FILE    *fout1 = 0;

    srand(time(0));

    for(int i = 0; i < 10; ++i)
        (void)rand();

    fout  = fopen("z_set.txt", "wb");
//    fout1 = fopen("z_check.txt", "wb");

    for(int i = 0; i < BUF_BMP_SIZE * 32; i++)
    {
        buf_idx = bslot_get_buffer(g_bitmap, BUF_BMP_SIZE);
        if( buf_idx < 0 )
        {
            msg("%s\n", "buffer full");
            continue;
        }

        fprintf(fout, "%u\n", buf_idx);

        bslot_set_buffer(g_bitmap, BUF_BMP_SIZE, buf_idx);
        if( (rand() >> 5) & 0x1 )
        {
            bslot_release_buffer(g_bitmap, BUF_BMP_SIZE, buf_idx);
            fprintf(fout, "--%u\n", buf_idx);
            continue;
        }
    }

    for(int i = 0; i < BUF_BMP_SIZE * 32; i++)
    {
        if( bslot_is_buffer_set(g_bitmap, BUF_BMP_SIZE, i) )
        {
//            fprintf(fout1, "%u\n", i);
        }
    }

    fclose(fout);
//    fclose(fout1);


    printf("----------------------- end \n");
    while(1);
    return 0;
}
