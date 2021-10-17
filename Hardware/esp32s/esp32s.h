/**
 * @file    esp32s.h
 * @author  Myth
 * @version 0.1
 * @date    2021.10.16
 * @brief   ESP32-S 库
 */

#ifndef _ESP32S_H
#define _ESP32S_H

#include "sys.h"

#define ESP32S_COM COM2

void ESP32S_Init(void);

void ESP32S_Send(uint8_t *pdata, uint8_t len);

uint8_t ESP32S_ReceiveByte(uint8_t *pbyte);

void ESP32S_ClearRx(void);

void ESP32S_ClearTx(void);

#endif
