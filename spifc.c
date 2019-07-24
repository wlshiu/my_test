/**
 * Copyright (c) 2019 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file spifc.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2019/06/09
 * @license
 * @description
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "spifc.h"

//=============================================================================
//                  Constant Definition
//=============================================================================
#define CONFIG_SPIFC_SIM_SIZE               (1 << 20)
#define CONFIG_SPIFC_1_BLOCK_SIZE           (64 << 10)
//=============================================================================
//                  Macro Definition
//=============================================================================
#define __unused                __attribute__((__unused__))
#define __packed                __attribute__((packed))
#define __aligned(x)            __attribute__((aligned(x)))
#define __section(x)            __attribute((section(x)))
#define __weak                  __attribute__((weak))
#define __naked                 __attribute__((naked))


#define err(str, ...)           do{ printf("[%s:%u] " str, __func__, __LINE__, ## __VA_ARGS__ ); while(1); }while(0)
//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================
static uint8_t      g_spifc_mem_sim[CONFIG_SPIFC_SIM_SIZE] __aligned(4) = {0};
static uint8_t      g_fc_sector_cache[SPIFC_1_SECTOR_SIZE] __aligned(4) = {0};
//=============================================================================
//                  Private Function Definition
//=============================================================================
static void
_spi_program_page(uint8_t *pData, uintptr_t flash_addr, uint32_t bytes)
{
    uint32_t    cnt = bytes >> 2;
    uint32_t    *pCur = (uint32_t*)pData;

    for(int i = 0; i < cnt; i++)
    {
        uint32_t    *pDest = (uint32_t*)g_spifc_mem_sim;

        pDest = (uint32_t*)((uintptr_t)pDest + (flash_addr + (i << 2)));

        *pDest &= *pCur++;
    }
    return;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
spifc_err_t
spifc_erase(
    spifc_erase_mode_t  mode,
    uintptr_t           flash_addr,
    int                 unit_num)
{
    spifc_err_t     rval = SPIFC_ERR_OK;
    do {
        if( unit_num == 0 )
        {
            rval = SPIFC_ERR_WRONG_ARGUMENT;
            break;
        }

        if( flash_addr < CONFIG_SPIFC_MEM_BASE ||
            flash_addr >= CONFIG_SPIFC_MEM_BASE + CONFIG_SPIFC_SIM_SIZE )
        {
            rval = SPIFC_ERR_WRONG_ADDR;
            break;
        }

        flash_addr -= CONFIG_SPIFC_MEM_BASE;

        if( mode == SPIFC_ERASE_SECTOR )
        {
            if( flash_addr & (SPIFC_1_SECTOR_SIZE - 1) )
            {
                rval = SPIFC_ERR_ADDR_NOT_ALIGNMENT;
                break;
            }

            memset(&g_spifc_mem_sim[flash_addr], 0xFF, unit_num * SPIFC_1_SECTOR_SIZE);
        }
        else if( mode == SPIFC_ERASE_BLOCK )
        {
            if( flash_addr & (CONFIG_SPIFC_1_BLOCK_SIZE - 1) )
            {
                rval = SPIFC_ERR_ADDR_NOT_ALIGNMENT;
                break;
            }

            memset(&g_spifc_mem_sim[flash_addr], 0xFF, unit_num * CONFIG_SPIFC_1_BLOCK_SIZE);
        }
        else if( mode == SPIFC_ERASE_CHIP )
        {
            memset(&g_spifc_mem_sim[0], 0xFF, sizeof(g_spifc_mem_sim));
        }
        else        rval = SPIFC_ERR_WRONG_ARGUMENT;

    } while(0);
    return rval;
}

spifc_err_t
spifc_read(
    uint8_t     *pSys_buf,
    uintptr_t   flash_addr,
    int         bytes)
{
    spifc_err_t     rval = SPIFC_ERR_OK;
    do {
        if( bytes == 0 )
        {
            rval = SPIFC_ERR_WRONG_ARGUMENT;
            break;
        }

        if( flash_addr < CONFIG_SPIFC_MEM_BASE ||
            flash_addr >= CONFIG_SPIFC_MEM_BASE + CONFIG_SPIFC_SIM_SIZE )
        {
            rval = SPIFC_ERR_WRONG_ADDR;
            break;
        }

        flash_addr -= CONFIG_SPIFC_MEM_BASE;

        // TODO: use sector cache
        memcpy(pSys_buf, &g_spifc_mem_sim[flash_addr], bytes);
    } while(0);
    return rval;
}

spifc_err_t
spifc_write(
    uint8_t     *pSys_buf,
    uintptr_t   flash_addr,
    int         bytes)
{
    spifc_err_t     rval = SPIFC_ERR_OK;
    do {
        uintptr_t   from_pos = 0;
        uintptr_t   to_pos = 0;
        uint32_t    offset = 0;
        uint32_t    valid_len = 0;
        int         remain = bytes;

        if( bytes == 0 )
        {
            rval = SPIFC_ERR_WRONG_ARGUMENT;
            break;
        }

        if( flash_addr < CONFIG_SPIFC_MEM_BASE ||
            flash_addr >= CONFIG_SPIFC_MEM_BASE + CONFIG_SPIFC_SIM_SIZE )
        {
            rval = SPIFC_ERR_WRONG_ADDR;
            break;
        }

        offset    = flash_addr & (SPIFC_1_SECTOR_SIZE - 1);
        valid_len = SPIFC_1_SECTOR_SIZE - offset;
        to_pos    = flash_addr & ~(SPIFC_1_SECTOR_SIZE - 1);
        from_pos  = (uintptr_t)pSys_buf;

        // read data to cache
        spifc_read(g_fc_sector_cache, to_pos, SPIFC_1_SECTOR_SIZE);

        valid_len = (valid_len < bytes) ? valid_len : bytes;

        // merge data
        memcpy(&g_fc_sector_cache[offset], (void*)from_pos, valid_len);
        from_pos += valid_len;

        // erase the sector
        spifc_erase(SPIFC_ERASE_SECTOR, to_pos, 1);

        // page program
        for(int i = 0; i < (SPIFC_1_SECTOR_SIZE >> SPIFC_PAGE_SIZE_POW_2); i++)
        {
            _spi_program_page((uint8_t*)&g_fc_sector_cache[i * (0x1 << SPIFC_PAGE_SIZE_POW_2)],
                              to_pos - CONFIG_SPIFC_MEM_BASE,
                              (0x1 << SPIFC_PAGE_SIZE_POW_2));

            to_pos += (0x1 << SPIFC_PAGE_SIZE_POW_2);
        }

        remain -= valid_len;
        while( (remain & ~(SPIFC_1_SECTOR_SIZE - 1)) )
        {
            // erase sector
            spifc_erase(SPIFC_ERASE_SECTOR, to_pos, 1);

            // page program
            for(int i = 0; i < (SPIFC_1_SECTOR_SIZE >> SPIFC_PAGE_SIZE_POW_2); i++)
            {
                _spi_program_page((uint8_t*)from_pos, to_pos - CONFIG_SPIFC_MEM_BASE, (0x1 << SPIFC_PAGE_SIZE_POW_2));

                from_pos += (0x1 << SPIFC_PAGE_SIZE_POW_2);
                to_pos   += (0x1 << SPIFC_PAGE_SIZE_POW_2);
            }

            // update info
            remain -= SPIFC_1_SECTOR_SIZE;
        }

        if( remain )
        {
            // read data to cache
            spifc_read(g_fc_sector_cache, to_pos, SPIFC_1_SECTOR_SIZE);

            // merge data
            memcpy(g_fc_sector_cache, (void*)from_pos, remain);

            // erase sector
            spifc_erase(SPIFC_ERASE_SECTOR, to_pos, 1);

            // page program
            for(int i = 0; i < (SPIFC_1_SECTOR_SIZE >> SPIFC_PAGE_SIZE_POW_2); i++)
            {
                _spi_program_page((uint8_t*)&g_fc_sector_cache[i * (0x1 << SPIFC_PAGE_SIZE_POW_2)],
                                  to_pos - CONFIG_SPIFC_MEM_BASE,
                                  (0x1 << SPIFC_PAGE_SIZE_POW_2));

                to_pos += (0x1 << SPIFC_PAGE_SIZE_POW_2);
            }
        }
    } while(0);
    return rval;
}

spifc_err_t
spifc_program(
    uint8_t     *pSys_buf,
    uintptr_t   flash_addr,
    int         bytes)
{
    spifc_err_t     rval = SPIFC_ERR_OK;
    do {
        uintptr_t   from_pos = 0;
        uintptr_t   to_pos = 0;

        if( bytes == 0 )
        {
            rval = SPIFC_ERR_WRONG_ARGUMENT;
            break;
        }

        if( flash_addr < CONFIG_SPIFC_MEM_BASE ||
            flash_addr >= CONFIG_SPIFC_MEM_BASE + CONFIG_SPIFC_SIM_SIZE )
        {
            rval = SPIFC_ERR_WRONG_ADDR;
            break;
        }

        flash_addr -= CONFIG_SPIFC_MEM_BASE;

        to_pos    = flash_addr;
        from_pos  = (uintptr_t)pSys_buf;

        for(int i = 0; i < (bytes >> SPIFC_PAGE_SIZE_POW_2); i++)
        {
            _spi_program_page((uint8_t*)from_pos, to_pos, (0x1 << SPIFC_PAGE_SIZE_POW_2));

            from_pos += (0x1 << SPIFC_PAGE_SIZE_POW_2);
            to_pos   += (0x1 << SPIFC_PAGE_SIZE_POW_2);
        }

        bytes = bytes & ((0x1 << SPIFC_PAGE_SIZE_POW_2) - 1);
        if( bytes )
        {
            _spi_program_page((uint8_t*)from_pos, to_pos, bytes);

            from_pos += bytes;
            to_pos   += bytes;
        }
    } while(0);
    return rval;
}
