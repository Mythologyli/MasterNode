/**
 * @file    led.h
 * @author  Myth
 * @version 0.1
 * @date    2021.10.11
 * @brief   stm32f103x6 LED 库
 * @details 提供 LED 开、关、闪烁、慢速闪烁功能
 * @note    LED1: PC13
 */

#ifndef _LED_H
#define _LED_H

#include "sys.h"

#define LED1(n) (n ? HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET) : HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET))
#define LED1_On LED1(0)
#define LED1_Off LED1(1)
#define LED1_Toggle HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13) //LED1 翻转
#define LED1_SYSTICK_TIMER_ID 0
#define LED1_Slow_Toggle LED_Slow_Toggle(1) //LED1 慢速翻转，无阻塞，防止因调用过快导致无法观察

void LED_Init(void);
void LED_Slow_Toggle(uint8_t led_num);

#endif
