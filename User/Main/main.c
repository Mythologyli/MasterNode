/**
 * @file    main.c
 * @author  Myth
 * @version 0.1
 * @date    2021.10.11
 * @brief   工程主函数文件
 * @details 初始化及主循环
 * @note    工程模版实现的功能：串口回显，回显时 PC13 上的 LED 闪烁
 */

#include "sys.h"
#include "systick.h"
#include "uart.h"

#include "led.h"

void Echo(uint8_t byte);

int main(void)
{
    if (HAL_Init() != HAL_OK) //初始化 HAL 库
    {
        Error_Handler(__FILE__, __LINE__); //错误处理
    }
    SystemClock_Config(); //初始化系统时钟为 72MHz
    SysTick_Init();       //初始化 SysTick 和软件定时器
    UART_Init();          //初始化串口

    LED_Init(); //初始化 LED

    UART_BindReceiveHandle(COM1, Echo); //绑定 COM1 串口接收中断至 Echo 函数

    while (1)
    {
        //程序主循环
    }

    return 1;
}

/**
  * @brief  串口回显函数
  * @param  byte: 本次中断接收到的字节
  */
void Echo(uint8_t byte)
{
    LED1_Toggle;
    UART_SendChar(COM1, byte);
}
