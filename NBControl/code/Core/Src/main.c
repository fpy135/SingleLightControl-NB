/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "SysConfig.h"
#include "StorageConfig.h"
#include "bsp_uart.h"
#include "bsp_rtc.h"
#include "bsp_timer.h"
#include "bsp_wdt.h"
#include "bsp_spi.h"

#include "MyFlash.h"
#include "IDConfig.h"
#include "BL6523GX.h"
#include "Led_App.h"
#include "Uart_App.h"
#include "LoraApp.h"
#include "Platform.h"
#include "bc26.h"

#include <cm_backtrace.h>

/* USER CODE END Includes */

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
static void MX_GPIO_Init(void);
void StartDefaultTask(void const * argument);

/* USER CODE BEGIN PFP */
xTaskHandle pvCreatedTask_Start;

void QueSemMuxInit(void);
void Start_Task(void * argument);
void CreatStartTask(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

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
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_IWDG_Init();
  MX_USART1_UART_Init();	//用户串口
  MX_USART2_UART_Init();	//电量计串口
  MX_USART3_UART_Init();	//NB串口
  MX_RTC_Init();
//  MX_SPI2_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
	_myprintf("\r\nUser Uart Init success\r\n");
	MCU_RESET_STATUS();
	HAL_UART_Receive_IT(&huart1, (uint8_t *)&aRxBuffer, 1);
	HAL_UART_Receive_IT(&huart2, (uint8_t *)&aRxBuffer, 1);
	HAL_UART_Receive_IT(&huart3, (uint8_t *)&aRxBuffer, 1);
//	HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_2);//开启PWM通道2
	
	HAL_TIM_Base_Start_IT(&htim3);
//	OPEN_BC26();
//	BL6526B_Init();
	BL6523GX_Init();
	cm_backtrace_init("NBControl", "1.0.1", "1.0.1");
	QueSemMuxInit();
	CreatStartTask();
	vTaskStartScheduler();//启动调度，开始执行任务	
	while(1);

  /* USER CODE END 2 */

}
void QueSemMuxInit(void)
{
	PrintMutex = xSemaphoreCreateMutex();		//Print mutex	
	if(NULL == PrintMutex)
	{
		_myprintf("\r\n PrintMutex error\r\n");
		while(1);
	}

	TimeControlMutex = xSemaphoreCreateMutex();		//TimeControl Mutex
	if(NULL == TimeControlMutex)
	{
		_myprintf("\r\n TimeControlMutex error\r\n");
		while(1);
	}
	
	ElectricDataMutex = xSemaphoreCreateMutex();		//TimeControl Mutex
	if(NULL == ElectricDataMutex)
	{
		_myprintf("\r\n ElectricDataMutex error\r\n");
		while(1);
	}
	
	LedShow_Queue = xQueueCreate( 2, 2 );
	if(NULL == LedShow_Queue)
	{
		_myprintf("\r\n LedShow_Queue error\r\n");
		while(1);
	}
	
	NB_CMD_Semaphore = xSemaphoreCreateBinary();
	if(NULL == NB_CMD_Semaphore)
	{
		myprintf("\r\n NB_CMD_Semaphore error");
		while(1);
	}
	xSemaphoreTake(NB_CMD_Semaphore, ( TickType_t ) 0);
	
	NB_ACK_Semaphore = xSemaphoreCreateBinary();
	if(NULL == NB_ACK_Semaphore)
	{
		myprintf("\r\n NB_ACK_Semaphore error");
		while(1);
	}
	xSemaphoreTake(NB_ACK_Semaphore, ( TickType_t ) 0);
	
	NB_CMD_Mutex = xSemaphoreCreateMutex();
	if(NULL == NB_CMD_Mutex)
	{
		myprintf("\r\n NB_CMD_Mutex error");
		while(1);
	}
	
	NB_STATUS_Mutex = xSemaphoreCreateMutex();
	if(NULL == NB_STATUS_Mutex)
	{
		myprintf("\r\n NB_STATUS_Mutex error");
		while(1);
	}
//	xSemaphoreTake(NB_STATUS_Mutex, ( TickType_t ) 0);
	
	NB_CMD_Queue = xQueueCreate( NB_RECEIVE_QUEUE_LENGTH, 128 );
	if(NULL == NB_CMD_Queue)
	{
		_myprintf("\r\n NB_CMD_Queue error");
		while(1);
	}
	NB_RECEIVE_Queue = xQueueCreate( NB_RECEIVE_QUEUE_LENGTH, Uart3RxBufferSize+2 );
	if(NULL == NB_RECEIVE_Queue)
	{
		_myprintf("\r\n NB_RECEIVE_Queue error");
		while(1);
	}
	
	NB_SEND_Queue = xQueueCreate( NB_RECEIVE_QUEUE_LENGTH, Uart3RxBufferSize+2 );
	if(NULL == NB_SEND_Queue)
	{
		_myprintf("\r\n NB_SEND_Queue error");
		while(1);
	}
	
	BL6523GX_RECEIVE_Queue = xQueueCreate( NB_RECEIVE_QUEUE_LENGTH, Uart2RxBufferSize );
	if(NULL == BL6523GX_RECEIVE_Queue)
	{
		_myprintf("\r\n BL6523GX_RECEIVE_Queue error");
		while(1);
	}
}

void Start_Task(void * argument)
{
	ReadID();
	IDStatus = 1;
	taskENTER_CRITICAL();           //进入临界区
	if(IDStatus == 1)
	{
		CreatLedTask();
		CreatUartTask();
//		CreatLoraTask();
		CreatNBTask();
		CreatPlatformTask();
	}
	else if(IDStatus == 0)
	{
		CreatLedTask();
		CreatUartTask();
	}
	vTaskDelete(pvCreatedTask_Start);
    taskEXIT_CRITICAL();            //退出临界区
}

void CreatStartTask(void)
{	
	BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为pdPASS */
	//创建开始任务
    xReturn = xTaskCreate(Start_Task, "Start", Start_StkSIZE, NULL, Start_TaskPrio, &pvCreatedTask_Start);
    if(pdPASS != xReturn)
    {
        _myprintf("\r\n Start_TaskID creat error");
        while(1);
    }
}


/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE
                              |RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV2;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}


/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(REL_EN_GPIO_Port, REL_EN_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(NB_NRST_GPIO_Port, NB_NRST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(NB_PWR_GPIO_Port, NB_PWR_Pin, GPIO_PIN_RESET);
  
  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_R_GPIO_Port, LED_R_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(LED_B_GPIO_Port, LED_B_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(LED_G_GPIO_Port, LED_G_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(PWM1_GPIO_Port, PWM1_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(BL_NRST_GPIO_Port, BL_NRST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : REL_EN_Pin */
  GPIO_InitStruct.Pin = REL_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(REL_EN_GPIO_Port, &GPIO_InitStruct);
  HAL_GPIO_WritePin(REL_EN_GPIO_Port, REL_EN_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : NB_NRST_Pin */
  GPIO_InitStruct.Pin = NB_NRST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(NB_NRST_GPIO_Port, &GPIO_InitStruct);
  
  /*Configure GPIO pin : NB_PWR_Pin */
  GPIO_InitStruct.Pin = NB_PWR_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(NB_PWR_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : NB_RI_Pin */
  GPIO_InitStruct.Pin = NB_RI_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(NB_RI_GPIO_Port, &GPIO_InitStruct);
  
  /*Configure GPIO pin : LED_Pin */
  GPIO_InitStruct.Pin = LED_R_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(LED_R_GPIO_Port, &GPIO_InitStruct);
  
  /*Configure GPIO pin : LED_Pin */
  GPIO_InitStruct.Pin = LED_B_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(LED_B_GPIO_Port, &GPIO_InitStruct);
  
  /*Configure GPIO pin : LED_Pin */
  GPIO_InitStruct.Pin = LED_G_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(LED_G_GPIO_Port, &GPIO_InitStruct);
  
  /*Configure GPIO pin : PWM1_Pin|PWM2_Pin */
  GPIO_InitStruct.Pin = PWM1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(PWM1_GPIO_Port, &GPIO_InitStruct);
  
  /*Configure GPIO pin : BL_CF_Pin */
  GPIO_InitStruct.Pin = BL_CF_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(BL_CF_GPIO_Port, &GPIO_InitStruct);
  
  /*Configure GPIO pins : BL_NRST_Pin */
  GPIO_InitStruct.Pin = BL_NRST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(BL_NRST_GPIO_Port, &GPIO_InitStruct);
  
//  GPIO_InitStruct.Pin = GPIO_PIN_13;
//  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//  GPIO_InitStruct.Pull = GPIO_NOPULL;
//  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

}


/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM2 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
//void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
//{
//  /* USER CODE BEGIN Callback 0 */

//  /* USER CODE END Callback 0 */
//  if (htim->Instance == TIM2) {
//    HAL_IncTick();
//  }
//  /* USER CODE BEGIN Callback 1 */

//  /* USER CODE END Callback 1 */
//}

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
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
/*====================================================
 * 函数功能: 	空闲任务 钩函数
 * 输入参数:	无
 * 输出参数: 
 * 返    回:	无
=====================================================*/

unsigned long getRunTimeCounterValue(void)
{
	unsigned long ret;
	ret = SysRunTimeTick;
	return ret;
}
void configureTimerForRunTimeStats(void)
{
	SysRunTimeTick = 0;
}
uint8_t task_state[6][10] = {"运行","就绪","阻塞","挂起","任务删除","无效"};
void vApplicationIdleHook(void)
{
	static TickType_t osTickTemp_IdleHook = 0;
	
	WDG_Feed();
	if((xTaskGetTickCount() - osTickTemp_IdleHook) >= 1000)	//1000ms
	{
#if configUSE_TRACE_FACILITY
		uint32_t TaskRunTimeTick = 1;
		UBaseType_t   ArraySize = 0;
		uint8_t       x = 0;
		TaskStatus_t  *StatusArray;

		ArraySize = uxTaskGetNumberOfTasks(); //获取任务个数
		StatusArray = pvPortMalloc(ArraySize*sizeof(TaskStatus_t));
		
//		myprintf("\r\nosTickTemp_IdleHook 100ms %d", osTickTemp_IdleHook);
		if(StatusArray != NULL){ //内存申请成功

            ArraySize = uxTaskGetSystemState( (TaskStatus_t *)  StatusArray,
                                              (UBaseType_t   ) ArraySize,
                                              (uint32_t *    )  &TaskRunTimeTick );

            myprintf("\r\nTaskName    Priority  TaskNumber     MinStk     time    state\r\n");
            for(x = 0;x<ArraySize;x++)
			{
                myprintf("%s     \t%d       \t%d     \t%d  \t%d%%   \t%s\r\n",
				StatusArray[x].pcTaskName,
				(int)StatusArray[x].uxCurrentPriority,
				(int)StatusArray[x].xTaskNumber,
				(int)StatusArray[x].usStackHighWaterMark,
				(int)((float)StatusArray[x].ulRunTimeCounter/TaskRunTimeTick*100),
				task_state[StatusArray[x].eCurrentState]);
            }
            myprintf("\n\n");
        }
		vPortFree(StatusArray);
#endif
		osTickTemp_IdleHook = xTaskGetTickCount();
	}
}

void vApplicationStackOverflowHook( TaskHandle_t xTask,char * pcTaskName )
{
	myprintf("\r\n\t\t\t\t发生堆栈溢出:%s\t\t\t\t\r\n",pcTaskName);
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
