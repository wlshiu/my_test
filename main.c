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

#include <stdio.h>
#include <stdlib.h>

#include "libbtn.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
typedef enum button_id
{
    BUTTON_ID_0,
    BUTTON_ID_1,
    BUTTON_ID_ALL,
} button_id_t;
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================
static int              g_fake_pin0_state = 0;
static int              g_fake_pin1_state = 0;

btn_handle_t     g_hBtn = {0};
//=============================================================================
//                  Private Function Definition
//=============================================================================
static int _get_pin0_state(btn_key_t *pBtn_key)
{
    return g_fake_pin0_state;
}

static int _get_pin1_state(btn_key_t *pBtn_key)
{
    return g_fake_pin1_state;
}

static int _btn0_event(btn_key_t *pBtn_key)
{
    switch( pBtn_key->btn_event )
    {
        default:
        case BTN_EVENT_PRESS_NONE:
            break;

        case BTN_EVENT_PRESS_DOWN:
            printf("\n  Get down\n");
            break;
        case BTN_EVENT_PRESS_UP:
            printf("\n  Get up\n");
            break;
        case BTN_EVENT_PRESS_REPEAT:
            printf("\n  Get Press Repeat (%d)\n", pBtn_key->u.repeat);
            break;
        case BTN_EVENT_CLICK_SINGLE:
            printf("\n  Get Single Click\n");
            break;
        case BTN_EVENT_CLICK_DOUBLE:
            printf("\n  Get Double Click\n");
            break;
        case BTN_EVENT_LONG_PRESS_START:
            printf("\n  Get Long Press Start\n");
            break;
        case BTN_EVENT_LONG_PRESS_HOLD:
//            printf("  Get Long Press Hold...\n");
            printf(".");
            break;
    }
    return 0;
}

static int _btn1_event(btn_key_t *pBtn_key)
{
    return 0;
}


static void _sim_button_press(button_id_t btn_id, int press_duration_ms)
{
    printf("\n  # Simulating button %d press for %d ms...\n", (int)btn_id, press_duration_ms);

    if( btn_id == BUTTON_ID_0 )
        g_fake_pin0_state = 1;
    else if( btn_id == BUTTON_ID_1 )
        g_fake_pin1_state = 1;

    for(int i = 0; i < press_duration_ms; i++)
    {
        btn_routine(&g_hBtn);
    }


    if( btn_id == BUTTON_ID_0 )
        g_fake_pin0_state = 0;
    else if( btn_id == BUTTON_ID_1 )
        g_fake_pin1_state = 0;

    for(int i = 0; i < 320; i++)
    {
        btn_routine(&g_hBtn);
    }
    return;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
int main()
{
    btn_key_t       btn_key[BUTTON_ID_ALL] = {0};

    btn_key[BUTTON_ID_0].u.debounce_cnt  = 3;
    btn_key[BUTTON_ID_0].u.pin_act_lv    = 1;
    btn_key[BUTTON_ID_0].cb_get_pin_state = _get_pin0_state;
    btn_key[BUTTON_ID_0].cb_btn_event     = _btn0_event;

    btn_key[BUTTON_ID_1].u.debounce_cnt  = 3;
    btn_key[BUTTON_ID_1].u.pin_act_lv    = 1;
    btn_key[BUTTON_ID_1].cb_get_pin_state = _get_pin1_state;
    btn_key[BUTTON_ID_1].cb_btn_event     = _btn1_event;

    g_hBtn.button_num = 1;//sizeof(btn_key)/sizeof(btn_key[0]);
    g_hBtn.pBtn_key   = (btn_key_t*)&btn_key;

    btn_init(&g_hBtn);

    printf("--- Single Click Demo ---\n");
    _sim_button_press(BUTTON_ID_0, 100);

    printf("\n--- Double Click Demo ---\n");
    _sim_button_press(BUTTON_ID_0, 350);
    _sim_button_press(BUTTON_ID_0, 350);

    printf("\n--- Long Press Demo ---\n");
    _sim_button_press(BUTTON_ID_0, 1500); // Long press

    printf("\n--- Repeat Press Demo ---\n");
    for (int i = 0; i < 3; i++)
    {
        _sim_button_press(BUTTON_ID_0, 80);
    }

    while(1)
    {

    }

    return 0;
}
