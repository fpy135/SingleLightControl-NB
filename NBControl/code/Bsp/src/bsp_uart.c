/**
  ******************************************************************************
  * File Name          : USART.c
  * Description        : This file provides code for the configuration
  *                      of the USART instances.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "bsp_uart.h"

/* USER CODE BEGIN 0 */
#include "stdarg.h"
#include "stdio.h"
#include <string.h>
//#include "CRC16.h"
#include "Protocol.h"
#include "SysConfig.h"

SemaphoreHandle_t PrintMutex; //printf

uint8_t aRxBuffer;

#if UART1_BAUD
UART_HandleTypeDef huart1;
uint8_t Uart1RxBuffer[Uart1RxBufferSize];	//接收数据
uint16_t Uart1_Rx_Cnt = 0;					//接收缓冲计数
uint8_t Uart1_TimeOut = 0xF0;
#endif

#if UART2_BAUD
UART_HandleTypeDef huart2;
uint8_t Uart2RxBuffer[Uart2RxBufferSize];	//接收数据
uint16_t Uart2_Rx_Cnt = 0;					//接收缓冲计数
uint8_t Uart2_TimeOut = 0xF0;
#endif

#if UART3_BAUD
UART_HandleTypeDef huart3;
uint8_t Uart3RxBuffer[Uart3RxBufferSize];	//接收数据
uint16_t Uart3_Rx_Cnt = 0;					//接收缓冲计数
uint8_t Uart3_TimeOut = 0xF0;
#endif

#if UART4_BAUD
UART_HandleTypeDef huart4;
uint8_t Uart5RxBuffer[Uart5RxBufferSize];	//接收数据
uint16_t Uart5_Rx_Cnt = 0;					//接收缓冲计数
uint8_t Uart5_TimeOut = 0xF0;
#endif

#if UART5_BAUD
UART_HandleTypeDef huart5;
uint8_t Uart5RxBuffer[Uart5RxBufferSize];	//接收数据
uint16_t Uart5_Rx_Cnt = 0;					//接收缓冲计数
uint8_t Uart5_TimeOut = 0xF0;
#endif

#if UART7_BAUD
UART_HandleTypeDef huart7;
uint8_t Uart7RxBuffer[Uart7RxBufferSize];	//接收数据
uint16_t Uart7_Rx_Cnt = 0;					//接收缓冲计数
uint8_t Uart7_TimeOut = 0xF0;
#endif

#if UART8_BAUD
UART_HandleTypeDef huart8;
uint8_t Uart8RxBuffer[Uart8RxBufferSize];	//接收数据
uint16_t Uart8_Rx_Cnt = 0;					//接收缓冲计数
uint8_t Uart8_TimeOut = 0xF0;
#endif
/* USER CODE END 0 */


#if UART4_BAUD
/* UART4 init function */
void MX_UART4_Init(void)
{
  huart4.Instance = UART4;
  huart4.Init.BaudRate = UART4_BAUD;
  huart4.Init.WordLength = UART_WORDLENGTH_8B;
  huart4.Init.StopBits = UART_STOPBITS_1;
  huart4.Init.Parity = UART_PARITY_NONE;
  huart4.Init.Mode = UART_MODE_TX_RX;
  huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart4.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart4) != HAL_OK)
  {
    Error_Handler();
  }
}
#endif

#if UART5_BAUD
/* UART5 init function */
void MX_UART5_Init(void)
{
  huart5.Instance = UART5;
  huart5.Init.BaudRate = UART5_BAUD;
  huart5.Init.WordLength = UART_WORDLENGTH_8B;
  huart5.Init.StopBits = UART_STOPBITS_1;
  huart5.Init.Parity = UART_PARITY_NONE;
  huart5.Init.Mode = UART_MODE_TX_RX;
  huart5.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart5.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart5) != HAL_OK)
  {
    Error_Handler();
  }
}
#endif

#if UART7_BAUD
/* UART5 init function */
void MX_UART7_Init(void)
{
  huart7.Instance = UART7;
  huart7.Init.BaudRate = UART7_BAUD;
  huart7.Init.WordLength = UART_WORDLENGTH_8B;
  huart7.Init.StopBits = UART_STOPBITS_1;
  huart7.Init.Parity = UART_PARITY_NONE;
  huart7.Init.Mode = UART_MODE_TX_RX;
  huart7.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart7.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart7) != HAL_OK)
  {
    Error_Handler();
  }
}
#endif

#if UART8_BAUD
/* UART8 init function */
void MX_UART8_Init(void)
{
  huart8.Instance = UART8;
  huart8.Init.BaudRate = UART8_BAUD;
  huart8.Init.WordLength = UART_WORDLENGTH_8B;
  huart8.Init.StopBits = UART_STOPBITS_1;
  huart8.Init.Parity = UART_PARITY_NONE;
  huart8.Init.Mode = UART_MODE_TX_RX;
  huart8.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart8.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart8) != HAL_OK)
  {
    Error_Handler();
  }
}
#endif

#if UART1_BAUD
/* USART1 init function */
void MX_USART1_UART_Init(void)
{
  huart1.Instance = USART1;
  huart1.Init.BaudRate = UART1_BAUD;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
}
#endif

#if UART2_BAUD
/* USART2 init function */
void MX_USART2_UART_Init(void)
{
  huart2.Instance = USART2;
  huart2.Init.BaudRate = UART2_BAUD;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
}
#endif

#if UART3_BAUD
/* USART3 init function */

void MX_USART3_UART_Init(void)
{

  huart3.Instance = USART3;
  huart3.Init.BaudRate = UART3_BAUD;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }

}
#endif

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
#if UART4_BAUD
  if(uartHandle->Instance==UART4)
  {
  /* USER CODE BEGIN UART4_MspInit 0 */

  /* USER CODE END UART4_MspInit 0 */
    /* UART4 clock enable */
    __HAL_RCC_UART4_CLK_ENABLE();
  
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    /**UART4 GPIO Configuration    
    PA0/WKUP     ------> UART4_TX
    PC11     ------> UART4_RX 
    */
    GPIO_InitStruct.Pin = UART4_TX_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF8_UART4;
    HAL_GPIO_Init(UART4_TX_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = UART4_RX_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF8_UART4;
    HAL_GPIO_Init(UART4_RX_GPIO_Port, &GPIO_InitStruct);

    /* UART4 interrupt Init */
    HAL_NVIC_SetPriority(UART4_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(UART4_IRQn);
  /* USER CODE BEGIN UART4_MspInit 1 */

  /* USER CODE END UART4_MspInit 1 */
  }
#endif
#if UART5_BAUD
  if(uartHandle->Instance==UART5)
  {
  /* USER CODE BEGIN UART5_MspInit 0 */

  /* USER CODE END UART5_MspInit 0 */
    /* UART5 clock enable */
    __HAL_RCC_UART5_CLK_ENABLE();
  
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    /**UART5 GPIO Configuration    
    PC12     ------> UART5_TX
    PD2     ------> UART5_RX 
    */
    GPIO_InitStruct.Pin = UART5_TX_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF8_UART5;
    HAL_GPIO_Init(UART5_TX_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = UART5_RX_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF8_UART5;
    HAL_GPIO_Init(UART5_RX_GPIO_Port, &GPIO_InitStruct);

    /* UART5 interrupt Init */
    HAL_NVIC_SetPriority(UART5_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(UART5_IRQn);
  /* USER CODE BEGIN UART5_MspInit 1 */

  /* USER CODE END UART5_MspInit 1 */
  }
#endif
#if UART7_BAUD
  if(uartHandle->Instance==UART7)
  {
  /* USER CODE BEGIN UART7_MspInit 0 */

  /* USER CODE END UART7_MspInit 0 */
    /* UART7 clock enable */
    __HAL_RCC_UART7_CLK_ENABLE();
  
    __HAL_RCC_GPIOE_CLK_ENABLE();
    /**UART7 GPIO Configuration    
    PE7     ------> UART7_RX
    PE8     ------> UART7_TX 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_7|GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF8_UART7;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    /* UART8 interrupt Init */
    HAL_NVIC_SetPriority(UART7_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(UART7_IRQn);
  /* USER CODE BEGIN UART7_MspInit 1 */

  /* USER CODE END UART7_MspInit 1 */
  }
  #endif
#if UART8_BAUD
  if(uartHandle->Instance==UART8)
  {
  /* USER CODE BEGIN UART8_MspInit 0 */

  /* USER CODE END UART8_MspInit 0 */
    /* UART8 clock enable */
    __HAL_RCC_UART8_CLK_ENABLE();
  
    __HAL_RCC_GPIOE_CLK_ENABLE();
    /**UART8 GPIO Configuration    
    PE0     ------> UART8_RX
    PE1     ------> UART8_TX 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF8_UART8;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    /* UART8 interrupt Init */
    HAL_NVIC_SetPriority(UART8_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(UART8_IRQn);
  /* USER CODE BEGIN UART8_MspInit 1 */

  /* USER CODE END UART8_MspInit 1 */
  }
#endif
#if UART1_BAUD
  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspInit 0 */

  /* USER CODE END USART1_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_USART1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    GPIO_InitStruct.Pin = UART1_TX_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(UART1_TX_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = UART1_RX_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(UART1_RX_GPIO_Port, &GPIO_InitStruct);

    __HAL_AFIO_REMAP_USART1_DISABLE();

  /* USER CODE BEGIN USART1_MspInit 1 */
	HAL_NVIC_SetPriority(USART1_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
  /* USER CODE END USART1_MspInit 1 */
  }
#endif
#if UART2_BAUD
  if(uartHandle->Instance==USART2)
   {
  /* USER CODE BEGIN USART2_MspInit 0 */

  /* USER CODE END USART2_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_USART2_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART2 GPIO Configuration
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX
    */
    GPIO_InitStruct.Pin = UART2_TX_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(UART2_TX_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = UART2_RX_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(UART2_RX_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN USART2_MspInit 1 */
	HAL_NVIC_SetPriority(USART2_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);
  /* USER CODE END USART2_MspInit 1 */
  }
#endif
#if UART3_BAUD
  if(uartHandle->Instance==USART3)
  {
  /* USER CODE BEGIN USART3_MspInit 0 */

  /* USER CODE END USART3_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_USART3_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**USART3 GPIO Configuration
    PB10     ------> USART3_TX
    PB11     ------> USART3_RX
    */
    GPIO_InitStruct.Pin = UART3_TX_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(UART3_TX_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = UART3_RX_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(UART3_RX_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN USART3_MspInit 1 */
	HAL_NVIC_SetPriority(USART3_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USART3_IRQn);
  /* USER CODE END USART3_MspInit 1 */
  }
#endif
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{
#if UART4_BAUD
  if(uartHandle->Instance==UART4)
  {
  /* USER CODE BEGIN UART4_MspDeInit 0 */

  /* USER CODE END UART4_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_UART4_CLK_DISABLE();
  
    /**UART4 GPIO Configuration    
    PA0/WKUP     ------> UART4_TX
    PC11     ------> UART4_RX 
    */
    HAL_GPIO_DeInit(UART4_TX_GPIO_Port, UART4_TX_Pin);

    HAL_GPIO_DeInit(UART4_RX_GPIO_Port, UART4_RX_Pin);

    /* UART4 interrupt Deinit */
    HAL_NVIC_DisableIRQ(UART4_IRQn);
  /* USER CODE BEGIN UART4_MspDeInit 1 */

  /* USER CODE END UART4_MspDeInit 1 */
  }
#endif
#if UART5_BAUD
  if(uartHandle->Instance==UART5)
  {
  /* USER CODE BEGIN UART5_MspDeInit 0 */

  /* USER CODE END UART5_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_UART5_CLK_DISABLE();
  
    /**UART5 GPIO Configuration    
    PC12     ------> UART5_TX
    PD2     ------> UART5_RX 
    */
    HAL_GPIO_DeInit(UART5_TX_GPIO_Port, UART5_TX_Pin);

    HAL_GPIO_DeInit(UART5_RX_GPIO_Port, UART5_RX_Pin);

    /* UART5 interrupt Deinit */
    HAL_NVIC_DisableIRQ(UART5_IRQn);
  /* USER CODE BEGIN UART5_MspDeInit 1 */

  /* USER CODE END UART5_MspDeInit 1 */
  }
#endif
#if UART7_BAUD
  if(uartHandle->Instance==UART7)
  {
  /* USER CODE BEGIN UART7_MspDeInit 0 */

  /* USER CODE END UART7_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_UART7_CLK_DISABLE();
  
    /**UART7 GPIO Configuration    
    PE7     ------> UART7_RX
    PE8     ------> UART7_TX 
    */
    HAL_GPIO_DeInit(GPIOE, GPIO_PIN_7|GPIO_PIN_8);

    /* UART7 interrupt Deinit */
    HAL_NVIC_DisableIRQ(UART7_IRQn);
  /* USER CODE BEGIN UART7_MspDeInit 1 */

  /* USER CODE END UART7_MspDeInit 1 */
  }
#endif
#if UART8_BAUD
  if(uartHandle->Instance==UART8)
  {
  /* USER CODE BEGIN UART8_MspDeInit 0 */

  /* USER CODE END UART8_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_UART8_CLK_DISABLE();
  
    /**UART8 GPIO Configuration    
    PE0     ------> UART8_RX
    PE1     ------> UART8_TX 
    */
    HAL_GPIO_DeInit(GPIOE, GPIO_PIN_0|GPIO_PIN_1);

    /* UART8 interrupt Deinit */
    HAL_NVIC_DisableIRQ(UART8_IRQn);
  /* USER CODE BEGIN UART8_MspDeInit 1 */

  /* USER CODE END UART8_MspDeInit 1 */
  }
#endif
#if UART1_BAUD
  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspDeInit 0 */

  /* USER CODE END USART1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART1_CLK_DISABLE();

    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10                                                                                           ------> USART1_RX
    */
    HAL_GPIO_DeInit(GPIOA, UART1_TX_Pin|UART1_RX_Pin);
    /* USART1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspDeInit 1 */

  /* USER CODE END USART1_MspDeInit 1 */
  }
#endif
#if UART2_BAUD
  if(uartHandle->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspDeInit 0 */

  /* USER CODE END USART2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART2_CLK_DISABLE();
 
    /**USART2 GPIO Configuration
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX
    */
    HAL_GPIO_DeInit(GPIOA, UART2_TX_Pin|UART2_RX_Pin);

    /* USART2 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART2_IRQn);
  /* USER CODE BEGIN USART2_MspDeInit 1 */

  /* USER CODE END USART2_MspDeInit 1 */
  }
#endif
#if UART3_BAUD
  if(uartHandle->Instance==USART3)
  {
  /* USER CODE BEGIN USART3_MspDeInit 0 */

  /* USER CODE END USART3_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART3_CLK_DISABLE();

    /**USART3 GPIO Configuration
    PB10     ------> USART3_TX
    PB11     ------> USART3_RX
    */
    HAL_GPIO_DeInit(GPIOB, UART3_TX_Pin|UART3_RX_Pin);

    /* USART3 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART3_IRQn);
  /* USER CODE BEGIN USART3_MspDeInit 1 */

  /* USER CODE END USART3_MspDeInit 1 */
  }
#endif
} 

/* USER CODE BEGIN 1 */

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    UNUSED(huart);
	#if UART1_BAUD
    if(huart->Instance == USART1)	// 判断是由哪个串口触发的中断
    {
        if(Uart1_TimeOut != 0xFE)
        {
            Uart1_TimeOut = 0;
            if(Uart1_Rx_Cnt >= Uart1RxBufferSize)  //溢出判断
            {
                Uart1_Rx_Cnt = 0;
                memset(Uart1RxBuffer,0x00,sizeof(Uart1RxBuffer));
                _myprintf("数据溢出");
            }
            else
            {
                Uart1RxBuffer[Uart1_Rx_Cnt] = aRxBuffer;   //接收数据转存
                Uart1_Rx_Cnt++;
			}
		}
		HAL_UART_Receive_IT(&huart1, (uint8_t *)&aRxBuffer, 1);   //再开启接收中断
	}
	#endif
	
	#if UART2_BAUD
	if(huart->Instance == USART2)	// 判断是由哪个串口触发的中断
    {
        if(Uart2_TimeOut != 0xFE)
        {
            Uart2_TimeOut = 0;
            if(Uart2_Rx_Cnt >= Uart2RxBufferSize)  //溢出判断
            {
                Uart2_Rx_Cnt = 0;
                memset(Uart2RxBuffer,0x00,sizeof(Uart2RxBuffer));
                _myprintf("数据溢出");
            }
            else
            {
                Uart2RxBuffer[Uart2_Rx_Cnt] = aRxBuffer;   //接收数据转存
                Uart2_Rx_Cnt++;
			}
		}
		HAL_UART_Receive_IT(&huart2, (uint8_t *)&aRxBuffer, 1);   //再开启接收中断
	}
	#endif
	
	#if UART3_BAUD
	else if(huart->Instance == USART3)
	{
		if(Uart3_TimeOut != 0xFE)
		{
			Uart3_TimeOut = 0;
			if(Uart3_Rx_Cnt >= Uart3RxBufferSize)  //溢出判断
			{
				Uart3_Rx_Cnt = 0;
				memset(Uart3RxBuffer,0x00,sizeof(Uart3RxBuffer));
				_myprintf("数据溢出");
			}
			else
			{
				Uart3RxBuffer[Uart3_Rx_Cnt] = aRxBuffer;   //接收数据转存
				Uart3_Rx_Cnt++;
			}
		}
		HAL_UART_Receive_IT(&huart3, (uint8_t *)&aRxBuffer, 1);   //再开启接收中断
	}
	#endif
	
	#if UART5_BAUD
	else if(huart->Instance == UART5)
	{
		if(Uart5_TimeOut != 0xFE)
		{
			Uart5_TimeOut = 0;
			if(Uart5_Rx_Cnt >= Uart5RxBufferSize)  //溢出判断
			{
				Uart5_Rx_Cnt = 0;
				memset(Uart5RxBuffer,0x00,sizeof(Uart5RxBuffer));
				_myprintf("数据溢出");
			}
			else
			{
				Uart5RxBuffer[Uart5_Rx_Cnt] = aRxBuffer;   //接收数据转存
				Uart5_Rx_Cnt++;
			}
		}
		HAL_UART_Receive_IT(&huart5, (uint8_t *)&aRxBuffer, 1);   //再开启接收中断
	}
	#endif
	
	#if UART7_BAUD
	else if(huart->Instance == UART7)
	{
		if(Uart7_TimeOut != 0xFE)
		{
			Uart7_TimeOut = 0;
			if(Uart7_Rx_Cnt >= Uart7RxBufferSize)  //溢出判断
			{
				Uart7_Rx_Cnt = 0;
				memset(Uart7RxBuffer,0x00,sizeof(Uart7RxBuffer));
				_myprintf("数据溢出");
			}
			else
			{
				Uart7RxBuffer[Uart7_Rx_Cnt] = aRxBuffer;   //接收数据转存
				Uart7_Rx_Cnt++;
			}
		}
		HAL_UART_Receive_IT(&huart7, (uint8_t *)&aRxBuffer, 1);   //再开启接收中断
	}
	#endif
	
	#if UART8_BAUD
	else if(huart->Instance == UART8)
	{
		if(Uart8_TimeOut != 0xFE)
		{
			Uart8_TimeOut = 0;
			if(Uart8_Rx_Cnt >= Uart8RxBufferSize)  //溢出判断
			{
				Uart8_Rx_Cnt = 0;
				memset(Uart8RxBuffer,0x00,sizeof(Uart8RxBuffer));
				_myprintf("数据溢出");
			}
			else
			{
				Uart8RxBuffer[Uart8_Rx_Cnt] = aRxBuffer;   //接收数据转存
				Uart8_Rx_Cnt++;
			}
		}
		HAL_UART_Receive_IT(&huart8, (uint8_t *)&aRxBuffer, 1);   //再开启接收中断
	}
	#endif
}

/*====================================================
 * 函数功能: 	标准打印 互斥保护
 * 输入参数:	char *s, ...
 * 输出参数: 
 * 返    回:	无
=====================================================*/
static char buffer[1000];
void myprintf(char *s, ...) 
{	
	va_list argptr;	

	xSemaphoreTake (PrintMutex, portMAX_DELAY);
	
	va_start(argptr, s);
	vsprintf(buffer,s, argptr);
	va_end(argptr);
    
	PrintWrite((uint8_t *)buffer, strlen(buffer));
	
	xSemaphoreGive (PrintMutex);
}

void _myprintf(char *s, ...)
{
    va_list argptr;

    va_start(argptr, s);
    vsprintf(buffer,s, argptr);
    va_end(argptr);

	PrintWrite((uint8_t *)buffer, strlen(buffer));
}

/*====================================================
 * 函数功能: 	打印字符串HEX 互斥保护
 * 输入参数:	u8 *s, u16 len
 * 输出参数: 
 * 返    回:	无
=====================================================*/
void myprintf_buf_unic(uint8_t *s, uint16_t len) 
{
	uint16_t i=0;
	
	xSemaphoreTake (PrintMutex, portMAX_DELAY);
	
//	_myprintf("\r\n---------HEX------------\r\n");
	for(i=0; i<len; i++)
	{
		_myprintf("%02X ", s[i]);
	}
//	_myprintf("\r\n------------------------\r\n");
	xSemaphoreGive (PrintMutex);
}

#if UART1_BAUD
void UART1Write(uint8_t *buf, uint16_t len)
{
    HAL_UART_Transmit(&huart1, buf, len,1000);
    return;
}
#endif

#if UART2_BAUD
void UART2Write(uint8_t *buf, uint16_t len)
{
    HAL_UART_Transmit(&huart2, buf, len,1000);
    return;
}
#endif

#if UART3_BAUD
void UART3Write(uint8_t *buf, uint16_t len)
{
     HAL_UART_Transmit(&huart3, buf, len,1000);
    return;
}
#endif

#if UART5_BAUD
void UART5Write(uint8_t *buf, uint16_t len)
{
    HAL_UART_Transmit(&huart5, buf, len,1000);
    return;
}
#endif

#if UART7_BAUD
void UART7Write(uint8_t *buf, uint16_t len)
{
    HAL_UART_Transmit(&huart7, buf, len,1000);
    return;
}
#endif

#if UART8_BAUD
void UART8Write(uint8_t *buf, uint16_t len)
{
    HAL_UART_Transmit(&huart8, buf, len,1000);
    return;
}
#endif


///重定向c库函数printf到串口DEBUG_USART，重定向后可使用printf函数
int fputc(int ch, FILE *f)
{
	/* 发送一个字节数据到串口DEBUG_USART */
//	HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 1000);	
	PrintWrite((uint8_t *)&ch, 1);
	return (ch);
}
/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
