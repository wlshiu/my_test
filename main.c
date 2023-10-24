
#include "stm32f10x_conf.h"
GPIO_InitTypeDef GPIO_InitStructure;

void delay(void)
{
    unsigned int i;
    for(i = 0; i < 0x3ffff; ++i)
    {
        asm("nop");
    }
}


int main(void)
{

    /* GPIOD Periph clock enable */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    /* Configure PD0 and PD2 in output pushpull mode */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* Set PB5  */
    GPIOB->BSRR = 0x20;

    delay();

    while (1)
    {
        /* Set PB5  */
        GPIOB->BSRR = 0x20;

        delay();

        /* Reset PB5 */
        GPIOB->BRR  = 0x20;

        delay();

    }
}
