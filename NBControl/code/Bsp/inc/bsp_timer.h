#ifndef __BSP_TIMER_H
#define __BSP_TIMER_H

#include "main.h"

extern TIM_HandleTypeDef htim3;
extern unsigned long SysRunTimeTick;

extern void MX_TIM3_Init(void);
extern void TIM_SetTIM3Compare2(uint32_t duty);

#endif