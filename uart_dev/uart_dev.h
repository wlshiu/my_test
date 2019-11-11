/**
 * Copyright (c) 2019 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file uart_dev.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2019/11/11
 * @license
 * @description
 */

#ifndef __uart_dev_H_wZakHPLR_lbAK_Hman_shyf_uBhS4J1H9ryg__
#define __uart_dev_H_wZakHPLR_lbAK_Hman_shyf_uBhS4J1H9ryg__

#ifdef __cplusplus
extern "C" {
#endif


#include "uart_dev_desc.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
typedef enum uart_err
{
    UART_ERR_OK             = 0,
    UART_ERR_ALLOCATE_FAIL,
    UART_ERR_INVALID_PARAM,
    UART_ERR_UNKNOWN,

} uart_err_t;

typedef uart_ops_t* (*cb_set_user_ops_t)(uart_cfg_t *pCfg);
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

//=============================================================================
//                  Public Function Definition
//=============================================================================
uart_handle_t
uart_dev_init(
    uart_cfg_t          *pCfg,
    cb_set_user_ops_t   cb_set_user_ops);


uart_err_t
uart_dev_deinit(
    uart_handle_t   hUart);


uart_err_t
uart_dev_send_bytes(
    uart_handle_t   hUart,
    uint8_t         *pData,
    int             len);


uart_err_t
uart_dev_recv_bytes(
    uart_handle_t   hUart,
    uint8_t         *pData,
    int             *pLen);


uart_err_t
uart_dev_reset_buf(
    uart_handle_t   hUart,
    uint8_t         *pBuf,
    uint32_t        buf_len);


int
uart_dev_get_state(
    uart_handle_t   hUart,
    uart_state_t    state);


int
uart_dev_ctrl(
    uart_handle_t   hUart,
    uint32_t        op_code,
    uint32_t        value,
    void            *pExtra);


#ifdef __cplusplus
}
#endif

#endif
