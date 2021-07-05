#include "IDConfig.h"
#include "SysConfig.h"
#include "mystring.h"
#include "MyFlash.h"
#include "Protocol.h"
#include "bsp_uart.h"
#include "Led_App.h"
#include "StorageConfig.h"
#include "Platform.h"

/********初始化不为0变量***********/
uint32_t NoInitFlag __no_init;

uint16_t Device_TYPE = 0;	//产品ID或者产品类型，设备类型
uint32_t Device_ID = 0;		//设备ID
uint8_t Sn = 0;				//数据包流水号
uint8_t IDStatus = 0;		//ID预制状态

void MCU_RESET_STATUS(void)
{
	if (__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST) != RESET)
	{
		_myprintf("\r\n发生来自 NRST 引脚的复位");
	}
	if(__HAL_RCC_GET_FLAG(RCC_FLAG_PORRST) != RESET)
	{
		_myprintf("\r\n发生 POR/PDR 复位");
	}
	if (__HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST)!= RESET)
	{
		_myprintf("\r\n发生软件复位");
	}
	if(__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST) != RESET)
	{
		_myprintf("\r\n发生来自独立看门狗复位");
	}
	else if (__HAL_RCC_GET_FLAG(RCC_FLAG_WWDGRST) != RESET)
	{
		_myprintf("\r\n发生窗口看门狗复位");
	}
	else if (__HAL_RCC_GET_FLAG(RCC_FLAG_LPWRRST)!= RESET)
	{
		_myprintf("\r\n发生低功耗管理复位");
	}
	__HAL_RCC_CLEAR_RESET_FLAGS();//清除RCC中复位标志
}

/**************************************************
功能：生产预制ID
输入参数：buf预制数据    len 长度
**************************************************/
void ReadID(void)
{
	
	bsp_ReadCpuFlash (IDSTATUS_ADDR, (uint8_t *)&IDStatus, 1);
//	bsp_ReadCpuFlash (LEDSTATUS_ADDR, (uint8_t *)&LedStatus, 1);
//	bsp_ReadCpuFlash (LEDSTATUS_ADDR+1, (uint8_t *)&LedPwm, 1);
	if(IDStatus == 1)		//ID预制过后读出ID和产品类型
	{
		bsp_ReadCpuFlash (DEVICETYPE_ADDR, (uint8_t *)&Device_TYPE, 2);
		bsp_ReadCpuFlash (DEVICEID_ADDR, (uint8_t *)&Device_ID, 4);
	}
	else
	{
		IDStatus = 0;
		Device_TYPE = 0x1A03;
		Device_ID = 0;
	}
	
	if(NoInitFlag == 0x1234abcd)		//软复位
	{
		myprintf("\r\n软复位!!!");
		LedPwm = LedPwm;
		LedStatus = LedStatus;
		ledshowFlag = ledshowFlag;
	}
	else
	{
		myprintf("\r\n上电复位!!!");
		NoInitFlag = 0x1234abcd;
		LedStatus = REL_OFF;
		LedPwm = 0;
		ledshowFlag = 0;
	}
	if(LedStatus == 0xff || LedPwm>100)
	{
		LedStatus = REL_OFF;
		LedPwm = 0;
	}
	REL1_Write(LedStatus);
//	Led_pwm_change(LedPwm);		//改变pwm占空比
	myprintf("\r\nLedStatus:%d,LedPwm:%d",!LedStatus,LedPwm);
#if USE_Winext_Protocol
	{
		LoraTimeControl_Type loratimecontrol_data;
		bsp_ReadCpuFlash (LORATIMECONTROL_ADDR, (uint8_t *)&LoraTimeControl_Data,sizeof(LoraTimeControl_Type));
		Get_LoraTimeControl_Data(&loratimecontrol_data);
		if(loratimecontrol_data.start_hour1 >= 0x24 || loratimecontrol_data.start_min1 >= 0x60\
			|| loratimecontrol_data.start_sec1 >= 0x60 || loratimecontrol_data.count > 5)
		{
			//默认模式
			loratimecontrol_data.start_hour1 = 0x18;
			loratimecontrol_data.start_min1 = 0x00;
			loratimecontrol_data.start_sec1 = 0x00;
			loratimecontrol_data.led_status1 = 0x01;
			loratimecontrol_data.phase1_Pwm1 = 99;
			
			loratimecontrol_data.start_hour2 = 0x22;
			loratimecontrol_data.start_min2 = 0x00;
			loratimecontrol_data.start_sec2 = 0x00;
			loratimecontrol_data.led_status2 = 0x01;
			loratimecontrol_data.phase1_Pwm2 = 50;
			
			loratimecontrol_data.start_hour3 = 0x05;
			loratimecontrol_data.start_min3 = 0x00;
			loratimecontrol_data.start_sec3 = 0x00;
			loratimecontrol_data.led_status3 = 0x00;
			loratimecontrol_data.phase1_Pwm3 = 0;
			
			loratimecontrol_data.count = 3;
		}
		Write_LoraTimeControl_Data(&loratimecontrol_data);
		myprintf("\r\n产品类型:0x%2X,设备ID:%d",Device_TYPE,Device_ID);
		myprintf("\r\n时间控制策略个数:%d\r\n",loratimecontrol_data.count);
		myprintf("策略一:%x:%x:%x led:%d pwm:%d\r\n策略二:%x:%x:%x led:%d pwm:%d\r\n策略三:%x:%x:%x led:%d pwm:%d\r\n策略四:%x:%x:%x led:%d pwm:%d\r\n策略五:%x:%x:%x led:%d pwm:%d\r\n",\
		loratimecontrol_data.start_hour1,loratimecontrol_data.start_min1,loratimecontrol_data.start_sec1,loratimecontrol_data.led_status1,loratimecontrol_data.phase1_Pwm1,\
		loratimecontrol_data.start_hour2,loratimecontrol_data.start_min2,loratimecontrol_data.start_sec2,loratimecontrol_data.led_status2,loratimecontrol_data.phase1_Pwm2,\
		loratimecontrol_data.start_hour3,loratimecontrol_data.start_min3,loratimecontrol_data.start_sec3,loratimecontrol_data.led_status3,loratimecontrol_data.phase1_Pwm3,\
		loratimecontrol_data.start_hour4,loratimecontrol_data.start_min4,loratimecontrol_data.start_sec4,loratimecontrol_data.led_status4,loratimecontrol_data.phase1_Pwm4,\
		loratimecontrol_data.start_hour5,loratimecontrol_data.start_min5,loratimecontrol_data.start_sec5,loratimecontrol_data.led_status5,loratimecontrol_data.phase1_Pwm5);
	}
#else
	{
		TimeControl_Type timecontrol_data;
		bsp_ReadCpuFlash (TIMECONTROL_ADDR, (uint8_t *)&timecontrol_data,sizeof(timecontrol_data));
		if(timecontrol_data.start_hour >= 24 || timecontrol_data.start_min >= 60)
		{
			//默认模式
			timecontrol_data.start_hour = 18;
			timecontrol_data.start_min = 0;
			timecontrol_data.phase1_time = 60*6;
			timecontrol_data.phase1_Pwm = 100;
			timecontrol_data.phase2_time = 60*6;
			timecontrol_data.phase2_Pwm = 50;
			timecontrol_data.phase3_time = 0;
			timecontrol_data.phase3_Pwm = 0;
			timecontrol_data.phase4_time = 0;
			timecontrol_data.phase4_Pwm = 0;
		}
		if(timecontrol_data.phase1_Pwm > 100)
			timecontrol_data.phase1_Pwm = 100;
		if(timecontrol_data.phase2_Pwm > 100)
			timecontrol_data.phase2_Pwm = 100;
		if(timecontrol_data.phase3_Pwm > 100)
			timecontrol_data.phase3_Pwm = 100;
		if(timecontrol_data.phase4_Pwm > 100)
			timecontrol_data.phase4_Pwm = 100;
		Write_TimeControl_Data(&timecontrol_data);
		myprintf("\r\n产品类型:0x%2X,设备ID:%d",Device_TYPE,Device_ID);
		myprintf("\r\n时间控制策略:开始时间:%02d:%02d,阶段一:time:%d  pwm:%d  阶段二:time:%d  pwm:%d  阶段三:time:%d  pwm:%d  阶段四:time:%d  pwm:%d\r\n",\
		timecontrol_data.start_hour,timecontrol_data.start_min,timecontrol_data.phase1_time,timecontrol_data.phase1_Pwm,\
		timecontrol_data.phase2_time,timecontrol_data.phase2_Pwm,timecontrol_data.phase3_time,timecontrol_data.phase3_Pwm,\
		timecontrol_data.phase4_time,timecontrol_data.phase4_Pwm);
	}
#endif
}

/**************************************************
功能：生产预制ID
输入参数：buf预制数据    len 长度
**************************************************/
void IDCofing(uint8_t *buf, uint16_t len)
{
	if(buf[0] == 0xAA)
    {
        uint16_t crc_tmp1,crc_tmp2;

        crc_tmp2 = CRC16_calc(buf,len-2);
        crc_tmp1 = buf[len-1]<<8 | buf[len-2];
        if(crc_tmp1 == crc_tmp2)
        {
            if(buf[8] == ReadID_Cmd)		//查询ID
            {
                uint8_t *pbuf;

				bsp_ReadCpuFlash (DEVICETYPE_ADDR, (uint8_t *)&Device_TYPE, 2);
				bsp_ReadCpuFlash (DEVICEID_ADDR, (uint8_t *)&Device_ID, 4);
                pbuf = Protocol_Pack(0,Device_TYPE,Device_ID,ReadID_Cmd,0,(uint8_t *)buf);
                PrintWrite(pbuf, len);		//返回的长度和写入长度一致

            }
            else if(buf[8] == WriteID_Cmd)		//预制ID
            {
//				if(IDStatus == 0)				//仅未预制ID的情况下可以预制ID
				{
					uint8_t *pbuf;

					IDStatus = 1;
					Device_TYPE = ((buf[2]<<8) | buf[3]);
					Device_ID = (buf[4]<<24 | buf[5]<<16 | buf[6]<<8 | buf[7]);
					Write_UserMem_Flash(IDSTATUS_ADDR,(uint8_t *)&IDStatus,1);
					bsp_ReadCpuFlash (IDSTATUS_ADDR, (uint8_t *)&IDStatus, 1);
					Write_UserMem_Flash(DEVICETYPE_ADDR,(uint8_t *)&Device_TYPE,2);
					bsp_ReadCpuFlash (DEVICETYPE_ADDR, (uint8_t *)&Device_TYPE, 2);
					Write_UserMem_Flash(DEVICEID_ADDR,(uint8_t *)&Device_ID,4);
					bsp_ReadCpuFlash (DEVICEID_ADDR, (uint8_t *)&Device_ID, 4);
					pbuf = Protocol_Pack(0,Device_TYPE,Device_ID,WriteID_Cmd,0,(uint8_t *)buf);
					PrintWrite(pbuf, len);		//返回的长度和写入长度一致
				}
            }
        }
    }
}
