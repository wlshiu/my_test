
#include <stdio.h>
#include <stdlib.h>

#include "fat_format.h"

static int
_diskio_read(uint32 sector, uint8 *buffer, uint32 sector_count)
{
    return 0;
}

static int
_diskio_write(uint32 sector, uint8 *buffer, uint32 sector_count)
{
    FILE    *fout = 0;
    char    *out_name[3] = { "sec_boot.bin", "sec_FSInfo.bin", "sec_fat_table.bin" };
    char    *pAct_out_name = 0;

    if( sector == 0 )
        pAct_out_name = out_name[0];
    else if( sector == 1 )
        pAct_out_name = out_name[1];
    else if( sector == 32 )
       pAct_out_name = out_name[2];

    printf("sector %d-th\n", sector);

    if( !pAct_out_name )
        return 1;

    if( !(fout = fopen(pAct_out_name, "wb")) )
    {
        return 0;
    }

    fwrite(buffer, 1, sector_count * FAT_SECTOR_SIZE, fout);
    fclose(fout);

    return 1;
}

int main()
{
    struct fatfs    fs;
    /**
     *  The minimum logical sector size for standard FAT32 volumes is 512 bytes.
     *      define at FAT_SECTOR_SIZE
     */
    fs.disk_io.read_media  = _diskio_read;
    fs.disk_io.write_media = _diskio_write;
    fatfs_format(&fs, 4194306, "123");

    printf("\ntype enter key\n");
    getchar();
    return 0;
}
