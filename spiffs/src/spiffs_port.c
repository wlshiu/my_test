/**
 * Copyright (c) 2023 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file spiffs_port.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2023/03/13
 * @license
 * @description
 */


#include "spiffs_port.h"

#if defined(CONFING_ENABLE_SIM)
#include <stdint.h>
#include <stdbool.h>
#include "extfc.h"

#else   /* CONFING_ENABLE_SIM */

#include "stm32f1xx_hal.h"

#endif  /* CONFING_ENABLE_SIM */

//=============================================================================
//                  Constant Definition
//=============================================================================
#define CONFIG_EXT_FLASH_PHYS_SZ                    (512ul << 10)
#define CONFIG_EXT_FLASH_PHYS_ERASE_SZ              (32ul << 10)
#define CONFIG_EXT_FLASH_PHYS_ADDR                  (0)
#define CONFIG_EXT_FLASH_LOG_PAGE_SZ                (256)
#define CONFIG_EXT_FLASH_LOG_BLOCK_SZ               (32ul << 10)

#define EXT_FLASH_SECTOR_SZ                         (4ul << 10)
#define EXT_FLASH_PAGE_SZ                           256

#define CONFIG_DUMMY_BYTE                           0xFF

#define SPIT_FLAG_TIMEOUT           ((uint32_t)0x1000)

#define CMD_PageProgram		        0x02
#define CMD_ReadData                0x03
#define CMD_SectorErase		        0x20

#define CMD_READ_STATU_REG_1        0x05
#define CMD_READ_STATU_REG_2        0x35
#define CMD_WriteEnable		        0x06


#define EXT_FLASH_WIP_FLAG          0x01  /* Write In Progress (WIP) flag */


#if defined(CONFING_ENABLE_SIM)


#else   /* CONFING_ENABLE_SIM */

/**
 *  SPI Pins configuration
 */
#define SPIx                             SPI1
#define SPIx_CLK_ENABLE()                __HAL_RCC_SPI1_CLK_ENABLE()
#define SPIx_SCK_GPIO_CLK_ENABLE()       __HAL_RCC_GPIOA_CLK_ENABLE()
#define SPIx_MISO_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()
#define SPIx_MOSI_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()
#define SPIx_CS_GPIO_CLK_ENABLE()        __HAL_RCC_GPIOC_CLK_ENABLE()

#define SPIx_FORCE_RESET()               __HAL_RCC_SPI1_FORCE_RESET()
#define SPIx_RELEASE_RESET()             __HAL_RCC_SPI1_RELEASE_RESET()

/* Definition for SPIx Pins */
#define SPIx_SCK_PIN                     GPIO_PIN_5
#define SPIx_SCK_GPIO_PORT               GPIOA

#define SPIx_MISO_PIN                    GPIO_PIN_6
#define SPIx_MISO_GPIO_PORT              GPIOA

#define SPIx_MOSI_PIN                    GPIO_PIN_7
#define SPIx_MOSI_GPIO_PORT              GPIOA

#define SPIx_CS_PIN                      GPIO_PIN_0
#define SPIx_CS_GPIO_PORT                GPIOC

#endif  /* CONFING_ENABLE_SIM */
//=============================================================================
//                  Macro Definition
//=============================================================================
#if defined(CONFING_ENABLE_SIM)
#define __SPI_FLASH_CS_LOW()
#define __SPI_FLASH_CS_HIGH()
#else   /* CONFING_ENABLE_SIM */

#define __SPI_FLASH_CS_LOW()          (SPIx_CS_GPIO_PORT)->BSRR = (uint32_t)(SPIx_CS_PIN) << 16
#define __SPI_FLASH_CS_HIGH()         (SPIx_CS_GPIO_PORT)->BSRR = (SPIx_CS_PIN)

#endif  /* CONFING_ENABLE_SIM */
//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================
static u8_t     g_spiffs_cache_buf[32 + (32 + CONFIG_EXT_FLASH_LOG_PAGE_SZ * 8)];
static u8_t     g_spiffs_fd_buf[32 * 8];
static u8_t     g_spiffs_work[CONFIG_EXT_FLASH_LOG_PAGE_SZ * 2];

#if defined(CONFING_ENABLE_SIM)

#else   /* CONFING_ENABLE_SIM */

SPI_HandleTypeDef       g_HSpi = {0};

#endif  /* CONFING_ENABLE_SIM */

static spiffs           g_hFS = {0};
//=============================================================================
//                  Private Function Definition
//=============================================================================

#if defined(CONFING_ENABLE_SIM)

static s32_t _spiffs_erase(uint32_t addr, uint32_t len)
{
    int     rval = 0;
    int     count = (len + EXT_FLASH_SECTOR_SZ - 1) >> 12;

    for(int i = 0; i < count; i++)
    {
        extfc_erase(EXTFC_ERASE_SECTOR, addr + i * EXT_FLASH_SECTOR_SZ, 1);
    }

    return rval;
}

static s32_t _spiffs_read(uint32_t addr, uint32_t size, uint8_t *dst)
{
    int     rval = 0;

    extfc_read(dst, addr, size);
    return rval;
}

static s32_t _spiffs_write(uint32_t addr, uint32_t size, uint8_t *src)
{
    int         rval = 0;

    uint8_t *pBuffer = src;
    uint32_t WriteAddr = addr;
    uint16_t NBytes = size;

    uint8_t NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0;

    Addr = WriteAddr & (EXT_FLASH_PAGE_SZ - 1);

    count = EXT_FLASH_PAGE_SZ - Addr;

    NumOfPage =  NBytes / EXT_FLASH_PAGE_SZ;

    NumOfSingle = NBytes % EXT_FLASH_PAGE_SZ;

    if( Addr == 0 )
    {
        /* NBytes < EXT_FLASH_PAGE_SZ */
        if (NumOfPage == 0)
        {
            extfc_program(pBuffer, WriteAddr, NBytes);
        }
        else /* NBytes > EXT_FLASH_PAGE_SZ */
        {
            while (NumOfPage--)
            {
                extfc_program(pBuffer, WriteAddr, EXT_FLASH_PAGE_SZ);

                WriteAddr +=  EXT_FLASH_PAGE_SZ;
                pBuffer   += EXT_FLASH_PAGE_SZ;
            }

            extfc_program(pBuffer, WriteAddr, NumOfSingle);
        }
    }
    else
    {
        /* NBytes < EXT_FLASH_PAGE_SZ */
        if( NumOfPage == 0 )
        {
            if( NumOfSingle > count )
            {
                uint8_t     temp = NumOfSingle - count;

                extfc_program(pBuffer, WriteAddr, count);

                WriteAddr +=  count;
                pBuffer   += count;

                extfc_program(pBuffer, WriteAddr, temp);
            }
            else
            {
                extfc_program(pBuffer, WriteAddr, NBytes);
            }
        }
        else /* NBytes > EXT_FLASH_PAGE_SZ */
        {
            NBytes -= count;
            NumOfPage   = NBytes / EXT_FLASH_PAGE_SZ;
            NumOfSingle = NBytes % EXT_FLASH_PAGE_SZ;

            extfc_program(pBuffer, WriteAddr, count);

            WriteAddr += count;
            pBuffer   += count;

            while( NumOfPage-- )
            {
                extfc_program(pBuffer, WriteAddr, EXT_FLASH_PAGE_SZ);

                WriteAddr += EXT_FLASH_PAGE_SZ;
                pBuffer   += EXT_FLASH_PAGE_SZ;
            }

            if( NumOfSingle )
            {
                extfc_program(pBuffer, WriteAddr, NumOfSingle);
            }
        }
    }

    return rval;
}

#else   /* CONFING_ENABLE_SIM */

#if 0
void HAL_SPI_MspInit(SPI_HandleTypeDef *hSpi)
{
    GPIO_InitTypeDef    GPIO_InitStruct = {0};

    /* Enable GPIO TX/RX clock */
    SPIx_SCK_GPIO_CLK_ENABLE();
    SPIx_MISO_GPIO_CLK_ENABLE();
    SPIx_MOSI_GPIO_CLK_ENABLE();
    SPIx_CS_GPIO_CLK_ENABLE();

    /* Enable SPI clock */
    SPIx_CLK_ENABLE();

    /* SPI SCK GPIO pin configuration  */
    GPIO_InitStruct.Pin       = SPIx_SCK_PIN;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_PULLUP;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(SPIx_SCK_GPIO_PORT, &GPIO_InitStruct);

    /* SPI MISO GPIO pin configuration  */
    GPIO_InitStruct.Pin = SPIx_MISO_PIN;
    HAL_GPIO_Init(SPIx_MISO_GPIO_PORT, &GPIO_InitStruct);

    /* SPI MOSI GPIO pin configuration  */
    GPIO_InitStruct.Pin = SPIx_MOSI_PIN;
    HAL_GPIO_Init(SPIx_MOSI_GPIO_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin  = SPIx_CS_PIN ;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(SPIx_CS_GPIO_PORT, &GPIO_InitStruct);
    return;
}
#endif

static int
_SPI_Init(void)
{
    int     rval = 0;

    do {
        g_HSpi.Instance               = SPIx;
        g_HSpi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
        g_HSpi.Init.Direction         = SPI_DIRECTION_2LINES;
        g_HSpi.Init.CLKPhase          = SPI_PHASE_2EDGE;
        g_HSpi.Init.CLKPolarity       = SPI_POLARITY_HIGH;
        g_HSpi.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
        g_HSpi.Init.CRCPolynomial     = 7;
        g_HSpi.Init.DataSize          = SPI_DATASIZE_8BIT;
        g_HSpi.Init.FirstBit          = SPI_FIRSTBIT_MSB;
        g_HSpi.Init.NSS               = SPI_NSS_SOFT;
        g_HSpi.Init.TIMode            = SPI_TIMODE_DISABLE;

        g_HSpi.Init.Mode              = SPI_MODE_MASTER;

        rval = HAL_SPI_Init(&g_HSpi);
        if( rval )  break;

        __HAL_SPI_ENABLE(&g_HSpi);
    } while(0);

    return rval;
}

static int
_SPI_FLASH_SendByte(uint8_t byte, uint8_t *pValue)
{
    uint32_t  SPITimeout = SPIT_FLAG_TIMEOUT;

    while( __HAL_SPI_GET_FLAG(&g_HSpi, SPI_FLAG_TXE) == RESET )
    {
        if( SPITimeout-- == 0 )
            return -1;
    }

    WRITE_REG(g_HSpi.Instance->DR, byte);

    SPITimeout = SPIT_FLAG_TIMEOUT;

    while( __HAL_SPI_GET_FLAG(&g_HSpi, SPI_FLAG_RXNE) == RESET )
    {
        if( SPITimeout-- == 0 )
            return -2;
    }

    if( pValue )    *pValue = READ_REG(g_HSpi.Instance->DR) & 0xFF;

    return 0;
}

static void
_SPI_FLASH_WriteEnable(void)
{
    __SPI_FLASH_CS_LOW();

    _SPI_FLASH_SendByte(CMD_WriteEnable, 0);

    __SPI_FLASH_CS_HIGH();
    return;
}

static int
_SPI_FLASH_WaitForWriteEnd(void)
{
    int         rval = 0;
    uint32_t    SPITimeout = SPIT_FLAG_TIMEOUT;
    uint8_t     Status = 0;

    __SPI_FLASH_CS_LOW();

    _SPI_FLASH_SendByte(CMD_READ_STATU_REG_1, 0);

    do {
        rval = _SPI_FLASH_SendByte(CONFIG_DUMMY_BYTE, &Status);
        if( rval )  break;

        if( SPITimeout-- == 0 )
        {
            rval = -1;
            break;
        }
    } while( (Status & EXT_FLASH_WIP_FLAG) == SET );

    __SPI_FLASH_CS_HIGH();

    return rval;
}

static void
_SPI_FLASH_PageWrite(uint8_t *pBuffer, uint32_t WriteAddr, uint16_t NBytes)
{
    _SPI_FLASH_WriteEnable();

    __SPI_FLASH_CS_LOW();

    _SPI_FLASH_SendByte(CMD_PageProgram, 0);

    _SPI_FLASH_SendByte((WriteAddr & 0xFF0000) >> 16, 0);
    _SPI_FLASH_SendByte((WriteAddr & 0xFF00) >> 8, 0);
    _SPI_FLASH_SendByte(WriteAddr & 0xFF, 0);

    NBytes = (NBytes > EXT_FLASH_PAGE_SZ) ? EXT_FLASH_PAGE_SZ : NBytes;

    while( NBytes-- )
    {
        _SPI_FLASH_SendByte(*pBuffer++, 0);
    }

    __SPI_FLASH_CS_HIGH();

    _SPI_FLASH_WaitForWriteEnd();
    return;
}



static void
_SPI_FLASH_SectorErase(uint32_t SectorAddr)
{
    _SPI_FLASH_WriteEnable();
    _SPI_FLASH_WaitForWriteEnd();

    __SPI_FLASH_CS_LOW();

    _SPI_FLASH_SendByte(CMD_SectorErase, 0);

    _SPI_FLASH_SendByte((SectorAddr & 0xFF0000) >> 16, 0);
    _SPI_FLASH_SendByte((SectorAddr & 0xFF00) >> 8, 0);
    _SPI_FLASH_SendByte(SectorAddr & 0xFF, 0);

    __SPI_FLASH_CS_HIGH();

    _SPI_FLASH_WaitForWriteEnd();
    return;
}

static void
_SPI_FLASH_BufferRead(uint8_t *pBuffer, uint32_t ReadAddr, uint16_t NBytes)
{
    __SPI_FLASH_CS_LOW();

    _SPI_FLASH_SendByte(CMD_ReadData, 0);

    _SPI_FLASH_SendByte((ReadAddr & 0xFF0000) >> 16, 0);
    _SPI_FLASH_SendByte((ReadAddr & 0xFF00) >> 8, 0);
    _SPI_FLASH_SendByte(ReadAddr & 0xFF, 0);

    while( NBytes-- )
    {
        *pBuffer++ = _SPI_FLASH_SendByte(CONFIG_DUMMY_BYTE, 0);
    }

    __SPI_FLASH_CS_HIGH();
    return;
}


static void
_SPI_FLASH_BufferWrite(uint8_t *pBuffer, uint32_t WriteAddr, uint16_t NBytes)
{
    uint8_t NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0;

    Addr = WriteAddr & (EXT_FLASH_PAGE_SZ - 1);

    count = EXT_FLASH_PAGE_SZ - Addr;

    NumOfPage =  NBytes / EXT_FLASH_PAGE_SZ;

    NumOfSingle = NBytes % EXT_FLASH_PAGE_SZ;

    if( Addr == 0 )
    {
        /* NBytes < EXT_FLASH_PAGE_SZ */
        if (NumOfPage == 0)
        {
            _SPI_FLASH_PageWrite(pBuffer, WriteAddr, NBytes);
        }
        else /* NBytes > EXT_FLASH_PAGE_SZ */
        {
            while (NumOfPage--)
            {
                _SPI_FLASH_PageWrite(pBuffer, WriteAddr, EXT_FLASH_PAGE_SZ);

                WriteAddr +=  EXT_FLASH_PAGE_SZ;
                pBuffer   += EXT_FLASH_PAGE_SZ;
            }

            _SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumOfSingle);
        }
    }
    else
    {
        /* NBytes < EXT_FLASH_PAGE_SZ */
        if( NumOfPage == 0 )
        {
            if( NumOfSingle > count )
            {
                uint8_t     temp = NumOfSingle - count;

                _SPI_FLASH_PageWrite(pBuffer, WriteAddr, count);

                WriteAddr +=  count;
                pBuffer   += count;

                _SPI_FLASH_PageWrite(pBuffer, WriteAddr, temp);
            }
            else
            {
                _SPI_FLASH_PageWrite(pBuffer, WriteAddr, NBytes);
            }
        }
        else /* NBytes > EXT_FLASH_PAGE_SZ */
        {
            NBytes -= count;
            NumOfPage   = NBytes / EXT_FLASH_PAGE_SZ;
            NumOfSingle = NBytes % EXT_FLASH_PAGE_SZ;

            _SPI_FLASH_PageWrite(pBuffer, WriteAddr, count);

            WriteAddr += count;
            pBuffer   += count;

            while( NumOfPage-- )
            {
                _SPI_FLASH_PageWrite(pBuffer, WriteAddr, EXT_FLASH_PAGE_SZ);

                WriteAddr += EXT_FLASH_PAGE_SZ;
                pBuffer   += EXT_FLASH_PAGE_SZ;
            }

            if( NumOfSingle )
            {
                _SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumOfSingle);
            }
        }
    }

    return;
}

static s32_t _spiffs_erase(uint32_t addr, uint32_t len)
{
    int     rval = 0;
    int     count = (len + EXT_FLASH_SECTOR_SZ - 1) >> 12;

    for(int i = 0; i < count; i++)
    {
        _SPI_FLASH_SectorErase(addr + i * EXT_FLASH_SECTOR_SZ);
    }

    return rval;
}

static s32_t _spiffs_read(uint32_t addr, uint32_t size, uint8_t *dst)
{
    int     rval = 0;

    _SPI_FLASH_BufferRead(dst, addr, size);
    return rval;
}

static s32_t _spiffs_write(uint32_t addr, uint32_t size, uint8_t *src)
{
    int     rval = 0;

    _SPI_FLASH_BufferWrite(src, addr, size);
    return rval;
}
#endif  /* CONFING_ENABLE_SIM */

static const spiffs_config        g_spiffs_def_cfg =
{
    .hal_erase_f        = _spiffs_erase,
    .hal_read_f         = _spiffs_read,
    .hal_write_f        = _spiffs_write,

#if (SPIFFS_SINGLETON == 0)
    .phys_size          = CONFIG_EXT_FLASH_PHYS_SZ,
    .phys_addr          = CONFIG_EXT_FLASH_PHYS_ADDR,
    .phys_erase_block   = CONFIG_EXT_FLASH_PHYS_ERASE_SZ,
    .log_block_size     = CONFIG_EXT_FLASH_LOG_BLOCK_SZ,
    .log_page_size      = CONFIG_EXT_FLASH_LOG_PAGE_SZ,

#if SPIFFS_FILEHDL_OFFSET
    .fh_ix_offset = 1000,
#endif

#endif
};

static const char *spiffs_errstr(s32_t err)
{
    if( err > 0 )   return "";

    switch (err)
    {
        case SPIFFS_OK                        :     return " OK";
        case SPIFFS_ERR_NOT_MOUNTED           :     return " not mounted";
        case SPIFFS_ERR_FULL                  :     return " full";
        case SPIFFS_ERR_NOT_FOUND             :     return " not found";
        case SPIFFS_ERR_END_OF_OBJECT         :     return " end of object";
        case SPIFFS_ERR_DELETED               :     return " deleted";
        case SPIFFS_ERR_NOT_FINALIZED         :     return " not finalized";
        case SPIFFS_ERR_NOT_INDEX             :     return " not index";
        case SPIFFS_ERR_OUT_OF_FILE_DESCS     :     return " out of filedescs";
        case SPIFFS_ERR_FILE_CLOSED           :     return " file closed";
        case SPIFFS_ERR_FILE_DELETED          :     return " file deleted";
        case SPIFFS_ERR_BAD_DESCRIPTOR        :     return " bad descriptor";
        case SPIFFS_ERR_IS_INDEX              :     return " is index";
        case SPIFFS_ERR_IS_FREE               :     return " is free";
        case SPIFFS_ERR_INDEX_SPAN_MISMATCH   :     return " index span mismatch";
        case SPIFFS_ERR_DATA_SPAN_MISMATCH    :     return " data span mismatch";
        case SPIFFS_ERR_INDEX_REF_FREE        :     return " index ref free";
        case SPIFFS_ERR_INDEX_REF_LU          :     return " index ref lu";
        case SPIFFS_ERR_INDEX_REF_INVALID     :     return " index ref invalid";
        case SPIFFS_ERR_INDEX_FREE            :     return " index free";
        case SPIFFS_ERR_INDEX_LU              :     return " index lu";
        case SPIFFS_ERR_INDEX_INVALID         :     return " index invalid";
        case SPIFFS_ERR_NOT_WRITABLE          :     return " not writable";
        case SPIFFS_ERR_NOT_READABLE          :     return " not readable";
        case SPIFFS_ERR_CONFLICTING_NAME      :     return " conflicting name";
        case SPIFFS_ERR_NOT_CONFIGURED        :     return " not configured";

        case SPIFFS_ERR_NOT_A_FS              :     return " not a fs";
        case SPIFFS_ERR_MOUNTED               :     return " mounted";
        case SPIFFS_ERR_ERASE_FAIL            :     return " erase fail";
        case SPIFFS_ERR_MAGIC_NOT_POSSIBLE    :     return " magic not possible";

        case SPIFFS_ERR_NO_DELETED_BLOCKS     :     return " no deleted blocks";

        case SPIFFS_ERR_FILE_EXISTS           :     return " file exists";

        case SPIFFS_ERR_NOT_A_FILE            :     return " not a file";
        case SPIFFS_ERR_RO_NOT_IMPL           :     return " ro not impl";
        case SPIFFS_ERR_RO_ABORTED_OPERATION  :     return " ro aborted operation";
        case SPIFFS_ERR_PROBE_TOO_FEW_BLOCKS  :     return " probe too few blocks";
        case SPIFFS_ERR_PROBE_NOT_A_FS        :     return " probe not a fs";
        case SPIFFS_ERR_NAME_TOO_LONG         :     return " name too long";

        case SPIFFS_ERR_IX_MAP_UNMAPPED       :     return " ix map unmapped";
        case SPIFFS_ERR_IX_MAP_MAPPED         :     return " ix map mapped";
        case SPIFFS_ERR_IX_MAP_BAD_RANGE      :     return " ix map bad range";

        default                               :     return " <unknown>";
    }
}


static void
_spiffs_check_cb_f(
    spiffs_check_type   type,
    spiffs_check_report report,
    u32_t               arg1,
    u32_t               arg2)
{
    static u32_t    old_perc = 999;

    u32_t   perc = arg1 * 100 / 256;
    if( report == SPIFFS_CHECK_PROGRESS && old_perc != perc )
    {
        old_perc = perc;
        printf("CHECK REPORT: ");
        switch(type)
        {
            case SPIFFS_CHECK_LOOKUP:   printf("LU ");    break;
            case SPIFFS_CHECK_INDEX:    printf("IX ");    break;
            case SPIFFS_CHECK_PAGE:     printf("PA ");    break;
        }
        printf("%i%%\n", perc);
    }

    if( report != SPIFFS_CHECK_PROGRESS )
    {
        printf("   check: ");
        switch (type)
        {
            case SPIFFS_CHECK_INDEX:    printf("INDEX  "); break;
            case SPIFFS_CHECK_LOOKUP:   printf("LOOKUP "); break;
            case SPIFFS_CHECK_PAGE:     printf("PAGE   "); break;
            default:
                printf("????   ");
                break;
        }

        switch( report )
        {
            case SPIFFS_CHECK_ERROR:                    printf("ERROR %i", arg1);                   break;
            case SPIFFS_CHECK_DELETE_BAD_FILE:          printf("DELETE BAD FILE %04x", arg1);       break;
            case SPIFFS_CHECK_DELETE_ORPHANED_INDEX:    printf("DELETE ORPHANED INDEX %04x", arg1); break;
            case SPIFFS_CHECK_DELETE_PAGE:              printf("DELETE PAGE %04x", arg1);           break;
            case SPIFFS_CHECK_FIX_INDEX:                printf("FIX INDEX %04x:%04x", arg1, arg2);  break;
            case SPIFFS_CHECK_FIX_LOOKUP:               printf("FIX INDEX %04x:%04x", arg1, arg2);  break;
            default:                                    printf("??");                               break;
        }
        printf("\n");
    }
    return;
}

//=============================================================================
//                  Public Function Definition
//=============================================================================
int spiffs_init(spiffs_config *pCfg_user)
{
    int             rval = 0;
    spiffs_config   *pCfg = (spiffs_config*)&g_spiffs_def_cfg;

    pCfg = (pCfg_user) ? pCfg_user : pCfg;

    do {
    #if defined(CONFING_ENABLE_SIM)

    #else   /* CONFING_ENABLE_SIM */
        rval = _SPI_Init();
        if( rval )  break;
    #endif  /* CONFING_ENABLE_SIM */

        rval = SPIFFS_mount(&g_hFS,
                            pCfg,
                            g_spiffs_work,
                            g_spiffs_fd_buf, sizeof(g_spiffs_fd_buf),
                            g_spiffs_cache_buf, sizeof(g_spiffs_cache_buf),
                            _spiffs_check_cb_f);

        if( rval != SPIFFS_OK && SPIFFS_errno(&g_hFS) == SPIFFS_ERR_NOT_A_FS )
        {
            msg("formatting file system ...\n");
            rval = SPIFFS_format(&g_hFS);
            if( rval != SPIFFS_OK )
            {
                err("SPIFFS format failed: %i\n", SPIFFS_errno(&g_hFS));
                break;
            }

            rval = SPIFFS_mount(&g_hFS,
                                pCfg,
                                g_spiffs_work,
                                g_spiffs_fd_buf, sizeof(g_spiffs_fd_buf),
                                g_spiffs_cache_buf, sizeof(g_spiffs_cache_buf),
                                _spiffs_check_cb_f);
        }

        if( rval != SPIFFS_OK )
        {
            err("SPIFFS mount failed: %i\n", SPIFFS_errno(&g_hFS));
            break;
        }

        msg("SPIFFS mounted\n");

    } while(0);

    return rval;
}


