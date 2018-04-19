/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/
#include <stdint.h>
#include <stdio.h>
#include "ffconf.h"     /* FatFs configuration options */
#include "diskio.h"		/* FatFs lower layer API */

#ifdef _MSC_VER
#include "windows.h"
#endif


#define SECTOR_SIZE         512
#define DISK_SIZE           (1 << 20) //(128 * SECTOR_SIZE)

static uint8_t      disk_space[DISK_SIZE];

#define VIRTUAL_SD_IMG_PATH     "./tool/mksdcard/hello.img"
static FILE         *fSd_Img = 0;
/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
    BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
    switch (pdrv)
    {
        case DEV_RAM:
            return 0;

        case DEV_SD:
            if( fSd_Img )     return RES_OK;
            break;

        default:
            return STA_NOINIT;
    }
    return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
    BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
    switch (pdrv)
    {
        case DEV_RAM :
            return 0;

        case DEV_SD:
            if( !fSd_Img )
            {
                if( !(fSd_Img = fopen(VIRTUAL_SD_IMG_PATH, "rb+")) )
                    return STA_NOINIT;
            }
            return RES_OK;
            break;

        default:
            return STA_NOINIT;//RES_PARERR;
    }
    return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
    BYTE pdrv,		/* Physical drive nmuber to identify the drive */
    BYTE *buff,		/* Data buffer to store read data */
    DWORD sector,	/* Start sector in LBA */
    UINT count		/* Number of sectors to read */
)
{
    switch (pdrv)
    {
        case DEV_RAM :
            memcpy(buff, disk_space + sector * SECTOR_SIZE, SECTOR_SIZE * count);
            return RES_OK;

        case DEV_SD:
            {
                BYTE    *pOut_buf = buff;

                if( !fSd_Img )      return RES_ERROR;

                fseek(fSd_Img, SECTOR_SIZE * sector, SEEK_SET);

                for(int i = 0; i < count; i++)
                {
                    fread(pOut_buf, 1, SECTOR_SIZE, fSd_Img);

                    pOut_buf += SECTOR_SIZE;
                }
                return RES_OK;
            }
            break;

        default:
            return RES_PARERR;
    }

    return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

DRESULT disk_write (
    BYTE pdrv,			/* Physical drive nmuber to identify the drive */
    const BYTE *buff,	/* Data to be written */
    DWORD sector,		/* Start sector in LBA */
    UINT count			/* Number of sectors to write */
)
{
    switch (pdrv)
    {
        case DEV_RAM :
            memcpy(disk_space + sector * SECTOR_SIZE, buff, SECTOR_SIZE * count);
            return RES_OK;

        case DEV_SD:
            {
                BYTE *pOut_buf = (BYTE*)buff;
                if( !fSd_Img )   return RES_ERROR;

                fseek(fSd_Img, SECTOR_SIZE * sector, SEEK_SET);

                for(int i = 0; i < count; i++)
                {
                    //fread(pOut_buf, SECTOR_SIZE, 1, fSd_Img);

                    fwrite(pOut_buf, 1, SECTOR_SIZE, fSd_Img);

                    pOut_buf += SECTOR_SIZE;
                }
                return RES_OK;
            }
            break;

        default:
            return RES_PARERR;
    }

    return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
    BYTE pdrv,		/* Physical drive nmuber (0..) */
    BYTE cmd,		/* Control code */
    void *buff		/* Buffer to send/receive control data */
)
{
    DRESULT res = RES_OK;

    switch (pdrv)
    {
        case DEV_RAM :
            switch (cmd)
            {
                case GET_SECTOR_COUNT:
                    *(uint32_t *)buff = DISK_SIZE / SECTOR_SIZE;
                    break;
                case GET_SECTOR_SIZE:
                    *(uint32_t *)buff = SECTOR_SIZE;
                    break;
                case CTRL_SYNC:
                    break;
                default:
                    res = RES_PARERR;
                    break;
            }
            break;

        case DEV_SD:
            switch (cmd)
            {
                case GET_SECTOR_COUNT:
                    {
                        long    cur_pos = 0l;
                        long    filesize = 0l;
                        if( !fSd_Img )   return RES_ERROR;

                        cur_pos = ftell(fSd_Img);
                        fseek(fSd_Img, 0, SEEK_END);

                        filesize = ftell(fSd_Img);

                        fseek(fSd_Img, cur_pos, SEEK_SET);

                        *(long*)buff = (filesize / SECTOR_SIZE);
                    }
                    break;
                case GET_BLOCK_SIZE:
                case GET_SECTOR_SIZE:
                    *(uint32_t *)buff = SECTOR_SIZE;
                    break;
                case CTRL_SYNC:
                    break;
                default:
                    res = RES_PARERR;
                    break;
            }
            break;

        default:
            res = RES_PARERR;
            break;
    }

    return res;
}


DWORD get_fattime(void)
{
#ifdef _MSC_VER
    return GetTickCount();
#endif
    return 0;
}
