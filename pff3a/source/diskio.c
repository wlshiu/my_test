/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for Petit FatFs (C)ChaN, 2014      */
/*-----------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>

#include "diskio.h"
#include "fat.h"
#include "img_fat12.h"

#define CONFIG_1_SECTOR_SIZE        512

#define SET_1BYTES(_Addr_, _Val_)                            \
    *((uint8_t*)(_Addr_)) = ((_Val_) & 0xFF)

#define SET_2BYTES(_Addr_, _Val_)                            \
    do{ *((uint8_t*)(_Addr_)    ) = ((_Val_) & 0xFF);        \
        *((uint8_t*)(_Addr_) + 1) = (((_Val_) >> 8) & 0xFF); \
    } while(0)

#define SET_4BYTES(_Addr_, _Val_)                             \
    do{ *((uint8_t*)(_Addr_)    ) = ((_Val_) & 0xFF);         \
        *((uint8_t*)(_Addr_) + 1) = (((_Val_) >> 8) & 0xFF);  \
        *((uint8_t*)(_Addr_) + 2) = (((_Val_) >> 16) & 0xFF); \
        *((uint8_t*)(_Addr_) + 3) = (((_Val_) >> 24) & 0xFF); \
    } while(0)

static uint8_t  g_ramdisk[128 << 10] = {0};
static uint8_t  *g_pRamDisk = 0;
static int      g_disk_size = 0;
/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (void)
{
    DSTATUS     stat = 0;

#if 0
    memset(&g_ramdisk, 0xFF, sizeof(g_ramdisk));
    memcpy(&g_ramdisk, __fat12_img, sizeof(__fat12_img));
    g_pRamDisk = &g_ramdisk;
#else

//#define CONFIG_IMG_PATH     "ex_fat12.img"
#define CONFIG_IMG_PATH     "fat12_wl.img"
    FILE    *fin = 0;
    int     filesize = 0;
    if( !(fin = fopen(CONFIG_IMG_PATH, "rb")) )
    {
        printf("open %s fail !\n", CONFIG_IMG_PATH);
        while(1);
    }

    fseek(fin, 0, SEEK_END);
    filesize = ftell(fin);
    fseek(fin, 0, SEEK_SET);

    if( !(g_pRamDisk = malloc(filesize)) )
    {
        printf("malloc %d fail !\n", filesize);
        while(1);
    }

    fread(g_pRamDisk, 1, filesize, fin);
    fclose(fin);

    g_disk_size = filesize;

#endif

    dbr_t   *pDBR = (dbr_t*)&_dbr;

    pDBR = (dbr_t*)g_pRamDisk;

#if 0
    pDBR->fats          = 1;
    pDBR->fat_length    = 1;
    pDBR->secs_track    = 0;
    pDBR->heads         = 0;
    *(uint16_t*)&pDBR->dir_entries = 16;
    *(uint16_t*)&pDBR->sectors     = 14;
#endif // 0

    printf("\n=============== DBR (size= %d)==========\n", sizeof(dbr_t));
    printf("offset: 0 dbr->jmpBoot[0] = 0x%02X\n", pDBR->jmp_boot[0]);
    printf("offset: 1 dbr->jmpBoot[1] = 0x%02X\n", pDBR->jmp_boot[1]);
    printf("offset: 2 dbr->jmpBoot[2] = 0x%02X\n", pDBR->jmp_boot[2]);
    for(int k = 0; k < 8; k++)
        printf("offset:%d dbr->OEMName[%d] = %c\n", 3+k, k, pDBR->system_id[k]);

    printf("offset:11 dbr->BytsPerSec   = 0x%02X\n", *(uint16_t*)&pDBR->sector_size);
    printf("offset:13 dbr->SecPerClus   = 0x%02X\n", pDBR->cluster_size   );
    printf("offset:14 dbr->RsvdSecCnt   = 0x%02X\n", pDBR->reserved       );
    printf("offset:16 dbr->NumFATs      = 0x%02X\n", pDBR->fats           );
    printf("offset:17 dbr->RootEntCnt   = 0x%02X\n", *(uint16_t*)&pDBR->dir_entries);
    printf("offset:19 dbr->TotSec16     = 0x%02X\n", *(uint16_t*)&pDBR->sectors);
    printf("offset:21 dbr->Media        = 0x%02X\n", pDBR->media          );
    printf("offset:22 dbr->FATSz16      = 0x%02X\n", pDBR->fat_length     );
    printf("offset:24 dbr->SecPerTrk    = 0x%02X\n", pDBR->secs_track     );
    printf("offset:26 dbr->NumHeads     = 0x%02X\n", pDBR->heads          );
    printf("offset:28 dbr->HiddSec      = 0x%02X\n", pDBR->hidden         );
    printf("offset:32 dbr->TotSec32     = 0x%02X\n", pDBR->total_sect     );
    printf("\n\n");


#if 0
    FILE    *fout = 0;
    if( !(fout = fopen("fat12_wl.img", "wb")) )
    {
        printf("open file fail !\n");
    }

    fwrite(g_pRamDisk, 1, g_disk_size, fout);
    fclose(fout);


#endif // 1


    return stat;
}



/*-----------------------------------------------------------------------*/
/* Read Partial Sector                                                   */
/*-----------------------------------------------------------------------*/

DRESULT disk_readp (
    BYTE* buff,     /* Pointer to the destination object */
    DWORD sector,   /* Sector number (LBA) */
    UINT offset,    /* Offset in the sector */
    UINT count      /* Byte count (bit15:destination) */
)
{
    DRESULT     res = RES_OK;
    uint8_t     *pCur = g_pRamDisk;

    pCur = pCur + (sector * CONFIG_1_SECTOR_SIZE) + offset;

    memcpy(buff, pCur, count);

    return res;
}



/*-----------------------------------------------------------------------*/
/* Write Partial Sector                                                  */
/*-----------------------------------------------------------------------*/

DRESULT disk_writep (
    const BYTE* buff,   /* Pointer to the data to be written, NULL:Initiate/Finalize write operation */
    DWORD sc            /* Sector number (LBA) or Number of bytes to send */
)
{
    DRESULT             res = RES_OK;
    uint32_t            byte_cnt = 0;
    static uint32_t     target_addr = 0;
	static uint32_t     write_cnt = 0;

    if( buff )
    {   /* Send data bytes */
        uint8_t     *pCur = (uint8_t*)target_addr;

        byte_cnt = (uint32_t)sc;

		while( byte_cnt && write_cnt )
        {
            *pCur++ = *buff++;

			write_cnt--;
            byte_cnt--;
		}
        return res;
    }

    if( sc )
    {   /* Initiate sector write process */
        int     sector_idx = sc;

        // ToDo: erase sector sector_idx

        target_addr = (uint32_t)&g_pRamDisk[sector_idx * CONFIG_1_SECTOR_SIZE];
        write_cnt   = CONFIG_1_SECTOR_SIZE;
    }
    else
    {   /* Finalize sector write process */

    }

    return res;
}

