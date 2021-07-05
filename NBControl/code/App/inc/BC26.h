#ifndef __BC26_H
#define __BC26_H	

#include "main.h"

#define NBSend			UART3Write
#define NBCount			NBRecvCount
#define NBBuf			NBBufTmp
#define NBBuf_Size		Uart3RxBufferSize

#define PWRKEY(x) 	HAL_GPIO_WritePin(NB_PWR_GPIO_Port, NB_PWR_Pin, x)
#define NREST(x) 	HAL_GPIO_WritePin(NB_NRST_GPIO_Port, NB_NRST_Pin, x)

typedef enum{
	TCP	= 1,
	UDP	= 2,
}net_connet_TYPE;

typedef enum{
	OFFLINE	= 0,
	ONLINE	= 1,
}NET_STATUS_TYPE;

void Clear_Buffer(void);//清空缓存	
void OPEN_BC26(void);
int BC26_Init(void);
void BC26_PDPACT(void);
void BC26_ConTCP(void);
void BC26_CreateTCPSokcet(void);
void BC26_RECData(void);
void BC26_Senddata(uint8_t *len,uint8_t *data);
uint8_t BC26_Senddatahex(uint8_t *senddata,uint8_t sendlen);//发送十六进制数据
void BC26_CreateSokcet(void);
void Clear_Buffer(void);
void BC26_ChecekConStatus(void);
void aliyunMQTT_PUBdata(uint8_t temp,uint8_t humi);

typedef struct
{
    uint8_t CSQ;
    uint8_t netstatus;//网络指示灯
	uint8_t rsrq;
	uint8_t rsrp;
} BC26Status;

extern uint8_t ServerIP[4];
extern uint32_t ServerPort;

extern uint8_t BC26_CheckSelf(void);

extern void Get_or_Set_NBStatus(uint8_t get_or_set, BC26Status *nbstatus);
extern uint8_t MSG_NBReceiveDataFromISR(uint8_t *buf, uint16_t len);
extern void NBDataProcess(void);
extern void CreatNBTask(void);
extern void OPEN_BC26_Power(void);
#endif





