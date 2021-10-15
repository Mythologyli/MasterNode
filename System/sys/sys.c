/**
 * @file    sys.c
 * @author  Myth
 * @version 0.1
 * @date    2021.10.11
 * @brief   stm32f103x6 系统配置及常用函数
 * @note    请勿更改此文件内容
 */

#include "sys.h"

/**
 * @brief  系统时钟配置
 *         System Clock source            = PLL (HSE)
 *         SYSCLK(Hz)                     = 72000000
 *         HCLK(Hz)                       = 72000000
 *         AHB Prescaler                  = 1
 *         APB1 Prescaler                 = 2
 *         APB2 Prescaler                 = 1
 *         HSE Frequency(Hz)              = 8000000
 *         HSE PREDIV1                    = 1
 *         PLLMUL                         = 9
 *         Flash Latency(WS)              = 2
 */
void SystemClock_Config(void)
{
    RCC_ClkInitTypeDef clkinitstruct = {0};
    RCC_OscInitTypeDef oscinitstruct = {0};

    /* Enable HSE Oscillator and activate PLL with HSE as source */
    oscinitstruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    oscinitstruct.HSEState = RCC_HSE_ON;
    oscinitstruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    oscinitstruct.PLL.PLLState = RCC_PLL_ON;
    oscinitstruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    oscinitstruct.PLL.PLLMUL = RCC_PLL_MUL9;
    if (HAL_RCC_OscConfig(&oscinitstruct) != HAL_OK)
    {
        Error_Handler(__FILE__, __LINE__);
    }

    /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
     clocks dividers */
    clkinitstruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
    clkinitstruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    clkinitstruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    clkinitstruct.APB2CLKDivider = RCC_HCLK_DIV1;
    clkinitstruct.APB1CLKDivider = RCC_HCLK_DIV2;
    if (HAL_RCC_ClockConfig(&clkinitstruct, FLASH_LATENCY_2) != HAL_OK)
    {
        Error_Handler(__FILE__, __LINE__);
    }
}

/**
 * @brief  禁用 JTAG，释放相应 GPIO
 */
void DisableJTAG(void)
{
    __HAL_RCC_AFIO_CLK_ENABLE();
    __HAL_AFIO_REMAP_SWJ_NOJTAG();
}

/**
 * @brief  错误处理函数。调用后通过 printf 报告错误文件、行数并进入死循环
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void Error_Handler(uint8_t *file, uint32_t line)
{
    printf("Error occurred: file %s on line %d\r\n", file, line);

    while (1)
        ;
}

#ifdef USE_FULL_ASSERT

/**
 * @brief  assert_param 依赖函数
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 */
void assert_failed(uint8_t *file, uint32_t line)
{
    printf("Wrong parameters value: file %s on line %d\r\n", file, line);

    while (1)
        ;
}

#endif
