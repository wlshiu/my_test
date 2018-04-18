/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/
#include <stdint.h>
#include "ffconf.h"     /* FatFs configuration options */
#include "diskio.h"		/* FatFs lower layer API */

#ifdef _MSC_VER
#include "windows.h"
#endif


#define SECTOR_SIZE         512
#define DISK_SIZE           (128 * SECTOR_SIZE)

static uint8_t      disk_space[DISK_SIZE];
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
    DRESULT res;

    switch (pdrv)
    {
        case DEV_RAM :
            switch (cmd)
            {
                case GET_SECTOR_COUNT:
                    *(uint32_t *)buff = DISK_SIZE / SECTOR_SIZE;
                    return RES_OK;
                    break;
                case GET_SECTOR_SIZE:
                    *(uint32_t *)buff = SECTOR_SIZE;
                    return RES_OK;
                    break;
                case CTRL_SYNC:
                    return RES_OK;
                    break;
                default:
                    res = RES_PARERR;
                    break;
            }
            return res;

        default:
            return RES_PARERR;
    }

    return RES_PARERR;
}


DWORD get_fattime(void)
{
#ifdef _MSC_VER
    return GetTickCount();
#endif
    return 0;
}
