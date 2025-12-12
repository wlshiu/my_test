/**
 * Copyright (c) 2025 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file libbtn.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2025/12/12
 * @license
 * @description
 */

#ifndef __libbtn_H_wXC84cXC_lo1L_HgOg_sK3S_ul0YzntPIua0__
#define __libbtn_H_wXC84cXC_lo1L_HgOg_sK3S_ul0YzntPIua0__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
//=============================================================================
//                  Constant Definition
//=============================================================================
typedef enum btn_err
{
    BTN_ERR_OK      = 0,
    BTN_ERR_NULL_POINTER,

} btn_err_t;

typedef enum btn_event
{
    BTN_EVENT_PRESS_NONE,           // no event
    BTN_EVENT_PRESS_DOWN,           // button pressed down
    BTN_EVENT_PRESS_UP,             // button released
    BTN_EVENT_PRESS_REPEAT,         // repeated press detected
    BTN_EVENT_CLICK_SINGLE,         // single click completed
    BTN_EVENT_CLICK_DOUBLE,         // double click completed
    BTN_EVENT_LONG_PRESS_START,     // long press started
    BTN_EVENT_LONG_PRESS_HOLD,      // long press holding
} btn_event_t;


/**
 *  \brief  Get the level status of the specific I/O pin
 *
 *  \param [in] pHBtn       pointer to the instance of button-handle, @ref btn_handle_t
 *  \return
 *          0: The pin currently is low level
 *      other: The pin currently is high level
 */
struct btn_key;
typedef int (*cb_get_pin_state_t)(struct btn_key *pBtn_key);


/**
 *  \brief  Callback of button events
 *
 *  \param [in] pHBtn       pointer to the instance of button-handle, @ref btn_handle_t
 *  \return
 *          0: success
 *      other: fail
 */
typedef int (*cb_btn_event_t)(struct btn_key *pBtn_key);
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct btn_key
{
    uint16_t        ticks;      /*<! tick counter */

    union {
        uint8_t     value;
        struct {
            uint8_t     debounce_cnt : 3; /*<! de-bounce counter */
            uint8_t     pin_rt_lv    : 1; /*<! the run-time level of the specific pin  */
            uint8_t     pin_act_lv   : 1; /*<! the active level of the specific pin    */
            uint8_t     repeat       : 3;     /*<! repeat counter */
        } u;
    };

    uint8_t         state;      /*<! internal state machine */
    btn_event_t     btn_event;

    cb_get_pin_state_t      cb_get_pin_state;
    cb_btn_event_t          cb_btn_event;
} btn_key_t;

typedef struct btn_handle
{
    int         button_num;
    btn_key_t   *pBtn_key;
} btn_handle_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
int btn_init(btn_handle_t *pHBtn);

int btn_deinit(btn_handle_t *pHBtn);

int btn_routine(btn_handle_t *pHBtn);


#ifdef __cplusplus
}
#endif

#endif
