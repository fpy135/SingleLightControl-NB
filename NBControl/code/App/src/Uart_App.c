#include "Uart_App.h"
#include "cmsis_os.h"
#include "task.h"
#include "main.h"
#include "bsp_uart.h"
#include "stdio.h"
#include "string.h"
#include "mystring.h"
//#include "iwdg.h"

#include "IDConfig.h"
#include "Protocol.h"
#include "SysConfig.h"
#include "Led_App.h"
#include "LoraApp.h"
#include "bc26.h"

extern QueueHandle_t TCP_SEND_Queue;

BaseType_t Uart_TaskID;
xTaskHandle pvCreatedTask_Uart;


/*
	在定时器中断中调用
*/
void UartTimeOutCnt(void)
{
	#if UART1_BAUD
    if(Uart1_TimeOut < 0xF0)
    {
        if(++Uart1_TimeOut >= UART_TimeOUT_MAX)
        {
            Uart1_TimeOut = 0xFE;
        }
    }
	#endif
	
	#if UART2_BAUD
	if(Uart2_TimeOut < 0xF0)
    {
        if(++Uart2_TimeOut >= UART_TimeOUT_MAX)
        {
			Uart2_TimeOut = 0xFE;
        }
    }
	#endif
	
	#if UART3_BAUD
	if(Uart3_TimeOut < 0xF0)
    {
        if(++Uart3_TimeOut >= UART_TimeOUT_MAX)
        {
            Uart3_TimeOut = 0xFE;
			MSG_NBReceiveDataFromISR(Uart3RxBuffer, Uart3_Rx_Cnt);
			Uart3_Rx_Cnt = 0;
			Uart3_TimeOut = 0xF0;
        }
    }
	#endif
	
	#if UART8_BAUD
	if(Uart8_TimeOut < 0xF0)
    {
        if(++Uart8_TimeOut >= UART_TimeOUT_MAX)
        {
            Uart8_TimeOut = 0xFE;
			#if USE_4G_UART_TO_INTERNET
			MSG_GPRSReceiveDataFromISR(Uart8RxBuffer, Uart8_Rx_Cnt);
			#endif
			Uart8_Rx_Cnt = 0;
			Uart8_TimeOut = 0xF0;
        }
    }
	#endif
	
	#if UART5_BAUD
    if(Uart5_TimeOut < 0xF0)
    {
        if(++Uart5_TimeOut >= UART_TimeOUT_MAX)
        {
            Uart5_TimeOut = 0xFE;
        }
    }
	#endif
	#if UART7_BAUD
//	if(Uart7_TimeOut < 0xF0)
//    {
//        if(++Uart7_TimeOut >= UART_TimeOUT_MAX)
//        {
//            Uart7_TimeOut = 0xFE;
//			MSG_GPRSReceiveDataFromISR(Uart7RxBuffer, Uart7_Rx_Cnt);
//			Uart7_Rx_Cnt = 0;
//			Uart7_TimeOut = 0xF0;
//        }
//    }
	#endif
}

#if UART1_BAUD
void Uart1DataPro(uint8_t *buf, uint16_t len)
{
	uint8_t tmp[4];
	if(len>4)
		len = 4;
	memcpy(tmp,buf,len);
	if(xQueueSend(BL6523GX_RECEIVE_Queue, tmp, 0) != pdPASS)
	{
		_myprintf("\r\nxQueueSend error, BL6523GX_RECEIVE_Queue");
	}
}
#endif

#if UART2_BAUD
void Uart2DataPro(uint8_t *buf, uint16_t len)	//用户串口
{
	IDCofing(buf,  len);
	
}
#endif

#if UART3_BAUD
void Uart3DataPro(uint8_t *buf, uint16_t len)
{
	
}
#endif

#if UART5_BAUD
void Uart5DataPro(uint8_t *buf, uint16_t len)
{
	IDCofing(buf,  len);
	RemoteUpdata(buf,  len);
}
#endif
#if UART7_BAUD
void Uart7DataPro(uint8_t *buf, uint16_t len)
{
	
}
#endif

void Uart_Task(void * argument)
{
    while(1)
    {
		#if UART1_BAUD
        if(Uart1_TimeOut == 0xFE)
        {
            if(Uart1_Rx_Cnt)     //有数据
            {
                Uart1DataPro(Uart1RxBuffer, Uart1_Rx_Cnt);
                Uart1_Rx_Cnt = 0;
            }
            Uart1_TimeOut = 0xF0;
        }
		#endif
		
		#if UART2_BAUD
		if(Uart2_TimeOut == 0xFE)
        {
            if(Uart2_Rx_Cnt)     //有数据
            {
                Uart2DataPro(Uart2RxBuffer, Uart2_Rx_Cnt);
                Uart2_Rx_Cnt = 0;
            }
            Uart2_TimeOut = 0xF0;
        }
		#endif
		
		#if UART3_BAUD
		if(Uart3_TimeOut == 0xFE)
        {
            if(Uart3_Rx_Cnt)     //有数据
            {
                Uart3DataPro(Uart3RxBuffer, Uart3_Rx_Cnt);
                Uart3_Rx_Cnt = 0;
            }
            Uart3_TimeOut = 0xF0;
        }
		#endif
		
		#if UART5_BAUD
        if(Uart5_TimeOut == 0xFE)
        {
            if(Uart5_Rx_Cnt)     //有数据
            {
//				UART4_LEN = Uart5_Rx_Cnt;
                Uart5DataPro(Uart5RxBuffer, Uart5_Rx_Cnt);
                Uart5_Rx_Cnt = 0;
            }
            Uart5_TimeOut = 0xF0;
        }
		#endif
        vTaskDelay(100);
    }
}

void CreatUartTask(void)
{
	_myprintf("\r\n CreatUartTask");
    Uart_TaskID = xTaskCreate(Uart_Task, "UartApp", Uart_StkSIZE, NULL, Uart_TaskPrio, &pvCreatedTask_Uart);
    if(pdPASS != Uart_TaskID)
    {
        _myprintf("\r\nUartTask creat error");
        while(1);
    }
}

