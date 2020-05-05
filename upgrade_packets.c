/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file upgrade_packets.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/05/04
 * @license
 * @description
 */


#include "upgrade_packets.h"

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

//=============================================================================
//                  Private Function Definition
//=============================================================================
static uint16_t
_upg_chksum(uint16_t sum, const uint8_t *data, uint16_t len)
{
    uint16_t        t;
    const uint8_t   *pCur;
    const uint8_t   *pLast_byte;

    pCur       = data;
    pLast_byte = data + len - 1;

    while( pCur < pLast_byte )
    {
        t = (pCur[0] << 8) + pCur[1];
        sum += t;
        if(sum < t)
            sum++;

        pCur += 2;
    }

    if( pCur == pLast_byte )
    {
        t = (pCur[0] << 8);
        sum += t;
        if( sum < t )
            sum++;
    }

    return sum;
}

static int
_upg_pkt_discovery_pack(
    upg_pkt_info_t  *pInfo)
{
    int     rval = 0;
    do {
        upg_pkt_hdr_t   *pPkt_hdr = pInfo->pPkt_hdr;

        if( pInfo->buf_pkt_len < (sizeof(upg_pkt_hdr_t) + 2) )
        {
            rval = -2;
            break;
        }

        pPkt_hdr->cmd_opcode    = UPG_OPCODE_DISCOVERY_REQ;
        pPkt_hdr->serial_num    = 1;
        pPkt_hdr->total_packets = 1;
        pPkt_hdr->packet_length = sizeof(upg_pkt_hdr_t);
        pPkt_hdr->short_data    = UPG_IGNORE;

        pInfo->cur_pkt_len = sizeof(upg_pkt_hdr_t);

        //--------------------------------
        // user fill data
        if( pInfo->cb_fill_data )
        {
            rval = pInfo->cb_fill_data(pInfo);
            if( rval ) break;
        }

        // check 2-bytes alignment
        if( pInfo->cur_pkt_len & 0x1 )
        {
            rval = -3;
            break;
        }

        pPkt_hdr->packet_length = pInfo->cur_pkt_len + 2;

        {   // checksum
            uint16_t    sum = 0, *pSum = 0;
            pSum = (uint16_t*)((uint32_t)pInfo->pBuf_pkt + pInfo->cur_pkt_len);
            sum = ~(_upg_chksum(0, pInfo->pBuf_pkt, pInfo->cur_pkt_len));

            *pSum = (((sum << 8) & 0xFF00) | ((sum >> 8) & 0xFF));
        }

        pInfo->cur_pkt_len += 2;

    } while(0);
    return rval;
}

//=============================================================================
//                  Public Function Definition
//=============================================================================
static const upg_pkt_attr_t       const g_upg_pkts [] =
{
    { .opcode = UPG_OPCODE_DISCOVERY_REQ, .pf_pack = _upg_pkt_discovery_pack, .pf_unpack = 0, },
};

int
upg_pkt_pack(
    upg_pkt_info_t  *pInfo)
{
    int     rval = -1;
    do {
        upg_pkt_hdr_t   *pPkt_hdr = pInfo->pPkt_hdr;

        if( pInfo->buf_pkt_len < (sizeof(upg_pkt_hdr_t) + 2) )
        {
            rval = -2;
            break;
        }

        pPkt_hdr->cmd_opcode    = pInfo->opcode;
        pPkt_hdr->serial_num    = 1;
        pPkt_hdr->total_packets = 1;
        pPkt_hdr->short_data    = UPG_IGNORE;

        pInfo->cur_pkt_len = sizeof(upg_pkt_hdr_t);

        //--------------------------------
        // user fill data
        if( pInfo->cb_fill_data )
        {
            rval = pInfo->cb_fill_data(pInfo);
            if( rval ) break;
        }

        // check 2-bytes alignment
        if( pInfo->cur_pkt_len & 0x1 )
        {
            rval = -3;
            break;
        }

        pPkt_hdr->packet_length = pInfo->cur_pkt_len + 2;

        {   // checksum
            uint16_t    sum = 0, *pSum = 0;
            pSum = (uint16_t*)((uint32_t)pInfo->pBuf_pkt + pInfo->cur_pkt_len);
            sum = ~(_upg_chksum(0, pInfo->pBuf_pkt, pInfo->cur_pkt_len));

            *pSum = (((sum << 8) & 0xFF00) | ((sum >> 8) & 0xFF));
        }

        pInfo->cur_pkt_len += 2;

#if 0
        int     total = sizeof(g_upg_pkts)/sizeof(g_upg_pkts[0]);

        for(int i = 0; i < total; i++)
        {
            upg_pkt_attr_t      *pPkt_cur = (upg_pkt_attr_t*)&g_upg_pkts[i];

            if( pInfo->opcode == pPkt_cur->opcode )
            {
                rval = pPkt_cur->pf_pack(pInfo);
                break;
            }
        }
#endif
    } while(0);
    return rval;
}

int
upg_pkt_unpack(
    upg_pkt_info_t  *pInfo)
{
    int     rval = 0;
    do {
        upg_pkt_hdr_t   *pPkt_hdr = 0;
        uint16_t        sum = 0;

        if( !pInfo || !pInfo->pBuf_pkt || !pInfo->buf_pkt_len )
        {
            rval = -1;
            break;
        }

        //-------------------------
        // parsing pkt_hdr
        pInfo->opcode      = UPG_OPCODE_UNKNOWN;
        pInfo->cur_pkt_len = 0;
        pInfo->pPkt_hdr    = 0;

        pPkt_hdr = (upg_pkt_hdr_t*)pInfo->pBuf_pkt;

        // calculate CRC
        sum = _upg_chksum(0, pInfo->pBuf_pkt, pPkt_hdr->packet_length);
        if( sum != 0xFFFF )
        {
            rval = -2;
            break;
        }

        pInfo->opcode      = pPkt_hdr->cmd_opcode;
        pInfo->cur_pkt_len = pPkt_hdr->packet_length - 2;
        pInfo->buf_pkt_len = pInfo->cur_pkt_len;
        pInfo->pPkt_hdr    = pPkt_hdr;

#if 0
        //-------------------------
        // unpack the data
        int     total = sizeof(g_upg_pkts)/sizeof(g_upg_pkts[0]);

        for(int i = 0; i < total; i++)
        {
            upg_pkt_attr_t      *pPkt_cur = (upg_pkt_attr_t*)&g_upg_pkts[i];

            if( pInfo->opcode == pPkt_cur->opcode &&
                pPkt_cur->pf_unpack )
            {
                rval = pPkt_cur->pf_unpack(pInfo);
                break;
            }
        }
#endif
    } while(0);
    return rval;
}

