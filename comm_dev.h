/**
 * Copyright (c) 2021 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file comm_dev.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2021/06/09
 * @license
 * @description
 */

#ifndef __comm_dev_H_wnxn3z26_lA7P_HTZn_s9ZE_u6EBPNPXOHXT__
#define __comm_dev_H_wnxn3z26_lA7P_HTZn_s9ZE_u6EBPNPXOHXT__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
//=============================================================================
//                  Constant Definition
//=============================================================================
typedef enum comm_state
{
    COMM_STATE_UNKNOWN      = 0,
    COMM_STATE_GET_RX_EVENT,

} comm_state_t;
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef void*   comm_handle_t;

typedef struct comm_cfg
{
    union {
        struct {
            char        *pComport_name;
            uint32_t    baudrate;
        } comport;

        struct {
            void    *pTunnel_info;
        } def;
    };

    uint8_t     *pBuf;
    uint32_t    buf_len;

} comm_cfg_t;

typedef struct comm_dev_desc
{
    comm_handle_t       (*init)(comm_cfg_t *pCfg);
    int                 (*deinit)(comm_handle_t pHandle);

    /**
     *  @brief  send_bytes
     *
     *  @param [in] pHandle     communication handle
     *  @param [in] pData       data buffer pointer
     *  @param [in] data_len    length of data
     *  @return
     *      0: ok, other: fail
     */
    int     (*send_bytes)(comm_handle_t pHandle, uint8_t *pData, int data_len);

    /**
     *  @brief  recv_bytes
     *
     *  @param [in] pHandle     communication handle
     *  @param [in] pData       data buffer pointer
     *  @param [in] pData_len   length of data
     *  @return
     *      0: ok, other: fail
     */
    int     (*recv_bytes)(comm_handle_t pHandle, uint8_t *pData, int *pData_len);

    /**
     *  @brief  reset_buf
     *              This api will reset read/write index.
     *              It also support to assign new buffer adderss and new buffer length
     *
     *  @param [in] pBuf        new buffer address
     *  @param [in] buf_len     new buffer length
     *  @return
     *      0: ok, other: fail
     */
    int     (*reset_buf)(comm_handle_t pHandle, uint8_t *pBuf, uint32_t buf_len);

    /**
     *  @brief
     *
     *  @param [in] pHandle     communication handle
     *  @param [in] state       state type, enum comm_state
     *  @return
     *      0    : nothing
     *      other: the state is triggered
     */
    int     (*get_state)(comm_handle_t pHandle, comm_state_t state);

#define COMM_CMD_NORMAL_TX      0
#define COMM_CMD_FORCE_TX       1
    int     (*ctrl)(comm_handle_t pHandle, uint32_t cmd, void *pExtra);

} comm_dev_desc_t;

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
