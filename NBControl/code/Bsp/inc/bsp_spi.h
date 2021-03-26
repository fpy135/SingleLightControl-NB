#ifndef __BSP_SPI_H
#define __BSP_SPI_H

#include "main.h"

extern void MX_SPI2_Init(void);
extern void SPI2_SetSpeed(uint8_t SPI_BaudRatePrescaler);
extern uint8_t SPI2_ReadWriteByte(uint8_t TxData);

#endif
