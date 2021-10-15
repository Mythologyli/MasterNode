/**
 * @file    lora.h
 * @author  Myth
 * @version 0.1
 * @date    2021.10.15
 * @brief   LoRa sx1278 Library for STM32 HAL
 * @note    DO NOT USE THIS LIB IF YOU HAVE DIO0
 */

#ifndef __LORA_H
#define __LORA_H

#include "sys.h"

// SX1278 引脚设置
#define SX1278_SCLK_GPIO GPIOB
#define SX1278_SCLK_PIN GPIO_PIN_10

#define SX1278_MOSI_GPIO GPIOA
#define SX1278_MOSI_PIN GPIO_PIN_5

#define SX1278_MISO_GPIO GPIOA
#define SX1278_MISO_PIN GPIO_PIN_4

#define SX1278_SS_GPIO GPIOA
#define SX1278_SS_PIN GPIO_PIN_6

#define SX1278_RESET_GPIO GPIOA
#define SX1278_RESET_PIN GPIO_PIN_7

void LoRa_Init(void);
uint8_t LoRa_Send(uint8_t *pdata, uint8_t len);
uint8_t LoRa_Receive(uint8_t *pdata);

#endif
