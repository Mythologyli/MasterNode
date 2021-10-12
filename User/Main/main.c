/**
 * @file    main.c
 * @author  Myth
 * @version 0.2
 * @date    2021.10.12
 * @brief   工程主函数文件
 * @details 初始化及主循环
 * @note    此版本实现功能：
 *          串口回显，回显时 PC13 上的 LED 闪烁
 *          软件 SPI
 */

#include "sys.h"
#include "systick.h"
#include "uart.h"

#include "softspi.h"

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

    SoftSPI_TypeDef SoftSPI;

    SoftSPI.SCLK_GPIO = GPIOB;
    SoftSPI.SCLK_Pin = GPIO_PIN_10;
    SoftSPI.MOSI_GPIO = GPIOA;
    SoftSPI.MOSI_Pin = GPIO_PIN_5;
    SoftSPI.MISO_GPIO = GPIOA;
    SoftSPI.MISO_Pin = GPIO_PIN_4;
    SoftSPI.SS_GPIO = GPIOA;
    SoftSPI.SS_Pin = GPIO_PIN_6;
    SoftSPI.Delay_Time = 10;

    if (SoftSPI_Init(&SoftSPI) != HAL_OK) //初始化软件 SPI
    {
        Error_Handler(__FILE__, __LINE__); //错误处理
    }

    uint8_t write[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    uint8_t read[10];
    while (1)
    {
        //程序主循环
        SoftSPI_WriteReadBuff(&SoftSPI, write, read, 10);
        Delay_us(100);
    }

    return 1;
}

/**
  * @brief  串口回显函数
  * @param  byte: 本次中断接收到的字节
  */
void Echo(uint8_t byte)
{
    LED1_Slow_Toggle;
    UART_SendChar(COM1, byte);
}
