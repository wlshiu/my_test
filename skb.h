/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file skb.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/07/15
 * @license
 * @description
 */

#ifndef __skb_H_wdF36tif_lrpw_HphW_sqJF_utJ2iDQUDvCv__
#define __skb_H_wdF36tif_lrpw_HphW_sqJF_utJ2iDQUDvCv__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================
#define skb_pos_t           uint16_t
//=============================================================================
//                  Structure Definition
//=============================================================================
typedef void* (*cb_malloc_t)(uint32_t len);
typedef void  (*cb_free_t)(void *p);

#pragma pack(1)
/**
 *  the number of skb_frag_t should be limited
 */
typedef struct skb_frag
{
    struct skb_frag     *next;

    uint8_t     *pBuf;
    uint16_t    offset : 4; // for alignment of buffer;
    uint16_t    len    : 12;
} skb_frag_t;

typedef struct skb
{
    struct skb      *next;

    uint8_t     ref_cnt;

	uint16_t    len;
	uint16_t    data_len;

#if 1
	uint16_t    transport_hdr;
	uint16_t    network_hdr;
	uint16_t    mac_hdr;

	uint8_t		*head;
	skb_pos_t   data;
    skb_pos_t   tail;
	skb_pos_t   end;
#else
    /**
     *  the max skb buffer is 1 KBytes
     */
	uint32_t    transport_hdr : 10;
	uint32_t    network_hdr   : 10;
	uint32_t    mac_hdr       : 10;

    uint8_t		*head;
    uint32_t    data : 10;
    uint32_t    tail : 10;
    uint32_t    end  : 10;
    uint32_t    has_extern : 2;
#endif

} skb_t;

#pragma pack()

typedef struct skb_conf
{
    cb_malloc_t     pf_malloc;
    cb_free_t       pf_free;


    /**
     *  skb memory pool
     */
    uint8_t     *pMem_pool;
    uint32_t    mem_pool_len;
} skb_conf_t;

//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================






static inline void skb_ref(skb_t *pSkb)
{
    if( pSkb ) pSkb->ref_cnt++;
    return;
}



#ifdef __cplusplus
}
#endif

#endif
