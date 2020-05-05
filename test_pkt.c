/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file test_pkt.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/05/05
 * @license
 * @description
 */



#include "common.h"
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
static uint8_t      g_buf_pkt[2048] __attribute__ ((aligned(4))) = {0};
//=============================================================================
//                  Private Function Definition
//=============================================================================
static int
_fill_req(
    upg_pkt_info_t  *pInfo)
{
    upg_pkt_hdr_t   *pPkt_hdr = pInfo->pPkt_hdr;

    switch( pInfo->opcode )
    {
        default:    break;
        case UPG_OPCODE_DISCOVERY_RESP:
            pPkt_hdr->short_data = UPG_DISCOVERY_ACK;
            break;
        case UPG_OPCODE_DATA_WR:
            pPkt_hdr->short_data = 0x3333;

            // payload is appended to packet header
            snprintf((char*)pPkt_hdr->pPayload, pInfo->buf_pkt_len - pInfo->cur_pkt_len, "%s", "check write data");

            pInfo->cur_pkt_len += (strlen((const char*)pPkt_hdr->pPayload) + 1);

            if( pInfo->cur_pkt_len & 0x1 )
            {
                pPkt_hdr->pPayload[pInfo->cur_pkt_len] = 0;

                // packet length MUST be 2-bytes alignment
                pInfo->cur_pkt_len++;
            }
            break;
    }
    return 0;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
int main(void)
{
    int     rval = 0;
    do {
        upg_pkt_info_t      pkt_info = {0};

//        pkt_info.opcode       = UPG_OPCODE_DISCOVERY_REQ;
        pkt_info.opcode       = UPG_OPCODE_DATA_WR;
        pkt_info.cur_pkt_len  = 0;
        pkt_info.pPkt_hdr     = (upg_pkt_hdr_t*)g_buf_pkt;
        pkt_info.pBuf_pkt     = g_buf_pkt;
        pkt_info.buf_pkt_len  = sizeof(g_buf_pkt);
        pkt_info.cb_fill_data = _fill_req;

        rval = upg_pkt_pack(&pkt_info);
        if( rval )
        {
            printf("pack fail %d\n", rval);
            break;
        }

        memset(&pkt_info, 0x0, sizeof(upg_pkt_info_t));

        pkt_info.pBuf_pkt     = g_buf_pkt;
        pkt_info.buf_pkt_len  = sizeof(g_buf_pkt);
        rval = upg_pkt_unpack(&pkt_info);
        if( pkt_info.pPkt_hdr )
        {
            upg_pkt_hdr_t   *pPkt_hdr = pkt_info.pPkt_hdr;
            printf("get pkt %d/%d\n", pPkt_hdr->serial_num, pPkt_hdr->total_packets);
            printf("payload: -%s-\n", (char*)pPkt_hdr->pPayload);
        }

    } while(0);

    system("pause");
    return 0;
}
