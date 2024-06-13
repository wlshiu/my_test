/*
 * Copyright (c) 2020, Armink, <armink.ztl@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include <fal.h>


#define PAGE_SIZE       256

#define __aligned(x)            __attribute__((aligned(x)))

extern const struct fal_flash_dev       sim_flash;

static uint8_t      g_spifc_mem_sim[CONFIG_FLASH_CAPACITY] __aligned(4) = {0};

static int init(void)
{
    /* do nothing now */
    return 1;
}


void feed_dog(void)
{

}

static int read(long offset, uint8_t *buf, size_t size)
{
    do {
        uint8_t     *pCur = 0;
        if( (offset + size) > sim_flash.len )
        {
            size = 0;
            printf("[%s: %d] size out range (%d + %d) > %d\n",
                   __func__, __LINE__,
                   offset, size,
                   sim_flash.len);
            break;
        }

        pCur = &g_spifc_mem_sim[offset];

        while( size-- )
        {
            *buf++ = *pCur++;
        }
    } while(0);

    return size;
}


static int write(long offset, const uint8_t *buf, size_t size)
{
    do {
        uint8_t     *pCur = 0;
        if( (offset + size) > sim_flash.len )
        {
            size = 0;
            printf("[%s: %d] size out range (%d + %d) > %d\n",
                   __func__, __LINE__,
                   offset, size,
                   sim_flash.len);
            break;
        }

        pCur = &g_spifc_mem_sim[offset];

        while( size--)
        {
            *pCur++ &= *buf++;
        }

    } while(0);
    return size;
}


static int erase(long offset, size_t size)
{
    do {
        uint8_t     *pCur = 0;
        if( (offset + size) > sim_flash.len )
        {
            size = 0;
            printf("[%s: %d] size out range (%d + %d) > %d\n",
                   __func__, __LINE__,
                   offset, size,
                   sim_flash.len);
            break;
        }

        pCur = &g_spifc_mem_sim[offset];
        if( (offset & (sim_flash.blk_size - 1)) )
        {
            size = 0;
            printf("[%s: %d] address not %d align (0x%X)",
                   __func__, __LINE__,
                   sim_flash.blk_size,
                   offset);
            break;
        }

        if( size & (sim_flash.blk_size - 1) )
        {
            size = 0;
            printf("[%s: %d] size not %d align (0x%X)",
                   __func__, __LINE__,
                   sim_flash.blk_size,
                   size);
            break;
        }

        memset(pCur, 0xFF, size);

    } while(0);
    return size;
}


const struct fal_flash_dev      sim_flash =
{
    .name       = "sim_chip",
    .addr       = CONFIG_FLASH_MEM_BASE,
    .len        = CONFIG_FLASH_CAPACITY,
    .blk_size   = 4 * 1024,
    .ops        = {init, read, write, erase},
    .write_gran = 32
};


