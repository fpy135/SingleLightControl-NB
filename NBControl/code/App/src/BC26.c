#include "bc26.h"
#include "string.h"
#include "SysConfig.h"
#include "bsp_uart.h"
#include "bsp_wdt.h"
#include "bsp_rtc.h"

#include "Platform.h"
#include "mystring.h"
#include "Unixtimer.h"

SemaphoreHandle_t NB_CMD_Mutex;
SemaphoreHandle_t NB_STATUS_Mutex;
SemaphoreHandle_t NB_ACK_Semaphore;
SemaphoreHandle_t NB_CMD_Semaphore;
QueueHandle_t NB_CMD_Queue;
QueueHandle_t NB_RECEIVE_Queue;
QueueHandle_t NB_SEND_Queue;

extern uint8_t platform_recvbuf[PLATFORM_BUFF_SIZE];
extern uint8_t platform_sendbuf[PLATFORM_BUFF_SIZE];

extern void SetRTC(rtc_time_t *rtc_time);

char *strx,*extstrx;
uint8_t NBBufTmp[NBBuf_Size];
uint16_t NBRecvCount = 0;

//uint8_t	ServerIP[4] ={106,54,98,19};
//uint32_t ServerPort = 44233;
//uint8_t	ServerIP[4] ={47,98,223,38};
//uint32_t ServerPort = 9000;

uint8_t	ServerIP[4] ={121,199,69,67};
uint32_t ServerPort = 9001;

extern char  RxBuffer[100],RxCounter;
BC26Status BC26_Status;
unsigned char socketnum=0;//当前的socket号码

/*=====================================================
 * 函数功能: 	读写NB 状态
 * 输入参数:	get_or_set： 0：读取   1：写入   
 * 输出参数: 
 * 返    回:	
=====================================================*/
void Get_or_Set_NBStatus(uint8_t get_or_set, BC26Status *nbstatus)
{
	xSemaphoreTake(NB_STATUS_Mutex,portMAX_DELAY);
	if(get_or_set == 0)
	{
		nbstatus->CSQ = BC26_Status.CSQ;
		nbstatus->netstatus = BC26_Status.netstatus;
		nbstatus->rsrq = BC26_Status.rsrq;
		nbstatus->rsrp = BC26_Status.rsrp;		
	}
	else
	{
		BC26_Status.CSQ = nbstatus->CSQ;
		BC26_Status.netstatus = nbstatus->netstatus;
		BC26_Status.rsrq = nbstatus->rsrq;
		BC26_Status.rsrp = nbstatus->rsrp;
	}
	xSemaphoreGive(NB_STATUS_Mutex);
}


/*=====================================================
 * 函数功能: 	GPRS接收数据队列发送 中断调用
 * 输入参数:	长度 + 内容
 * 输出参数: 
 * 返    回:	1 失败, 0 成功
=====================================================*/
uint8_t MSG_NBReceiveDataFromISR(uint8_t *buf, uint16_t len)
{
	uint8_t Data[len+2];
	//申请缓存

	//复制消息
	Data[0] = len;
	Data[1] = len>>8;
	memcpy((void *)&Data[2],(const void *)buf,len);
	//发送消息

	if(xQueueSendToBackFromISR(NB_RECEIVE_Queue, Data, 0) != pdPASS)
	{
		_myprintf("\r\nxQueueSend error, GPRS_RECEIVE_Queue");
		return 1;
	}

	return 0;
}


void Clear_Buffer(void)//清空缓存
{
		uint8_t i;
		myprintf_buf_unic(NBBuf,NBRecvCount);
		for(i=0;i<NBRecvCount;i++)
			NBBuf[i]=0;//缓存
		NBRecvCount=0;
//		WDG_Feed();//喂狗
	
}

/*=====================================================
 * 函数功能: 	NB发送命令
 * 输入参数:	const char *cmd, u16 len, char *str, u16 Wtime
 * 输出参数: 
 * 返    回:	1 失败, 0 成功
=====================================================*/
uint8_t NBSendCMD(const char *cmd, uint16_t len, char *str, uint16_t Wtime)
{
	uint8_t *p;
	uint8_t ret = 0;
	BaseType_t res;
	
	xSemaphoreTake(NB_CMD_Mutex,portMAX_DELAY);
	p = pvPortMalloc(sizeof(uint8_t)*128);
	xSemaphoreTake(NB_CMD_Semaphore, ( TickType_t ) 0);	//获取信号量
	xQueueReceive( NB_CMD_Queue, (void *)p, ( TickType_t ) 0);	//获取消息队列 清空队列
	vPortFree(p);
//	myprintf("\r\n LoraSendCMD\r\n");
//	PrintWrite((uint8_t *)cmd,len);
	NBSend((uint8_t *)cmd,len);
	if(xQueueSend(NB_CMD_Queue, (void *)str, 0) != pdPASS)		//发送需要检索的内容消息队列
	{
		myprintf("\r\nx QueueSend error, NB_CMD_Queue");
		ret = 0;
	}
	res = xSemaphoreTake(NB_CMD_Semaphore, ( TickType_t )(100 * Wtime));
	if(res == pdTRUE)	//等待检索结果
	{
//		myprintf("\r\n NB_CMD_Semaphore OK, %X", res);
		ret = 1;
	}
	else
	{
//		myprintf("\r\n NB_CMD_Semaphore NOK, %X", res);
		ret = 0;
	}
	xSemaphoreGive(NB_CMD_Mutex);
	return ret;
}

uint8_t GET_CSQ(void)
{
	uint8_t ret = 0;
	uint8_t rssi = 99;
	uint8_t rsrq = 255;
	uint8_t rsrp = 255;
	BC26Status nbstatus;
	
	ret = NBSendCMD("AT+CESQ\r\n",9,"+CESQ",300/100);//查看获取CSQ值
//	while(ret == 0)
//	{
//		ret = NBSendCMD("AT+CESQ\r\n",9,"+CESQ",300/100);//查看获取CSQ值
//		
//	}
	Get_or_Set_NBStatus(0,&nbstatus);	//读出NB联网状态
	if(ret == 1)
	{
		uint16_t p, q;
		p = GetNStr(NBBuf, NBCount,':', 1);
		if(p != 0xffff)
		{
			Str2Num(&NBBuf[p+2], 2, (uint32_t *)&rssi);
				nbstatus.CSQ = rssi;
			p = GetNStr(NBBuf, NBCount,',', 4);
			if(p != 0xffff)
			{
				q = GetNStr(NBBuf, NBCount,',', 5);
				Str2Num(&NBBuf[p+1], q-p, (uint32_t *)&rsrq);
				nbstatus.rsrq = rsrq;
				Str2Num(&NBBuf[q+1], NBCount-1-q-8, (uint32_t *)&rsrp);
				nbstatus.rsrp = rsrp;
				
				Get_or_Set_NBStatus(1,&nbstatus);	//写入NB联网状态
			}
		}
		
//		myprintf("\r\n信号强度：rssi:%d    rsrq:%d    rsrp:%d, ",rssi,rsrq,rsrp);

	}
	return rssi;
}

uint8_t BC26_CheckSelf(void)
{
	uint8_t ret = 0;
	
	
	ret = NBSendCMD("AT\r\n",4,"OK",300/100); 
	ret = NBSendCMD("AT\r\n",4,"OK",300/100); 
	ret = NBSendCMD("AT\r\n",4,"OK",300/100); 
	ret = NBSendCMD("AT\r\n",4,"OK",300/100); 
	return ret;
}

void OPEN_BC26_Power(void)
{
	uint8_t ret = 0;
//	if(ret==0)
	{
		NREST(1);
		vTaskDelay(100);
		NREST(0);
		PWRKEY(1);//拉低
		HAL_Delay(1000);
		PWRKEY(0);//拉高正常开机
	}
}

void OPEN_BC26(void)
{
	char *strx;
	uint8_t ret = 0;

//	if(ret==0)
	{
		NREST(1);
		vTaskDelay(100);
		NREST(0);
		vTaskDelay(100);
		PWRKEY(1);//拉低
		vTaskDelay(1000);
		PWRKEY(0);//拉高正常开机
	}
	NBSendCMD("AT\r\n",4,"OK",300/100);   
}


int8_t Get_NTP_Time(void)
{
	uint8_t ret = 0;
	uint16_t p;
	 //使用域名为 ntp1.aliyun.com 的 NTP 服务器同步本地时间。
	ret = NBSendCMD("AT+QNTP=1,\"ntp1.aliyun.com\"\r\n",29,"+QNTP:",3000/100); 
	//ret = NBSendCMD("AT+QNTP=1,\"ntp1.aliyun.com\"\r\n",29,"OK",300/100); 
    if(ret==0)
    {
		//ret = NBSendCMD("AT+QNTP=1,\"ntp1.aliyun.com\"\r\n",29,"+QNTP: 0",300/100); 
		if(ret==0)
			return -1;
    }
	p = StrFindString(NBBuf, NBCount,"+QNTP: 0", 8);
	if(p != 0xffff)
	{
		char ZZ;
		rtc_time_t rtc_time;
		rtc_time.ui8Year = (NBBuf[p+10]-'0')*10+(NBBuf[p+11]-'0')+2000;
		rtc_time.ui8Month = (NBBuf[p+13]-'0')*10+(NBBuf[p+14]-'0');
		rtc_time.ui8DayOfMonth = (NBBuf[p+16]-'0')*10+(NBBuf[p+17]-'0');
		
		rtc_time.ui8Hour = (NBBuf[p+19]-'0')*10+(NBBuf[p+20]-'0');
		rtc_time.ui8Minute = (NBBuf[p+22]-'0')*10+(NBBuf[p+23]-'0');
		rtc_time.ui8Second = (NBBuf[p+25]-'0')*10+(NBBuf[p+26]-'0');
		if((NBBuf[p+27]) == '-')
			ZZ = -((NBBuf[p+28]-'0')*10+(NBBuf[p+29]-'0'));
		if((NBBuf[p+27]) == '+')
			ZZ = (NBBuf[p+28]-'0')*10+(NBBuf[p+29]-'0');
		rtc_time.ui8Hour += ZZ/4;
		rtc_time.ui8Minute += ZZ%4*15;
		SetRTC(&rtc_time);
		return 0;
	}
	return -1;
}

/*
int BC26_Init(void)
{
	uint8_t ret = 0;
	BC26Status nbstatus;
	
	ret = NBSendCMD("AT\r\n",4,"OK",300/100); 
    while(ret==0)
    {
        ret = NBSendCMD("AT\r\n",4,"OK",300/100); 
    }
	ret = NBSendCMD("ATE0\r\n",6,"OK",300/100); 	//关闭回显
    while(ret==0)
    {
        ret = NBSendCMD("ATE0\r\n",6,"OK",300/100); 
    }
	NBSendCMD("ATI\r\n",5,"OK",300/100); 
	ret = NBSendCMD("AT+CFUN=1\r\n",11,"OK",300/100); //开启射频
	while(ret==0)
	{
		ret = NBSendCMD("AT+CFUN=1\r\n",11,"OK",300/100); //开启射频
	}
	ret = NBSendCMD("AT+QICLOSE=0\r\n",14,"OK",300/100); //关闭连接
	ret = NBSendCMD("AT+CIMI\r\n",9,"460",300/100); //获取卡号，类似是否存在卡的意思，比较重要。
    
    while(ret==0)
    {
		ret = NBSendCMD("AT+CIMI\r\n",9,"460",300/100); //获取卡号，类似是否存在卡的意思，比较重要。
    }
	
	ret = NBSendCMD("AT+QICFG=\"dataformat\",1,1\r\n",27,"OK",300/100);//配置发送，接收的数据格式为十六进制格式
	while(ret==0)
	{
		ret = NBSendCMD("AT+QICFG=\"dataformat\",1,1\r\n",27,"OK",300/100);//配置发送，接收的数据格式为十六进制格式
	}
	
	ret = NBSendCMD("AT+CGATT=1\r\n",12,"OK",300/100); //激活网络，PDP
	ret = NBSendCMD("AT+CGATT?\r\n",11,"+CGATT: 1",300/100);//查询激活状态
	while(ret==0)//返回1,表明注网成功
	{
		ret = NBSendCMD("AT+CGATT?\r\n",11,"+CGATT: 1",300/100);//查询激活状态
	}
	ret = NBSendCMD("AT+CEREG?\r\n",11,"+CEREG: 0,1",300/100);//查询EPS 网络注册状态
	while(ret==0)//返回1,表明注网成功
	{
		ret = NBSendCMD("AT+CEREG?\r\n",11,"+CEREG: 0,1",300/100);//查询EPS 网络注册状态
	}
	ret = NBSendCMD("AT+CGPADDR?\r\n",13,"+CGPADDR: 1",300/100);//获取模块 IP 地址
	while(ret==0)
	{
		ret = NBSendCMD("AT+CGPADDR?\r\n",13,"+CGPADDR: 1",300/100);//获取模块 IP 地址
	}
	nbstatus.CSQ = GET_CSQ();//查看获取CSQ值
	if((nbstatus.CSQ==99))//说明扫网失败
	{
//		while(1)
		{
			nbstatus.netstatus=OFFLINE;
			myprintf("信号搜索失败，请查看原因!\r\n");
			
		}
	}
	Get_or_Set_NBStatus(1,&nbstatus);	//写入NB联网状态
} */


int BC26_Init(void)
{
	uint8_t ret = 0;
	uint8_t recnt = 0;
	uint8_t sim_cnt = 0;
	BC26Status nbstatus;
	
	while(1)
	{
		recnt++;
		if( recnt > 5 )
		{
			myprintf("AT Error!\r\n");
			return -1;
		}
		vTaskDelay(1000);
		
		ret = NBSendCMD("AT\r\n",4,"OK",300/100); 
		if(ret == 0)
		{
			//ret = NBSendCMD("AT\r\n",4,"OK",300/100); 
			continue;
		}
		ret = NBSendCMD("ATE0\r\n",6,"OK",300/100); 	//关闭回显
		if(ret == 0)
		{
			//ret = NBSendCMD("ATE0\r\n",6,"OK",300/100); 
			continue;
		}
		NBSendCMD("ATI\r\n",5,"OK",300/100); 
		ret = NBSendCMD("AT+CFUN=1\r\n",11,"OK",300/100); //开启射频
		if(ret == 0)
		{
			//ret = NBSendCMD("AT+CFUN=1\r\n",11,"OK",300/100); //开启射频
			continue;
		}
		ret = NBSendCMD("AT+QICLOSE=0\r\n",14,"OK",300/100); //关闭连接
		
//		while(1)
//		{
//			sim_cnt++;
//			ret = NBSendCMD("AT+CPIN?\r\n",10,"+CPIN: READY",5000/100); //查询 USIM 卡的 PIN 码是否已解
//			if(ret == 1)
//				break;
////			vTaskDelay(1000);
////			if(sim_cnt > 2)
////			{
////				break;
////			}
//			
//		}
		ret = NBSendCMD("AT+CPIN?\r\n",10,"+CPIN: READY",5000/100); //查询 USIM 卡的 PIN 码是否已解
		if(ret == 0)
			continue;
		ret = NBSendCMD("AT+CIMI\r\n",9,"460",300/100); //获取卡号，类似是否存在卡的意思，比较重要。
		
		if(ret == 0)
		{
			//ret = NBSendCMD("AT+CIMI\r\n",9,"460",300/100); //获取卡号，类似是否存在卡的意思，比较重要。
			continue;
		}
		
		ret = NBSendCMD("AT+QICFG=\"dataformat\",1,1\r\n",27,"OK",300/100);//配置发送，接收的数据格式为十六进制格式
		if(ret == 0)
		{
			//ret = NBSendCMD("AT+QICFG=\"dataformat\",1,1\r\n",27,"OK",300/100);//配置发送，接收的数据格式为十六进制格式
			continue;
		}
		
		ret = NBSendCMD("AT+CGATT=1\r\n",12,"OK",300/100); //激活网络，PDP
		ret = NBSendCMD("AT+CGATT?\r\n",11,"+CGATT: 1",300/100);//查询激活状态
		if(ret == 0)//返回1,表明注网成功
		{
			//ret = NBSendCMD("AT+CGATT?\r\n",11,"+CGATT: 1",300/100);//查询激活状态
			continue;
		}
		ret = NBSendCMD("AT+CEREG?\r\n",11,"+CEREG: 0,1",300/100);//查询EPS 网络注册状态
		if(ret == 0)//返回1,表明注网成功
		{
			//ret = NBSendCMD("AT+CEREG?\r\n",11,"+CEREG: 0,1",300/100);//查询EPS 网络注册状态
			continue;
		}
		ret = NBSendCMD("AT+CGPADDR?\r\n",13,"+CGPADDR: 1",300/100);//获取模块 IP 地址
		if(ret == 0)
		{
			//ret = NBSendCMD("AT+CGPADDR?\r\n",13,"+CGPADDR: 1",300/100);//获取模块 IP 地址
			continue;
		}
		nbstatus.CSQ = GET_CSQ();//查看获取CSQ值
		if(nbstatus.CSQ == 99 || nbstatus.CSQ < 10)//说明扫网失败
		{
	//		while(1)
			{
				nbstatus.netstatus=OFFLINE;
				myprintf("信号搜索失败，请查看原因!\r\n");
				
			}
		}
		Get_or_Set_NBStatus(1,&nbstatus);	//写入NB联网状态
		
		return 0;
	}
}


void BC26_CreateTCPSokcet(void)//创建sokcet
{
	uint8_t ret = 0;
	
	ret = NBSendCMD("AT\r\n",4,"OK",300/100); 
    while(ret==0)
    {
        ret = NBSendCMD("AT\r\n",4,"OK",300/100); 
    }
    printf("AT+QIOPEN=1,0,\"TCP\",\"47.92.146.210\",8888,1234,1\r\n");//创建连接TCP,输入IP以及服务器端口号码 ,采用直接吐出的方式
    vTaskDelay(300);//TCP连接模式，会检测连接状态
		strx=strstr((const char*)NBBuf,(const char*)"+QIOPEN: 0,0");//检查是否登陆成功
 	while(strx==NULL)
		{
            strx=strstr((const char*)NBBuf,(const char*)"+QIOPEN: 0,0");//检查是否登陆成功
            vTaskDelay(100);
		}  

    Clear_Buffer();	
    
}

void BC26_Close_Connect(uint8_t cipType)//关闭连接
{
	uint8_t ret = 0;
	char tmp[50] = {0};
	uint8_t len = 0;
	len = sprintf(tmp,"AT+QICLOSE=%d\r\n", cipType);
	ret = NBSendCMD(tmp,len,"CLOSE OK",300/100); 
    while(ret==0)
    {
        ret = NBSendCMD(tmp,len,"CLOSE OK",300/100);
    }
	myprintf("\r\n服务器断开连接");
}


int8_t BC26_Connect_Platform(uint8_t cipType, uint8_t * serverIP, uint32_t serverPort)//连接平台,直吐模式
{
	uint8_t ret = 0;
	char cipStr[4] = {0};
	char tmp[50] = {0};
	uint8_t len = 0;
	uint8_t recnt = 0;

	if(cipType == TCP)
	{
		memcpy((void *)cipStr, "TCP", sizeof("TCP"));
	}
	else if(cipType == UDP)
	{
		memcpy((void *)cipStr, "UDP", sizeof("UDP"));
	}
	//创建连接TCP,输入IP以及服务器端口号码 ,采用直接吐出的方式
	len = sprintf(tmp,"AT+QIOPEN=%d,0,\"%s\",\"%d.%d.%d.%d\",%d,1234,1\r\n", cipType, cipStr, serverIP[0], serverIP[1], serverIP[2], serverIP[3], serverPort);

	ret = NBSendCMD(tmp,len,"+QIOPEN: 0,0",300/100); //检查是否登陆成功
    while(ret==0)
    {
		uint16_t p;
		 //可查询连接状态信息
		recnt++;
		if(recnt>=5)
		{
			myprintf("\r\n%s服务器连接失败", cipStr);
			return -1;
		}
		p = StrFindString(NBBuf, NBCount,"+QIOPEN: 0,0", 12);
		if(p!=0xffff)
			break;
		vTaskDelay(1000);
    }
	myprintf("\r\n%s服务器连接成功", cipStr);
	return 0;
}


void BC26_Senddata(uint8_t *len,uint8_t *data)//字符串形式
{
	
    printf("AT+QISEND=0,%s,%s\r\n",len,data);
	  vTaskDelay(300);
// 	while(strx==NULL)
//		{
//            strx=strstr((const char*)NBBuf,(const char*)"SEND OK");//检查是否发送成功
//            vTaskDelay(100);
//		}  
//	  Uart1_SendStr(NBBuf);
//		RxCounter=0;
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

uint8_t BC26_Senddatahex(uint8_t *senddata,uint8_t sendlen)//发送十六进制数据
{
	uint8_t ret = 0;
	char tmp[128] = {0};
	uint8_t tmplen = 0;
	tmplen = sprintf(tmp,"AT+QISENDEX=0,%d,",sendlen);
	Data_Convert(senddata,sendlen,(uint8_t *)&tmp[tmplen]);
	tmp[tmplen+sendlen*2] = '\r';
	tmp[tmplen+sendlen*2+1] = '\n';
	if(tmplen+sendlen*2+2 > 128)
	{
		myprintf("\r\n发送数据长度过长");
		return 0;
	}
	ret = NBSendCMD(tmp,tmplen+sendlen*2+2,"SEND OK",300/100);
//    printf("AT+QISENDEX=0,%s,%s\r\n",len,data);
//        vTaskDelay(300);
// 	while(strx==NULL)
//		{
//            strx=strstr((const char*)NBBuf,(const char*)"SEND OK");//检查是否发送成功
//            vTaskDelay(100);
//		}  
//     Clear_Buffer();	
	return ret;
}

void NBDataProcess(void)
{
	uint32_t num;
	uint16_t p,q;
	uint8_t *msgdata = NULL;
	uint16_t NBRecviveLen = 0;
	uint8_t *s;
		
	if(xQueueReceive(NB_RECEIVE_Queue, (void *)&platform_recvbuf, ( TickType_t )0) != pdPASS){
		return;
	}
	NBRecviveLen = platform_recvbuf[0] | (platform_recvbuf[1]<<8);
	msgdata = &platform_recvbuf[2];

	if((NBRecviveLen == 0) || (NBRecviveLen > PLATFORM_BUFF_SIZE))
	{
		myprintf("\r\n NB长度错误:%d",NBRecviveLen);
		return;
	}
#if 1
	myprintf("\r\nAT接收, len: %d\r\n",NBRecviveLen);
	PrintWrite(&platform_recvbuf[2], NBRecviveLen);
#endif
	s = pvPortMalloc(sizeof(uint8_t)*128);
	if(xQueueReceive( NB_CMD_Queue, (void *)s, ( TickType_t ) 0) == pdPASS)	//获取消息队列
	{
		p = StrFindString(&platform_recvbuf[2], NBRecviveLen, s, strlen((const char *)s));		
		if(p != 0xffff)
		{
//				myprintf("\r\nAT接收, len: %d",NBRecviveLen);
//				PrintWrite(&platform_recvbuf[2], NBRecviveLen);
			if(NBRecviveLen > NBBuf_Size)
			{
				NBCount = NBBuf_Size;
				memcpy((void *)NBBuf, (const void *)&platform_recvbuf[2], NBBuf_Size); 
			}
			else
			{
				NBCount = NBRecviveLen;
				memcpy((void *)NBBuf, (const void *)&platform_recvbuf[2], NBRecviveLen); 
			}
			xSemaphoreGive(NB_CMD_Semaphore);
		}
		else
		{
//				uint8_t len=0;
//				uint8_t *tmp;
//				tmp = pvPortMalloc(sizeof(uint8_t)*(strlen((const char *)s)));
//				memcpy(tmp,s,strlen((const char *)s));
			xQueueSend(NB_CMD_Queue, (void *)s, 0);		//当前数据包无需要内容，重新发送需要检索的内容消息队列
//				vPortFree(tmp);
		}
	}
	else
	{
		if((p = StrFindString(&platform_recvbuf[2], NBRecviveLen,(uint8_t *)"+QIURC: \"recv\"", strlen((const char *)"+QIURC: \"recv\""))) != 0xffff)
		{	//接收数据
			uint8_t str = 0x0d;
			uint8_t len_num = 0;
			uint32_t rev_len = 0;
			q = GetNStr(&platform_recvbuf[2],NBRecviveLen, ',', 2);
			len_num = GetNStr(&platform_recvbuf[q+2+1],NBRecviveLen-q-1, str, 1);
			Str2Num(&platform_recvbuf[q+2+1],len_num,&rev_len);
			Data_Recover(&platform_recvbuf[2+q+1+len_num+2],rev_len,NBBuf);
			Platform_Data_Process(NBBuf);

		}
		else if((p = StrFindString(&platform_recvbuf[2], NBRecviveLen,(uint8_t *)"+QIURC: \"closed\"", strlen((const char *)"+QIURC: \"closed\""))) != 0xffff)
		{	//连接断开
			BC26Status nbstatus;
			Get_or_Set_NBStatus(0,&nbstatus);
			nbstatus.netstatus = OFFLINE;		//将网络状态设为离线
			Get_or_Set_NBStatus(1,&nbstatus);	//获取NB联网状态
		}
		else if((p = StrFindString(&platform_recvbuf[2], NBRecviveLen,(uint8_t *)"+CPIN: NOT READY", strlen((const char *)"+CPIN: NOT READY"))) != 0xffff)
		{	//sim卡丢失
			BC26Status nbstatus;
			Get_or_Set_NBStatus(0,&nbstatus);
			nbstatus.netstatus = OFFLINE;		//将网络状态设为离线
			Get_or_Set_NBStatus(1,&nbstatus);	//获取NB联网状态
		}
		else
		{
			if(NBRecviveLen > NBBuf_Size)
			{
				NBCount = NBBuf_Size;
				memcpy((void *)NBBuf, (const void *)&platform_recvbuf[2], NBBuf_Size); 
			}
			else
			{
				NBCount = NBRecviveLen;
				memcpy((void *)NBBuf, (const void *)&platform_recvbuf[2], NBRecviveLen); 
			}
		}
	}
	vPortFree(s);
}

/*====================================================
 * 函数功能: 	GPRS任务
 * 输入参数:	void *c
 * 输出参数: 
 * 返    回:	无
=====================================================*/
void NB_Task(void *argument)
{
	int8_t connect_status;
	uint8_t lora_status_new;
	uint8_t offline_cnt = 0;
	uint8_t get_status_cnt = 0;
	uint32_t tcp_heart_tick = 0;
	static uint32_t get_time_tick = 0;
	uint8_t TT = 0X00;
	BC26Status nbstatus;
	static uint16_t connet_cnt=0;
	
BC26Rest:
	myprintf("\r\n NB复位");
	OPEN_BC26();
	if( BC26_Init() < 0)//对设备初始化
	{
		goto BC26Rest;
	}
ReConnect:
    connect_status = BC26_Connect_Platform(TCP,ServerIP, ServerPort);//创建一个SOCKET连接
	if(connect_status < 0)
	{
		nbstatus.netstatus = OFFLINE;
		connet_cnt++;
		myprintf("\r\n第%d重新连接服务器",connet_cnt);
		//if(connet_cnt >= 5)
		{
			goto BC26Rest;
		}
	}
	nbstatus.netstatus = ONLINE;
	Get_or_Set_NBStatus(1,&nbstatus);	//获取NB联网状态
	while(1)
	{
		GET_CSQ();		//信号强度
		Get_or_Set_NBStatus(0,&nbstatus);	//获取NB联网状态
		if(nbstatus.netstatus == OFFLINE)
		{
			BC26_Close_Connect(TCP);
			goto ReConnect;
		}
		//定时同步时间
		if((getRunTimeCounterValue() - get_time_tick)>=3600*24*3*1000 || get_time_tick == 0)
		{
			get_time_tick = getRunTimeCounterValue();
			if(Get_NTP_Time() < 0)
			{
				get_time_tick = 0;
			}
		}
		
		
		if(xQueueReceive(NB_SEND_Queue, (void *)&platform_sendbuf, ( TickType_t )1000) == pdPASS)
		{
			tcp_heart_tick = getRunTimeCounterValue();
			connect_status = BC26_Senddatahex((uint8_t *)platform_sendbuf, 10+platform_sendbuf[9]+2);		//10为负载以前的固定数据的数据长度+负载数据长度+crc
			if(connect_status == 0)
			{
				offline_cnt++;
				if(offline_cnt>=8)	//连续2分钟tcp失去连接，则重新建立连接
				{
					offline_cnt = 0;
					nbstatus.netstatus = OFFLINE;
					Get_or_Set_NBStatus(1,&nbstatus);	//获取NB联网状态
				}
			}
			else
				offline_cnt = 0;
		}
		else if((getRunTimeCounterValue() - tcp_heart_tick)>=15*1000 || tcp_heart_tick == 0)
		{
			tcp_heart_tick = getRunTimeCounterValue();
			myprintf("发送 tcp 保活数据包\r\n");
			{
//				BC26_Senddata("1",&TT);
				connect_status = BC26_Senddatahex(&TT,1);
				if(connect_status == 0)
				{
					offline_cnt++;
					if(offline_cnt>=8)	//连续2分钟tcp失去连接，则重新建立连接
					{
						offline_cnt = 0;
						nbstatus.netstatus = OFFLINE;
						Get_or_Set_NBStatus(1,&nbstatus);	//获取NB联网状态
					}
				}
				else
					offline_cnt = 0;
			}
		}
//		vTaskDelay(1000);
	}
}

void CreatNBTask(void)
{
	BaseType_t NB_TaskID;
	_myprintf("\r\n CreatNBTask");
	NB_TaskID = xTaskCreate (NB_Task, "NB", NB_StkSIZE, NULL, NB_TaskPrio, NULL);
	if(pdPASS != NB_TaskID)
	{
		while(1);
	}
}

