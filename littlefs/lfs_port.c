/**
 * Copyright (c) 2023 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file lfs_port.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2023/03/14
 * @license
 * @description
 */


#include "lfs_port.h"

#if defined(CONFING_ENABLE_SIM)
#include "extfc.h"
#else  /* CONFING_ENABLE_SIM */
#include "stm32f1xx_hal.h"
#endif /* CONFING_ENABLE_SIM */

//=============================================================================
//                  Constant Definition
//=============================================================================

#if defined(CONFING_ENABLE_SIM)

#else   /* CONFING_ENABLE_SIM */

#define EXT_FLASH_CS_PIN            GPIO_PIN_2
#define EXT_FLASH_CS_GPIO_PORT      GPIOA

#endif  /* CONFING_ENABLE_SIM */

#define EXT_FLASH_SECTOR_SZ         (4ul << 10)
#define EXT_FLASH_PAGE_SZ           256

#define CMD_PageProgram         0x02
#define CMD_ReadData            0x03

#define CMD_WriteEnable         0x06

#define CMD_ReadStatusReg       0x05
#define CMD_SectorErase         0x20
#define CMD_WriteStatusReg      0x01

// #define W25X_WriteDisable       0x04
// #define W25X_FastReadData       0x0B
// #define W25X_FastReadDual       0x3B
// #define W25X_BlockErase         0xD8
// #define W25X_ChipErase          0xC7
// #define W25X_PowerDown          0xB9
// #define W25X_ReleasePowerDown   0xAB
// #define W25X_DeviceID           0xAB
// #define W25X_ManufactDeviceID   0x90
// #define W25X_JedecDeviceID      0x9F
//=============================================================================
//                  Macro Definition
//=============================================================================
#if defined(CONFING_ENABLE_SIM)
#define SPI_FLASH_CS_L()
#define SPI_FLASH_CS_H()

#else   /* CONFING_ENABLE_SIM */

#define SPI_FLASH_CS_L()       HAL_GPIO_WritePin(EXT_FLASH_CS_GPIO_PORT, EXT_FLASH_CS_PIN, GPIO_PIN_RESET)
#define SPI_FLASH_CS_H()       HAL_GPIO_WritePin(EXT_FLASH_CS_GPIO_PORT, EXT_FLASH_CS_PIN, GPIO_PIN_SET)

#endif  /* CONFING_ENABLE_SIM */
//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================
#if defined(CONFING_ENABLE_SIM)

#else   /* CONFING_ENABLE_SIM */

SPI_HandleTypeDef       g_hSpi = {0};

#endif  /* CONFING_ENABLE_SIM */

static lfs_t                g_hLFS = {0};
static uint32_t             g_jedec_id = 0;

static uint32_t             g_read_buf[16 >> 2];
static uint32_t             g_prog_buf[16 >> 2];
static uint32_t             g_lookahead_buf[16 >> 2];
//=============================================================================
//                  Private Function Definition
//=============================================================================
#if defined(CONFING_ENABLE_SIM)
static void
_delay_us(uint32_t us)
{
    uint32_t delay = us * 10;
    while (delay--){}
    return;
}

static int lfs_deskio_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size)
{
    extfc_read((uint8_t *)buffer, c->block_size * block + off, size);
    return LFS_ERR_OK;
}


static int lfs_deskio_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size)
{
    uint8_t     *pBuffer = (uint8_t*)buffer;
    uint32_t    WriteAddr = c->block_size * block + off;
    uint16_t    NumByteToWrite = size;
    uint16_t    page_remain;

    page_remain = EXT_FLASH_PAGE_SZ - WriteAddr % EXT_FLASH_PAGE_SZ;
    if( NumByteToWrite <= page_remain )
        page_remain = NumByteToWrite;

    while (1)
    {
        extfc_program(pBuffer, WriteAddr, page_remain);

        if( NumByteToWrite == page_remain )
            break;

        pBuffer   += page_remain;
        WriteAddr += page_remain;

        NumByteToWrite -= page_remain;

        page_remain = (NumByteToWrite > EXT_FLASH_PAGE_SZ)
                    ? EXT_FLASH_PAGE_SZ : NumByteToWrite;
    }
    return LFS_ERR_OK;
}

static int lfs_deskio_erase(const struct lfs_config *c, lfs_block_t block)
{
    uint32_t    Dst_Addr = block * 4096;

    extfc_erase(EXTFC_ERASE_SECTOR, Dst_Addr, 1);
    return LFS_ERR_OK;
}
#else   /* CONFING_ENABLE_SIM */


void HAL_SPI_MspInit(SPI_HandleTypeDef *pHSPI)
{
    GPIO_InitTypeDef    GPIO_InitStruct = {0};
    if( pHSPI->Instance == SPI1 )
    {
        /* SPI1 clock enable */
        __HAL_RCC_SPI1_CLK_ENABLE();

        __HAL_RCC_GPIOA_CLK_ENABLE();
        /**
         *  SPI1 GPIO Configuration
         *  PA5     ------> SPI1_SCK
         *  PA6     ------> SPI1_MISO
         *  PA7     ------> SPI1_MOSI
         */
        GPIO_InitStruct.Pin   = GPIO_PIN_5 | GPIO_PIN_7;
        GPIO_InitStruct.Mode  = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        GPIO_InitStruct.Pin  = GPIO_PIN_6;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }
    return;
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef *pHSPI)
{
    if( pHSPI->Instance == SPI1 )
    {
        /* Peripheral clock disable */
        __HAL_RCC_SPI1_CLK_DISABLE();

        /**
         *  SPI1 GPIO Configuration
         *  PA5     ------> SPI1_SCK
         *  PA6     ------> SPI1_MISO
         *  PA7     ------> SPI1_MOSI
         */
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);
    }
    return;
}

static void
_delay_us(uint32_t us)
{
    uint32_t delay = (HAL_RCC_GetHCLKFreq() / 4000000 * us);
    while (delay--){}
    return;
}

static uint8_t
_SPI_ReadWriteByte(uint8_t TxData)
{
    uint8_t     RxData = 0X00;
    if( HAL_SPI_TransmitReceive(&g_hSpi, &TxData, &RxData, 1, 100) != HAL_OK )
    {
        RxData = 0XFF;
    }
    return RxData;
}

uint16_t _SPI_FLASH_ReadID(void)
{
    g_jedec_id = 0;

    SPI_FLASH_CS_L();

    _SPI_ReadWriteByte(0x90);
    _SPI_ReadWriteByte(0x00);
    _SPI_ReadWriteByte(0x00);
    _SPI_ReadWriteByte(0x00);

    g_jedec_id |= _SPI_ReadWriteByte(0xFF) << 8;
    g_jedec_id |= _SPI_ReadWriteByte(0xFF);

    SPI_FLASH_CS_H();

    return (uint16_t)(g_jedec_id & 0xFFFF);
}

uint32_t _SPI_FLASH_GetCapacity(void)
{
    if( g_jedec_id == 0 )
        return 0;

    return (0x1ul << (g_jedec_id & 0xFF));
}

static void _SPI_FLASH_Write_Enable(void)
{
    SPI_FLASH_CS_L();
    _SPI_ReadWriteByte(CMD_WriteEnable);
    SPI_FLASH_CS_H();
    return;
}

static uint8_t _SPI_FLASH_ReadSR(void)
{
    uint8_t     byte = 0;

    SPI_FLASH_CS_L();

    _SPI_ReadWriteByte(CMD_ReadStatusReg);
    byte = _SPI_ReadWriteByte(0xff);  // write dummy for receiving data

    SPI_FLASH_CS_H();

    return byte;
}

static void _SPI_FLASH_Wait_Busy(void)
{
    while( (_SPI_FLASH_ReadSR() & 0x01) == 0x01 ) {}

    return;
}

static int _SPI_Init(void)
{
    int     rval = 0;

    do {
        g_hSpi.Instance               = SPI1;
        g_hSpi.Init.Mode              = SPI_MODE_MASTER;
        g_hSpi.Init.Direction         = SPI_DIRECTION_2LINES;
        g_hSpi.Init.DataSize          = SPI_DATASIZE_8BIT;
        g_hSpi.Init.CLKPolarity       = SPI_POLARITY_LOW;
        g_hSpi.Init.CLKPhase          = SPI_PHASE_1EDGE;
        g_hSpi.Init.NSS               = SPI_NSS_SOFT;
        g_hSpi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
        g_hSpi.Init.FirstBit          = SPI_FIRSTBIT_MSB;
        g_hSpi.Init.TIMode            = SPI_TIMODE_DISABLE;
        g_hSpi.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
        g_hSpi.Init.CRCPolynomial     = 10;
        if( HAL_SPI_Init(&g_hSpi) != HAL_OK )
        {
            rval = -1;
            break;
        }
    } while(0);
    return rval;
}

static void _SPI_FLASH_Erase_Sector(uint32_t Dst_Addr)
{
    Dst_Addr *= 4096;

    _SPI_FLASH_Write_Enable();
    _SPI_FLASH_Wait_Busy();

    SPI_FLASH_CS_L();

    _SPI_ReadWriteByte(CMD_SectorErase);
    _SPI_ReadWriteByte((uint8_t)((Dst_Addr) >> 16));
    _SPI_ReadWriteByte((uint8_t)((Dst_Addr) >> 8));
    _SPI_ReadWriteByte((uint8_t)Dst_Addr);

    SPI_FLASH_CS_H();

    _SPI_FLASH_Wait_Busy();
    return;
}

static void _SPI_FLASH_Read(uint8_t *pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead)
{
    uint16_t i;

    SPI_FLASH_CS_L();

    _SPI_ReadWriteByte(CMD_ReadData);
    _SPI_ReadWriteByte((uint8_t)((ReadAddr) >> 16)); // 24-bits address
    _SPI_ReadWriteByte((uint8_t)((ReadAddr) >> 8));
    _SPI_ReadWriteByte((uint8_t)ReadAddr);

    for(int i = 0; i < NumByteToRead; i++)
    {
        pBuffer[i] = _SPI_ReadWriteByte(0XFF);
    }
    SPI_FLASH_CS_H();

    return;
}

static void _SPI_FLASH_Write_Page(uint8_t *pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
    _SPI_FLASH_Write_Enable();

    SPI_FLASH_CS_L();

    _SPI_ReadWriteByte(CMD_PageProgram);
    _SPI_ReadWriteByte((uint8_t)((WriteAddr) >> 16));
    _SPI_ReadWriteByte((uint8_t)((WriteAddr) >> 8));
    _SPI_ReadWriteByte((uint8_t)WriteAddr);

    for(int i = 0; i < NumByteToWrite; i++)
        _SPI_ReadWriteByte(pBuffer[i]);

    SPI_FLASH_CS_H();

    _SPI_FLASH_Wait_Busy();

    return;
}

static void _SPI_FLASH_Write_NoCheck(uint8_t *pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
    uint16_t    page_remain;

    page_remain = EXT_FLASH_PAGE_SZ - WriteAddr % EXT_FLASH_PAGE_SZ;
    if( NumByteToWrite <= page_remain )
        page_remain = NumByteToWrite;

    while (1)
    {
        _SPI_FLASH_Write_Page(pBuffer, WriteAddr, page_remain);
        if( NumByteToWrite == page_remain )
            break;

        pBuffer   += page_remain;
        WriteAddr += page_remain;

        NumByteToWrite -= page_remain;

        page_remain = (NumByteToWrite > EXT_FLASH_PAGE_SZ)
                    ? EXT_FLASH_PAGE_SZ : NumByteToWrite;
    }
    return;
}

static int lfs_deskio_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size)
{
    _SPI_FLASH_Read((uint8_t *)buffer, c->block_size * block + off, size);
    return LFS_ERR_OK;
}


static int lfs_deskio_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size)
{
    _SPI_FLASH_Write_NoCheck((uint8_t *)buffer, c->block_size * block + off, size);
    return LFS_ERR_OK;
}

static int lfs_deskio_erase(const struct lfs_config *c, lfs_block_t block)
{
    _SPI_FLASH_Erase_Sector(block);
    return LFS_ERR_OK;
}
#endif  /* CONFING_ENABLE_SIM */

static int lfs_deskio_sync(const struct lfs_config *c)
{
    return LFS_ERR_OK;
}


static const struct lfs_config  g_lfs_cfg =
{
    // block device operations
    .read  = lfs_deskio_read,
    .prog  = lfs_deskio_prog,
    .erase = lfs_deskio_erase,
    .sync  = lfs_deskio_sync,

    // block device configuration
    .read_size      = 16,
    .prog_size      = 16,
    .block_size     = 4096,
    .block_count    = 128,
    .cache_size     = 16,
    .lookahead_size = 16,
    .block_cycles   = 500,


    /**
     *  static buffer for cache
     */
    .read_buffer      = (void*)g_read_buf,
    .prog_buffer      = (void*)g_prog_buf,
    .lookahead_buffer = (void*)g_lookahead_buf,
};
//=============================================================================
//                  Public Function Definition
//=============================================================================
int lfs_fs_init(lfs_t *phLFS, lfs_config_t *pCfg)
{
    int     rval = 0;

    do {
    #if defined(CONFING_ENABLE_SIM)

    #else   /* CONFING_ENABLE_SIM */
        rval = _SPI_Init();
    #endif  /* CONFING_ENABLE_SIM */

        // mount the file system
        int err = lfs_mount(phLFS, pCfg);

        // reformat if we can't mount the file system
        // this should only happen on the first boot
        if( err )
        {
            lfs_format(phLFS, pCfg);
            lfs_mount(phLFS, pCfg);
        }

    } while(0);

    return rval;
}

int lfs_fs_get_cfg(lfs_config_t *pCfg)
{
    int     rval = 0;
    if( pCfg )  *pCfg = g_lfs_cfg;
    return rval;
}

void lfs_err_string(lfs_error_t err_code)
{
    switch( err_code )
    {
        case LFS_ERR_OK         :    printf("No error                      \n");    break;
        case LFS_ERR_IO         :    printf("Error during device operation \n");    break;
        case LFS_ERR_CORRUPT    :    printf("Corrupted                     \n");    break;
        case LFS_ERR_NOENT      :    printf("No directory entry            \n");    break;
        case LFS_ERR_EXIST      :    printf("Entry already exists          \n");    break;
        case LFS_ERR_NOTDIR     :    printf("Entry is not a dir            \n");    break;
        case LFS_ERR_ISDIR      :    printf("Entry is a dir                \n");    break;
        case LFS_ERR_NOTEMPTY   :    printf("Dir is not empty              \n");    break;
        case LFS_ERR_BADF       :    printf("Bad file number               \n");    break;
        case LFS_ERR_FBIG       :    printf("File too large                \n");    break;
        case LFS_ERR_INVAL      :    printf("Invalid parameter             \n");    break;
        case LFS_ERR_NOSPC      :    printf("No space left on device       \n");    break;
        case LFS_ERR_NOMEM      :    printf("No more memory available      \n");    break;
        case LFS_ERR_NOATTR     :    printf("No data/attr available        \n");    break;
        case LFS_ERR_NAMETOOLONG:    printf("File name too long            \n");    break;
        default:                     printf("Unknown\n");                           break;
    }
    return;
}
