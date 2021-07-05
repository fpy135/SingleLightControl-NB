#include "Platform.h"
#include "SysConfig.h"
#include "bsp_rtc.h"
#include "bsp_wdt.h"
#include "bsp_uart.h"
#include "mystring.h"
#include "Protocol.h"
#include "IDConfig.h"
#include "Unixtimer.h"
#include "Led_App.h"
#include "MyFlash.h"
#include "StorageConfig.h"
#include "BL6523GX.h"
#include "LoraApp.h"
#include "bc26.h"

QueueHandle_t GPRS_UART_SEND_Queue;
QueueHandle_t GPRS_RECEIVE_Queue;

uint8_t platform_recvbuf[PLATFORM_BUFF_SIZE];					//定义数据处理Buff大小为300（为100也无所谓，只要大于等于100就好）
uint8_t platform_sendbuf[PLATFORM_BUFF_SIZE];

uint8_t Updata_Falg  =0;

static uint32_t get_time_tick = 0;
uint32_t Heart_Tick = 0;
uint32_t Herat_Rate = 240;		//单位：秒

extern void GetRTC(rtc_time_t *rtc_time);
extern void SetRTC(rtc_time_t *rtc_time);
extern void SetRTC_BCD(rtc_time_t *rtc_time);
extern void GetRTC_BCD(rtc_time_t *rtc_time);

extern xTaskHandle RemoteUpdata_Task_Handle;

extern void Get_ElectricData(CollectionData *electricdata);
extern void Write_ElectricData(CollectionData *electricdata);

#if USE_LORA_UART_TO_INTERNET

#if USE_Winext_Protocol
uint8_t lora_senddata[256];
uint8_t *WinextProtocol_Pack(uint8_t type,uint8_t cmd, uint8_t datatype,uint8_t len, uint8_t *content)
{
    uint8_t *pBuf;
    uint8_t i = 0;
	
    lora_senddata[0] = type;			//传感器类型  0x03
    lora_senddata[1] = datatype | 0x20;	//高4位为协议版本，第四位为协议类型 01: 心跳帧 02：数据帧 03: 报警帧
	lora_senddata[2] = 0x9E;			//控制码
    lora_senddata[3] = len;
	lora_senddata[4] = cmd;
	
    memcpy(&lora_senddata[5], content, len-1);

    pBuf = (uint8_t *)(&lora_senddata);
    return pBuf;
}
#endif

/*=====================================================
* 函数功能: 	Lora发送数据
 * 输入参数:	
 * 输出参数: 
 * 返    回:	
=====================================================*/
#if USE_Winext_Protocol
void Platform_SendDataByLora(uint16_t type, uint8_t cmd, uint8_t datatype,uint16_t len,uint8_t *senddata)
{
    uint8_t *pbuf;
#if Platform_Data_Printf
    myprintf("\r\n Lora send:");
    myprintf_buf_unic((uint8_t *)pbuf, 4+len);
#endif
	pbuf = WinextProtocol_Pack(type,cmd, datatype,len,(uint8_t *)senddata);
	LoraSendData(45,0,4+len,(uint8_t *)pbuf);		//45号端口发送数据 10为负载以前的固定数据的数据长度+负载数据长度+crc

}
#else
void Platform_SendDataByLora(uint8_t sn,uint16_t type, uint32_t id, uint8_t cmd,uint16_t len,uint8_t *senddata)
{
    uint8_t *pbuf;
#if Platform_Data_Printf
    myprintf("\r\n Lora send:");
    myprintf_buf_unic((uint8_t *)pbuf, 10+len+2);
#endif

    pbuf = Protocol_Pack(Sn,type,id,cmd,len,(uint8_t *)senddata);
	LoraSendData(45,1,10+len+2,(uint8_t *)pbuf);		//10为负载以前的固定数据的数据长度+负载数据长度+crc
}
#endif


void Platform_SendData(uint8_t sn,uint16_t type, uint32_t id, uint8_t cmd, uint8_t datatype, uint16_t len,uint8_t *senddata)
{
	Sn++;
	Sn &= 0x3F;
#if USE_ETH_TO_INTERNET
	socket_SendData(sn,type,id,cmd,len,senddata);
#elif USE_4G_UART_TO_INTERNET
	Platform_SendDataByGPRS(sn,type,id,cmd,len,senddata);
#elif USE_LORA_UART_TO_INTERNET
	#if USE_Winext_Protocol
		Platform_SendDataByLora(type,cmd,datatype,len,senddata);
	#else
		Platform_SendDataByLora(sn,type,id,cmd,len,senddata);
	#endif
#endif
}

void Get_Platform_Time_Lora(void)
{
	if((getRunTimeCounterValue() - get_time_tick)>60*1000 || get_time_tick == 0)
	{
		static uint32_t last_tick = 0;
		static uint32_t now_tick = 0;
		if((now_tick - last_tick)>=2000 || now_tick == 0)		//间隔2s
		{
			uint8_t tmp[15] = "apply for time";
			
			last_tick = now_tick;
//			Lora_SendData((uint8_t *)tmp,15);
//			LoraSendData(22,0,15,(uint8_t *)tmp);
#if Platform_Data_Printf
			myprintf("\r\n 尝试获取时间");
#endif
		}
		now_tick = getRunTimeCounterValue();
	}
}

#endif

#if USE_4G_UART_TO_INTERNET

/*=====================================================
 * 函数功能: 	GPRS接收数据队列发送 中断调用
 * 输入参数:	长度 + 内容
 * 输出参数: 
 * 返    回:	1 失败, 0 成功
=====================================================*/
uint8_t MSG_GPRSReceiveDataFromISR(uint8_t *buf, uint16_t len)
{
	uint8_t Data[len+2];
	//申请缓存

	//复制消息
	Data[0] = len;
	Data[1] = len>>8;
	memcpy((void *)&Data[2],(const void *)buf,len);
	//发送消息

	if(xQueueSendToBackFromISR(GPRS_RECEIVE_Queue, Data, 0) != pdPASS)
	{
		_myprintf("\r\nxQueueSend error, GPRS_RECEIVE_Queue");
		return 1;
	}

	return 0;
}

/*=====================================================
* 函数功能: 	GPRS发送数据
 * 输入参数:	
 * 输出参数: 
 * 返    回:	
=====================================================*/
void Platform_SendDataByGPRS(uint8_t sn,uint16_t type, uint32_t id, uint8_t cmd,uint16_t len,uint8_t *senddata)
{
    uint8_t *pbuf;

    pbuf = Protocol_Pack(Sn,type,id,cmd,len,(uint8_t *)senddata);
#if Platform_Data_Printf
    myprintf("\r\n 4G send:");
    myprintf_buf_unic((uint8_t *)pbuf, 10+len+2);
#endif
	GPRS_SendData((uint8_t *)pbuf, 10+len+2);		//10为负载以前的固定数据的数据长度+负载数据长度+crc
}

void GPRSDataPro(void)
{
	uint16_t p;
	uint32_t num;
	rtc_time_t tempTime;
	
	uint8_t *msgdata = NULL;
	uint16_t GPRSRecviveLen = 0;
		
	if(xQueueReceive(GPRS_RECEIVE_Queue, (void *)&platform_recvbuf, ( TickType_t )100) != pdPASS){
		return;
	}
	GPRSRecviveLen = platform_recvbuf[0] | (platform_recvbuf[1]<<8);
	msgdata = &platform_recvbuf[2];

	if((GPRSRecviveLen == 0) || (GPRSRecviveLen > PLATFORM_BUFF_SIZE))
	{
		myprintf("\r\nGPRS长度错误:%d",GPRSRecviveLen);
	}
	//远程更新
	RemoteUpdata(msgdata,256);
	//时间查找
	p = StrFindString(msgdata, GPRSRecviveLen, (uint8_t *)"time:", 5);
	if(p != 0xffff)
	{
		if(Str2Num(&msgdata[p+5], 2, &num) == 0) return;
		tempTime.ui8Year = (uint8_t)num+2000;
		if(Str2Num(&msgdata[p+8], 2, &num) == 0) return;
		tempTime.ui8Month = (uint8_t)num;
		if(Str2Num(&msgdata[p+11], 2, &num) == 0) return;
		tempTime.ui8DayOfMonth = (uint8_t)num;
		if(Str2Num(&msgdata[p+14], 2, &num) == 0) return;
		tempTime.ui8Hour = (uint8_t)num;
		if(Str2Num(&msgdata[p+17], 2, &num) == 0) return;
		tempTime.ui8Minute = (uint8_t)num;
		if(Str2Num(&msgdata[p+20], 2, &num) == 0) return;
		tempTime.ui8Second = (uint8_t)num;
		SetRTC(&tempTime);
		GetRTC(&tempTime);
#if Platform_Data_Printf
		myprintf("\r\n获取到北京时间：");
		myprintf("%02d-%02d-%02d  %02d:%02d:%02d\r\n", tempTime.ui8Year, tempTime.ui8Month,\
		tempTime.ui8DayOfMonth, tempTime.ui8Hour, tempTime.ui8Minute, tempTime.ui8Second);
#endif
		get_time_tick = HAL_GetTick();
	}
	else
	{
#if Platform_Data_Printf
		myprintf("\r\n gprs recever:");
		myprintf_buf_unic((uint8_t *)msgdata,10+msgdata[9]+2);
#endif
		Platform_Data_Process(&platform_recvbuf[2]);
	}
}

void Get_Platform_Time_4G(void)
{
	if((HAL_GetTick() - get_time_tick)>60*1000 || get_time_tick == 0)
	{
		static uint32_t last_tick = 0;
		static uint32_t now_tick = 0;
		if((now_tick - last_tick)>=2000 || now_tick == 0)		//间隔2s
		{
			uint8_t tmp[15] = "apply for time";
			
			last_tick = now_tick;
			GPRS_SendData((uint8_t *)tmp,15);
#if Platform_Data_Printf
			myprintf("\r\n 尝试获取时间");
#endif
		}
		now_tick = HAL_GetTick();
	}
}

#endif

#if USE_NB_UART_TO_INTERNET

/*=====================================================
* 函数功能: 	GPRS发送数据
 * 输入参数:	
 * 输出参数: 
 * 返    回:	
=====================================================*/
void Platform_SendDataByNB(uint8_t sn,uint16_t type, uint32_t id, uint8_t cmd,uint16_t len,uint8_t *senddata)
{
    uint8_t *pbuf;

    pbuf = Protocol_Pack(Sn,type,id,cmd,len,(uint8_t *)senddata);
#if Platform_Data_Printf
    myprintf("\r\n NB send:");
    myprintf_buf_unic((uint8_t *)pbuf, 10+len+2);
#endif
	if(xQueueSend(NB_SEND_Queue, (void *)pbuf, 0) != pdPASS)		//发送需要检索的内容消息队列
	{
		myprintf("\r\nx QueueSend error, NB_SEND_Queue");
	}
//	BC26_Senddatahex((uint8_t *)pbuf, 10+len+2);		//10为负载以前的固定数据的数据长度+负载数据长度+crc
}

void Platform_SendData(uint8_t sn,uint16_t type, uint32_t id, uint8_t cmd, uint16_t len,uint8_t *senddata)
{
	sn++;
	sn &= 0x3F;
	Platform_SendDataByNB(sn,type,id,cmd,len,senddata);
}
#endif
/*=====================================================
* 函数功能: 	平台数据处理
 * 输入参数:	接受数据
 * 输出参数: 
 * 返    回:	
=====================================================*/
void Platform_Data_Process(uint8_t* msgdata)
{
#if USE_Winext_Protocol
	if(msgdata[0] == SensorType && msgdata[2] == 0x1E)
	{
		switch(msgdata[4])
		{
			case GetDataCmd:
				Device_Data_Send();
				break;
			case LedControlCmd:
			{
				uint8_t tmp[2];
				if(msgdata[5] == 0x01)	//LED开关状态更新
				{
					REL1_Write(REL_ON);	//打开继电器电源
				}
				else if(msgdata[5] == 0x00)
				{
					REL1_Write(REL_OFF);	//关闭继电器电源
				}
//				Device_Data_Send();
				Get_LED_Data(&tmp[0],&tmp[1]);
				tmp[0] = msgdata[5];
				xQueueSend(LedShow_Queue, (void *)tmp, 0);			//演示模式
				break;
			}
			case LedPwmCmd:
			{
				uint8_t tmp[2];
				if(msgdata[5]>=100)	//限位
					msgdata[5] = 100;
				if(msgdata[5]>0)
				{
					tmp[0] = 1;
					REL1_Write(REL_ON);	//打开继电器电源
				}
				else
				{
					tmp[0] = 0;
					REL1_Write(REL_OFF);	//关闭继电器电源
				}
				tmp[1] = msgdata[5];
				xQueueSend(LedShow_Queue, (void *)tmp, 0);			//演示模式
				break;
			}
			case GetHeartCycleCmd:
			{
				uint8_t tmp[4];
				
				tmp[0] = Herat_Rate>>24;
				tmp[1] = Herat_Rate>>16;
				tmp[2] = Herat_Rate>>8;
				tmp[3] = Herat_Rate;
				Platform_SendDataByLora(SensorType,GetHeartCycleCmd,DataType,4,(uint8_t *)tmp);
				break;
			}
			case SetHeartCycleCmd:
			{
				Herat_Rate = 0;
				Herat_Rate |= msgdata[5]<<24;
				Herat_Rate |= msgdata[6]<<16;
				Herat_Rate |= msgdata[7]<<8;
				Herat_Rate |= msgdata[8];
				Platform_SendDataByLora(SensorType,SetHeartCycleCmd,DataType,4,(uint8_t *)&msgdata[5]);
				break;
			}
			case GetTimeCmd:
			{
				rtc_time_t rtc_time;
				uint8_t tmp[8];
				
				GetRTC_BCD(&rtc_time);
				tmp[0] = 0x01;
				tmp[1] = rtc_time.ui8Second;
				tmp[2] = rtc_time.ui8Minute;
				tmp[3] = rtc_time.ui8Hour;
				tmp[4] = rtc_time.ui8Week;
				tmp[5] = rtc_time.ui8DayOfMonth;
				tmp[6] = rtc_time.ui8Month;
				tmp[7] = rtc_time.ui8Year;
				
				Platform_SendDataByLora(SensorType,GetTimeCmd,DataType,8,(uint8_t *)tmp);
				break;
			}
			case SetTimeCmd:
			{
				rtc_time_t rtc_time;
				
				rtc_time.ui8Second = msgdata[5];
				rtc_time.ui8Minute = msgdata[6];
				rtc_time.ui8Hour = msgdata[7];
				rtc_time.ui8Week = msgdata[8];
				rtc_time.ui8DayOfMonth = msgdata[9];
				rtc_time.ui8Month = msgdata[10];
				rtc_time.ui8Year = msgdata[11];
				SetRTC_BCD(&rtc_time);
				Platform_SendDataByLora(SensorType,SetTimeCmd,DataType,8,(uint8_t *)&msgdata[5]);
				break;
			}
			case SetTimeControlCmd:
			{
				LoraTimeControl_Type *loratimecontrol_data;
				uint8_t tmp = 1;
				uint8_t count = 0;
				
				count = msgdata[6];
				
				memcpy((uint8_t *)loratimecontrol_data,&msgdata[7],count*5);
				memset((uint8_t *)loratimecontrol_data+count*5,0xff,sizeof(LoraTimeControl_Type)-count*5);
				
				loratimecontrol_data->count = count;
				
				Write_LoraTimeControl_Data(loratimecontrol_data);
				//写入flash
				Write_UserMem_Flash(LORATIMECONTROL_ADDR,(uint8_t *)loratimecontrol_data, sizeof(LoraTimeControl_Type));
				
//				Platform_SendData(Sn,Device_TYPE,Device_ID,TimeControlBack_Cmd,0,&msgdata[10]);	//数据回传

				Platform_SendDataByLora(SensorType,SetTimeControlCmd,DataType,2,(uint8_t *)&tmp);
				break;
			}
			case GetTimeControlCmd:
			{
				LoraTimeControl_Type *loratimecontrol_data;
				uint8_t tmp[27] = {0};
				
				Get_LoraTimeControl_Data(loratimecontrol_data);
				tmp[0] = 0x01;
				tmp[1] = loratimecontrol_data->count;
				memcpy(&tmp[2],loratimecontrol_data, loratimecontrol_data->count*5);
				
				Platform_SendDataByLora(SensorType,GetTimeControlCmd,DataType,3+loratimecontrol_data->count*5,(uint8_t *)&tmp);
				break;
			}
			case ResetSysCmd:
			{
				myprintf("\r\n 系统复位");
				HAL_NVIC_SystemReset();
				break;
			}
		}
	}
	
#else
	if(msgdata[0] == 0xAA)
	{
		#if Platform_Data_Printf
			#if USE_ETH_TO_INTERNET
				myprintf("\r\n TCP recever:");
			#elif USE_4G_UART_TO_INTERNET
				myprintf("\r\n gprs recever:");
			#elif USE_LORA_UART_TO_INTERNET
				myprintf("\r\n Lora recever:");
			#elif USE_NB_UART_TO_INTERNET
				myprintf("\r\n NB recever:");
			#endif
			myprintf_buf_unic((uint8_t *)msgdata,10+msgdata[9]+2);
		#endif
		uint16_t tmp_type = 0;
		uint32_t tmp_id = 0;
		tmp_type = msgdata[2]<<8 | msgdata[3];
		tmp_id = msgdata[4]<<24 | msgdata[5]<<16 | msgdata[6]<<8 | msgdata[7];
		if(tmp_type == Device_TYPE && tmp_id == Device_ID)		//ID校验
		{
			uint16_t crc_tmp1,crc_tmp2;
			uint8_t datalen = 10+msgdata[9]+2;		//10为负载以前的固定数据的数据长度+负载数据长度+crc
			crc_tmp2 = CRC16_calc(msgdata,datalen-2);
			crc_tmp1 = msgdata[datalen-1]<<8 | msgdata[datalen-2];
			if(crc_tmp2 == crc_tmp1)
			{
				if(msgdata[8] == LedControl_Cmd)		//开关灯命令字
				{
					uint8_t tmp[2];
					if(msgdata[10] == 0x01)	//LED开关状态更新
					{
						if(msgdata[11]>100)	//限位
							msgdata[11] = 100;
						Write_LED_Data(&msgdata[10],&msgdata[11]);	//LED亮度目标百分比调整
						REL1_Write(REL_ON);	//打开继电器电源
#if Platform_Data_Printf
						myprintf("\r\n 开灯 亮度：%d",msgdata[11]);
#endif
					}
					else if(msgdata[10] == 0x00)
					{
						if(msgdata[11]>100)	//限位
							msgdata[11] = 100;
						Write_LED_Data(&msgdata[10],&msgdata[11]);	//LED亮度目标百分比调整
						REL1_Write(REL_OFF);	//关闭继电器电源
#if Platform_Data_Printf
						myprintf("\r\n 关灯 亮度：%d",msgdata[11]);
#endif
					}
					Platform_SendData(Sn,Device_TYPE,Device_ID,LedControlBack_Cmd,4,&msgdata[10]);	//数据回传

					tmp[0] = msgdata[10];
					tmp[1] = msgdata[11];
					xQueueSend(LedShow_Queue, (void *)tmp, 0);			//演示模式

//					xSemaphoreGive(LedShowBinary);			//演示模式
				}
				else if(msgdata[8] == TimeControl_Cmd)		//时间策略控制命令字
				{
					TimeControl_Type timecontrol_data;
					
					timecontrol_data.start_hour = msgdata[10];
					timecontrol_data.start_min = msgdata[11];
					timecontrol_data.phase1_time = (msgdata[12]<<8) | msgdata[13];
					if(msgdata[14]>100)	//限位
							msgdata[14] = 100;
					timecontrol_data.phase1_Pwm = msgdata[14];
					timecontrol_data.phase2_time = (msgdata[15]<<8) | msgdata[16];
					if(msgdata[17]>100)	//限位
							msgdata[17] = 100;
					timecontrol_data.phase2_Pwm = msgdata[17];
					timecontrol_data.phase3_time = (msgdata[18]<<8) | msgdata[19];
					if(msgdata[20]>100)	//限位
							msgdata[20] = 100;
					timecontrol_data.phase3_Pwm = msgdata[20];
					timecontrol_data.phase4_time = (msgdata[21]<<8) | msgdata[22];
					if(msgdata[23]>100)	//限位
							msgdata[23] = 100;
					timecontrol_data.phase4_Pwm = msgdata[23];
					Write_TimeControl_Data(&timecontrol_data);
					//写入flash
					Write_UserMem_Flash(TIMECONTROL_ADDR,(uint8_t *)&timecontrol_data, sizeof(timecontrol_data));
					
					Platform_SendData(Sn,Device_TYPE,Device_ID,TimeControlBack_Cmd,0,&msgdata[10]);	//数据回传
				}
				else if(msgdata[8] == FindTimeCtr_Cmd)		//查询时间策略控制命令字
				{
					TimeControl_Type timecontrol_data;
					
					Get_TimeControl_Data(&timecontrol_data);
					timecontrol_data.phase1_time = HALFWORD_Reverse(timecontrol_data.phase1_time);
					timecontrol_data.phase2_time = HALFWORD_Reverse(timecontrol_data.phase2_time);
					timecontrol_data.phase3_time = HALFWORD_Reverse(timecontrol_data.phase3_time);
					timecontrol_data.phase4_time = HALFWORD_Reverse(timecontrol_data.phase4_time);
					Platform_SendData(Sn,Device_TYPE,Device_ID,FindTimeCtrBack_Cmd,sizeof(timecontrol_data),&timecontrol_data.start_hour);	//数据回传
				}
				else if(msgdata[8] == FindElectric_Cmd)		//查询电压、电流电气命令字
				{
					CollectionData electricdata;
					uint8_t tmp[12] = {0};
					
					Get_ElectricData(&electricdata);
					
					tmp[0] = electricdata.Voltage>>24;
					tmp[1] = electricdata.Voltage>>16;
					tmp[2] = electricdata.Voltage>>8;
					tmp[3] = electricdata.Voltage;
					
					tmp[4] = electricdata.Current>>24;
					tmp[5] = electricdata.Current>>16;
					tmp[6] = electricdata.Current>>8;
					tmp[7] = electricdata.Current;
					
					tmp[8] = electricdata.Energy>>24;
					tmp[9] = electricdata.Energy>>16;
					tmp[10] = electricdata.Energy>>8;
					tmp[11] = electricdata.Energy;
////					BL6526B_ProcessTask(&electricdata);				/*电能计量芯片读取函数*/
//					electricdata.Voltage = WORD_Reverse(electricdata.Voltage);
//					electricdata.Current = WORD_Reverse(electricdata.Current);
//					electricdata.Energy = WORD_Reverse(electricdata.Energy);
					Platform_SendData(Sn,Device_TYPE,Device_ID,FindEleBack_Cmd,12,tmp);	//数据回传
				}
				else if(msgdata[8] == FindEnvironment_Cmd)		//查询环境数据命令字
				{
					Heart_Data_Send();		//回复心跳
				}
				else if(msgdata[8] == RemoteUpdata_Cmd)		//查询环境数据命令字
				{
					Platform_SendData(Sn,Device_TYPE,Device_ID,RemoteUpdataBack_Cmd,0,msgdata);	//数据回传
					#if USE_ETH_TO_INTERNET
					Get_File_inform(msgdata);
//					Updata_Falg = 1;		//开始远程更新
					xSemaphoreGive (RemoteUpdataMutex);//开始远程更新
					taskENTER_CRITICAL();
					CreatRemoteUpdataTask();			//创建远程升级任务
					RemoteUpdata_tick = HAL_GetTick();
//					vTaskResume(RemoteUpdata_Task_Handle);	//恢复任务
					taskEXIT_CRITICAL();
//					return;
					#endif
				}
			}
		}
	}
	#if USE_ETH_TO_INTERNET
//	if(Updata_Falg)
	if(xSemaphoreTake (RemoteUpdataMutex, 0))
	{
//				RemoteUpDate(msgdata);
		xSemaphoreGive (RemoteUpdataMutex);
		xQueueSendToBack(RemoteUpdata_Queue,msgdata,0);
	}
	#endif
#endif

}

void Device_Data_Send(void)
{
		uint8_t TCP_PAYLOAD[18] = {0};
		CollectionData electricdata;
		
		Get_ElectricData(&electricdata);
		myprintf("目前电压：%d v \n",electricdata.Voltage / 10);
		myprintf("目前电流：%.3f mA \n",electricdata.Current / 1000.0);
		myprintf("目前功率：%.1f w \n",electricdata.Power / 10.0);
		myprintf("目前电量：%.1f kwh \n",electricdata.Energy/ 100.0);
		myprintf("目前继电器状态：%d \n",!electricdata.RelayState);
		myprintf("目前调光值：%d \n",electricdata.LedPwm);
		myprintf("目前设备状态：%x \n",electricdata.Bl6526bState);
#if USE_LORA_UART_TO_INTERNET
		
		TCP_PAYLOAD[0] = GetDataCmd;		//命令码
		TCP_PAYLOAD[1] = electricdata.Voltage>>8;
		TCP_PAYLOAD[2] = electricdata.Voltage;
		TCP_PAYLOAD[3] = electricdata.Current>>24;
		TCP_PAYLOAD[4] = electricdata.Current>>16;
		TCP_PAYLOAD[5] = electricdata.Current>>8;
		TCP_PAYLOAD[6] = electricdata.Current;
		TCP_PAYLOAD[7] = electricdata.Power>>24;
		TCP_PAYLOAD[8] = electricdata.Power>>16;
		TCP_PAYLOAD[9] = electricdata.Power>>8;
		TCP_PAYLOAD[10] = electricdata.Power;
		TCP_PAYLOAD[11] = electricdata.Energy>>24;
		TCP_PAYLOAD[12] = electricdata.Energy>>16;
		TCP_PAYLOAD[13] = electricdata.Energy>>8;
		TCP_PAYLOAD[14] = electricdata.Energy;
		TCP_PAYLOAD[15] = !electricdata.RelayState;
		TCP_PAYLOAD[16] = electricdata.LedPwm;
		TCP_PAYLOAD[17] = electricdata.Bl6526bState;
		Platform_SendData(0,SensorType,0,GetDataCmd,DataType,18,(uint8_t *)&TCP_PAYLOAD[1]);
#endif

}

void Heart_Data_Send(void)
{
	uint8_t TCP_PAYLOAD[18] = {0};
	CollectionData electricdata;
	
	Get_ElectricData(&electricdata);
	myprintf("目前电压：%d v \n",electricdata.Voltage / 10);
	myprintf("目前电流：%.3f mA \n",electricdata.Current / 1000.0);
	myprintf("目前功率：%.1f w \n",electricdata.Power / 10.0);
	myprintf("目前电量：%.1f kwh \n",electricdata.Energy/ 100.0);
	myprintf("目前继电器状态：%d \n",!electricdata.RelayState);
	myprintf("目前调光值：%d \n",electricdata.LedPwm);
	myprintf("目前设备状态：%x \n",electricdata.Bl6526bState);
#if USE_LORA_UART_TO_INTERNET
	
	TCP_PAYLOAD[0] = GetDataCmd;		//命令码
	TCP_PAYLOAD[1] = electricdata.Voltage>>8;
	TCP_PAYLOAD[2] = electricdata.Voltage;
	TCP_PAYLOAD[3] = electricdata.Current>>24;
	TCP_PAYLOAD[4] = electricdata.Current>>16;
	TCP_PAYLOAD[5] = electricdata.Current>>8;
	TCP_PAYLOAD[6] = electricdata.Current;
	TCP_PAYLOAD[7] = electricdata.Power>>24;
	TCP_PAYLOAD[8] = electricdata.Power>>16;
	TCP_PAYLOAD[9] = electricdata.Power>>8;
	TCP_PAYLOAD[10] = electricdata.Power;
	TCP_PAYLOAD[11] = electricdata.Energy>>24;
	TCP_PAYLOAD[12] = electricdata.Energy>>16;
	TCP_PAYLOAD[13] = electricdata.Energy>>8;
	TCP_PAYLOAD[14] = electricdata.Energy;
	TCP_PAYLOAD[15] = !electricdata.RelayState;
	TCP_PAYLOAD[16] = electricdata.LedPwm;
	TCP_PAYLOAD[17] = electricdata.Bl6526bState;
	Platform_SendData(0,SensorType,0,GetDataCmd,HeartType,18,(uint8_t *)&TCP_PAYLOAD[1]);
#endif

#if USE_NB_UART_TO_INTERNET
	TCP_PAYLOAD[0] = electricdata.Voltage>>24;
	TCP_PAYLOAD[1] = electricdata.Voltage>>16;
	TCP_PAYLOAD[2] = electricdata.Voltage>>8;
	TCP_PAYLOAD[3] = electricdata.Voltage;
	TCP_PAYLOAD[4] = electricdata.Current>>24;
	TCP_PAYLOAD[5] = electricdata.Current>>16;
	TCP_PAYLOAD[6] = electricdata.Current>>8;
	TCP_PAYLOAD[7] = electricdata.Current;
	TCP_PAYLOAD[8] = electricdata.Energy>>24;
	TCP_PAYLOAD[9] = electricdata.Energy>>16;
	TCP_PAYLOAD[10] = electricdata.Energy>>8;
	TCP_PAYLOAD[11] = electricdata.Energy;
	TCP_PAYLOAD[12] = !electricdata.RelayState;
	TCP_PAYLOAD[13] = electricdata.LedPwm;
	TCP_PAYLOAD[14] = (uint8_t)(HARDWARE_VERSION>>8);
	TCP_PAYLOAD[15] = (uint8_t)HARDWARE_VERSION;
	TCP_PAYLOAD[16] = (uint8_t)(SOFTWARE_VERSION>>8);
	TCP_PAYLOAD[17] = (uint8_t)SOFTWARE_VERSION;
	Platform_SendData(Sn,Device_TYPE,Device_ID,Heart_Cmd,18,(uint8_t *)TCP_PAYLOAD);
#endif
}

extern uint8_t ETH_Link_flag;

void Platform_Task(void *argument)
{
	uint8_t * link_state;
	uint8_t lora_state = 0;
	BC26Status nbstatus;
	_myprintf("\r\nStart Platform_Task");
	
#if USE_ETH_TO_INTERNET
	link_state = &ETH_Link_flag;
	
	while(1)
	{
		if(*link_state == 1)
		{
			break;
		}
		vTaskDelay(100);
	}
	Create_TCP_connect();
	httpd_init();

#elif USE_LORA_UART_TO_INTERNET

#endif
	while(1)
	{
#if USE_ETH_TO_INTERNET
		if(*link_state == 1)
		{
			ntpTime();
			rebuild_TCP_connect();
			if((HAL_GetTick() - Heart_Tick)>=Herat_Rate*1000 || Heart_Tick == 0)
			{
				Heart_Tick = HAL_GetTick();
	//			if(Tcp_Connect_Flag == ConnectOnLine)
				{
					Herat_Data_Send();
				}
			}
		}
#elif USE_4G_UART_TO_INTERNET
		Get_Platform_Time_4G();
		GPRSDataPro();	//处理GPRS数据 等待消息队列阻塞100ms
		if((HAL_GetTick() - Heart_Tick)>=Herat_Rate*1000 || Heart_Tick == 0)
		{
			Heart_Tick = HAL_GetTick();
//			if(Tcp_Connect_Flag == ConnectOnLine)
			{
				Herat_Data_Send();
			}
		}
#elif USE_LORA_UART_TO_INTERNET
		Get_or_Set_LoraStatus(0,&lora_state);	//获取Lora联网状态
		LoraDataProcess();	//处理Lora数据 等待消息队列阻塞100ms
		if(lora_state == LoraOnLine)	//Lora在线下在发送数据
		{
			if((getRunTimeCounterValue() - Heart_Tick)>=Herat_Rate*1000 || Heart_Tick == 0)
			{
				Heart_Tick = getRunTimeCounterValue();
	//			if(Tcp_Connect_Flag == ConnectOnLine)
				{
					Heart_Data_Send();
				}
			}
		}

#elif USE_NB_UART_TO_INTERNET
		Get_or_Set_NBStatus(0,&nbstatus);	//获取NB联网状态
		NBDataProcess();//处理Lora数据 等待消息队列阻塞100ms
		if(nbstatus.netstatus == ONLINE)	//nb在线在发送数据
		{
			if((getRunTimeCounterValue() - Heart_Tick)>=Herat_Rate*1000 || Heart_Tick == 0)
			{
				Heart_Tick = getRunTimeCounterValue();
	//			if(Tcp_Connect_Flag == ConnectOnLine)
				{
					Heart_Data_Send();
				}
			}
		}
#endif

		vTaskDelay(100);
	}
}

void CreatPlatformTask(void)
{
	BaseType_t Platform_TaskID;
	
	_myprintf("\r\n CreatPlatformTask");
	Platform_TaskID = xTaskCreate (Platform_Task, "Platform", Platform_StkSIZE, NULL, Platform_TaskPrio, NULL);
	if(pdPASS != Platform_TaskID)
	{
        _myprintf("\r\n CreatPlatformTask creat error");
        while(1);
    }
}
