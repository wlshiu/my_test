/**
 * Copyright (c) 2019 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file uart_dev_desc.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2019/11/11
 * @license
 * @description
 */

#ifndef __uart_dev_desc_H_wN5kj50n_lvfl_HkSS_s1Mw_uU1mSGThxhyg__
#define __uart_dev_desc_H_wN5kj50n_lvfl_HkSS_s1Mw_uU1mSGThxhyg__

#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>
#include <stdbool.h>
//=============================================================================
//                  Constant Definition
//=============================================================================
typedef enum uart_dev_type
{
    UART_DEV_TYPE_RASP_PI  = 0,
    UART_DEV_TYPE_SIM,
    UART_DEV_TYPE_TOTAL,
} uart_dev_type_t;

typedef enum uart_state
{
    UART_STATE_UNKNOWN      = 0,
    UART_STATE_GET_RX_EVENT,

} uart_state_t;
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef void*   uart_handle_t;

typedef struct uart_cfg
{
    uart_dev_type_t     act_dev_type;
    char                *pDev_name;

    union {
        struct {
            unsigned long    port;
            unsigned long    baud_rate;
        } uart;

        struct {
            void    *pTunnel_info;
        } def;
    };

    uint8_t     *pBuf;
    uint32_t    buf_len;

} uart_cfg_t;


typedef struct uart_ops
{
    uart_dev_type_t     dev_type;

    uart_handle_t       (*init)(uart_cfg_t *pCfg);
    int                 (*deinit)(uart_handle_t pHandle);

    /**
     *  @brief
     *
     *  @param [in] pHandle     uart handle
     *  @param [in] state       state type, enum uart_state
     *  @return
     *      0    : nothing
     *      other: the state is triggered
     */
    int     (*get_state)(uart_handle_t pHandle, uart_state_t state);

    /**
     *  @brief  send_bytes
     *
     *  @param [in] pHandle     uart handle
     *  @param [in] pData       data buffer pointer
     *  @param [in] data_len    length of data
     *  @return
     *      0: ok, other: fail
     */
    int     (*send_bytes)(uart_handle_t pHandle, uint8_t *pData, int data_len);

    /**
     *  @brief  recv_bytes
     *
     *  @param [in] pHandle     uart handle
     *  @param [in] pData       data buffer pointer
     *  @param [in] pData_len   length of data
     *  @return
     *      0: ok, other: fail
     */
    int     (*recv_bytes)(uart_handle_t pHandle, uint8_t *pData, int *pData_len);

    /**
     *  @brief  reset_buf
     *              This api will reset read/write index.
     *              It also support to assign new buffer address and new buffer length
     *
     *  @param [in] pBuf        new buffer address
     *  @param [in] buf_len     new buffer length
     *  @return
     *      0: ok, other: fail
     */
    int     (*reset_buf)(uart_handle_t pHandle, uint8_t *pBuf, uint32_t buf_len);

    /**
     *  @brief  control misc
     *
     *  @param [in] pHandle     uart handle
     *  @param [in] op_code     private operation code
     *  @param [in] value       private value
     *  @param [in] pExtra      private extra data
     *  @return
     *      0: ok, other: fail
     */
    int     (*ctrl)(uart_handle_t pHandle, uint32_t op_code, uint32_t value, void *pExtra);
} uart_ops_t;
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
