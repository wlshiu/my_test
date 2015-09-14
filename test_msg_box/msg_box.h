/**
 * Copyright (c) 2015 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file msg_box.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2015/08/21
 * @license
 * @description
 */

#ifndef __msg_box_H_wWBnyYxc_lS29_HS2v_sAEo_uL2fBI258zTr__
#define __msg_box_H_wWBnyYxc_lS29_HS2v_sAEo_uL2fBI258zTr__

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
//=============================================================================
//                Constant Definition
//=============================================================================
/**
 *  message global type
 */
typedef enum msg_global_type
{
    MSG_GLOBAL_TYPE_EMERGENCY       = 0xBA000000,

} msg_global_type_t;
//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================
/**
 *
 */

/**
 *  message arguments
 */
typedef struct msg_arg
{
    uint32_t    reserved;

    struct {
        uint8_t     *pTunnel_info;
        uint32_t    value;
    } rece[2];

    struct {
        uint8_t     *pTunnel_info;
        uint32_t    value;
    } reply[2];

} msg_arg_t;

/**
 *  message node
 */
typedef struct msg_node
{
    uint32_t    type;
    msg_arg_t   msg_arg;

    int     (*destroy_node)(struct msg_node **ppMsg_node);
    int     (*handle_node)(struct msg_node *pMsg_node);

} msg_node_t;

/**
 *  message info
 */
typedef struct msg_info
{
    uint32_t        type;

    msg_arg_t   *pMsg_arg;

    msg_node_t  *pMsg_node;

    int     (*create_node)(struct msg_info *pMsg_info);
    int     (*destroy_node)(msg_node_t **ppMsg_node);
    int     (*handle_node)(msg_node_t *pMsg_node);
} msg_info_t;
//=============================================================================
//                Global Data Definition
//=============================================================================

//=============================================================================
//                Private Function Definition
//=============================================================================

//=============================================================================
//                Public Function Definition
//=============================================================================
int
msg_box_post_node(
    msg_info_t  *pMsg_info);


int
msg_box_fetch_node(
    msg_node_t   **ppMsg_node);


int
msg_box_erase_all(void);


#ifdef __cplusplus
}
#endif

#endif
