#include "LoraApp.h"
#include "SysConfig.h"
#include "mystring.h"
#include "bsp_uart.h"
#include "Platform.h"
#include "Unixtimer.h"
#include "bsp_rtc.h"

SemaphoreHandle_t Lora_CMD_Mutex;
SemaphoreHandle_t Lora_STATUS_Mutex;
SemaphoreHandle_t Lora_ACK_Semaphore;
SemaphoreHandle_t Lora_CMD_Semaphore;
QueueHandle_t Lora_CMD_Queue;
QueueHandle_t Lora_RECEIVE_Queue;

uint8_t LoraStatus = LoraReset;

uint8_t LoraCmdBuf[LoraBuf_Size];

uint8_t LoraBufTmp[LoraBuf_Size];
uint16_t LoraRecvCount = 0;

const uint8_t memzero[16] = {0};
uint8_t DEVEUI[16];
uint8_t APPEUI[16];

uint8_t LoraActiveMode = 0;

int LoraRssi = 0;	//信号强度
int LoraSNR = 0;	//信噪比

extern uint8_t platform_recvbuf[PLATFORM_BUFF_SIZE];

extern void SetRTC(rtc_time_t *rtc_time);
extern void GetRTC(rtc_time_t *rtc_time);

/*=====================================================
 * 函数功能: 	读写lora 状态
 * 输入参数:	get_or_set： 0：读取   1：写入   
 * 输出参数: 
 * 返    回:	
=====================================================*/
void Get_or_Set_LoraStatus(uint8_t get_or_set, uint8_t *lorastatus)
{
	xSemaphoreTake(Lora_STATUS_Mutex,portMAX_DELAY);
	if(get_or_set == 0)
	{
		*lorastatus = LoraStatus;
	}
	else
	{
		LoraStatus = *lorastatus;
	}
	xSemaphoreGive(Lora_STATUS_Mutex);
}

/*=====================================================
 * 函数功能: 	GPRS接收数据队列发送 中断调用
 * 输入参数:	长度 + 内容
 * 输出参数: 
 * 返    回:	1 失败, 0 成功
=====================================================*/
uint8_t MSG_LoraReceiveDataFromISR(uint8_t *buf, uint16_t len)
{
	uint8_t Data[len+2];
	//申请缓存

	//复制消息
	Data[0] = len;
	Data[1] = len>>8;
	memcpy((void *)&Data[2],(const void *)buf,len);
	//发送消息

	if(xQueueSendToBackFromISR(Lora_RECEIVE_Queue, Data, 0) != pdPASS)
	{
		_myprintf("\r\nxQueueSend error, GPRS_RECEIVE_Queue");
		return 1;
	}

	return 0;
}

/*=====================================================
 * 函数功能: 	Lora发送命令
 * 输入参数:	const char *cmd, u16 len, char *str, u16 Wtime
 * 输出参数: 
 * 返    回:	1 失败, 0 成功
=====================================================*/
uint8_t LoraSendCMD(const char *cmd, uint16_t len, char *str, uint16_t Wtime)
{
	uint8_t *p;
	uint8_t ret = 0;
	BaseType_t res;
	
	xSemaphoreTake(Lora_CMD_Mutex,portMAX_DELAY);
	p = pvPortMalloc(sizeof(uint8_t)*128);
	xSemaphoreTake(Lora_CMD_Semaphore, ( TickType_t ) 0);	//获取信号量
	xQueueReceive( Lora_CMD_Queue, (void *)p, ( TickType_t ) 0);	//获取消息队列 清空队列
	vPortFree(p);
//	myprintf("\r\n LoraSendCMD\r\n");
//	PrintWrite((uint8_t *)cmd,len);
	LoraSend((uint8_t *)cmd,len);
	if(xQueueSend(Lora_CMD_Queue, (void *)str, 0) != pdPASS)		//发送需要检索的内容消息队列
	{
		myprintf("\r\nx QueueSend error, Lora_CMD_Queue");
		ret = 1;
	}
	res = xSemaphoreTake(Lora_CMD_Semaphore, ( TickType_t )(100 * Wtime));
	if(res == pdTRUE)	//等待检索结果
	{
//		myprintf("\r\n Lora_CMD_Semaphore OK, %X", res);
		ret = 0;
	}
	else
	{
//		myprintf("\r\n Lora_CMD_Semaphore NOK, %X", res);
		ret = 1;
	}
	xSemaphoreGive(Lora_CMD_Mutex);
	return ret;
}

/*=====================================================
 * 函数功能: 	获取Lora网段
 * 输入参数:	
 * 输出参数: 
 * 返    回:	0 失败
=====================================================*/
uint16_t GetRegion(uint8_t *data,uint16_t len)
{
	uint16_t region = 0;
	uint16_t p;
	p = GetNStr(data, len, ':', 1);
	if(p != 0xffff){
		if(Str2Num(&data[p+1], 4, (uint32_t *)&region) != 0){
			return region;
		}
	}
	return 0;
}

/*=====================================================
 * 函数功能: 	获取Lora入网模式
 * 输入参数:	
 * 输出参数: 
 * 返    回:	0xff 失败
=====================================================*/
uint8_t GetActiveMode(uint8_t *data,uint16_t len)
{
	uint8_t mode = 0;
	uint16_t p;
	p = GetNStr(data, len, ':', 1);
	if(p != 0xffff){
		if(Str2Num(&data[p+1], 4, (uint32_t *)&mode) != 0){
			return mode;
		}
	}
	return 0xff;
}

/*=====================================================
 * 函数功能: 	获取EUI
 * 输入参数:	char *buf
 * 输出参数: 	char *str
 * 返    回:	
=====================================================*/
void GetEUI(uint8_t *buf,uint8_t len, uint8_t *str)
{
	uint16_t p;
	p = StrFindString(buf, len, (uint8_t *)":>", 2);
	if(p != 0xffff)
	{
		memcpy((void *)str,(const void *)&(buf[p+2]),16);
	}
}

/*=====================================================
 * 函数功能: 	设置APPEUI
 * 输入参数:	char *buf
 * 输出参数: 	char *str
 * 返    回:	
=====================================================*/
void SetAPPEUI(uint8_t *str)
{
	uint8_t len;
	char tmp[50] = {0};
	len = sprintf(tmp,"AT+APPEUI=>%s\r\n",str);
	if(LoraSendCMD(tmp, len, "OK", 10))
	{
		LoraSendCMD(tmp, len, "OK", 10);
	}
}


/*=====================================================
 * 函数功能: 	获取Lora状态
 * 输入参数:	
 * 输出参数: 	
 * 返    回:	
=====================================================*/
uint8_t GetLoraStatus(uint8_t *buf,uint8_t len)
{
	uint8_t ret = 0xff;
	uint16_t p;
	
	myprintf("\r\nget Lora sattus");
	if(LoraSendCMD("AT+STATUS=?\r\n", 13, "+STATUS:", 15) == 0)
	{
		p = StrFindString(buf, len, (uint8_t *)"+STATUS:",8);
		if(p != 0xffff)
		{
			ret = buf[p+8]-'0';
		}
		else
		{
			myprintf("\r\n Lora get sattus error:%d",ret);
			myprintf("\r\n Lora buf:%s",buf);
		}
	}
	else
	{
//		if(LoraSendCMD("AT+STATUS=?\r\n", 13, "OK", 10) == 0)
//		{
//			p = StrFindString(buf, len, (uint8_t *)"+STATUS:",8);
//			if(p != 0xffff)
//			{
//				ret = buf[p+1]-'0';
//			}
//		}
//		else
		{
			myprintf("\r\n Lora get sattus error");
			ret = 0xff;
		}
	}
	return ret;
}

/*=====================================================
 * 函数功能: 	设置Lora发射功率
 * 输入参数:	
 * 输出参数: 	
 * 返    回:	
=====================================================*/
uint8_t SetLoraTxPower(uint8_t power)
{
	uint8_t ret = 0xff;
	uint16_t p;
	uint8_t len;
	char tmp[50] = {0};
	
	len = sprintf(tmp,"AT+DEFAULTPW=%d\r\n",power);
	if(len > 50)
		return ret;
	if(LoraSendCMD(tmp, len, "OK", 10))
	{
//		ret = 0xff;
		len = sprintf(tmp,"AT+CURRENTPW=%d\r\n",power);
		if(LoraSendCMD(tmp, len, "OK", 10))
		{
			ret = 0xff;
		}
		else
		{
			ret = power;
		}
	}
	else
	{
		ret = power;
	}
	return ret;
}

/*=====================================================
 * 函数功能: 	Lora重启
 * 输入参数:	
 * 输出参数: 	
 * 返    回:	
=====================================================*/
void LoraReBoot(void)
{
	if(LoraSendCMD("AT+REBOOT\r\n", 11, "OK", 10))
	{
		LoraSendCMD("AT+REBOOT\r\n", 11, "OK", 10);
	}
}

/*=====================================================
 * 函数功能: 	设置组播地址
 * 输入参数:	
 * 输出参数: 	
 * 返    回:	
 * 注意	   ：组播 0 默认短地址为 0xFFFFFFFF， APPSKEY 为<FFEEDDCC8C7FC6CBC33D0809FB565001， NWKSKEY
			 为<FFEEDDCC8C7FC6CBC33D0809FB565002，不允许修改，仅流水号允许修改。
			 组播 1 默认短地址为 0x074F1EF1， APPSKEY 为<148867448C7FC6CBC33D0809FB565114,NWKSKEY为
			 <A3485F52B6D70D1AECA0E92EB42E27EE，可以修改。
=====================================================*/
uint8_t SetMulticast(uint8_t power)
{
	uint8_t ret = 0xff;
	uint16_t p;
	uint8_t len;
	char tmp[128] = {0};
	uint8_t APPSKEY[32];
	uint8_t NWKSKEY[32];
	len = sprintf(tmp,"AT+MULTICAST%d,%s,<%s,<%s\r\n",power,"0xFFFFFFFF",APPSKEY,NWKSKEY);
	if(len > 128)
		return ret;
	if(LoraSendCMD(tmp, len, "OK", 10))
	{
		ret = 0xff;
		if(LoraSendCMD(tmp, len, "OK", 10))
		{
			ret = 0xff;
		}
		else
		{
			ret = power;
		}
	}
	else
	{
		ret = power;
	}
	return ret;
}


uint8_t DataToAscall(uint8_t data)
{
	uint8_t ret = 0;
	if(data>15)
		return ret;
	else if(data >= 10)
	{
		ret = data-10+'A';
	}
	else
	{
		ret = data+'0';
	}
	return ret;
}

uint8_t AscallToData(uint8_t data)
{
	uint8_t ret = 0xff;
	if(data >= '0' && data<='9')
	{
		ret = data-'0';
	}
	else if(data >= 'A' && data<='Z')
	{
		ret = data-'A'+10;
	}
	else if(data >= 'a' && data<='z')
	{
		ret = data-'a'+10;
	}
	return ret;
}

void Data_Convert(uint8_t *convert_data,uint16_t convert_len,uint8_t *result_data)
{
	uint16_t i = 0;
	uint8_t high_data,low_data;
	for(i = 0;i<convert_len;i++)
	{
		high_data = (convert_data[i]>>4)&0x0f;
		low_data = convert_data[i]&0x0f;
		result_data[i*2] = DataToAscall(high_data);
		result_data[i*2+1] = DataToAscall(low_data);
	}
}

void Data_Recover(uint8_t *recover_data,uint16_t convert_len,uint8_t *result_data)
{
	uint16_t i = 0;
	uint8_t high_data,low_data;
	for(i = 0;i<convert_len;i++)
	{
		high_data = AscallToData(recover_data[i*2]);
		low_data = AscallToData(recover_data[i*2+1]);
		result_data[i] = (high_data<<4) | low_data;
	}
}


/*=====================================================
* 函数功能: Lora发送数据
* 输入参数:	sendport：发送端口  confirm：网关确认  sendlen：数据长度  senddata：发送数据
* 输出参数: 	
* 返    回:	
* 注	意：数据块类型，最长 80 个字节
=====================================================*/
uint8_t LoraSendData(uint8_t sendport,uint8_t confirm,uint16_t sendlen,uint8_t *senddata)
{
	uint8_t ret = 0xff;
	uint16_t p;
	uint8_t len;
	char tmp[128] = {0};
	
	len = sprintf(tmp,"AT+LRSEND=%d,%d,%d,<",sendport,confirm,sendlen);
	Data_Convert(senddata,sendlen,(uint8_t *)&tmp[len]);
//	memcpy(&tmp[len],senddata,sendlen);
	tmp[len+sendlen*2] = '\r';
	tmp[len+sendlen*2+1] = '\n';
	if(len+sendlen+1 > 128)
		return ret;
	myprintf("\r\n lora senddata: %s",tmp);
	if(LoraSendCMD(tmp, len+sendlen*2+2, "OK", 20))
	{
		ret = 0xff;
//		vTaskDelay(10);
//		if(LoraSendCMD(tmp, len+sendlen*2+2, "OK", 10))
//		{
//			ret = 0xff;
//		}
//		else
//		{
//			ret = 0;
//		}
	}
	else
	{
		ret = 0;
	}
	return ret;
}

/*=====================================================
* 函数功能: Lora带应答发送数据
* 输入参数:	sendport：发送端口  confirm：网关确认  sendlen：数据长度  senddata：发送数据
* 输出参数: 	
* 返    回:	
* 注	意：数据块类型，最长 80 个字节
=====================================================*/
uint8_t LoraSendData_ACK(uint8_t sendport,uint16_t sendlen,uint8_t *senddata)
{
	uint8_t ret = 0xff;
	BaseType_t res;
	
	xSemaphoreTake(Lora_CMD_Semaphore, ( TickType_t ) 0);	//获取信号量
	LoraSendData(sendport,1,sendlen,senddata);
	
	res = xSemaphoreTake(Lora_CMD_Semaphore, ( TickType_t )(1000));
	if(res == pdTRUE)	//等待检索结果
	{
		myprintf("\r\n Gateway ACK");
		return 0;
	}
	else
	{
		myprintf("\r\n Gateway ACK fail");
		return 1;
	}
	return ret;
}



/*=====================================================
 * 函数功能: 	Lora 配置
 * 输入参数:	
 * 输出参数: 	
 * 返    回:	
=====================================================*/
uint8_t Lora_Config(void)
{
	uint8_t ReConfigCnt = 0;
	while(1)
	{
		ReConfigCnt++;
		if(ReConfigCnt > 10)
		{
			return LoraInitErro;
		}
		vTaskDelay(10);
		if(LoraSendCMD("AT\r\n", 4, "OK", 10))
		{
			myprintf("\r\nAT continue. ReConfigCnt: %d", ReConfigCnt);
		 	continue;
		}
		vTaskDelay(10);
		if(LoraSendCMD("AT+ACTIVEMODE=?\r\n", 17, "OK", 10))
		{
			myprintf("\r\n check ACTIVEMODE error");
		 	continue;
		}
		vTaskDelay(10);
		LoraActiveMode = GetActiveMode(LoraBuf,LoraCount);
		if(LoraActiveMode != OTAA)
		{
			if(LoraSendCMD("AT+ACTIVEMODE=0\r\n", 17, "OK", 10))
			{
				myprintf("\r\n set ACTIVEMODE error");
				continue;
			}
			LoraActiveMode = GetActiveMode(LoraBuf,LoraCount);
		}
		vTaskDelay(10);
		//请求DEVEUI
		if(memcmp((const void *)DEVEUI, (const void *)memzero, sizeof(DEVEUI)) == 0)
		{
			if(LoraSendCMD("AT+DEVEUI=?\r\n", 13, "OK", 10))
			{
				myprintf("\r\n AT+DEVEUI=? error");
				continue;
			}
			GetEUI((uint8_t *)LoraBuf,LoraCount,(uint8_t *)DEVEUI);
			myprintf("\r\n DEVEUI %s",DEVEUI);
		}
		vTaskDelay(10);
		//请求APPEUI
		if(memcmp((const void *)APPEUI, (const void *)memzero, sizeof(APPEUI)) == 0)
		{
			if(LoraSendCMD("AT+APPEUI=?\r\n", 13, "OK", 10))
			{
				myprintf("\r\n AT+APPEUI=? error");
				continue;
			}
			GetEUI((uint8_t *)LoraBuf,LoraCount,(uint8_t *)APPEUI);
			if(strcmp((const char *)APPEUI,Lora_APPEUI) != 0)
			{
				SetAPPEUI((uint8_t *)Lora_APPEUI);
				GetEUI((uint8_t *)LoraBuf,LoraCount,(uint8_t *)APPEUI);
			}
			myprintf("\r\n APPEUI %s",APPEUI);
		}
		vTaskDelay(10);
		if(SetLoraTxPower(0) == 0xff)		//设置发射功率为19dB
			continue;
		vTaskDelay(10);
		if(LoraSendCMD("AT+DEVCLASS=2\r\n", 15, "OK", 10))	//设置中断类型为Class C
		{
			myprintf("\r\n AT+DEVCLASS=2 error");
		 	continue;
		}
		break;
	}
	return LoraInitOk;
}

void LoraDataProcess(void)
{
	uint16_t p,q;
	uint32_t num;
	rtc_time_t tempTime;
	
	uint8_t *msgdata = NULL;
	uint16_t LoraRecviveLen = 0;
		
	if(xQueueReceive(Lora_RECEIVE_Queue, (void *)&platform_recvbuf, ( TickType_t )0) != pdPASS){
		return;
	}
	LoraRecviveLen = platform_recvbuf[0] | (platform_recvbuf[1]<<8);
	msgdata = &platform_recvbuf[2];

	if((LoraRecviveLen == 0) || (LoraRecviveLen > PLATFORM_BUFF_SIZE))
	{
		myprintf("\r\n Lora长度错误:%d",LoraRecviveLen);
		return;
	}
//	if(platform_recvbuf[2] == '^')
//	{
		myprintf("\r\nAT接收, len: %d",LoraRecviveLen);
		PrintWrite(&platform_recvbuf[2], LoraRecviveLen);
		p = StrFindString(&platform_recvbuf[2], LoraRecviveLen,(uint8_t *)"^LRSEND:", strlen((const char *)"^LRSEND:"));
		if(StrFindString(&platform_recvbuf[2], LoraRecviveLen,(uint8_t *)"^LRSEND:", strlen((const char *)"^LRSEND:")) != 0xffff)			//发送数据报告数据
		{
			uint16_t q;
//			msgdata = &platform_recvbuf[2+p+7];
		}
		if((p = StrFindString(&platform_recvbuf[2], LoraRecviveLen,(uint8_t *)"^LRRECV:", strlen((const char *)"^LRRECV:"))) != 0xffff)	//接收数据
		{
			uint8_t tmp;
			uint16_t rev_len = 0;
			uint8_t *revdata;

			tmp = p+2+8;
			p = StrFindString(&platform_recvbuf[tmp], LoraRecviveLen,(uint8_t *)"<", strlen((const char *)"<"));
			q = StrFindString(&platform_recvbuf[tmp+p], LoraRecviveLen-p,(uint8_t *)",", strlen((const char *)","));
//			Str2Num(&platform_recvbuf[tmp+num1+1],num2-num1-1,(uint32_t *)&LoraRssi);
			rev_len = q;		//数据长度		
			Data_Recover(&platform_recvbuf[tmp+p+1],rev_len/2,LoraBufTmp);
			Platform_Data_Process(LoraBufTmp);
		}
//		p = StrFindString(&platform_recvbuf[3], LoraRecviveLen-1,(uint8_t *)"CONFIRM", strlen((const char *)"CONFIRM"));
		if((p = StrFindString(&platform_recvbuf[2], LoraRecviveLen,(uint8_t *)"^LRCONFIRM:", strlen((const char *)"^LRCONFIRM:"))) != 0xffff)	//网关收到确认数据
		{
			uint8_t num1,num2;
			uint8_t tmp;
			tmp = p+2+11;
			num1 = GetNStr(&platform_recvbuf[tmp],LoraRecviveLen, ',',1);
			num2 = GetNStr(&platform_recvbuf[tmp],LoraRecviveLen, ',',2);
			if(platform_recvbuf[tmp+num1+1] == '-')
			{
				Str2Num(&platform_recvbuf[tmp+num1+1+1],num2-num1-1,(uint32_t *)&LoraRssi);
				LoraRssi = 0-LoraRssi;
			}
			else
			{
				Str2Num(&platform_recvbuf[tmp+num1+1],num2-num1-1,(uint32_t *)&LoraRssi);
			}
			num1 = GetNStr(&platform_recvbuf[tmp],LoraRecviveLen, ',',3);
			if(platform_recvbuf[tmp+num2+1] == '-')
			{
				Str2Num(&platform_recvbuf[tmp+num2+1+1],num1-num2-1,(uint32_t *)&LoraSNR);
				LoraSNR = 0-LoraSNR;
			}
			else
			{
				Str2Num(&platform_recvbuf[tmp+num2+1],num1-num2-1,(uint32_t *)&LoraSNR);
			}
			myprintf("\r\n Rssi:%d SNR:%d",LoraRssi,LoraSNR);
			xSemaphoreGive(Lora_CMD_Semaphore);
		}
//	}
	else
	{
		uint8_t *s;
		s = pvPortMalloc(sizeof(uint8_t)*128);
		if(xQueueReceive( Lora_CMD_Queue, (void *)s, ( TickType_t ) 0) == pdPASS){	//获取消息队列
			p = StrFindString(&platform_recvbuf[2], LoraRecviveLen, s, strlen((const char *)s));		
			if(p != 0xffff)
			{
				myprintf("\r\nAT接收, len: %d",LoraRecviveLen);
				PrintWrite(&platform_recvbuf[2], LoraRecviveLen);
				if(LoraRecviveLen > LoraBuf_Size)
				{
					LoraCount = LoraBuf_Size;
					memcpy((void *)LoraBuf, (const void *)&platform_recvbuf[2], LoraBuf_Size); 
				}
				else
				{
					LoraCount = LoraRecviveLen;
					memcpy((void *)LoraBuf, (const void *)&platform_recvbuf[2], LoraRecviveLen); 
				}
				xSemaphoreGive(Lora_CMD_Semaphore);
			}
			else
			{
				uint8_t len=0;
				uint8_t *tmp;
				tmp = pvPortMalloc(sizeof(uint8_t)*(strlen((const char *)s)));
				xQueueSend(Lora_CMD_Queue, (void *)tmp, 0);		//当前数据包无需要内容，重新发送需要检索的内容消息队列
				vPortFree(tmp);
			}
		}
		vPortFree(s);
	}
	//远程更新
//	RemoteUpdata(msgdata,256);
	//时间查找
//	p = StrFindString(msgdata, GPRSRecviveLen, (uint8_t *)"time:", 5);
//	if(p != 0xffff)
//	{
//		if(Str2Num(&msgdata[p+5], 2, &num) == 0) return;
//		tempTime.ui8Year = (uint8_t)num+2000;
//		if(Str2Num(&msgdata[p+8], 2, &num) == 0) return;
//		tempTime.ui8Month = (uint8_t)num;
//		if(Str2Num(&msgdata[p+11], 2, &num) == 0) return;
//		tempTime.ui8DayOfMonth = (uint8_t)num;
//		if(Str2Num(&msgdata[p+14], 2, &num) == 0) return;
//		tempTime.ui8Hour = (uint8_t)num;
//		if(Str2Num(&msgdata[p+17], 2, &num) == 0) return;
//		tempTime.ui8Minute = (uint8_t)num;
//		if(Str2Num(&msgdata[p+20], 2, &num) == 0) return;
//		tempTime.ui8Second = (uint8_t)num;
//		SetRTC(&tempTime);
//		GetRTC(&tempTime);
//#if Platform_Data_Printf
//		myprintf("\r\n获取到北京时间：");
//		myprintf("%02d-%02d-%02d  %02d:%02d:%02d\r\n", tempTime.ui8Year, tempTime.ui8Month,\
//		tempTime.ui8DayOfMonth, tempTime.ui8Hour, tempTime.ui8Minute, tempTime.ui8Second);
//#endif
//	}

}


/*====================================================
 * 函数功能: 	GPRS任务
 * 输入参数:	void *c
 * 输出参数: 
 * 返    回:	无
=====================================================*/
void Lora_Task(void *argument)
{
	uint8_t lora_status;
	uint8_t lora_status_new;
	uint8_t offline_cnt = 0;
	uint8_t get_status_cnt = 0;
	while(lora_status != LoraInitOk)
	{
		lora_status = Lora_Config();
		Get_or_Set_LoraStatus(1,&lora_status);
	}
	while(1)
	{
		get_status_cnt++;
		if(get_status_cnt%5==0)
		{
			static uint8_t recnt = 0;
			lora_status_new = GetLoraStatus(LoraBuf,LoraCount);
			if(lora_status_new == 0xff)		//获取状态失败连续10次后才算
			{
				recnt++;
				if(recnt<10)
				{
					lora_status_new = lora_status;
				}
				else
					recnt = 0;
			}
			else
				recnt = 0;
		}
		if(lora_status_new != lora_status)
		{
			if(lora_status != LoraInitErro)		//如果Lora原状态为初始化失败，则不更新状态
			{
				lora_status = lora_status_new;
				Get_or_Set_LoraStatus(1,&lora_status);
			}
		}
		if(lora_status == LoraReset || lora_status == LoraInitErro)
		{
			LoraReBoot();			//Lora复位并重新配置
			vTaskDelay(1000);
			lora_status = Lora_Config();
		}
		else if(lora_status == LoraOffLine)			//Lora离线
		{
			offline_cnt++;
			if(offline_cnt>=60)
			{
				offline_cnt = 0;
				LoraReBoot();			//Lora复位并重新配置
				vTaskDelay(1000);
				lora_status = Lora_Config();
			}
		}
		else if(lora_status == LoraOnLine)
		{
			offline_cnt = 0;
		}
		vTaskDelay(1000);
	}
}

void CreatLoraTask(void)
{
	BaseType_t Lora_TaskID;
	_myprintf("\r\n CreatLoraTask");
	Lora_TaskID = xTaskCreate (Lora_Task, "Lora", Lora_StkSIZE, NULL, Lora_TaskPrio, NULL);
	if(pdPASS != Lora_TaskID)
	{
		while(1);
	}
}

