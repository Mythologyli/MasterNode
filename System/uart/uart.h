/**
 * @file    uart.h
 * @author  armfly
 * @author  Myth
 * @version 0.1
 * @date    2021.10.11
 * @brief   stm32f103x6 串口 FIFO 库
 * @details 提供串口软件 FIFO 接口
 * @note    在此文件内通过 UARTx_FIFO_EN 宏启用、禁用特定串口
 *          在此文件内通过 UARTx_BAUD 宏设置串口初始波特率
 *          在此文件内通过 UARTx_TX_BUF_SIZE 设置串口发送 FIFO 缓冲区大小
 *          在此文件内通过 UARTx_RX_BUF_SIZE 设置串口接受 FIFO 缓冲区大小
 */

#ifndef __UART_H
#define __UART_H

//注释此处语句可禁用特定串口
#define UART1_FIFO_EN 1
#define UART2_FIFO_EN 1

//定义端口号
typedef enum
{
    COM1 = 0, //USART1
    COM2 = 1  //USART2
} COM_PORT_E;

//定义串口波特率和 FIFO 缓冲区大小，分为发送缓冲区和接收缓冲区
#if UART1_FIFO_EN == 1
#define UART1_BAUD 115200
#define UART1_TX_BUF_SIZE 1 * 512
#define UART1_RX_BUF_SIZE 1 * 512
#endif

#if UART2_FIFO_EN == 1
#define UART2_BAUD 115200
#define UART2_TX_BUF_SIZE 1 * 64
#define UART2_RX_BUF_SIZE 1 * 64
#endif

//回调函数定义
typedef void (*UARTReceiveHandler)(uint8_t _byte);

//串口设备结构体
typedef struct
{
    USART_TypeDef *uart;     //内部串口设备指针
    uint8_t *pTxBuf;         //发送缓冲区
    uint8_t *pRxBuf;         //接收缓冲区
    uint16_t usTxBufSize;    //发送缓冲区大小
    uint16_t usRxBufSize;    //接收缓冲区大小
    __IO uint16_t usTxWrite; //发送缓冲区写指针
    __IO uint16_t usTxRead;  //发送缓冲区读指针
    __IO uint16_t usTxCount; //等待发送的数据个数

    __IO uint16_t usRxWrite; //接收缓冲区写指针
    __IO uint16_t usRxRead;  //接收缓冲区读指针
    __IO uint16_t usRxCount; //还未读取的新数据个数

    UARTReceiveHandler Receive; //串口收到数据的回调函数指针

    uint8_t Sending; //正在发送中
} UART_T;

void UART_Init(void);
void UART_SendBuff(COM_PORT_E _ucPort, uint8_t *_ucaBuf, uint16_t _usLen);
void UART_SendChar(COM_PORT_E _ucPort, uint8_t _ucByte);
uint8_t UART_GetChar(COM_PORT_E _ucPort, uint8_t *_pByte);
uint8_t UART_GetBuffUntil(COM_PORT_E _ucPort, uint8_t *_pBuf, uint8_t _endByte, uint16_t _timeout);
void UART_ClearTxFIFO(COM_PORT_E _ucPort);
void UART_ClearRxFIFO(COM_PORT_E _ucPort);
void UART_SetBaud(COM_PORT_E _ucPort, uint32_t _BaudRate);
void UART_SetUARTParam(USART_TypeDef *Instance, uint32_t BaudRate, uint32_t Parity, uint32_t Mode);
uint8_t UART_IsTxEmpty(COM_PORT_E _ucPort);

void UART_BindReceiveHandle(COM_PORT_E _ucPort, UARTReceiveHandler Receive);

void UART_SetprintfCom(COM_PORT_E _ucPort);
void UART_SetgetcharCom(COM_PORT_E _ucPort);

#endif
