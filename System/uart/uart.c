/**
 * @file    uart.c
 * @author  armfly
 * @author  Myth
 * @version 0.1
 * @date    2021.10.11
 * @brief   stm32f103x6 串口 FIFO 库
 * @note    请勿更改此文件内容
 */

#include "stdio.h"

#include "systick.h"

#include "uart.h"

COM_PORT_E printf_Com = COM1;
COM_PORT_E getchar_Com = COM1;

// USART1 PA9 PA10
#define USART1_CLK_ENABLE() __HAL_RCC_USART1_CLK_ENABLE()

#define USART1_TX_GPIO_CLK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()
#define USART1_TX_GPIO_PORT GPIOA
#define USART1_TX_PIN GPIO_PIN_9

#define USART1_RX_GPIO_CLK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()
#define USART1_RX_GPIO_PORT GPIOA
#define USART1_RX_PIN GPIO_PIN_10

// USART2 PA2 PA3
#define USART2_CLK_ENABLE() __HAL_RCC_USART2_CLK_ENABLE()

#define USART2_TX_GPIO_CLK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()
#define USART2_TX_GPIO_PORT GPIOA
#define USART2_TX_PIN GPIO_PIN_2

#define USART2_RX_GPIO_CLK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()
#define USART2_RX_GPIO_PORT GPIOA
#define USART2_RX_PIN GPIO_PIN_3

//定义每个串口结构体变量
#if UART1_FIFO_EN == 1
static UART_T g_tUART1;
static uint8_t g_TxBuf1[UART1_TX_BUF_SIZE]; //发送缓冲区
static uint8_t g_RxBuf1[UART1_RX_BUF_SIZE]; //接收缓冲区
#endif

#if UART2_FIFO_EN == 1
static UART_T g_tUART2;
static uint8_t g_TxBuf2[UART2_TX_BUF_SIZE]; //发送缓冲区
static uint8_t g_RxBuf2[UART2_RX_BUF_SIZE]; //接收缓冲区
#endif

static void UART_Var_Init(void);
static void UART_Hard_Init(void);
static void UART_T_Send(UART_T *_pUART, uint8_t *_ucaBuf, uint16_t _usLen);
static uint8_t UART_T_GetChar(UART_T *_pUART, uint8_t *_pByte);
static void UART_T_IRQ(UART_T *_pUART);

/**
 * @brief  初始化串口
 */
void UART_Init(void)
{
    UART_Var_Init(); //初始化全局变量

    UART_Hard_Init(); //配置串口的硬件参数
}

/**
 * @brief  将 COM 端口号转换为 UART 指针
 * @param  _ucPort: 端口号 (COM1-2)
 * @retval UART 指针
 */
UART_T *ComToUART(COM_PORT_E _ucPort)
{
    if (_ucPort == COM1)
    {
#if UART1_FIFO_EN == 1
        return &g_tUART1;
#else
        return 0;
#endif
    }
    else if (_ucPort == COM2)
    {
#if UART2_FIFO_EN == 1
        return &g_tUART2;
#else
        return 0;
#endif
    }
    else
    {
        return 0;
    }
}

/**
 * @brief  将 COM 端口号转换为 USART_TypeDef*
 * @param  _ucPort: 端口号 (COM1-2)
 * @retval USART_TypeDef*
 */
USART_TypeDef *ComToUSARTx(COM_PORT_E _ucPort)
{
    if (_ucPort == COM1)
    {
#if UART1_FIFO_EN == 1
        return USART1;
#else
        return 0;
#endif
    }
    else if (_ucPort == COM2)
    {
#if UART2_FIFO_EN == 1
        return USART2;
#else
        return 0;
#endif
    }
    else
    {
        return 0;
    }
}

/**
 * @brief  向串口发送一组数据。数据放到发送缓冲区后立即返回，由中断服务程序在后台完成发送
 * @param  _ucPort: 端口号 (COM1-2)
 * @param  _ucaBuf: 待发送的数据缓冲区
 * @param  _usLen : 数据长度
 */
void UART_SendBuff(COM_PORT_E _ucPort, uint8_t *_ucaBuf, uint16_t _usLen)
{
    UART_T *pUART;

    pUART = ComToUART(_ucPort);
    if (pUART == 0)
    {
        return;
    }

    UART_T_Send(pUART, _ucaBuf, _usLen);
}

/**
 * @brief  向串口发送 1 个字节。数据放到发送缓冲区后立即返回，由中断服务程序在后台完成发送
 * @param  _ucPort: 端口号 (COM1-2)
 * @param  _ucByte: 待发送的数据
 */
void UART_SendChar(COM_PORT_E _ucPort, uint8_t _ucByte)
{
    UART_SendBuff(_ucPort, &_ucByte, 1);
}

/**
 * @brief  从接收缓冲区读取 1 字节，非阻塞。无论有无数据均立即返回
 * @param  _ucPort: 端口号 (COM1-2)
 * @param  _pByte: 接收到的数据指针
 * @retval 0 表示无数据，1 表示读取到有效字节
 */
uint8_t UART_GetChar(COM_PORT_E _ucPort, uint8_t *_pByte)
{
    UART_T *pUART;

    pUART = ComToUART(_ucPort);
    if (pUART == 0)
    {
        return 0;
    }

    return UART_T_GetChar(pUART, _pByte);
}

/**
 * @brief  从接收缓冲区读取到特定字节，阻塞，有超时
 * @param  _ucPort: 端口号 (COM1-2)
 * @param  _pBuf: 接收到的数据指针
 * @param  _endByte: 终止字节
 * @param  _timeout: 超时时间
 * @retval 0 表示超时，1 表示成功
 */
uint8_t UART_GetBuffUntil(COM_PORT_E _ucPort, uint8_t *_pBuf, uint8_t _endByte, uint16_t _timeout)
{
    uint8_t ch;
    int32_t start_t = SysTick_GetRunTime();
    uint16_t i = 0;

    while (1)
    {
        if (UART_GetChar(_ucPort, &ch) == 1)
        {
            if (ch == _endByte)
            {
                _pBuf[i] = '\0';

                return 1;
            }

            _pBuf[i++] = ch;
        }
        else if (SysTick_GetRunTime() - start_t > _timeout)
        {
            return 0;
        }
    }
}

/**
 * @brief  清零串口发送缓冲区
 * @param  _ucPort: 端口号 (COM1-2)
 */
void UART_ClearTxFIFO(COM_PORT_E _ucPort)
{
    UART_T *pUART;

    pUART = ComToUART(_ucPort);
    if (pUART == 0)
    {
        return;
    }

    pUART->usTxWrite = 0;
    pUART->usTxRead = 0;
    pUART->usTxCount = 0;
}

/**
 * @brief  清零串口接收缓冲区
 * @param  _ucPort: 端口号 (COM1-2)
 */
void UART_ClearRxFIFO(COM_PORT_E _ucPort)
{
    UART_T *pUART;

    pUART = ComToUART(_ucPort);
    if (pUART == 0)
    {
        return;
    }

    pUART->usRxWrite = 0;
    pUART->usRxRead = 0;
    pUART->usRxCount = 0;
}

/**
 * @brief  设置串口的波特率。无校验，收发都使能
 * @param  _ucPort: 端口号 (COM1-2)
 * @param  _BaudRate: 波特率
 */
void UART_SetBaud(COM_PORT_E _ucPort, uint32_t _BaudRate)
{
    USART_TypeDef *USARTx;

    USARTx = ComToUSARTx(_ucPort);
    if (USARTx == 0)
    {
        return;
    }

    UART_SetUARTParam(USARTx, _BaudRate, UART_PARITY_NONE, UART_MODE_TX_RX);
}

/**
 * @brief  设置串口接受中断处理函数
 * @param  _ucPort: 端口号 (COM1-2)
 * @param  Receive: 中断处理函数
 */
void UART_BindReceiveHandle(COM_PORT_E _ucPort, UARTReceiveHandler Receive)
{
    UART_T *pUART;

    pUART = ComToUART(_ucPort);
    if (pUART == 0)
    {
        return;
    }

    pUART->Receive = Receive;
}

/**
 * @brief  初始化串口相关的变量
 */
static void UART_Var_Init(void)
{
#if UART1_FIFO_EN == 1
    g_tUART1.uart = USART1;                   //串口设备
    g_tUART1.pTxBuf = g_TxBuf1;               //发送缓冲区指针
    g_tUART1.pRxBuf = g_RxBuf1;               //接收缓冲区指针
    g_tUART1.usTxBufSize = UART1_TX_BUF_SIZE; //发送缓冲区大小
    g_tUART1.usRxBufSize = UART1_RX_BUF_SIZE; //接收缓冲区大小
    g_tUART1.usTxWrite = 0;                   //发送 FIFO 写索引
    g_tUART1.usTxRead = 0;                    //发送 FIFO 读索引
    g_tUART1.usRxWrite = 0;                   //接收 FIFO 写索引
    g_tUART1.usRxRead = 0;                    //接收 FIFO 读索引
    g_tUART1.usRxCount = 0;                   //接收到的新数据个数
    g_tUART1.usTxCount = 0;                   //待发送的数据个数
    g_tUART1.Receive = 0;                     //接收到新数据后的回调函数
    g_tUART1.Sending = 0;                     //正在发送中标志
#endif

#if UART2_FIFO_EN == 1
    g_tUART2.uart = USART2;                   //串口设备
    g_tUART2.pTxBuf = g_TxBuf2;               //发送缓冲区指针
    g_tUART2.pRxBuf = g_RxBuf2;               //接收缓冲区指针
    g_tUART2.usTxBufSize = UART2_TX_BUF_SIZE; //发送缓冲区大小
    g_tUART2.usRxBufSize = UART2_RX_BUF_SIZE; //接收缓冲区大小
    g_tUART2.usTxWrite = 0;                   //发送 FIFO 写索引
    g_tUART2.usTxRead = 0;                    //发送 FIFO 读索引
    g_tUART2.usRxWrite = 0;                   //接收 FIFO 写索引
    g_tUART2.usRxRead = 0;                    //接收 FIFO 读索引
    g_tUART2.usRxCount = 0;                   //接收到的新数据个数
    g_tUART2.usTxCount = 0;                   //待发送的数据个数
    g_tUART2.Receive = 0;                     //接收到新数据后的回调函数
    g_tUART2.Sending = 0;                     //正在发送中标志
#endif
}

/**
 * @brief  配置串口的硬件参数
 * @param  Instance: USART_TypeDef*
 * @param  BaudRate: 波特率
 * @param  Parity: 校验类型，奇校验或者偶校验
 * @param  Mode: 发送和接收模式使能
 */
void UART_SetUARTParam(USART_TypeDef *Instance, uint32_t BaudRate, uint32_t Parity, uint32_t Mode)
{
    UART_HandleTypeDef UARTHandle;

    UARTHandle.Instance = Instance;

    UARTHandle.Init.BaudRate = BaudRate;
    UARTHandle.Init.WordLength = UART_WORDLENGTH_8B;
    UARTHandle.Init.StopBits = UART_STOPBITS_1;
    UARTHandle.Init.Parity = Parity;
    UARTHandle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    UARTHandle.Init.Mode = Mode;
    UARTHandle.Init.OverSampling = UART_OVERSAMPLING_16;

    HAL_UART_Init(&UARTHandle);
}

/**
 * @brief  配置串口的硬件参数
 */
static void UART_Hard_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

#if UART1_FIFO_EN == 1
    USART1_TX_GPIO_CLK_ENABLE();
    USART1_RX_GPIO_CLK_ENABLE();

    USART1_CLK_ENABLE();

    GPIO_InitStruct.Pin = USART1_TX_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(USART1_TX_GPIO_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = USART1_RX_PIN;
    HAL_GPIO_Init(USART1_RX_GPIO_PORT, &GPIO_InitStruct);

    //配置NVIC the NVIC for UART
    HAL_NVIC_SetPriority(USART1_IRQn, 0, 1);
    HAL_NVIC_EnableIRQ(USART1_IRQn);

    //配置波特率、奇偶校验
    UART_SetUARTParam(USART1, UART1_BAUD, UART_PARITY_NONE, UART_MODE_TX_RX);

    CLEAR_BIT(USART1->SR, USART_SR_TC);     //清除 TC 发送完成标志
    CLEAR_BIT(USART1->SR, USART_SR_RXNE);   //清除 RXNE 接收标志
    SET_BIT(USART1->CR1, USART_CR1_RXNEIE); //使能 PE RX 接受中断
#endif

#if UART2_FIFO_EN == 1
    USART2_TX_GPIO_CLK_ENABLE();
    USART2_RX_GPIO_CLK_ENABLE();

    USART2_CLK_ENABLE();

    GPIO_InitStruct.Pin = USART2_TX_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(USART2_TX_GPIO_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = USART2_RX_PIN;
    HAL_GPIO_Init(USART2_RX_GPIO_PORT, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(USART2_IRQn, 0, 2);
    HAL_NVIC_EnableIRQ(USART2_IRQn);

    UART_SetUARTParam(USART2, UART2_BAUD, UART_PARITY_NONE, UART_MODE_TX_RX);

    CLEAR_BIT(USART2->SR, USART_SR_TC);     //清除 TC 发送完成标志
    CLEAR_BIT(USART2->SR, USART_SR_RXNE);   //清除 RXNE 接收标志
    SET_BIT(USART2->CR1, USART_CR1_RXNEIE); //使能 PE RX 接受中断
#endif
}

/**
 * @brief  填写数据到 UART 发送缓冲区，并启动发送中断。中断处理函数发送完毕后，自动关闭发送中断
 * @param  _pUART
 * @param  _ucaBuf
 * @param  _usLen
 */
static void UART_T_Send(UART_T *_pUART, uint8_t *_ucaBuf, uint16_t _usLen)
{
    uint16_t i;

    for (i = 0; i < _usLen; i++)
    {
        //如果发送缓冲区已经满了，则等待缓冲区空
        while (1)
        {
            __IO uint16_t usCount;

            DISABLE_INT();
            usCount = _pUART->usTxCount;
            ENABLE_INT();

            if (usCount < _pUART->usTxBufSize)
            {
                break;
            }
            else if (usCount == _pUART->usTxBufSize) //数据已填满缓冲区
            {
                if ((_pUART->uart->CR1 & USART_CR1_TXEIE) == 0)
                {
                    SET_BIT(_pUART->uart->CR1, USART_CR1_TXEIE);
                }
            }
        }

        //将新数据填入发送缓冲区
        _pUART->pTxBuf[_pUART->usTxWrite] = _ucaBuf[i];

        DISABLE_INT();
        if (++_pUART->usTxWrite >= _pUART->usTxBufSize)
        {
            _pUART->usTxWrite = 0;
        }
        _pUART->usTxCount++;
        ENABLE_INT();
    }

    SET_BIT(_pUART->uart->CR1, USART_CR1_TXEIE); //使能发送中断（缓冲区空）
}

/**
 * @brief  填写数据到 UART 发送缓冲区，并启动发送中断。中断处理函数发送完毕后，自动关闭发送中断
 * @param  _pUART
 * @param  _pByte
 * @retval 成功返回 1，失败返回 0
 */
static uint8_t UART_T_GetChar(UART_T *_pUART, uint8_t *_pByte)
{
    uint16_t usCount;

    DISABLE_INT();
    usCount = _pUART->usRxCount;
    ENABLE_INT();

    //如果读和写索引相同，则返回 0
    if (usCount == 0)
    {
        return 0;
    }
    else
    {
        *_pByte = _pUART->pRxBuf[_pUART->usRxRead]; //从串口接收 FIFO 取 1 个数据

        //改写 FIFO 读索引
        DISABLE_INT();
        if (++_pUART->usRxRead >= _pUART->usRxBufSize)
        {
            _pUART->usRxRead = 0;
        }
        _pUART->usRxCount--;
        ENABLE_INT();
        return 1;
    }
}

/**
 * @brief  判断发送缓冲区是否为空
 * @param  _ucPort
 * @retval 1 为空，0 为不空
 */
uint8_t UART_IsTxEmpty(COM_PORT_E _ucPort)
{
    UART_T *pUART;
    uint8_t Sending;

    pUART = ComToUART(_ucPort);
    if (pUART == 0)
    {
        return 0;
    }

    Sending = pUART->Sending;

    if (Sending != 0)
    {
        return 0;
    }
    return 1;
}

/**
 * @brief  供中断服务程序调用，通用串口中断处理函数
 * @param  _pUART
 */
static void UART_T_IRQ(UART_T *_pUART)
{
    uint32_t isrflags = READ_REG(_pUART->uart->SR);
    uint32_t cr1its = READ_REG(_pUART->uart->CR1);
    uint32_t cr3its = READ_REG(_pUART->uart->CR3);

    //处理接收中断
    if ((isrflags & USART_SR_RXNE) != RESET)
    {
        uint8_t ch;

        //从串口接收数据寄存器读取数据存放到接收 FIFO
        ch = READ_REG(_pUART->uart->DR);
        _pUART->pRxBuf[_pUART->usRxWrite] = ch;
        if (++_pUART->usRxWrite >= _pUART->usRxBufSize)
        {
            _pUART->usRxWrite = 0;
        }
        if (_pUART->usRxCount < _pUART->usRxBufSize)
        {
            _pUART->usRxCount++;
        }

        //回调函数
        if (_pUART->Receive)
        {
            _pUART->Receive(ch);
        }
    }

    //处理发送缓冲区空中断
    if (((isrflags & USART_SR_TXE) != RESET) && (cr1its & USART_CR1_TXEIE) != RESET)
    {
        if (_pUART->usTxCount == 0)
        {
            //发送缓冲区的数据已取完时，禁止发送缓冲区空中断（注意：此时最后1个数据还未真正发送完毕）
            CLEAR_BIT(_pUART->uart->CR1, USART_CR1_TXEIE);

            //使能数据发送完毕中断
            SET_BIT(_pUART->uart->CR1, USART_CR1_TCIE);
        }
        else
        {
            _pUART->Sending = 1;

            //从发送 FIFO 取 1 个字节写入串口发送数据寄存器
            _pUART->uart->DR = _pUART->pTxBuf[_pUART->usTxRead];
            if (++_pUART->usTxRead >= _pUART->usTxBufSize)
            {
                _pUART->usTxRead = 0;
            }
            _pUART->usTxCount--;
        }
    }
    //数据bit位全部发送完毕的中断
    if (((isrflags & USART_SR_TC) != RESET) && ((cr1its & USART_CR1_TCIE) != RESET))
    {
        if (_pUART->usTxCount == 0)
        {
            //如果发送 FIFO 的数据全部发送完毕，禁止数据发送完毕中断
            CLEAR_BIT(_pUART->uart->CR1, USART_CR1_TCIE);
            _pUART->Sending = 0;
        }
        else
        {
            //正常情况下，不会进入此分支
            //如果发送 FIFO 的数据还未完毕，则从发送 FIFO 取 1 个数据写入发送数据寄存器
            _pUART->uart->DR = _pUART->pTxBuf[_pUART->usTxRead];
            if (++_pUART->usTxRead >= _pUART->usTxBufSize)
            {
                _pUART->usTxRead = 0;
            }
            _pUART->usTxCount--;
        }
    }
}

#if UART1_FIFO_EN == 1
void USART1_IRQHandler(void)
{
    UART_T_IRQ(&g_tUART1);
}
#endif

#if UART2_FIFO_EN == 1
void USART2_IRQHandler(void)
{
    UART_T_IRQ(&g_tUART2);
}
#endif

void UART_SetprintfCom(COM_PORT_E _ucPort)
{
    printf_Com = _ucPort;
}

void UART_SetgetcharCom(COM_PORT_E _ucPort)
{
    getchar_Com = _ucPort;
}

//重定义 fputc 函数，用于 printf
int fputc(int ch, FILE *f)
{
    UART_SendChar(printf_Com, ch);

    return ch;
}

//重定义 fgetc 函数，用于 getchar
int fgetc(FILE *f)
{
    uint8_t ucData;

    while (UART_GetChar(getchar_Com, &ucData) == 0)
        ;

    return ucData;
}
