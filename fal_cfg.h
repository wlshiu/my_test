/*
 * Copyright (c) 2020, Armink, <armink.ztl@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _FAL_CFG_H_
#define _FAL_CFG_H_

#define CONFIG_FLASH_MEM_BASE           0x90000000ul
#define CONFIG_FLASH_CAPACITY           (256ul << 10)

#define FAL_DEBUG 1
#define FAL_PART_HAS_TABLE_CFG



/* ===================== Flash device Configuration ========================= */
extern const struct fal_flash_dev sim_flash;

/* flash device table */
#define FAL_FLASH_DEV_TABLE                     \
{                                               \
    &sim_flash,                                 \
}

/* ====================== Partition Configuration ========================== */
#ifdef FAL_PART_HAS_TABLE_CFG

/* partition table */
#define FAL_PART_TABLE                                                                \
{                                                                                     \
    {FAL_PART_MAGIC_WORD,  "fdb_tsdb1",    "sim_extflash",   0,        64*1024, 0},  \
    {FAL_PART_MAGIC_WORD,  "fdb_kvdb1",    "sim_eflash",     64*1024,  128*1024, 0},  \
}

#endif /* FAL_PART_HAS_TABLE_CFG */

#endif /* _FAL_CFG_H_ */
