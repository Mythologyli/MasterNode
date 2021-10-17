/**
 * @file    main.c
 * @author  Myth
 * @version 1.0
 * @date    2021.10.17
 * @brief   工程主函数文件
 * @details 初始化及主循环
 * @note    此版本实现功能：
 *          网关（本机）通过 LoRa 模块轮流向其它 4 个节点请求数据，节点将数据返回给主节点
 *          收到有效数据后，网关通过 ESP32-S 的串口透传模式，将数据发送给 TCP 服务器
 *
 *          基本流程：
 *          发送采集指令 -> 在等待过程中将上一次采集的数据发送 -> 收到本次采集的数据
 *          -> 若上一次采集的数据仍未发送成功则重试 -> 发送下一次采集指令
 *
 *          JTAG 已禁用，请使用 SWD 调试
 */

#include "stdio.h"

#include "sys.h"
#include "systick.h"
#include "uart.h"

#include "led.h"
#include "lora.h"
#include "esp32s.h"

typedef struct
{
    uint8_t seq;
    float humi;
    float temp;
    float light;
    uint8_t end;
} DataPack; //数据包定义，由于平台相同，网关和节点通讯无需考虑对齐问题

int main(void)
{
    if (HAL_Init() != HAL_OK) //初始化 HAL 库
    {
        Error_Handler(__FILE__, __LINE__); //错误处理
    }
    SystemClock_Config(); //初始化系统时钟为 72MHz
    DisableJTAG();        //禁用 JTAG
    SysTick_Init();       //初始化 SysTick 和软件定时器
    UART_Init();          //初始化串口

    LED_Init();    //初始化 LED
    LoRa_Init();   //初始化 LoRa 模块
    ESP32S_Init(); //初始化 ESP32-S 模块

    uint8_t current_seq = '2';                  //起始时向 2 号子节点查询
    uint8_t control_bytes[3] = {'#', '2', '@'}; //查询数据序列，子节点校验此序列判断自己是否为接收方

    DataPack pack; //接收到的数据包
    uint8_t size;  //数据包大小，用以校验数据包是否正确

    int32_t time; //时间，控制时序

    uint8_t send_buff[50];    //通过 ESP32-S 上行数据缓冲区
    uint8_t send_len;         //上行数据长度
    uint8_t receive_byte;     // TCP 回传的字节
    uint8_t send_times = 0;   // TCP 发送次数
    uint8_t is_first_run = 1; //是否为初次运行，此时不存在上一个包

    while (1)
    {
        //程序主循环
        if (LoRa_Send(control_bytes, 3) == 0) //发送查询数据指令
        {
            Error_Handler(__FILE__, __LINE__); //错误处理
        }

        Delay_ms(100); //延时确保发送结束

        //正确的方式是读取 DIO0 上的发送空中断。由于实验板 DIO0 无法接线，无法判断数据是否发送完毕，因此采用延时

        time = SysTick_GetRunTime();              //获取当前时间
        ESP32S_ClearRx();                         //清接收 FIFO
        ESP32S_ClearTx();                         //清发送 FIFO
        while (SysTick_CheckRunTime(time) < 1000) // 1000ms 时间用以等待子节点返回数据
        {
            //在等待 LoRa 模块回传数据的同时，发送上一次收到的包。200ms 发送一次，无阻塞，检测是否有正确返回
            if (!is_first_run && SysTick_CheckRunTime(time) >= send_times * 200)
            {
                ESP32S_Send(send_buff, send_len); //通过 ESP32-S 发向 TCP 服务器
                send_times++;

                if (ESP32S_ReceiveByte(&receive_byte) && (receive_byte == 'O' || receive_byte == 'K'))
                    send_times = 255; //标记为 255，表示已正确返回。TCP 服务器应对符合格式的数据返回 OK

                //注意：TCP 服务端实现
                //即使收到测量错误或重复的数据，只要符合 & 格式也应返回 OK
                //网关侧并未对数据作合理性判断，如果 TCP 服务端不对不合理数据返回 OK，网关端会认为是网络问题而不断重传
            }

            size = LoRa_Receive(&pack); //由于 DIO0 无法使用，此函数不阻塞，需判断是否接收成功
            if (size < 1)               // size = 0，一般为无数据到达
            {
                continue;
            }
            else if (size != sizeof(DataPack) || pack.seq != current_seq || pack.end != '@') //判断数据包是否正确，是否来自要查询的节点
            {
                printf("Wrong Data, Size: %d\n", size);
                continue;
            }

            LED1_Toggle; //收到有效数据，LED1 翻转

            //通过 COM1 向电脑发送数据
            printf("Get data:\n");
            printf("%c&%.1f&%.1f&%.1f&\n", pack.seq, pack.humi, pack.temp, pack.light);

            if (!is_first_run && send_times != 255) //上一个包未正确返回，先将上一个包发送完成，防止丢包
            {
                while (1)
                {
                    ESP32S_Send(send_buff, send_len);                                                      //通过 ESP32-S 发向 TCP 服务器
                    if (ESP32S_ReceiveByte(&receive_byte) && (receive_byte == 'O' || receive_byte == 'K')) //检测是否有回传数据证明发送成功
                        break;

                    Delay_ms(200); //防止发送过快
                }
            }

            //生成下一个包，这个包在下次查询时发送
            send_len = sprintf(send_buff, "%c&%.1f&%.1f&%.1f&", pack.seq, pack.humi, pack.temp, pack.light) + 1; //上行数据
            is_first_run = 0;
            send_times = 0;

            break;
        }

        //向下一个节点查询，节点范围为 2 ~ 5 共 4 个节点
        current_seq++;
        if (current_seq > '5') //一轮查询完成
            current_seq = '2';

        control_bytes[1] = current_seq;
    }

    return 1;
}
