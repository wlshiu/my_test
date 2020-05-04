/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file upgrade_packages.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/05/04
 * @license
 * @description
 */

#ifndef __upgrade_packages_H_wVpOOKjX_lpGN_H4HY_sWlr_uYurDLITqxIP__
#define __upgrade_packages_H_wVpOOKjX_lpGN_H4HY_sWlr_uYurDLITqxIP__

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"
//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct upg_pkg_hdr
{
    uint32_t    dest_id;
    uint32_t    package_length; // header + payload

    uint32_t    cmd_type        : 4;
    uint32_t    cmd_opcode      : 4;
    uint32_t    serial_num      : 12;
    uint32_t    total_packages  : 12;

    union {
        uint32_t    dest_mem_addr;
        uint32_t    data;
    };

} upg_pkg_hdr_t;

typedef struct upg_pkg_data_req
{
    uint32_t    serial_num;
} upg_pkg_data_req_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================

#ifdef __cplusplus
}
#endif

#endif
