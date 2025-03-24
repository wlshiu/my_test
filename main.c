/**
 * Copyright (c) 2025 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file main.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2025/03/22
 * @license
 * @description
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "hal_sim.h"
#include "hal_tim.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
#define CONFIG_OUTPUT_DATA_ENABLE

/**
 *  The samples of a period of the Timer
 */
#define CONFIG_TIM_PERIOD           100//8000

/**
 *  Output "CONFIG_OUT_PERIOD_MAX * CONFIG_TIM_PERIOD" samples
 */
#define CONFIG_OUT_PERIOD_MAX       2


#define CONFIG_PWM_DEAD_TIME        64
//=============================================================================
//                  Macro Definition
//=============================================================================
#define __TIM_Get_Ch1_PWM_Mode(_Tim_) \
            (((_Tim_)->CCMR1_OUTPUT & TIM_CCMR1_OUTPUT_OC1M_Msk) >> TIM_CCMR1_OUTPUT_OC1M_Pos)

#define __TIM_Get_Ch2_PWM_Mode(_Tim_) \
            (((_Tim_)->CCMR1_OUTPUT & TIM_CCMR1_OUTPUT_OC2M_Msk) >> TIM_CCMR1_OUTPUT_OC2M_Pos)

#define __TIM_Get_Ch3_PWM_Mode(_Tim_) \
            (((_Tim_)->CCMR2_OUTPUT & TIM_CCMR2_OUTPUT_OC3M_Msk) >> TIM_CCMR2_OUTPUT_OC3M_Pos)

#define __Is_TIM_PWM_Ch1_Out_Enable(_Tim_) \
            (((_Tim_)->CCER & TIM_CCER_CC1E_Msk) >> TIM_CCER_CC1E_Pos)

#define __Is_TIM_PWM_Ch1N_Out_Enable(_Tim_) \
            (((_Tim_)->CCER & TIM_CCER_CC1NE_Msk) >> TIM_CCER_CC1NE_Pos)


#define __Is_TIM_PWM_Ch2_Out_Enable(_Tim_) \
            (((_Tim_)->CCER & TIM_CCER_CC2E_Msk) >> TIM_CCER_CC2E_Pos)

#define __Is_TIM_PWM_Ch2N_Out_Enable(_Tim_) \
            (((_Tim_)->CCER & TIM_CCER_CC2NE_Msk) >> TIM_CCER_CC2NE_Pos)

#define __Is_TIM_PWM_Ch3_Out_Enable(_Tim_) \
            (((_Tim_)->CCER & TIM_CCER_CC3E_Msk) >> TIM_CCER_CC3E_Pos)

#define __Is_TIM_PWM_Ch3N_Out_Enable(_Tim_) \
            (((_Tim_)->CCER & TIM_CCER_CC3NE_Msk) >> TIM_CCER_CC3NE_Pos)
//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct pwm_out
{
    int     pwm_out_ch1;
    int     pwm_out_ch1n;

    int     pwm_out_ch2;
    int     pwm_out_ch2n;

    int     pwm_out_ch3;
    int     pwm_out_ch3n;
} pwm_out_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================
int pwm_init(uint32_t carrier_freq_hz)
{
    int     rval = 0;

    do { /* PWM initialize */
        uint32_t    reload_value = 0;

        #if 1
        carrier_freq_hz = 32000;
        reload_value = SYS_HIRC_VALUE / carrier_freq_hz;
        #else
        reload_value = (SYS_HIRC_VALUE + (carrier_freq_hz >> 1)) / carrier_freq_hz;
        #endif // 1

        {   /* Time Base configuration */
            TIM_TimeBaseInitTypeDef     tim_init = {0};

            TIM_DeInit(EPWM);

            tim_init.TIM_Prescaler         = 0;
            tim_init.TIM_CounterMode       = TIM_CounterMode_Up;
//            tim_init.TIM_CounterMode       = TIM_CounterMode_CenterAligned1;
            tim_init.TIM_Period            = reload_value;
            tim_init.TIM_ClockDivision     = 0;
            tim_init.TIM_RepetitionCounter = 0;

            TIM_TimeBaseInit(EPWM, &tim_init);
        }

        {   /* TIM Channel 1, 2, 3 Configuration in PWM mode */
            TIM_OCInitTypeDef   Out_comp_init = {0};

            Out_comp_init.TIM_OCMode       = TIM_OCMode_PWM1;
            Out_comp_init.TIM_OutputState  = TIM_OutputState_Enable;
            Out_comp_init.TIM_OutputNState = TIM_OutputNState_Enable;
            Out_comp_init.TIM_Pulse        = reload_value;
            Out_comp_init.TIM_OCPolarity   = TIM_OCPolarity_High;
            Out_comp_init.TIM_OCNPolarity  = TIM_OCPolarity_Low;
            Out_comp_init.TIM_OCIdleState  = TIM_OCIdleState_Reset;
            Out_comp_init.TIM_OCNIdleState = TIM_OCIdleState_Reset;

            TIM_OC1Init(EPWM, &Out_comp_init);
            TIM_OC2Init(EPWM, &Out_comp_init);
            TIM_OC3Init(EPWM, &Out_comp_init);

            TIM_OC1PreloadConfig(EPWM, ENABLE);
            TIM_OC2PreloadConfig(EPWM, ENABLE);
            TIM_OC3PreloadConfig(EPWM, ENABLE);
        }

        {   /* Automatic Output enable, Break, dead time and lock configuration */
            TIM_BDTRInitTypeDef     bdtr_init = {0};
            bdtr_init.TIM_OSSRState       = TIM_OSSRState_Enable;
            bdtr_init.TIM_OSSIState       = TIM_OSSIState_Enable;
            bdtr_init.TIM_LOCKLevel       = TIM_LockLevel_OFF;
            bdtr_init.TIM_DeadTime        = CONFIG_PWM_DEAD_TIME;
            bdtr_init.TIM_Break           = TIM_Break_Enable;
            bdtr_init.TIM_BreakPolarity   = TIM_BreakPolarity_High;
            bdtr_init.TIM_AutomaticOutput = TIM_AutomaticOutput_Enable;

            TIM_BDTRConfig(EPWM, &bdtr_init);
        }

        TIM_CCPreloadControl(EPWM, ENABLE);

        /* EPWM counter enable */
        TIM_Cmd(EPWM, ENABLE);

        /* Main Output Enable */
        TIM_CtrlPWMOutputs(EPWM, ENABLE);
    } while(0);

    return rval;
}

void pwm_set_duty(uint16_t ccr_ch1, uint16_t ccr_ch2, uint16_t ccr_ch3)
{
    EPWM->CCR1 = ccr_ch1;
    EPWM->CCR2 = ccr_ch2;
    EPWM->CCR3 = ccr_ch3;
    return;
}

typedef enum tim_cnt_direction
{
    TIM_CNT_UP,
    TIM_CNT_DOWN,
} tim_cnt_direction_t;
//=============================================================================
//                  Public Function Definition
//=============================================================================
int main()
{
    FILE                    *fout = 0;
    int                     out_period_idx = 0;
    tim_cnt_direction_t     cnt_direction = TIM_CNT_UP;

    pwm_init(CONFIG_TIM_PERIOD);

#if defined(CONFIG_OUTPUT_DATA_ENABLE)
    if( !(fout = fopen("pwm.csv", "w")) )
    {
        printf("open 'pwm.csv' fail !\n");
        while(1);
    }

    // fprintf(fout, "pwm_out_ch1, pwm_out_ch1n, pwm_out_ch2, pwm_out_ch2n, pwm_out_ch3, pwm_out_ch3n\n");
#endif

    EPWM->CNT = 0;

    #if 1 // debug
    /**
     *  CounterMode:
     *  + TIM_CounterMode_Up
     *  + TIM_CounterMode_Down
     *  + TIM_CounterMode_CenterAligned1
     */
    REG_WRITE_MASK(EPWM->CR1, TIM_CR1_CMS_Msk, TIM_CounterMode_Up);
//    EPWM->ARR = CONFIG_TIM_PERIOD / 2;
    EPWM->ARR = CONFIG_TIM_PERIOD - 1;

    out_period_idx = 0;
    #endif // 1


    while(1)
    {
        int8_t      duty_ch1 = 50, duty_ch2 = 60, duty_ch3 = 30;
        uint16_t    ccr_ch1 = 0, ccr_ch2 = 0, ccr_ch3 = 0;

        if( out_period_idx++ == CONFIG_OUT_PERIOD_MAX )
        {
            break;
        }

        /**
         *  Duty_Cycle % = CCR / (ARR + 1) * 100%
         *  CCR = (ARR + 1) * Duty_Cycle % / 100%
         */
        ccr_ch1 = (EPWM->ARR + 1) * (float)duty_ch1/100;
        ccr_ch2 = (EPWM->ARR + 1) * (float)duty_ch2/100;
        ccr_ch3 = (EPWM->ARR + 1) * (float)duty_ch3/100;
        pwm_set_duty(ccr_ch1, ccr_ch2, ccr_ch3);

        printf("ARR= %d, CCR1= %d, CCR2= %d, CCR3= %d\n",
               EPWM->ARR, EPWM->CCR1, EPWM->CCR2, EPWM->CCR3);


        for(int t = 0; t < CONFIG_TIM_PERIOD; t++)
        {
            pwm_out_t       pwm_out = {0};

            /* PWM Ch1 output */
            if( __Is_TIM_PWM_Ch1_Out_Enable(EPWM) )
            {
                if( __TIM_Get_Ch1_PWM_Mode(EPWM) == TIM_OCMode_PWM1 )
                {
                    pwm_out.pwm_out_ch1 = (EPWM->CNT < EPWM->CCR1) ? 1 : 0;
                }
                else if( __TIM_Get_Ch1_PWM_Mode(EPWM) == TIM_OCMode_PWM2 )
                {
                    pwm_out.pwm_out_ch1 = (EPWM->CNT < EPWM->CCR1) ? 0 : 1;
                }
            }

            if( __Is_TIM_PWM_Ch1N_Out_Enable(EPWM) )
            {
                pwm_out.pwm_out_ch1n = !pwm_out.pwm_out_ch1;
            }

            /* PWM Ch2 output */
            if( __Is_TIM_PWM_Ch2_Out_Enable(EPWM) )
            {
                if( __TIM_Get_Ch2_PWM_Mode(EPWM) == TIM_OCMode_PWM1 )
                {
                    pwm_out.pwm_out_ch2 = (EPWM->CNT < EPWM->CCR2) ? 1 : 0;
                }
                else if( __TIM_Get_Ch2_PWM_Mode(EPWM) == TIM_OCMode_PWM2 )
                {
                    pwm_out.pwm_out_ch2 = (EPWM->CNT < EPWM->CCR2) ? 0 : 1;
                }
            }

            if( __Is_TIM_PWM_Ch2N_Out_Enable(EPWM) )
            {
                pwm_out.pwm_out_ch2n = !pwm_out.pwm_out_ch2;
            }

            /* PWM Ch3 output */
            if( __Is_TIM_PWM_Ch2_Out_Enable(EPWM) )
            {
                if( __TIM_Get_Ch3_PWM_Mode(EPWM) == TIM_OCMode_PWM1 )
                {
                    pwm_out.pwm_out_ch3 = (EPWM->CNT < EPWM->CCR3) ? 1 : 0;
                }
                else if( __TIM_Get_Ch3_PWM_Mode(EPWM) == TIM_OCMode_PWM2 )
                {
                    pwm_out.pwm_out_ch3 = (EPWM->CNT < EPWM->CCR3) ? 0 : 1;
                }
            }

            if( __Is_TIM_PWM_Ch3N_Out_Enable(EPWM) )
            {
                pwm_out.pwm_out_ch3n = !pwm_out.pwm_out_ch3;
            }

            #if 1
            if( fout )
            {
                fprintf(fout, "%d, %d, %d, %d, %d, %d\n",
                        pwm_out.pwm_out_ch1, pwm_out.pwm_out_ch1n,
                        pwm_out.pwm_out_ch2, pwm_out.pwm_out_ch2n,
                        pwm_out.pwm_out_ch3, pwm_out.pwm_out_ch3n);
            }
            #else
            printf("%3d-th (CNT= %d): %d, %d, %d, %d, %d, %d\n",
                    t, EPWM->CNT,
                    pwm_out.pwm_out_ch1, pwm_out.pwm_out_ch1n,
                    pwm_out.pwm_out_ch2, pwm_out.pwm_out_ch2n,
                    pwm_out.pwm_out_ch3, pwm_out.pwm_out_ch3n);
            #endif

            /**
             *  Timer CNT process
             */
            if( REG_READ_MASK(EPWM->CR1, TIM_CR1_CMS_Msk) == TIM_CounterMode_CenterAligned1 )
            {
                /* TIM_CounterMode_CenterAligned1 */
                switch( cnt_direction )
                {
                    default:
                    case TIM_CNT_UP:
                        if( EPWM->CNT == EPWM->ARR )
                        {
                            REG_SET_BITS(EPWM->SR, TIM_SR_UIF_Msk);
                            cnt_direction = TIM_CNT_DOWN;
                        }

                        EPWM->CNT++;
                        break;

                    case TIM_CNT_DOWN:
                        if( EPWM->CNT == 0 )
                        {
                            REG_SET_BITS(EPWM->SR, TIM_SR_UIF_Msk);
                            cnt_direction = TIM_CNT_UP;
                        }

                        EPWM->CNT--;
                        break;
                }
            }
            else if( REG_READ_MASK(EPWM->CR1, TIM_CR1_CMS_Msk) == TIM_CounterMode_Up )
            {
                /* TIM_CounterMode_Up */
                if( EPWM->CNT == EPWM->ARR )
                {
                    REG_SET_BITS(EPWM->SR, TIM_SR_UIF_Msk);
                    EPWM->CNT = 0;
                }
                else
                {
                    EPWM->CNT++;
                }
            }

        }

    }

    if( fout )  fclose(fout);
    return 0;
}
