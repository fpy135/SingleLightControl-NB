/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_B_Pin GPIO_PIN_14
#define LED_B_GPIO_Port GPIOB
#define LED_G_Pin GPIO_PIN_15
#define LED_G_GPIO_Port GPIOB
#define LED_R_Pin GPIO_PIN_8
#define LED_R_GPIO_Port GPIOA

//#define LED_Pin GPIO_PIN_4
//#define LED_GPIO_Port GPIOB

#define PWM1_Pin GPIO_PIN_7
#define PWM1_GPIO_Port GPIOB
//#define PWM2_Pin GPIO_PIN_11
//#define PWM2_GPIO_Port GPIOA

#define NB_PWR_Pin GPIO_PIN_0
#define NB_PWR_GPIO_Port GPIOB
#define NB_NRST_Pin GPIO_PIN_1
#define NB_NRST_GPIO_Port GPIOB
#define NB_RI_Pin GPIO_PIN_12
#define NB_RI_GPIO_Port GPIOB

//#define SPI2_CS_Pin GPIO_PIN_12
//#define SPI2_CS_GPIO_Port GPIOB
//#define SPI2_SCK_Pin GPIO_PIN_13
//#define SPI2_SCK_GPIO_Port GPIOB
//#define SPI2_MISO_Pin GPIO_PIN_14
//#define SPI2_MISO_GPIO_Port GPIOB
//#define SPI2_MOSI_Pin GPIO_PIN_15
//#define SPI2_MOSI_GPIO_Port GPIOB
//#define SPI2_CF_Pin GPIO_PIN_8
//#define SPI2_CF_GPIO_Port GPIOA
//#define SPI2_RST_Pin GPIO_PIN_9
//#define SPI2_RST_GPIO_Port GPIOA
//#define SPI2_IRQ_Pin GPIO_PIN_10
//#define SPI2_IRQ_GPIO_Port GPIOA

#define REL_EN_Pin GPIO_PIN_9
#define REL_EN_GPIO_Port GPIOB

#define UART1_TX_Pin GPIO_PIN_9
#define UART1_TX_GPIO_Port GPIOA
#define UART1_RX_Pin GPIO_PIN_10
#define UART1_RX_GPIO_Port GPIOA

#define UART2_TX_Pin GPIO_PIN_2
#define UART2_TX_GPIO_Port GPIOA
#define UART2_RX_Pin GPIO_PIN_3
#define UART2_RX_GPIO_Port GPIOA

#define UART3_TX_Pin GPIO_PIN_10
#define UART3_TX_GPIO_Port GPIOB
#define UART3_RX_Pin GPIO_PIN_11
#define UART3_RX_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */
#define BL_CF_Pin GPIO_PIN_4
#define BL_CF_GPIO_Port GPIOA
#define BL_NRST_Pin GPIO_PIN_5
#define BL_NRST_GPIO_Port GPIOA
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
