/**
 * @file    sys.h
 * @author  Myth
 * @version 0.1
 * @date    2021.10.11
 * @brief   stm32f103x6 系统配置及常用函数
 * @details 提供系统时钟初始化函数，开关总中断宏及错误处理函数
 * @note    若需开启全局 assert_param 支持，请在 stm32f1xx_hal_conf.h 中取消对 
 *          #define USE_FULL_ASSERT    1U
 *          的注释
 *          开启后会显著增加代码体积
 */

#ifndef __SYS_H
#define __SYS_H

#include "main.h"

#define ENABLE_INT() __set_PRIMASK(0)  //开总中断
#define DISABLE_INT() __set_PRIMASK(1) //关总中断

void SystemClock_Config(void);

void Error_Handler(uint8_t *file, uint32_t line);

#endif
