/**
 * @file    lora.c
 * @author  Myth
 * @version 0.1
 * @date    2021.10.15
 * @brief   LoRa sx1278 Library for STM32 HAL
 */

#include "softspi.h"
#include "SX1278.h"

#include "lora.h"

SX1278_hw_t SX1278_hw;
SX1278_t SX1278;
SoftSPI_TypeDef sx1278_spi;

uint8_t is_in_rx_mode = 0;

uint8_t LoRa_EnterTxMode(uint8_t len);
uint8_t LoRa_EnterRxMode(void);

/**
 * @brief  LoRa 模块初始化
 */
void LoRa_Init(void)
{
    //初始化各个引脚
    sx1278_spi.SCLK_GPIO = SX1278_SCLK_GPIO;
    sx1278_spi.SCLK_Pin = SX1278_SCLK_PIN;

    sx1278_spi.MOSI_GPIO = SX1278_MOSI_GPIO;
    sx1278_spi.MOSI_Pin = SX1278_MOSI_PIN;

    sx1278_spi.MISO_GPIO = SX1278_MISO_GPIO;
    sx1278_spi.MISO_Pin = SX1278_MISO_PIN;

    sx1278_spi.SS_GPIO = SX1278_SS_GPIO;
    sx1278_spi.SS_Pin = SX1278_SS_PIN;

    sx1278_spi.Delay_Time = 5;

    SX1278_hw.nss.port = GPIOA;
    SX1278_hw.nss.pin = GPIO_PIN_6;
    SX1278_hw.reset.port = GPIOA;
    SX1278_hw.reset.pin = GPIO_PIN_7;
    SX1278_hw.spi = &sx1278_spi;

    SX1278.hw = &SX1278_hw;

    SoftSPI_Init(&sx1278_spi);

    switch ((uint32_t)(SX1278_RESET_GPIO))
    {
    case (uint32_t)GPIOA:
    {
        GPIO_InitTypeDef GPIO_Initure;
        __HAL_RCC_GPIOA_CLK_ENABLE();

        GPIO_Initure.Pin = SX1278_RESET_PIN;
        GPIO_Initure.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_Initure.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOA, &GPIO_Initure);
    }
    break;

    case (uint32_t)GPIOB:
    {
        GPIO_InitTypeDef GPIO_Initure;
        __HAL_RCC_GPIOB_CLK_ENABLE();

        GPIO_Initure.Pin = SX1278_RESET_PIN;
        GPIO_Initure.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_Initure.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOB, &GPIO_Initure);
    }
    break;

    case (uint32_t)GPIOC:
    {
        GPIO_InitTypeDef GPIO_Initure;
        __HAL_RCC_GPIOC_CLK_ENABLE();

        GPIO_Initure.Pin = SX1278_RESET_PIN;
        GPIO_Initure.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_Initure.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOC, &GPIO_Initure);
    }
    break;

    case (uint32_t)GPIOD:
    {
        GPIO_InitTypeDef GPIO_Initure;
        __HAL_RCC_GPIOD_CLK_ENABLE();

        GPIO_Initure.Pin = SX1278_RESET_PIN;
        GPIO_Initure.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_Initure.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOD, &GPIO_Initure);
    }
    }

    SX1278_init(
        &SX1278,
        434000000,
        SX1278_POWER_17DBM,
        SX1278_LORA_SF_7,
        SX1278_LORA_BW_125KHZ,
        SX1278_LORA_CR_4_5,
        SX1278_LORA_CRC_EN,
        255);
}

uint8_t LoRa_EnterTxMode(uint8_t len)
{
    is_in_rx_mode = 0;
    return SX1278_LoRaEntryTx(&SX1278, len, 1000);
}

uint8_t LoRa_EnterRxMode(void)
{
    if (is_in_rx_mode)
        return 1;

    if (SX1278_LoRaEntryRx(&SX1278, 255, 1000))
        is_in_rx_mode = 1;

    return is_in_rx_mode;
}

/**
 * @brief  LoRa 模块发送
 * @param  pdata
 * @param  len
 * @retval 发送成功返回 1，发送失败返回 0
 */
uint8_t LoRa_Send(uint8_t *pdata, uint8_t len)
{
    if (LoRa_EnterTxMode(len) == 0)
        return 0;

    SX1278_LoRaTxPacket(&SX1278, pdata, len, 1000); //超时时间无意义，此函数立即返回
    return 1;
}

/**
 * @brief  LoRa 模块接收
 * @param  pdata
 * @retval 接收到包的长度，0 表示未收到数据
 */
uint8_t LoRa_Receive(uint8_t *pdata)
{
    if (LoRa_EnterRxMode() == 0)
        return 0;

    uint8_t len = SX1278_LoRaRxPacket(&SX1278);

    if (len == 0)
        return 0;

    SX1278_read(&SX1278, pdata, len);
    return len;
}
