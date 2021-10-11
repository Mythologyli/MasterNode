/**
 * @file    led.c
 * @author  Myth
 * @version 0.1
 * @date    2021.10.11
 * @brief   stm32f103x6 LED 库
 */

#include "sys.h"
#include "systick.h"

#include "led.h"

/**
  * @brief  LED 初始化
  */
void LED_Init(void)
{
    GPIO_InitTypeDef GPIO_Initure;
    __HAL_RCC_GPIOC_CLK_ENABLE(); //开启 PA 时钟

    GPIO_Initure.Pin = GPIO_PIN_13;            //PC13
    GPIO_Initure.Mode = GPIO_MODE_OUTPUT_PP;   //推挽输出
    GPIO_Initure.Pull = GPIO_PULLUP;           //上拉
    GPIO_Initure.Speed = GPIO_SPEED_FREQ_HIGH; //高速
    HAL_GPIO_Init(GPIOC, &GPIO_Initure);       //初始化 PC13

    LED1_Off;
    SysTick_StartTimer(LED1_SYSTICK_TIMER_ID, 50);
}

/**
  * @brief  LED1 慢速翻转，无阻塞，防止因调用过快导致无法观察
  * @param  led_num: 序号
  */
void LED_Slow_Toggle(uint8_t led_num)
{
    switch (led_num)
    {
    case 1:
    {
        if (SysTick_CheckTimer(LED1_SYSTICK_TIMER_ID))
        {
            HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
            SysTick_StartTimer(LED1_SYSTICK_TIMER_ID, 50);
        }
    }
    }
}
