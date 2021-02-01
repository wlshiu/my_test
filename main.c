#include <stdio.h>
#include <stdlib.h>
#include "tlsf.h"

#define POOL_SIZE 1024 * 1024


// Pool size is in bytes.
char pool[POOL_SIZE];

int main(void)
{
    tlsf_t  hTLSF = tlsf_create_with_pool(pool, POOL_SIZE);
    int     *ptr[100] = {0};

    printf("start = %08x\n", pool);
    for(int i = 0; i < 5; i++)
    {
        int     len = (i & 0x1) ? 40 : 32;
        if( !(ptr[i] = tlsf_malloc(hTLSF, len)) )
        {
            printf("Error %d: %d-th\n", __LINE__, i);
            while(1) {}
        }
        memset(ptr[i], 0xa0 | i, len);
        printf("allocate: %08x, len= %d\n", ptr[i], len);
    }

    for(int i = 0; i < 5; i++)
        tlsf_free(hTLSF, ptr[i]);

    printf("Test OK\n");
    system("pause");
    return 0;
}



