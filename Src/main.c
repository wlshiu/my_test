/**
 * Copyright (c) 2023 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file main.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2023/06/21
 * @license
 * @description
 */

#include <stdio.h>
#include <string.h>
#include "main.h"

#include "overlay.h"
//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================
static bool             g_IsButtonPress = false;
static int              g_cnt = 0;

DECLARE_OVERLAY_AREA(ER_OVERLAY_A, g_ovly_a);
DECLARE_OVERLAY_AREA(ER_OVERLAY_B, g_ovly_b);



extern int overlay_a_exec(void);
extern int overlay_b_exec(void);
//=============================================================================
//                  Private Function Definition
//=============================================================================
/**
 * @brief System Clock Configuration
 * @retval None
 */
static void _SystemClock_Config(void)
{
    HAL_StatusTypeDef       rval = HAL_OK;
    RCC_OscInitTypeDef      RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef      RCC_ClkInitStruct = { .SYSCLKSource  = RCC_SYSCLKSOURCE_HIRC, };

    RCC_OscInitStruct.OscillatorType            = RCC_OSCILLATORTYPE_HIRC;
    RCC_OscInitStruct.HIRCState                 = RCC_HIRC_ON;
    RCC_OscInitStruct.HIRCCalibrationValue      = RCC_HIRCCALIBRATION_24M;
    RCC_OscInitStruct.HIRCDivider               = RCC_HIRC_DIV1;


    rval = HAL_RCC_OscConfig(&RCC_OscInitStruct);
    if( rval != HAL_OK )
    {
        Error_Handler();
    }

    /* Initializes the CPU, AHB and APB busses clocks */
    RCC_ClkInitStruct.ClockType     = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APBCLKDivider = RCC_PCLK_DIV1;

    rval = HAL_RCC_ClockConfig(&RCC_ClkInitStruct);
    if( rval != HAL_OK )
    {
        Error_Handler();
    }
    return;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
    HAL_Init();
    _SystemClock_Config();

    BSP_PB_Init(BUTTON_0_USER, BUTTON_MODE_EXTI);

    LogInit();
    printf("Overlay example\n");

    g_IsButtonPress = false;

    overlay_load(&g_ovly_a);
    g_cnt = 0;

    while(1)
    {
        if( g_IsButtonPress == true )
        {
            g_cnt = !g_cnt;
            if( g_cnt )     overlay_load(&g_ovly_b);
            else            overlay_load(&g_ovly_a);

            g_IsButtonPress = false;
            continue;
        }

        if( g_cnt )     overlay_b_exec();
        else            overlay_a_exec();

        HAL_Delay(1000);
    }
}

void HAL_GPIO_EXTI_Callback(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
    if( GPIOx == USER_BUTTON_0_GPIO_PORT &&
        (GPIO_Pin & USER_BUTTON_0_PIN) )
    {
        g_IsButtonPress = true;
    }
    return;
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
    __BKPT(1);
}

#ifdef  USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(const char *func_name, uint32_t line)
{
    /* User can add his own implementation to report the file name and line number,
        tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
}
#endif /* USE_FULL_ASSERT */


