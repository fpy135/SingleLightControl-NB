#ifndef __LORAAPP_H
#define	__LORAAPP_H

#include "main.h"

#define LoraSend			UART1Write
#define LoraCount			LoraRecvCount
#define LoraBuf				LoraBufTmp
#define LoraBuf_Size		Uart1RxBufferSize

#define Lora_APPEUI			"0000000000000001"

#define OTAA	0	//OTAA 入网模式
#define ABP		1	//ABP 入网模式
#define OTAAABP	2	//OTAA 转 ABP 入网模式

typedef enum {
	LoraReset		= 0x00,
	LoraP2P			= 0x01,
	LoraOffLine		= 0x02,
	LoraOnLine		= 0x03,
	LoraInitOk		= 0x10,
	LoraInitErro	= 0x11,
}LoraStatus_Type;

extern uint8_t LoraStatus;

extern uint8_t MSG_LoraReceiveDataFromISR(uint8_t *buf, uint16_t len);
extern void Get_or_Set_LoraStatus(uint8_t get_or_set, uint8_t *lorastatus);
extern uint8_t LoraSendData_ACK(uint8_t sendport,uint16_t sendlen,uint8_t *senddata);
extern uint8_t LoraSendData(uint8_t sendport,uint8_t confirm,uint16_t sendlen,uint8_t *senddata);
extern void LoraDataProcess(void);

extern void CreatLoraTask(void);

#endif
