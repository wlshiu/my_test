
#include <stdio.h>
#include <stdlib.h>

#include "fat_format.h"


int main()
{
    struct fatfs    fs;
    fatfs_format(&fs, 32, "123");

    printf("\ntype enter key\n");
    getchar();
    return 0;
}
