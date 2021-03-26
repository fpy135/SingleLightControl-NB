#include "bsp_rtc.h"
#include "SysConfig.h"
#include "Unixtimer.h"
#include "bsp_uart.h"

RTC_HandleTypeDef hrtc;
void GetRTC(rtc_time_t *rtc_time);

void GetRTC_BCD_NO_RTOS(rtc_time_t *rtc_time)
{
	RTC_DateTypeDef sdate;
	RTC_TimeTypeDef stime;
	
	/* Get the RTC current Time */
	HAL_RTC_GetTime(&hrtc, &stime, RTC_FORMAT_BCD);
	/* Get the RTC current Date */
	HAL_RTC_GetDate(&hrtc, &sdate, RTC_FORMAT_BCD);
//    RtcTime.ui8Year = sdate.Year+2000;
//    RtcTime.ui8Month = sdate.Month;
//    RtcTime.ui8DayOfMonth = sdate.Date;
//    RtcTime.ui8Hour = stime.Hours;
//    RtcTime.ui8Minute = stime.Minutes;
//    RtcTime.ui8Second = stime.Seconds;
	
	rtc_time->ui8Year = sdate.Year;
    rtc_time->ui8Month = sdate.Month;
    rtc_time->ui8DayOfMonth = sdate.Date;
    rtc_time->ui8Hour = stime.Hours;
    rtc_time->ui8Minute = stime.Minutes;
    rtc_time->ui8Second = stime.Seconds;
	
}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */
	rtc_time_t rtc_time;
  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef DateToUpdate = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */
  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
  hrtc.Init.OutPut = RTC_OUTPUTSOURCE_NONE;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */
	GetRTC_BCD_NO_RTOS(&rtc_time);
  if(HAL_RTCEx_BKUPRead(&hrtc,RTC_BKP_DR1)!= 0x5051)
	{
	  /* USER CODE END Check_RTC_BKUP */

	  /** Initialize RTC and set the Time and Date
	  */
	  sTime.Hours = 0;
	  sTime.Minutes = 0;
	  sTime.Seconds = 1;

	  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
	  {
		Error_Handler();
	  }
	  DateToUpdate.WeekDay = RTC_WEEKDAY_MONDAY;
	  DateToUpdate.Month = RTC_MONTH_JANUARY;
	  DateToUpdate.Date = 0;
	  DateToUpdate.Year = 0;

	  if (HAL_RTC_SetDate(&hrtc, &DateToUpdate, RTC_FORMAT_BCD) != HAL_OK)
	  {
		Error_Handler();
	  }
  }
  /* USER CODE BEGIN RTC_Init 2 */
	else
	{
		RTC_DateTypeDef sdate;
		RTC_TimeTypeDef stime;
		/* Get the RTC current Time */
		HAL_RTC_GetTime(&hrtc, &stime, RTC_FORMAT_BIN);
		/* Get the RTC current Date */
		HAL_RTC_GetDate(&hrtc, &sdate, RTC_FORMAT_BIN);
			_myprintf("\r\n上电时间：%02d-%02d-%02d  %02d:%02d:%02d ", sdate.Year+2000, sdate.Month,\
			sdate.Date, stime.Hours, stime.Minutes, stime.Seconds);
		DateToUpdate.Year    = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR2);
		DateToUpdate.Month   = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR3);
		DateToUpdate.Date    = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR4);
		DateToUpdate.WeekDay = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR5);
		_myprintf("\r\n备份时间：%02d-%02d-%02d  %02d:%02d:%02d ", DateToUpdate.Year+2000, DateToUpdate.Month,\
			DateToUpdate.Date, stime.Hours, stime.Minutes, stime.Seconds);
		if (HAL_RTC_SetDate(&hrtc, &DateToUpdate, RTC_FORMAT_BIN) != HAL_OK)
		{
			Error_Handler();
		}
	}
  /* USER CODE END RTC_Init 2 */

}

void HAL_RTC_MspInit(RTC_HandleTypeDef* hrtc)
{
  if(hrtc->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspInit 0 */

  /* USER CODE END RTC_MspInit 0 */
    HAL_PWR_EnableBkUpAccess();
    /* Enable BKP CLK enable for backup registers */
    __HAL_RCC_BKP_CLK_ENABLE();
    /* Peripheral clock enable */
    __HAL_RCC_RTC_ENABLE();
  /* USER CODE BEGIN RTC_MspInit 1 */

  /* USER CODE END RTC_MspInit 1 */
  }

}

void HAL_RTC_MspDeInit(RTC_HandleTypeDef* hrtc)
{
  if(hrtc->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspDeInit 0 */

  /* USER CODE END RTC_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_RTC_DISABLE();
  /* USER CODE BEGIN RTC_MspDeInit 1 */

  /* USER CODE END RTC_MspDeInit 1 */
  }

}

void SetRTC(rtc_time_t *rtc_time)
{
    RTC_DateTypeDef sdate;
	RTC_TimeTypeDef stime;

	taskENTER_CRITICAL();
	
	RtcTime.ui8Year = rtc_time->ui8Year;
    RtcTime.ui8Month = rtc_time->ui8Month;
    RtcTime.ui8DayOfMonth = rtc_time->ui8DayOfMonth;
    RtcTime.ui8Hour = rtc_time->ui8Hour;
    RtcTime.ui8Minute = rtc_time->ui8Minute;
    RtcTime.ui8Second = rtc_time->ui8Second;
	
	sdate.Year = RtcTime.ui8Year-2000;
    sdate.Month = RtcTime.ui8Month;
    sdate.Date = RtcTime.ui8DayOfMonth;
	sdate.WeekDay = 0x01;

	/* Set the RTC current Date */
	HAL_RTC_SetDate(&hrtc, &sdate, RTC_FORMAT_BIN);	
	
    stime.Hours = RtcTime.ui8Hour;
    stime.Minutes = RtcTime.ui8Minute;
    stime.Seconds = RtcTime.ui8Second;
	/* Set the RTC current Time */
	HAL_RTC_SetTime(&hrtc, &stime, RTC_FORMAT_BIN);
	
	taskEXIT_CRITICAL();
}

void GetRTC(rtc_time_t *rtc_time)
{
	RTC_DateTypeDef sdate;
	RTC_TimeTypeDef stime;
	
	taskENTER_CRITICAL();

	/* Get the RTC current Time */
	HAL_RTC_GetTime(&hrtc, &stime, RTC_FORMAT_BIN);
	/* Get the RTC current Date */
	HAL_RTC_GetDate(&hrtc, &sdate, RTC_FORMAT_BIN);
    RtcTime.ui8Year = sdate.Year+2000;
    RtcTime.ui8Month = sdate.Month;
    RtcTime.ui8DayOfMonth = sdate.Date;
    RtcTime.ui8Hour = stime.Hours;
    RtcTime.ui8Minute = stime.Minutes;
    RtcTime.ui8Second = stime.Seconds;
	
	rtc_time->ui8Year = RtcTime.ui8Year;
    rtc_time->ui8Month = RtcTime.ui8Month;
    rtc_time->ui8DayOfMonth = RtcTime.ui8DayOfMonth;
    rtc_time->ui8Hour = RtcTime.ui8Hour;
    rtc_time->ui8Minute = RtcTime.ui8Minute;
    rtc_time->ui8Second = RtcTime.ui8Second;
	
	HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, 0x5051);//向指定的后备区域寄存器写入数据
	HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR2, (uint16_t)sdate.Year);
	HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR3, (uint16_t)sdate.Month);
	HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR4, (uint16_t)sdate.Date);
	HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR5, (uint16_t)sdate.WeekDay);
	taskEXIT_CRITICAL();

}

void SetRTC_BCD(rtc_time_t *rtc_time)
{
    RTC_DateTypeDef sdate;
	RTC_TimeTypeDef stime;

	taskENTER_CRITICAL();
	
//	RtcTime.ui8Year = rtc_time->ui8Year;
//    RtcTime.ui8Month = rtc_time->ui8Month;
//    RtcTime.ui8DayOfMonth = rtc_time->ui8DayOfMonth;
//    RtcTime.ui8Hour = rtc_time->ui8Hour;
//    RtcTime.ui8Minute = rtc_time->ui8Minute;
//    RtcTime.ui8Second = rtc_time->ui8Second;
	
	sdate.Year = rtc_time->ui8Year;
    sdate.Month = rtc_time->ui8Month;
    sdate.Date = rtc_time->ui8DayOfMonth;
//	sdate.WeekDay = 0x01;

	/* Set the RTC current Date */
	HAL_RTC_SetDate(&hrtc, &sdate, RTC_FORMAT_BCD);	
	
    stime.Hours = rtc_time->ui8Hour;
    stime.Minutes = rtc_time->ui8Minute;
    stime.Seconds = rtc_time->ui8Second;
	/* Set the RTC current Time */
	HAL_RTC_SetTime(&hrtc, &stime, RTC_FORMAT_BCD);
	
	taskEXIT_CRITICAL();
}

void GetRTC_BCD(rtc_time_t *rtc_time)
{
	RTC_DateTypeDef sdate;
	RTC_TimeTypeDef stime;
	
	taskENTER_CRITICAL();

	/* Get the RTC current Time */
	HAL_RTC_GetTime(&hrtc, &stime, RTC_FORMAT_BCD);
	/* Get the RTC current Date */
	HAL_RTC_GetDate(&hrtc, &sdate, RTC_FORMAT_BCD);
//    RtcTime.ui8Year = sdate.Year+2000;
//    RtcTime.ui8Month = sdate.Month;
//    RtcTime.ui8DayOfMonth = sdate.Date;
//    RtcTime.ui8Hour = stime.Hours;
//    RtcTime.ui8Minute = stime.Minutes;
//    RtcTime.ui8Second = stime.Seconds;
	
	rtc_time->ui8Year = sdate.Year;
    rtc_time->ui8Month = sdate.Month;
    rtc_time->ui8DayOfMonth = sdate.Date;
    rtc_time->ui8Hour = stime.Hours;
    rtc_time->ui8Minute = stime.Minutes;
    rtc_time->ui8Second = stime.Seconds;
//	
//	HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, 0x5051);//向指定的后备区域寄存器写入数据
//	HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR2, (uint16_t)sdate.Year);
//	HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR3, (uint16_t)sdate.Month);
//	HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR4, (uint16_t)sdate.Date);
//	HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR5, (uint16_t)sdate.WeekDay);
	
	taskEXIT_CRITICAL();

}
