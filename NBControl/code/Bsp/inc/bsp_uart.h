#ifndef __BSP_UART_H
#define __BSP_UART_H

#include "main.h"


/* USER CODE BEGIN Private defines */
#define UART1_BAUD		115200		//用户串口
#define UART2_BAUD		4800		//电量计串口
#define UART3_BAUD		115200		//NB串口
#define UART4_BAUD		0
#define UART5_BAUD		0
#define UART6_BAUD		0
#define UART7_BAUD		0
#define UART8_BAUD		0

#define PrintWrite		UART1Write

#define	UART_TimeOUT_MAX   		10		//10ms超时
/* USER CODE END Private defines */


/* USER CODE BEGIN Prototypes */
extern uint8_t aRxBuffer;				//接收中断缓冲

#if UART1_BAUD
#define Uart1RxBufferSize		256		//最大接收字节数
extern UART_HandleTypeDef huart1;
extern uint8_t Uart1RxBuffer[Uart1RxBufferSize];	//接收数据
extern uint16_t Uart1_Rx_Cnt;			//接收缓冲计数
extern uint8_t Uart1_TimeOut;			//串口接受超时
extern void MX_USART1_UART_Init(void);
extern void UART1Write(uint8_t *buf, uint16_t len);
#endif

#if UART2_BAUD
#define Uart2RxBufferSize		128		//最大接收字节数
extern UART_HandleTypeDef huart2;
extern uint8_t Uart2RxBuffer[Uart2RxBufferSize];	//接收数据
extern uint16_t Uart2_Rx_Cnt;			//接收缓冲计数
extern uint8_t Uart2_TimeOut;
extern void MX_USART2_UART_Init(void);
extern void UART2Write(uint8_t *buf, uint16_t len);

#endif

#if UART3_BAUD
#define Uart3RxBufferSize		128		//最大接收字节数
extern UART_HandleTypeDef huart3;
extern uint8_t Uart3RxBuffer[Uart3RxBufferSize];	//接收数据
extern uint16_t Uart3_Rx_Cnt;			//接收缓冲计数
extern uint8_t Uart3_TimeOut;			//串口接受超时
extern void MX_USART3_UART_Init(void);
extern void UART3Write(uint8_t *buf, uint16_t len);
#endif

#if UART4_BAUD
extern UART_HandleTypeDef huart4;
extern void MX_UART4_Init(void);

#endif

#if UART5_BAUD
#define Uart5RxBufferSize		256		//最大接收字节数
extern UART_HandleTypeDef huart5;
extern uint8_t Uart5RxBuffer[Uart5RxBufferSize];	//接收数据
extern uint16_t Uart5_Rx_Cnt;			//接收缓冲计数
extern uint8_t Uart5_TimeOut;
extern void MX_UART5_Init(void);
extern void UART5Write(uint8_t *buf, uint16_t len);
#endif

#if UART7_BAUD
#define Uart7RxBufferSize		256		//最大接收字节数
extern UART_HandleTypeDef huart7;
extern uint8_t Uart7RxBuffer[Uart7RxBufferSize];	//接收数据
extern uint16_t Uart7_Rx_Cnt;			//接收缓冲计数
extern uint8_t Uart7_TimeOut;			//串口接受超时
extern void MX_UART7_Init(void);
extern void UART7Write(uint8_t *buf, uint16_t len);
#endif

#if UART8_BAUD
#define Uart8RxBufferSize		256		//最大接收字节数
extern UART_HandleTypeDef huart8;
extern uint8_t Uart8RxBuffer[Uart8RxBufferSize];	//接收数据
extern uint16_t Uart8_Rx_Cnt;			//接收缓冲计数
extern uint8_t Uart8_TimeOut;
extern void MX_UART8_Init(void);
extern void UART8Write(uint8_t *buf, uint16_t len);
#endif

extern void myprintf(char *s, ...);
extern void _myprintf(char *s, ...);
extern void myprintf_buf_unic(uint8_t *s, uint16_t len);
#endif
