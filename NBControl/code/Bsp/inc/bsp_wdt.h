#ifndef __BSP_WDT_H
#define __BSP_WDT_H

#include "main.h"

extern IWDG_HandleTypeDef hiwdg;

extern void MX_IWDG_Init(void);
extern void WDG_Feed(void);


#endif