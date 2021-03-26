#ifndef __IDCONFIG_H
#define __IDCONFIG_H

#include "main.h"

/* Private macro -------------------------------------------------------------*/

extern uint16_t Device_TYPE;	//产品ID或者产品类型，设备类型
extern uint32_t Device_ID;		//设备ID
extern uint8_t IDStatus;		//ID状态
extern uint8_t Sn;				//数据包流水号

extern void ReadID(void);
extern void MCU_RESET_STATUS(void);
extern void IDCofing(uint8_t *buf, uint16_t len);


#endif
