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

#include "stdint.h"
//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct skb_conf
{
    uint32_t    reserved;
} skb_conf_t;

typedef struct skb
{
    struct skb      *next;

	uint16_t    len;
	uint16_t    data_len;

	uint16_t    transport_hdr;
	uint16_t    network_hdr;
	uint16_t    mac_hdr;

	uint8_t		*head;
	uint8_t     *data;
	uint8_t     *tail;
	uint8_t     *end;

} skb_t;
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
