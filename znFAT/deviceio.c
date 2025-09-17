/**
 * Copyright (c) 2025 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file deviceio.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2025/09/17
 * @license
 * @description
 */


#include "config.h"
#include "deviceio.h"
//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================
struct znFAT_IO_Ctl     ioctl; //用于扇区读写的IO控制，尽量减少物理扇区操作，提高效率
extern uint8_t            Dev_No; //设备号
extern uint8_t            *znFAT_Buffer;
//=============================================================================
//                  Private Function Definition
//=============================================================================
static int _Dev_Init(void)
{
    return 0;
}

static int _Dev_Read_Sector(uint32_t addr, uint8_t *buffer)
{
    return 0;
}

static int _Dev_Write_Sector(uint32_t addr, uint8_t *buffer)
{
    return 0;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
uint8_t znFAT_Device_Init(void)
{
    uint8_t res = 0, err = 0;

    ioctl.just_dev = 0;
    ioctl.just_sec = 0;

    res = _Dev_Init();
    if( res ) err |= 0x01;

    return err; //返回错误码，如果某一设备初始化失败，则err相应位为1
}

uint8_t znFAT_Device_Read_Sector(uint32_t addr, uint8_t *buffer)
{
    if( buffer == znFAT_Buffer ) //如果是针对znFAT内部缓冲区的操作
    {
        /**
         *  如果现在要读取的扇区与内部缓冲所对应的扇区(即最近一次操作的扇区)是同一扇区
         *  则不再进行读取，直接返回
         */
        if( ioctl.just_dev == Dev_No &&
            (ioctl.just_sec == addr && 0 != ioctl.just_sec) )
        {
            return 0;
        }
        else //否则，就将最近一次操作的扇区标记为当前扇区
        {
            ioctl.just_dev = Dev_No;
            ioctl.just_sec = addr;
        }
    }

    switch( Dev_No ) //有多少个存储设备，就有多少个case分支
    {
        case 0:
            while( _Dev_Read_Sector(addr, buffer) );
            break;
    }

    return 0;
}

uint8_t znFAT_Device_Write_Sector(uint32_t addr, uint8_t *buffer)
{
    if( buffer == znFAT_Buffer ) //如果数据缓冲区是内部缓冲
    {
        ioctl.just_dev = Dev_No; //更新为当前设备号
        ioctl.just_sec = addr; //更新为当前操作的扇区地址
    }

    switch(Dev_No)
    {
        default:
        case 0:
            while( _Dev_Write_Sector(addr, buffer) );
            break;
    }

    return 0;
}

uint8_t znFAT_Device_Read_nSector(uint32_t nsec, uint32_t addr, uint8_t *buffer)
{
    if( 0 == nsec )
        return 0;


    switch( Dev_No )
    {
        default:
        case 0:
            for(int i = 0; i < nsec; i++) //如果不使用硬件级连续扇区读取，则使用单扇区读取+循环的方式
            {
                while( _Dev_Read_Sector(addr + i, buffer) );
                buffer += 512;
            }
            break;
    }

    return 0;
}

uint8_t znFAT_Device_Write_nSector(uint32_t nsec, uint32_t addr, uint8_t *buffer)
{
    if( 0 == nsec )
        return 0;


    switch( Dev_No )
    {
        default:
        case 0:
            for(int i = 0; i < nsec; i++)
            {
                while( _Dev_Write_Sector(addr + i, buffer) );
                buffer += 512;
            }
            break;
    }

    return 0;
}

uint8_t znFAT_Device_Clear_nSector(uint32_t nsec, uint32_t addr)
{
    for(int i = 0; i < 512; i++) //清空内部缓冲区，用于连续扇区清0
    {
        znFAT_Buffer[i] = 0;
    }

    switch(Dev_No)
    {
        default:
        case 0:
            for(int i = 0; i < nsec; i++)
            {
                while( _Dev_Write_Sector(addr + i, znFAT_Buffer) );
            }
            break;
    }


    ioctl.just_dev = Dev_No; //更新为当前设备号
    ioctl.just_sec = (addr + nsec - 1); //更新为当前操作的扇区地址

    return 0;
}



