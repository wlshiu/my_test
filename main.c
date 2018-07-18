#include <stdio.h>
#include <stdlib.h>
#include "log2mem.h"
#include <windows.h>

int main()
{
    int         i;

    log2mem_init();

    for(i = 0; i < 12; i++)
    {
        Sleep(10);
        log2mem_push(i, i, i);
    }


    log2mem_show();
    return 0;
}
