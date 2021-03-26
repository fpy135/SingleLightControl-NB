#ifndef __UNIXTIMER_H
#define __UNIXTIMER_H

#include <stdio.h>
#include "stdint.h"
#include "string.h"

typedef struct rtc_time_struct
{
    uint16_t ui8Year;       // 1970~2038
    uint8_t ui8Month;       // 1~12
    uint8_t ui8DayOfMonth;  // 1~31
    uint8_t ui8Week;
    uint8_t ui8Hour;        // 0~23
    uint8_t ui8Minute;      // 0~59
    uint8_t ui8Second;      // 0~59
}rtc_time_t;

extern rtc_time_t RtcTime;
extern uint32_t UnixTimsStamp;

void covUnixTimeStp2Beijing(uint32_t unixTime, rtc_time_t *tempBeijing);
uint32_t covBeijing2UnixTimeStp(rtc_time_t *beijingTime);


#endif
