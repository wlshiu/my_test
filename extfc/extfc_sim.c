/**
 * Copyright (c) 2019 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file extfc_sim.c
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
#include "extfc.h"

//=============================================================================
//                  Constant Definition
//=============================================================================
#define CONFIG_EXTFC_MEM_MASK               (0xFF000000u)

#define CONFIG_EXTFC_SIM_SIZE               (16 << 20)
#define CONFIG_EXTFC_1_SECTOR_SIZE          (4 << 10)
#define CONFIG_EXTFC_1_BLOCK_SIZE           (64 << 10)

#define EXTFC_PAGE_SIZE_POW2                8
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
uint8_t      g_flash_mem_sim[CONFIG_EXTFC_SIM_SIZE] __aligned(4) = {0};
static uint8_t      g_fc_sector_cache[CONFIG_EXTFC_1_SECTOR_SIZE] __aligned(4) = {0};
//=============================================================================
//                  Private Function Definition
//=============================================================================
static void
_fc_program_page(uint8_t *pData, uintptr_t flash_addr, uint32_t bytes)
{
    for(int i = 0; i < bytes; i++)
    {
        uint8_t     *pDest = (uint8_t*)g_flash_mem_sim;

        pDest = pDest + flash_addr + i;

        *pDest &= *pData++;
    }
    return;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
extfc_err_t
extfc_erase(
    extfc_erase_mode_t  mode,
    uintptr_t           flash_addr,
    int                 unit_num)
{
    extfc_err_t     rval = EXTFC_ERR_OK;
    do {
        if( unit_num == 0 )
        {
            rval = EXTFC_ERR_WRONG_ARGUMENT;
            break;
        }

        if( flash_addr < CONFIG_EXTFC_MEM_BASE ||
            flash_addr >= (CONFIG_EXTFC_MEM_BASE + CONFIG_EXTFC_SIM_SIZE) )
        {
            rval = EXTFC_ERR_WRONG_ADDR;
            break;
        }

        flash_addr -= CONFIG_EXTFC_MEM_BASE;

        if( mode == EXTFC_ERASE_SECTOR )
        {
            if( flash_addr & (CONFIG_EXTFC_1_SECTOR_SIZE - 1) )
            {
                rval = EXTFC_ERR_ADDR_NOT_ALIGNMENT;
                break;
            }

            memset(&g_flash_mem_sim[flash_addr], 0xFF, unit_num * CONFIG_EXTFC_1_SECTOR_SIZE);
        }
        else if( mode == EXTFC_ERASE_BLOCK )
        {
            if( flash_addr & (CONFIG_EXTFC_1_BLOCK_SIZE - 1) )
            {
                rval = EXTFC_ERR_ADDR_NOT_ALIGNMENT;
                break;
            }

            memset(&g_flash_mem_sim[flash_addr], 0xFF, unit_num * CONFIG_EXTFC_1_BLOCK_SIZE);
        }
        else if( mode == EXTFC_ERASE_CHIP )
        {
            memset(&g_flash_mem_sim[0], 0xFF, sizeof(g_flash_mem_sim));
        }
        else        rval = EXTFC_ERR_WRONG_ARGUMENT;

    } while(0);
    return rval;
}

extfc_err_t
extfc_read(
    uint8_t     *pSys_buf,
    uintptr_t   flash_addr,
    int         bytes)
{
    extfc_err_t     rval = EXTFC_ERR_OK;
    do {
        if( bytes == 0 )
        {
            rval = EXTFC_ERR_WRONG_ARGUMENT;
            break;
        }

        if( flash_addr < CONFIG_EXTFC_MEM_BASE ||
            flash_addr >= (CONFIG_EXTFC_MEM_BASE + CONFIG_EXTFC_SIM_SIZE) )
        {
            rval = EXTFC_ERR_WRONG_ADDR;
            break;
        }

        flash_addr -= CONFIG_EXTFC_MEM_BASE;

        // TODO: use sector cache
        memcpy(pSys_buf, &g_flash_mem_sim[flash_addr], bytes);
    } while(0);
    return rval;
}

extfc_err_t
extfc_write(
    uint8_t     *pSys_buf,
    uintptr_t   flash_addr,
    int         bytes)
{
    extfc_err_t     rval = EXTFC_ERR_OK;
    do {
        uintptr_t   from_pos = 0;
        uintptr_t   to_pos = 0;
        uint32_t    offset = 0;
        uint32_t    valid_len = 0;
        int         remain = bytes;

        if( bytes == 0 )
        {
            rval = EXTFC_ERR_WRONG_ARGUMENT;
            break;
        }

        if( flash_addr < CONFIG_EXTFC_MEM_BASE ||
            flash_addr >= (CONFIG_EXTFC_MEM_BASE + CONFIG_EXTFC_SIM_SIZE) )
        {
            rval = EXTFC_ERR_WRONG_ADDR;
            break;
        }

        offset    = flash_addr & (CONFIG_EXTFC_1_SECTOR_SIZE - 1);
        valid_len = CONFIG_EXTFC_1_SECTOR_SIZE - offset;
        to_pos    = flash_addr & ~(CONFIG_EXTFC_1_SECTOR_SIZE - 1);
        from_pos  = (uintptr_t)pSys_buf;

        // read data to cache
        extfc_read(g_fc_sector_cache, to_pos, CONFIG_EXTFC_1_SECTOR_SIZE);

        valid_len = (valid_len < bytes) ? valid_len : bytes;

        // merge data
        memcpy(&g_fc_sector_cache[offset], (void*)from_pos, valid_len);
        from_pos += valid_len;

        // erase the sector
        extfc_erase(EXTFC_ERASE_SECTOR, to_pos, 1);

        // page program
        for(int i = 0; i < (CONFIG_EXTFC_1_SECTOR_SIZE >> EXTFC_PAGE_SIZE_POW2); i++)
        {
            _fc_program_page((uint8_t*)&g_fc_sector_cache[i * (0x1 << EXTFC_PAGE_SIZE_POW2)],
                             to_pos - CONFIG_EXTFC_MEM_BASE,
                             (0x1 << EXTFC_PAGE_SIZE_POW2));

            to_pos += (0x1 << EXTFC_PAGE_SIZE_POW2);
        }

        remain -= valid_len;
        while( (remain & ~(CONFIG_EXTFC_1_SECTOR_SIZE - 1)) )
        {
            // erase sector
            extfc_erase(EXTFC_ERASE_SECTOR, to_pos, 1);

            // page program
            for(int i = 0; i < (CONFIG_EXTFC_1_SECTOR_SIZE >> EXTFC_PAGE_SIZE_POW2); i++)
            {
                _fc_program_page((uint8_t*)from_pos, to_pos - CONFIG_EXTFC_MEM_BASE, (0x1 << EXTFC_PAGE_SIZE_POW2));

                from_pos += (0x1 << EXTFC_PAGE_SIZE_POW2);
                to_pos   += (0x1 << EXTFC_PAGE_SIZE_POW2);
            }

            // update info
            remain -= CONFIG_EXTFC_1_SECTOR_SIZE;
        }

        if( remain )
        {
            // read data to cache
            extfc_read(g_fc_sector_cache, to_pos, CONFIG_EXTFC_1_SECTOR_SIZE);

            // merge data
            memcpy(g_fc_sector_cache, (void*)from_pos, remain);

            // erase sector
            extfc_erase(EXTFC_ERASE_SECTOR, to_pos, 1);

            // page program
            for(int i = 0; i < (CONFIG_EXTFC_1_SECTOR_SIZE >> EXTFC_PAGE_SIZE_POW2); i++)
            {
                _fc_program_page((uint8_t*)&g_fc_sector_cache[i * (0x1 << EXTFC_PAGE_SIZE_POW2)],
                                  to_pos - CONFIG_EXTFC_MEM_BASE,
                                  (0x1 << EXTFC_PAGE_SIZE_POW2));

                to_pos += (0x1 << EXTFC_PAGE_SIZE_POW2);
            }
        }
    } while(0);
    return rval;
}

extfc_err_t
extfc_program(
    uint8_t     *pSys_buf,
    uintptr_t   flash_addr,
    int         bytes)
{
    extfc_err_t     rval = EXTFC_ERR_OK;
    do {
        uintptr_t   from_pos = 0;
        uintptr_t   to_pos = 0;

        if( bytes == 0 )
        {
            rval = EXTFC_ERR_WRONG_ARGUMENT;
            break;
        }

        if( flash_addr < CONFIG_EXTFC_MEM_BASE ||
            flash_addr >= (CONFIG_EXTFC_MEM_BASE + CONFIG_EXTFC_SIM_SIZE) )
        {
            rval = EXTFC_ERR_WRONG_ADDR;
            break;
        }

        flash_addr -= CONFIG_EXTFC_MEM_BASE;

        to_pos    = flash_addr;
        from_pos  = (uintptr_t)pSys_buf;

        for(int i = 0; i < (bytes >> EXTFC_PAGE_SIZE_POW2); i++)
        {
            _fc_program_page((uint8_t*)from_pos, to_pos, (0x1 << EXTFC_PAGE_SIZE_POW2));

            from_pos += (0x1 << EXTFC_PAGE_SIZE_POW2);
            to_pos   += (0x1 << EXTFC_PAGE_SIZE_POW2);
        }

        bytes = bytes & ((0x1 << EXTFC_PAGE_SIZE_POW2) - 1);
        if( bytes )
        {
            _fc_program_page((uint8_t*)from_pos, to_pos, bytes);

            from_pos += bytes;
            to_pos   += bytes;
        }
    } while(0);
    return rval;
}
