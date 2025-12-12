/**
 * Copyright (c) 2025 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file libbtn.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2025/12/12
 * @license
 * @description
 */


#include "libbtn.h"

//=============================================================================
//                  Constant Definition
//=============================================================================
#define CONFIG_PRESS_REPEAT_MAX_NUM     15

typedef enum btn_state
{
    BTN_STATE_IDLE = 0,     // idle state
    BTN_STATE_PRESS,        // pressed state
    BTN_STATE_RELEASE,      // released state waiting for timeout
    BTN_STATE_REPEAT,       // repeat press state
    BTN_STATE_LONG_HOLD     // long press hold state
} btn_state_t;
//=============================================================================
//                  Macro Definition
//=============================================================================
#define CB_BTN_EVENT(pBtn_key)     do{ if(pBtn_key->cb_btn_event)          \
                                         pBtn_key->cb_btn_event(pBtn_key); \
                                   }while(0)
//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================
static int      g_debounce_ticks = 3;
static int      g_tick_short_press = 300;
static int      g_tick_long_press = 1000;
//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
int btn_init(btn_handle_t *pHBtn)
{
    int     rval = 0;

    if( !pHBtn || !pHBtn->pBtn_key ||
        !pHBtn->button_num )
        return BTN_ERR_NULL_POINTER;

    for(int i = 0; i < pHBtn->button_num; i++)
    {
        pHBtn->pBtn_key[i].state     = BTN_STATE_IDLE;
        pHBtn->pBtn_key[i].btn_event = BTN_EVENT_PRESS_NONE;
        pHBtn->pBtn_key[i].repeat    = 0;
        pHBtn->pBtn_key[i].ticks     = 0u;
        pHBtn->pBtn_key[i].pin_rt_lv = !pHBtn->pBtn_key[i].pin_act_lv;
    }

    return rval;
}

int btn_deinit(btn_handle_t *pHBtn)
{
    int     rval = 0;
    if( !pHBtn || !pHBtn->pBtn_key ||
        !pHBtn->button_num )
        return BTN_ERR_NULL_POINTER;

    pHBtn->button_num = 0;
    return rval;
}

int btn_routine(btn_handle_t *pHBtn)
{
    int     rval = 0;

    for(int i = 0; i < pHBtn->button_num; i++)
    {
        btn_key_t   *pBtn_key = &pHBtn->pBtn_key[i];
        int         pin_rt_state = 0;

        pin_rt_state = pBtn_key->cb_get_pin_state(pBtn_key) ? 0x1 : 0x0;

        // Increment ticks counter when not in idle state
        if( pBtn_key->state > BTN_STATE_IDLE )
        {
            pBtn_key->ticks++;
        }

        /* Button de-bounce handling */
        if( pin_rt_state == pBtn_key->pin_rt_lv )
            pBtn_key->debounce_cnt = 0; // Level not changed, reset counter
        else
        {
            // Continue reading same new level for debounce
            if( g_debounce_ticks < pBtn_key->debounce_cnt++ )
            {
                pBtn_key->pin_rt_lv = pin_rt_state;
                pBtn_key->debounce_cnt   = 0;
            }
        }

        /* State machine */
        switch( pBtn_key->state )
        {
            default:
                // Invalid state, reset to idle
                pBtn_key->state = BTN_STATE_IDLE;
                break;

            case BTN_STATE_IDLE:
                if( pBtn_key->pin_rt_lv != pBtn_key->pin_act_lv )
                {
                    pBtn_key->btn_event = BTN_EVENT_PRESS_NONE;
                    break;
                }

                // Button press detected
                pBtn_key->btn_event = BTN_EVENT_PRESS_DOWN;

                CB_BTN_EVENT(pBtn_key);

                pBtn_key->ticks = 0;
                pBtn_key->state = BTN_STATE_PRESS;
                break;

            case BTN_STATE_PRESS:
                if( pBtn_key->pin_rt_lv != pBtn_key->pin_act_lv )
                {
                    // Button released
                    pBtn_key->btn_event = BTN_EVENT_PRESS_UP;
                    CB_BTN_EVENT(pBtn_key);

                    pBtn_key->ticks = 0;
                    pBtn_key->state = BTN_STATE_RELEASE;
                }
                else if( pBtn_key->ticks > g_tick_long_press )
                {
                    // Long press detected
                    pBtn_key->btn_event = BTN_EVENT_LONG_PRESS_START;
                    CB_BTN_EVENT(pBtn_key);

                    pBtn_key->state = BTN_STATE_LONG_HOLD;
                }
                break;

            case BTN_STATE_RELEASE:
                if( pBtn_key->pin_rt_lv == pBtn_key->pin_act_lv )
                {
                    // Button pressed again
                    pBtn_key->btn_event = BTN_EVENT_PRESS_DOWN;
                    CB_BTN_EVENT(pBtn_key);

                    if( pBtn_key->repeat < CONFIG_PRESS_REPEAT_MAX_NUM )
                    {
                        pBtn_key->repeat++;
                    }

                    pBtn_key->btn_event = BTN_EVENT_PRESS_REPEAT;
                    CB_BTN_EVENT(pBtn_key);

                    pBtn_key->ticks = 0;
                    pBtn_key->state = BTN_STATE_REPEAT;
                }
                else if( pBtn_key->ticks > g_tick_short_press )
                {
                    // Timeout reached, determine click type
                    if( pBtn_key->repeat == 1 )
                    {
                        pBtn_key->btn_event = BTN_EVENT_CLICK_SINGLE;
                        CB_BTN_EVENT(pBtn_key);
                    }
                    else if( pBtn_key->repeat == 2 )
                    {
                        pBtn_key->btn_event = BTN_EVENT_CLICK_DOUBLE;
                        CB_BTN_EVENT(pBtn_key);
                    }

                    pBtn_key->state = BTN_STATE_IDLE;
                }
                break;

            case BTN_STATE_REPEAT:
                if( pBtn_key->pin_rt_lv != pBtn_key->pin_act_lv )
                {
                    // Button released
                    pBtn_key->btn_event = BTN_EVENT_PRESS_UP;
                    CB_BTN_EVENT(pBtn_key);

                    if( pBtn_key->ticks > g_tick_short_press )
                    {
                        pBtn_key->ticks = 0;
                        pBtn_key->state = BTN_STATE_RELEASE;    // Continue waiting for more presses
                    }
                    else
                    {
                        pBtn_key->state = BTN_STATE_IDLE;       // End of sequence
                    }

                }
                else if( pBtn_key->ticks > g_tick_short_press )
                {
                    // Held down too long, treat as normal press
                    pBtn_key->state = BTN_STATE_PRESS;
                }
                break;

            case BTN_STATE_LONG_HOLD:
                if( pBtn_key->pin_rt_lv != pBtn_key->pin_act_lv )
                {
                    // Released from long press
                    pBtn_key->btn_event = BTN_EVENT_PRESS_UP;
                    CB_BTN_EVENT(pBtn_key);

                    pBtn_key->state = BTN_STATE_IDLE;
                    break;
                }

                // Continue holding
                pBtn_key->btn_event = BTN_EVENT_LONG_PRESS_HOLD;
                CB_BTN_EVENT(pBtn_key);
                break;
        }
    }

    return rval;
}

int btn_set_debounce_tick(int ticks)
{
    g_debounce_ticks = ticks;
    return 0;
}

int btn_set_short_press_tick(int ticks)
{
    g_tick_short_press = ticks;
    return 0;
}

int btn_set_long_press_tick(int ticks)
{
    g_tick_long_press = ticks;
    return 0;
}
