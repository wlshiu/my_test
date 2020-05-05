/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file upgrade_packets.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/05/04
 * @license
 * @description
 */

#ifndef __upgrade_packets_H_wVpOOKjX_lpGN_H4HY_sWlr_uYurDLITqxIP__
#define __upgrade_packets_H_wVpOOKjX_lpGN_H4HY_sWlr_uYurDLITqxIP__

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"
#include <stdint.h>
//=============================================================================
//                  Constant Definition
//=============================================================================
#define UPG_IGNORE              (-1)
#define UPG_DISCOVERY_ACK       0x55555

typedef enum upg_opcode
{
    UPG_OPCODE_UNKNOWN      = 0x0,

    UPG_OPCODE_BASE         = 0x30,
    UPG_OPCODE_DISCOVERY_REQ,
    UPG_OPCODE_DISCOVERY_RESP,

    UPG_OPCODE_REPORT       = 0x50,
    UPG_OPCODE_REPORT_VER_REQ,
    UPG_OPCODE_REPORT_VER_RESP,

    UPG_OPCODE_DATA         = 0xd0,
    UPG_OPCODE_DATA_SIZE,
    UPG_OPCODE_DATA_WR,
    UPG_OPCODE_DATA_WR_END,
    UPG_OPCODE_DATA_REQ,

} upg_opcode_t;
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
/**
 *  packet size = packet_header + payloay (2 bytes alignment) + checksum (2 bytes)
 *  ps. set dummy to '0'
 *
 */

#pragma pack(1)
typedef struct upg_pkt_hdr
{
    uint32_t    cmd_opcode    : 8;
    uint32_t    serial_num    : 12;
    uint32_t    total_packets : 12;

    uint32_t    packet_length : 12; // header + payload + checksum
    uint32_t    short_data    : 20; // short data or destination mem address


    // payload MUST be 2-bytes alignment
    uint8_t     *pPayload[];

//    uint16_t    crc;

} upg_pkt_hdr_t;


typedef struct upg_pkt_report_ver
{
    uint16_t    major : 4;
    uint16_t    minor : 4;
    uint16_t    patch : 8;
} upg_pkt_report_ver_t;


typedef struct upg_pkt_data_size
{
    uint32_t    total_packets : 16;
    uint32_t    unit_size     : 16;
} upg_pkt_data_size_t;

typedef struct upg_pkt_data_req
{
    uint16_t    serial_num;
} upg_pkt_data_req_t;

#pragma pack()


typedef struct upg_pkt_info
{
    upg_opcode_t    opcode;

    int             cur_pkt_len; // the real length of the output packet

    upg_pkt_hdr_t   *pPkt_hdr;

    uint8_t         *pBuf_pkt;
    int             buf_pkt_len;

    int (*cb_fill_data)(struct upg_pkt_info *pPkt_info);

    void            *pExtra;
} upg_pkt_info_t;

typedef struct upg_pkt_attr
{
    upg_opcode_t    opcode;

    int     (*pf_pack)(upg_pkt_info_t *pInfo);
    int     (*pf_unpack)(upg_pkt_info_t *pInfo);

} upg_pkt_attr_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
int
upg_pkt_pack(
    upg_pkt_info_t  *pInfo);


int
upg_pkt_unpack(
    upg_pkt_info_t  *pInfo);


#ifdef __cplusplus
}
#endif

#endif
