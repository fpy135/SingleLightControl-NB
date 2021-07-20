#ifndef __SYSCONFIG_H
#define __SYSCONFIG_H

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"


#define __no_init		__attribute__((section("NO_INIT"),zero_init))

/***************************************
软件版本号：448格式
***************************************/
#define HARDWARE_VERSION		0x1002		//v1.0.2
#define SOFTWARE_VERSION		0x1001		//v1.0.1

/***************************************
选择使用以太网作为通讯方式还是4G模块
4G模块采用luat开发不需要at指令，
直接传输数据，模块会透传
***************************************/
#define USE_ETH_TO_INTERNET				0
#define USE_4G_UART_TO_INTERNET			0
#define USE_LORA_UART_TO_INTERNET		0
#define USE_NB_UART_TO_INTERNET			1

/***************************************
选择使用十合一气象仪作为环境监测还是多个传感器拼接
***************************************/
#define USE_TEN_IN_ONE_DEVICE			1

#if	USE_TEN_IN_ONE_DEVICE == 0
	#define USE_MULTIPLE_DEVICE		1
#else
	#define USE_MULTIPLE_DEVICE		0
#endif

/***************************************
系统任务规划配置
TaskPrio 任务优先级，值越大，优先级越高  范围 0 ~ configMAX_PRIORITIES
StkSIZE	任务栈空间

**************************************/
#define Start_TaskPrio     				( tskIDLE_PRIORITY + 1 )
#define	Start_StkSIZE					( 128 )

#define Lora_TaskPrio     				( tskIDLE_PRIORITY + 5 )
#define	Lora_StkSIZE					( 300 )

#define NB_TaskPrio     				( tskIDLE_PRIORITY + 5 )
#define	NB_StkSIZE						( 300 )

#define Uart_TaskPrio     				( tskIDLE_PRIORITY + 4 )
#define	Uart_StkSIZE					( 128 )

#define Platform_TaskPrio				( tskIDLE_PRIORITY + 6 )
#define Platform_StkSIZE				( 400 )

#define Led_TaskPrio     				( tskIDLE_PRIORITY + 3 )
#define	Led_StkSIZE						( 256 )


/*************************************
队列的长度规划
*************************************/
#define LORA_RECEIVE_QUEUE_LENGTH	( 3 )
#define NB_RECEIVE_QUEUE_LENGTH		( 3 )

extern SemaphoreHandle_t PrintMutex; //myprintf
extern QueueHandle_t LedShow_Queue;	//演示模式
extern SemaphoreHandle_t TimeControlMutex; //TimeControlMutex
extern SemaphoreHandle_t ElectricDataMutex; //ElectricDataMutex

extern SemaphoreHandle_t TcpMutex; 
extern SemaphoreHandle_t Lora_CMD_Mutex;

extern SemaphoreHandle_t AlarmBinary;	//报警信号量

extern SemaphoreHandle_t Lora_CMD_Semaphore;
extern SemaphoreHandle_t Lora_ACK_Semaphore;
extern SemaphoreHandle_t Lora_STATUS_Mutex;

extern QueueHandle_t Lora_CMD_Queue;
extern QueueHandle_t Lora_RECEIVE_Queue;

extern SemaphoreHandle_t NB_CMD_Mutex;
extern SemaphoreHandle_t NB_STATUS_Mutex;
extern SemaphoreHandle_t NB_ACK_Semaphore;
extern SemaphoreHandle_t NB_CMD_Semaphore;
extern QueueHandle_t NB_CMD_Queue;
extern QueueHandle_t NB_RECEIVE_Queue;
extern QueueHandle_t NB_SEND_Queue;

extern QueueHandle_t BL6523GX_RECEIVE_Queue;

#endif
