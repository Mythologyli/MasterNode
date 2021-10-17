/**
 * @file    esp32s.c
 * @author  Myth
 * @version 0.1
 * @date    2021.10.16
 * @brief   ESP32-S 库
 */

#include "uart.h"

#include "esp32s.h"

/**
 * @brief  初始化 ESP32-S，请在 UART_Init 后调用
 */
void ESP32S_Init(void)
{
}

/**
 * @brief  向 ESP32-S 发送数据。数据放到发送缓冲区后立即返回，由中断服务程序在后台完成发送
 * @param  pdata: 待发送的数据缓冲区
 * @param  len : 数据长度
 */
void ESP32S_Send(uint8_t *pdata, uint8_t len)
{
    UART_SendBuff(ESP32S_COM, pdata, len);
}

/**
 * @brief  从 ESP32-S 接收一字节
 * @param  pbyte: 存放接收字节位置的指针
 * @retval 0 表示无数据，1 表示读取到有效字节
 */
uint8_t ESP32S_ReceiveByte(uint8_t *pbyte)
{
    return UART_GetChar(ESP32S_COM, pbyte);
}

/**
 * @brief  清空 ESP32-S 接收缓冲区
 */
void ESP32S_ClearRx(void)
{
    UART_ClearRxFIFO(ESP32S_COM);
}

/**
 * @brief  清空 ESP32-S 发送缓冲区
 */
void ESP32S_ClearTx(void)
{
    UART_ClearTxFIFO(ESP32S_COM);
}
