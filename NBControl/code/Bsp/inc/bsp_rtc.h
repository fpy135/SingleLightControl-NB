#ifndef __BSP_RTC_H
#define __BSP_RTC_H

#include "main.h"

extern RTC_HandleTypeDef hrtc;

extern void MX_RTC_Init(void);
//extern void SetRTC_BCD(rtc_time_t *rtc_time);
//extern void GetRTC_BCD(rtc_time_t *rtc_time);

#endif
