/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
#include <stdio.h>

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void proccesDmaData(uint8_t* sign, uint16_t len);
void setDutyCycle(uint8_t D);
MODE getMode();

/* Private user code ---------------------------------------------------------*/
uint8_t tx_data[128];
uint8_t counter = 0;
uint8_t mode = 0;  // 0 - auto, 1 - manual
uint8_t start = 0;
uint8_t get_pwm_value = 0;

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */

  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

  NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

  /* System interrupt init*/

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_TIM2_Init();

  USART2_RegisterCallback(proccesDmaData);
  LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH1);
  LL_TIM_EnableIT_UPDATE(TIM2);
  LL_TIM_EnableCounter(TIM2);

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  uint8_t occupied_memory = DMA_USART2_BUFFER_SIZE - LL_DMA_GetDataLength(DMA1, LL_DMA_CHANNEL_6);
	  float load = occupied_memory / (float) DMA_USART2_BUFFER_SIZE * 100;
	  char mode_string[STRING_SIZE];

	  if (mode) strcpy(mode_string, "manual");
	  else strcpy(mode_string, "auto");

	  snprintf(tx_data, sizeof(tx_data), "Buffer capacity: %d bytes, occupied memory: %d bytes, load: %.2f %%\n\rMode: %s\n\r\n\r",
			  DMA_USART2_BUFFER_SIZE, occupied_memory, load, mode_string);
	  USART2_PutBuffer(tx_data, sizeof(tx_data));

	  LL_mDelay(2000);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_0);

  if(LL_FLASH_GetLatency() != LL_FLASH_LATENCY_0)
  {
  Error_Handler();
  }
  LL_RCC_HSI_Enable();

   /* Wait till HSI is ready */
  while(LL_RCC_HSI_IsReady() != 1)
  {

  }
  LL_RCC_HSI_SetCalibTrimming(16);
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB1_DIV_1);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSI);

   /* Wait till System clock is ready */
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSI)
  {

  }
  LL_Init1msTick(8000000);
  LL_SYSTICK_SetClkSource(LL_SYSTICK_CLKSOURCE_HCLK);
  LL_SetSystemCoreClock(8000000);
}

void proccesDmaData(uint8_t* sign, uint16_t len)
{
	static char string[STRING_SIZE], pwm_string[3];
	static uint8_t it = 0, it2 = 0, pwm_value;

	for (uint8_t i = 0; i < len; i++) {
		if (*(sign+i) == '$') {
			start = 1;
		}

		if (start) {
			string[it++] = *(sign+i);

			if (strstr(string, "$auto$")) {
				mode = 0; start = 0; it = 0;
				for(uint8_t i = 0; i < STRING_SIZE; i++) string[i] = 0;
			}

			if (strstr(string, "$manual$")) {
				mode = 1; start = 0; it = 0;
				for(uint8_t i = 0; i < STRING_SIZE; i++) string[i] = 0;
			}

			if (it >= STRING_SIZE) for(uint8_t i = 0; i < STRING_SIZE; i++) string[i] = 0;

			if (mode) {
				if (get_pwm_value) {
					if (it2 >= 3) {
						for(uint8_t i = 0; i < 3; i++) pwm_string[i] = 0;
					}

					if (it2 == 2 && *(sign+i) != '$') {
						start = 0; get_pwm_value = 0; it2 = 0; it = 0;
						for(uint8_t i = 0; i < STRING_SIZE; i++) string[i] = 0;
						for(uint8_t i = 0; i < 3; i++) pwm_string[i] = 0;
					}

					if (it2 == 2 && *(sign+i) == '$') {
						sscanf(pwm_string, "%d", &pwm_value);
						start = 0; get_pwm_value = 0; it2 = 0; it = 0;
						for(uint8_t i = 0; i < STRING_SIZE; i++) string[i] = 0;
						for(uint8_t i = 0; i < 3; i++) pwm_string[i] = 0;
					}

					if ((*(sign+i) - '0') >= 0 && (*(sign+i) - '0') <= 9) pwm_string[it2++] = *(sign+i);
				}

				if (strstr(string, "$PWM")) get_pwm_value = 1;

			}
		}

		else {
			for (uint8_t i = 0; i < STRING_SIZE; i++) string[i] = 0;
			it = 0;
		}
	}

}

void setDutyCycle(uint8_t D)
{
	uint8_t pulse_length = ((TIM2->ARR) * D) / 100;
	TIM2->CCR1 = pulse_length;
}

MODE getMode()
{
	if (mode) return MANUAL;
	else return AUTO;
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(char *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
