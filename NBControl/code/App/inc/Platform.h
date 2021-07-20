#ifndef __PLATFORM_H
#define __PLATFORM_H

#include "main.h"
#include "SysConfig.h"

#define Platform_Data_Printf				1

#if USE_ETH_TO_INTERNET
#define PLATFORM_BUFF_SIZE			( TCP_BUFF_SIZE )
#endif

#if USE_4G_UART_TO_INTERNET
#define PLATFORM_BUFF_SIZE			( Uart1RxBufferSize )
#define GPRS_SendData				UART1Write
#endif

#define USE_Winext_Protocol			0
#if USE_LORA_UART_TO_INTERNET
#define PLATFORM_BUFF_SIZE			( Uart1RxBufferSize )
#define Lora_SendData				UART1Write


#if USE_Winext_Protocol
	
	#define SensorType				0x03
	
	typedef enum{
		GetDataCmd 			= 0x12,
		LedControlCmd		= 0x09,
		LedPwmCmd			= 0x0A,
		GetHeartCycleCmd	= 0x13,
		SetHeartCycleCmd	= 0x14,
		GetTimeCmd			= 0x1F,
		SetTimeCmd			= 0x20,
		SetTimeControlCmd	= 0x21,
		GetTimeControlCmd	= 0x23,
		ResetSysCmd			= 0x15,
	}WinextProtocolCmd_Type;
	
	typedef enum{
		HeartType	= 0x01,
		DataType	= 0x02,
		AlarmType	= 0x03,
	}WinextData_Type;
#endif

#endif

#if USE_NB_UART_TO_INTERNET
#define PLATFORM_BUFF_SIZE			( Uart3RxBufferSize )
#define GPRS_SendData				UART3Write
#endif

#define LORA_DEV 	00	//no use 00 use 01
#define NB_DEV 		02	//no use 00 use 02
#define GPRS4G_DEV	00	//no use 00 use 03
#define ETH_DEV 	00	//no use 00 use 04
#define ELECTRIC1_DEV 	05	//no use 00 use 05
#define ELECTRIC2_DEV 	00	//no use 00 use 06

#define DEVNUM			2	//设备总数

typedef struct AlarmData {
	unsigned char 	devNum;
#if LORA_DEV
	unsigned char  	LoraNum;
	unsigned char 	devLora;
#endif
#if NB_DEV
	unsigned char  	NBNum;
	unsigned char	devNB;
#endif
#if GPRS4G_DEV
	unsigned char  	GPRS4GNum;
	unsigned char	dev4G;
#endif
#if ETH_DEV
	unsigned char  	EthNum;
	unsigned char	devEth;
#endif
#if ELECTRIC1_DEV
	unsigned char  	Electric1Num;
	unsigned char	devElectric1;
#endif
#if ELECTRIC2_DEV
	unsigned char  	Electric2Num;
	unsigned char	devElectric2;
#endif
}AlarmData_Type;

extern AlarmData_Type alarmData;

//#define Herat_Rate					(5)		//单位：秒

extern uint8_t MSG_GPRSReceiveDataFromISR(uint8_t *buf, uint16_t len);
extern uint8_t MSG_LoraReceiveDataFromISR(uint8_t *buf, uint16_t len);
extern void Platform_SendDataByGPRS(uint8_t sn,uint16_t type, uint32_t id, uint8_t cmd,uint16_t len,uint8_t *senddata);
//extern void Platform_SendDataByLora(uint8_t sn,uint16_t type, uint32_t id, uint8_t cmd,uint16_t len,uint8_t *senddata);
extern void Platform_Data_Process(uint8_t* msgdata);
extern void CreatPlatformTask(void);
extern void Heart_Data_Send(void);
extern void Device_Data_Send(void);

#endif
