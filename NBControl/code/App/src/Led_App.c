#include "Led_App.h"
#include "SysConfig.h"
#include "bsp_uart.h"
#include "bsp_timer.h"

#include "bsp_rtc.h"
#include "Unixtimer.h"
#include "BL6523GX.h"
#include "mystring.h"
//#include "LoraApp.h"
#include "Platform.h"
#include "bc26.h"
#include "IDConfig.h"

extern void GetRTC(rtc_time_t *rtc_time);
extern void SetRTC(rtc_time_t *rtc_time);
extern void SetRTC_BCD(rtc_time_t *rtc_time);
extern void GetRTC_BCD(rtc_time_t *rtc_time);

QueueHandle_t LedShow_Queue;	//演示模式
SemaphoreHandle_t TimeControlMutex; //TimeControlMutex
SemaphoreHandle_t ElectricDataMutex; //ElectricDataMutex

TimeControl_Type TimeControl_Data;	//时控模式结构体
LoraTimeControl_Type LoraTimeControl_Data;	//时控模式结构体

xTaskHandle pvCreatedTask_Led;

uint8_t get_timecontrol_unix_flag = 0;//首次获取unix时间戳

uint8_t LedPwm __no_init;
uint8_t targetBrightness = 0;
uint8_t LedStatus __no_init; 
uint8_t ledshowFlag __no_init;		//演示模式

extern void Get_ElectricData(CollectionData *electricdata);
extern void Write_ElectricData(CollectionData *electricdata);

uint8_t Get_Timecontrol_Unix_Flag = 0;

void Write_TimeControl_flag(uint8_t flag)
{
	xSemaphoreTake (TimeControlMutex, portMAX_DELAY);
	
	Get_Timecontrol_Unix_Flag = flag;
	xSemaphoreGive (TimeControlMutex);
}

void Get_ElectricData(CollectionData *electricdata)
{
	xSemaphoreTake (ElectricDataMutex, portMAX_DELAY);
	
	electricdata->Voltage = ElectricData.Voltage;
	electricdata->RelayState = LedStatus;
	electricdata->LedPwm = LedPwm;
	electricdata->Power = ElectricData.Power;
	electricdata->Energy = ElectricData.Energy;
	electricdata->Current = ElectricData.Current;
	electricdata->Bl6526bState = ElectricData.Bl6526bState;
	xSemaphoreGive (ElectricDataMutex);
}

void Write_ElectricData(CollectionData *electricdata)
{
	xSemaphoreTake (ElectricDataMutex, portMAX_DELAY);
	
	ElectricData.Voltage = electricdata->Voltage;
	ElectricData.RelayState = LedStatus;
	ElectricData.LedPwm = LedPwm;
	ElectricData.Power = electricdata->Power;
	ElectricData.Energy = electricdata->Energy;
	ElectricData.Current = electricdata->Current;
	ElectricData.Bl6526bState = electricdata->Bl6526bState;               
	xSemaphoreGive (ElectricDataMutex);
}


void Write_TimeControl_Data(TimeControl_Type *timecontrol_data)
{
	xSemaphoreTake (TimeControlMutex, portMAX_DELAY);
	
	TimeControl_Data.start_hour = timecontrol_data->start_hour;
	TimeControl_Data.start_min = timecontrol_data->start_min;
	TimeControl_Data.phase1_Pwm = timecontrol_data->phase1_Pwm;
	TimeControl_Data.phase1_time = timecontrol_data->phase1_time;
	TimeControl_Data.phase2_Pwm = timecontrol_data->phase2_Pwm;
	TimeControl_Data.phase2_time = timecontrol_data->phase2_time;
	TimeControl_Data.phase3_Pwm = timecontrol_data->phase3_Pwm;
	TimeControl_Data.phase3_time = timecontrol_data->phase3_time;
	TimeControl_Data.phase4_Pwm = timecontrol_data->phase4_Pwm;
	TimeControl_Data.phase4_time = timecontrol_data->phase4_time;
	Get_Timecontrol_Unix_Flag = 0;
	xSemaphoreGive (TimeControlMutex);
}

uint8_t Get_TimeControl_Data(TimeControl_Type *timecontrol_data)
{
	uint8_t flag = 0;
	xSemaphoreTake (TimeControlMutex, portMAX_DELAY);
	
	timecontrol_data->start_hour = TimeControl_Data.start_hour;
	timecontrol_data->start_min = TimeControl_Data.start_min;
	timecontrol_data->phase1_Pwm = TimeControl_Data.phase1_Pwm;
	timecontrol_data->phase1_time = TimeControl_Data.phase1_time;
	timecontrol_data->phase2_Pwm = TimeControl_Data.phase2_Pwm;
	timecontrol_data->phase2_time = TimeControl_Data.phase2_time;
	timecontrol_data->phase3_Pwm = TimeControl_Data.phase3_Pwm;
	timecontrol_data->phase3_time = TimeControl_Data.phase3_time;
	timecontrol_data->phase4_Pwm = TimeControl_Data.phase4_Pwm;
	timecontrol_data->phase4_time = TimeControl_Data.phase4_time;
	flag = Get_Timecontrol_Unix_Flag;
	xSemaphoreGive (TimeControlMutex);
	return flag;
}

void Write_LoraTimeControl_Data(LoraTimeControl_Type *loratimecontrol_data)
{
	xSemaphoreTake (TimeControlMutex, portMAX_DELAY);
	get_timecontrol_unix_flag = 0;  //在更新时间策略的同时，需要跟新时间策略的时间戳
	LoraTimeControl_Data.led_status1 = loratimecontrol_data->led_status1;
	LoraTimeControl_Data.led_status2 = loratimecontrol_data->led_status2;
	LoraTimeControl_Data.led_status3 = loratimecontrol_data->led_status3;
	LoraTimeControl_Data.led_status4 = loratimecontrol_data->led_status4;
	LoraTimeControl_Data.led_status5 = loratimecontrol_data->led_status5;
	if(loratimecontrol_data->phase1_Pwm1>100)
		loratimecontrol_data->phase1_Pwm1 = 100;
	if(loratimecontrol_data->phase1_Pwm2>100)
		loratimecontrol_data->phase1_Pwm2 = 100;
	if(loratimecontrol_data->phase1_Pwm3>100)
		loratimecontrol_data->phase1_Pwm3 = 100;
	if(loratimecontrol_data->phase1_Pwm4>100)
		loratimecontrol_data->phase1_Pwm4 = 100;
	if(loratimecontrol_data->phase1_Pwm5>100)
		loratimecontrol_data->phase1_Pwm5 = 100;
	LoraTimeControl_Data.phase1_Pwm1 = loratimecontrol_data->phase1_Pwm1;
	LoraTimeControl_Data.phase1_Pwm2 = loratimecontrol_data->phase1_Pwm2;
	LoraTimeControl_Data.phase1_Pwm3 = loratimecontrol_data->phase1_Pwm3;
	LoraTimeControl_Data.phase1_Pwm4 = loratimecontrol_data->phase1_Pwm4;
	LoraTimeControl_Data.phase1_Pwm5 = loratimecontrol_data->phase1_Pwm5;
	LoraTimeControl_Data.start_hour1 = loratimecontrol_data->start_hour1;
	LoraTimeControl_Data.start_hour2 = loratimecontrol_data->start_hour2;
	LoraTimeControl_Data.start_hour3 = loratimecontrol_data->start_hour3;
	LoraTimeControl_Data.start_hour4 = loratimecontrol_data->start_hour4;
	LoraTimeControl_Data.start_hour5 = loratimecontrol_data->start_hour5;
	LoraTimeControl_Data.start_min1 = loratimecontrol_data->start_min1;
	LoraTimeControl_Data.start_min2 = loratimecontrol_data->start_min2;
	LoraTimeControl_Data.start_min3 = loratimecontrol_data->start_min3;
	LoraTimeControl_Data.start_min4 = loratimecontrol_data->start_min4;
	LoraTimeControl_Data.start_min5 = loratimecontrol_data->start_min5;
	LoraTimeControl_Data.start_sec1 = loratimecontrol_data->start_sec1;
	LoraTimeControl_Data.start_sec2 = loratimecontrol_data->start_sec2;
	LoraTimeControl_Data.start_sec3 = loratimecontrol_data->start_sec3;
	LoraTimeControl_Data.start_sec4 = loratimecontrol_data->start_sec4;
	LoraTimeControl_Data.start_sec5 = loratimecontrol_data->start_sec5;
	LoraTimeControl_Data.count = loratimecontrol_data->count;
	xSemaphoreGive (TimeControlMutex);
}

void Get_LoraTimeControl_Data(LoraTimeControl_Type *loratimecontrol_data)
{
	xSemaphoreTake (TimeControlMutex, portMAX_DELAY);
	
	loratimecontrol_data->led_status1 = LoraTimeControl_Data.led_status1;
	loratimecontrol_data->led_status2 = LoraTimeControl_Data.led_status2;
	loratimecontrol_data->led_status3 = LoraTimeControl_Data.led_status3;
	loratimecontrol_data->led_status4 = LoraTimeControl_Data.led_status4;
	loratimecontrol_data->led_status5 = LoraTimeControl_Data.led_status5;
	loratimecontrol_data->phase1_Pwm1 = LoraTimeControl_Data.phase1_Pwm1;
	loratimecontrol_data->phase1_Pwm2 = LoraTimeControl_Data.phase1_Pwm2;
	loratimecontrol_data->phase1_Pwm3 = LoraTimeControl_Data.phase1_Pwm3;
	loratimecontrol_data->phase1_Pwm4 = LoraTimeControl_Data.phase1_Pwm4;
	loratimecontrol_data->phase1_Pwm5 = LoraTimeControl_Data.phase1_Pwm5;
	loratimecontrol_data->start_hour1 = LoraTimeControl_Data.start_hour1;
	loratimecontrol_data->start_hour2 = LoraTimeControl_Data.start_hour2;
	loratimecontrol_data->start_hour3 = LoraTimeControl_Data.start_hour3;
	loratimecontrol_data->start_hour4 = LoraTimeControl_Data.start_hour4;
	loratimecontrol_data->start_hour5 = LoraTimeControl_Data.start_hour5;
	loratimecontrol_data->start_min1 = LoraTimeControl_Data.start_min1;
	loratimecontrol_data->start_min2 = LoraTimeControl_Data.start_min2;
	loratimecontrol_data->start_min3 = LoraTimeControl_Data.start_min3;
	loratimecontrol_data->start_min4 = LoraTimeControl_Data.start_min4;
	loratimecontrol_data->start_min5 = LoraTimeControl_Data.start_min5;
	loratimecontrol_data->start_sec1 = LoraTimeControl_Data.start_sec1;
	loratimecontrol_data->start_sec2 = LoraTimeControl_Data.start_sec2;
	loratimecontrol_data->start_sec3 = LoraTimeControl_Data.start_sec3;
	loratimecontrol_data->start_sec4 = LoraTimeControl_Data.start_sec4;
	loratimecontrol_data->start_sec5 = LoraTimeControl_Data.start_sec5;
	loratimecontrol_data->count = LoraTimeControl_Data.count;
	xSemaphoreGive (TimeControlMutex);
}

void Get_LED_Data(uint8_t *ledstatus,uint8_t *ledpwm)
{
	taskENTER_CRITICAL();
	*ledstatus = LedStatus;
	*ledpwm = LedPwm;
	taskEXIT_CRITICAL();
}

void Write_LED_Data(uint8_t *ledstatus,uint8_t *ledpwm)
{
	taskENTER_CRITICAL();
	LedStatus = *ledstatus;
	LedPwm = *ledpwm;
	if(LedStatus)
	{
		LedStatus = 0;
		REL1_Write(REL_ON);	//打开继电器电源
	}
	else
	{
		LedStatus = 1;
		REL1_Write(REL_OFF);	//关闭继电器电源
	}
	taskEXIT_CRITICAL();
}

#if HARDWARE_PWM
void Led_pwm_change(uint8_t ledpwm)
{
	static uint8_t temp_led_pwm = 0xff;
	if(temp_led_pwm != ledpwm)
	{
		temp_led_pwm = ledpwm;
		TIM_SetTIM3Compare2(ledpwm);
	}
	return;
}
#else
/******************************
功能：产生 pwm 波
用法：定时器中断内调用
注意：该版本仅适合单灯控制
******************************/
void Led_Pwm_Control(void)
{
    static uint8_t led_cnt = 1;
    if(led_cnt>LedPwm)
    {
//        Led0_Write(1);
		PWM1_Write(1);
    }
    else
    {
//        Led0_Write(0);
		PWM1_Write(0);
    }
    led_cnt++;
    if(led_cnt>=100)
    {
        led_cnt = 1;
    }
}

/******************************
功能：调整LED亮度
用法：使亮度调节变得丝滑,在定时器中断中调用
******************************/
void Switch_Brightness(void)
{
    if(targetBrightness > LedPwm)
    {
        LedPwm++;
    }
    else if (targetBrightness < LedPwm)
    {
        LedPwm--;
    }
    else
    {
        LedPwm = targetBrightness;
    }
}

#endif

#if USE_Winext_Protocol
void Led_Strategy(void)
{
	rtc_time_t rtc_time;
	rtc_time_t temp_time;
	static LoraTimeControl_Type loratimecontrol_data;
	uint32_t rtc_unix_time = 0;
	static uint32_t timecontro_unix_time[5] = {0};
	static uint8_t get_loratimecontrol_unix_flag = 0;
	static uint8_t *tmp;
	uint8_t ledstrategy = 0;
	uint8_t ledstatus,ledpwm;
	uint8_t i;
	
	GetRTC(&rtc_time);
	if(rtc_time.ui8Year < 2020)		//未获取到时间，不执行时间管理
		return;
	rtc_unix_time = covBeijing2UnixTimeStp(&rtc_time);
	tmp = (uint8_t*)&loratimecontrol_data;
	Get_LoraTimeControl_Data(&loratimecontrol_data);
	Get_LED_Data(&ledstatus,&ledpwm);
	ledstatus = !ledstatus;
	if(get_timecontrol_unix_flag == 0)		//首次进入时控策略模式，先获取Unix时间戳
	{
		get_timecontrol_unix_flag = 1;
		temp_time.ui8Year = rtc_time.ui8Year;
		temp_time.ui8Month = rtc_time.ui8Month;
		temp_time.ui8DayOfMonth = rtc_time.ui8DayOfMonth;
		for(i = 0 ;i < loratimecontrol_data.count ;i++)
		{
			temp_time.ui8Hour = BcdToHex(tmp[i*5+2]);
			temp_time.ui8Minute = BcdToHex(tmp[i*5+1]);
			temp_time.ui8Second = BcdToHex(tmp[i*5]);
			timecontro_unix_time[i] = covBeijing2UnixTimeStp(&temp_time);
			if(tmp[i*5+2] >= loratimecontrol_data.start_hour1)
				timecontro_unix_time[i] -= 86400;		//将时间定位到前一天的UINX时间戳
		}
	}
	//每天时间到达第一个策略开始时间时更新时控策略的Unix时间戳
	if((rtc_time.ui8Hour >= BcdToHex(loratimecontrol_data.start_hour1) && \
		rtc_time.ui8Minute >= BcdToHex(loratimecontrol_data.start_min1) && \
		rtc_time.ui8Second >= BcdToHex(loratimecontrol_data.start_sec1)))
//		|| get_loratimecontrol_unix_flag == 0)
	{
		get_loratimecontrol_unix_flag = 1;
		temp_time.ui8Year = rtc_time.ui8Year;
		temp_time.ui8Month = rtc_time.ui8Month;
		temp_time.ui8DayOfMonth = rtc_time.ui8DayOfMonth;
		for(i = 0 ;i < loratimecontrol_data.count ;i++)
		{
			temp_time.ui8Hour = BcdToHex(tmp[i*5+2]);
			temp_time.ui8Minute = BcdToHex(tmp[i*5+1]);
			temp_time.ui8Second = BcdToHex(tmp[i*5]);
			timecontro_unix_time[i] = covBeijing2UnixTimeStp(&temp_time);
			if(tmp[i*5+2] < loratimecontrol_data.start_hour1)		//隔天需要在UNIX时间戳上加一天
				timecontro_unix_time[i] += 86400;
		}
	}
//	else	//隔天需更新
//		get_loratimecontrol_unix_flag = 1;
	for(i = 0 ;i < loratimecontrol_data.count-1 ;i++)
	{
		if(rtc_unix_time >= timecontro_unix_time[i] && rtc_unix_time < timecontro_unix_time[i+1])
		{
			myprintf("\r\n 时控策略:%d",i+1);
			ledstatus = tmp[i*5+3];
			ledpwm = tmp[i*5+4];
		}
	}
	if(rtc_unix_time >= timecontro_unix_time[loratimecontrol_data.count-1])
	{
		ledstatus = tmp[(loratimecontrol_data.count-1)*5+3];
		ledpwm = tmp[(loratimecontrol_data.count-1)+4];
		myprintf("\r\n 时控策略:%d",loratimecontrol_data.count);
	}
	Write_LED_Data(&ledstatus,&ledpwm);
	Led_pwm_change(ledpwm);		//改变pwm占空比
}
#else
void Led_Strategy(void)
{
	rtc_time_t rtc_time;
	rtc_time_t temp_time;
	static TimeControl_Type timecontrol_data;
	uint32_t rtc_unix_time = 0;
	static uint32_t timecontro_unix_time = 0;
	static uint8_t get_timecontrol_unix_flag = 0;
	uint8_t ledpwm,ledstatus;
	
	GetRTC(&rtc_time);
	if(rtc_time.ui8Year < 2020)		//未获取到时间，不执行时间管理
		return;
	rtc_unix_time = covBeijing2UnixTimeStp(&rtc_time);
//	if(get_timecontrol_unix_flag == 0)		//获取的一个时控的uinx时间戳
//	{
//		Get_TimeControl_Data(&timecontrol_data);
//		temp_time.ui8Year = rtc_time.ui8Year;
//		temp_time.ui8Month = rtc_time.ui8Month;
//		temp_time.ui8DayOfMonth = rtc_time.ui8DayOfMonth;
//		temp_time.ui8Hour = timecontrol_data.start_hour;
//		temp_time.ui8Minute = timecontrol_data.start_min;
//		temp_time.ui8Second = 0;
//		timecontro_unix_time = covBeijing2UnixTimeStp(&temp_time);
//		
//	}
	get_timecontrol_unix_flag = Get_TimeControl_Data(&timecontrol_data);
	//每天时间到达第一个策略开始时间时更新时控策略的Unix时间戳
	if((rtc_time.ui8Hour == (timecontrol_data.start_hour) && \
		rtc_time.ui8Minute >= (timecontrol_data.start_min)) || \
		rtc_time.ui8Hour > (timecontrol_data.start_hour) || \
		get_timecontrol_unix_flag == 0)
	{
		get_timecontrol_unix_flag = 1;
		Write_TimeControl_flag(get_timecontrol_unix_flag);
		Get_TimeControl_Data(&timecontrol_data);
		temp_time.ui8Year = rtc_time.ui8Year;
		temp_time.ui8Month = rtc_time.ui8Month;
		temp_time.ui8DayOfMonth = rtc_time.ui8DayOfMonth;
		temp_time.ui8Hour = timecontrol_data.start_hour;
		temp_time.ui8Minute = timecontrol_data.start_min;
		temp_time.ui8Second = 0;
		timecontro_unix_time = covBeijing2UnixTimeStp(&temp_time);
	}
	
	//如果时间超过了策略的初始时间
	if(rtc_unix_time >= timecontro_unix_time)
	{
		get_timecontrol_unix_flag = 1;
		if((rtc_unix_time-timecontro_unix_time) <= timecontrol_data.phase1_time*60)	//阶段一
		{
			REL1_Write(REL_ON);
			ledstatus = LEDON;
			ledpwm = timecontrol_data.phase1_Pwm;
			myprintf("\r\n时控模式阶段一");
		}
		else if((rtc_unix_time-timecontro_unix_time) <= \
			(timecontrol_data.phase1_time+timecontrol_data.phase2_time)*60)	//阶段二
		{
			REL1_Write(REL_ON);
			ledstatus = LEDON;
			ledpwm = timecontrol_data.phase2_Pwm;
			myprintf("\r\n时控模式阶段二");
		}
		else if((rtc_unix_time-timecontro_unix_time) <= \
			(timecontrol_data.phase1_time+timecontrol_data.phase2_time+\
			timecontrol_data.phase3_time)*60)	//阶段三
		{
			REL1_Write(REL_ON);
			ledstatus = LEDON;
			ledpwm = timecontrol_data.phase3_Pwm;
			myprintf("\r\n时控模式阶段三");
		}
		else if((rtc_unix_time-timecontro_unix_time) <= \
			(timecontrol_data.phase1_time+timecontrol_data.phase2_time+\
			timecontrol_data.phase4_time)*60)	//阶段四
		{
			REL1_Write(REL_ON);
			ledstatus = LEDON;
			ledpwm = timecontrol_data.phase4_Pwm;
			myprintf("\r\n时控模式阶段四");
		}
		else	//策略模式亮灯结束
		{
			REL1_Write(REL_OFF);
			ledstatus = LEDOFF;
			ledpwm = 0;
			get_timecontrol_unix_flag = 0;		//重新获取新的时间戳
			myprintf("\r\n时控模式结束");
		}
//		Led_pwm_change(LedPwm);		//改变pwm占空比
	}
	else		//未到策略时间，则关灯
	{
		REL1_Write(REL_OFF);
		ledstatus = LEDOFF;
		ledpwm = 0;
	}
	Write_LED_Data(&ledstatus,&ledpwm);
}
#endif

uint8_t CheckSelf(void)
{
	uint8_t check_flag = 0;
	if(BL6523GX_CheckSelf())
	{
		check_flag |= 0x01;
		myprintf("\r\n电量计 ERROR！！！");
	}
	else
		myprintf("\r\n电量计 SUCCESS！！！");
	if(BC26_CheckSelf()==0)
	{
		check_flag |= 0x02;
		myprintf("\r\n NB ERROR！！！");
	}		
	else
		myprintf("\r\n NB SUCCESS！！！");
	return check_flag;
}

void Led_Task(void * argument)
{
	BaseType_t xReturn = pdTRUE;/* 定义一个创建信息返回值，默认为 pdPASS */
	CollectionData electricdata;
	BC26Status nb_state;
	uint16_t LedShowTime = 0;;
	uint32_t time_prinf_cnt = 0;
//	uint8_t ledshowFlag = 0;
	uint8_t led_show_mode[2];
	uint8_t check_flag = 0;
	
	check_flag = CheckSelf();	//自检程序
	
    while(1)
    {
		Get_or_Set_NBStatus(0,&nb_state);	//获取网络状态
		if(check_flag)
			HAL_GPIO_TogglePin(LED_B_GPIO_Port, LED_B_Pin);
		else
			HAL_GPIO_WritePin(LED_B_GPIO_Port, LED_B_Pin,GPIO_PIN_SET);
		if(nb_state.netstatus != ONLINE)
		{
			HAL_GPIO_WritePin(LED_G_GPIO_Port, LED_G_Pin,GPIO_PIN_SET);
			HAL_GPIO_TogglePin(LED_R_GPIO_Port, LED_R_Pin);
		}
		else if(time_prinf_cnt%10 == 0)			//1s
		{
			HAL_GPIO_WritePin(LED_R_GPIO_Port, LED_R_Pin,GPIO_PIN_SET);
			HAL_GPIO_TogglePin(LED_G_GPIO_Port, LED_G_Pin);
		}
		if(time_prinf_cnt %50 ==0 || time_prinf_cnt == 0)//每5秒打印一次时间
		{
			rtc_time_t rtc_time;
			
//			time_prinf_cnt = 0;
			GetRTC(&rtc_time);
			myprintf("\r\n%02d-%02d-%02d  %02d:%02d:%02d ", rtc_time.ui8Year, rtc_time.ui8Month,\
			rtc_time.ui8DayOfMonth, rtc_time.ui8Hour, rtc_time.ui8Minute, rtc_time.ui8Second);
			BL6523GX_ProcessTask(&electricdata);				/*电能计量芯片读取函数*/
			Write_ElectricData(&electricdata);
		}
		
		time_prinf_cnt++;
		if(time_prinf_cnt%(3600*10*24) == 0)
		{
			check_flag = CheckSelf();
		}
		if(xQueueReceive( LedShow_Queue, (void *)led_show_mode, ( TickType_t ) 0) == pdPASS)	//获取演示模式消息队列
		{
			ledshowFlag = 1;
			LedShowTime = 0;
			myprintf("\r\nled演示模式");
			if(led_show_mode[0] == 1)
			{
				REL1_Write(REL_ON);	//打开继电器电源
				myprintf("\r\n 开灯");				
			}
			else
			{
				REL1_Write(REL_OFF);	//关闭继电器电源
				myprintf("\r\n 关灯");				
			}
			myprintf("\r\n 亮度调整：%d",led_show_mode[1]);
			BL6523GX_ProcessTask(&electricdata);				/*电能计量芯片读取函数*/
			Write_ElectricData(&electricdata);
			Write_LED_Data(&led_show_mode[0],&led_show_mode[1]);
//			Led_pwm_change(led_show_mode[1]);		//改变pwm占空比
			Device_Data_Send();
		}
		if(ledshowFlag == 1)
		{
			LedShowTime++;
			if(LedShowTime>=LED_SHOW_CONTINUE_TIME)		//演示模式持续时间
			{
				ledshowFlag = 0;
				LedShowTime = 0;
			}
		}
		else
		{
			if(time_prinf_cnt%10 == 0)
				Led_Strategy();		//leds时间策略
		}
		vTaskDelay(100);
    }
}

void CreatLedTask(void)
{
	BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为pdPASS */

	_myprintf("\r\n CreatLedTask");
	xReturn = xTaskCreate(Led_Task, "Led", Led_StkSIZE, NULL, Led_TaskPrio, &pvCreatedTask_Led);
    if(pdPASS != xReturn)
    {
        _myprintf("\r\n Led_Task creat error");
        while(1);
    }
}
