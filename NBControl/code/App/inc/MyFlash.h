#ifndef __MYFLASH_H
#define __MYFLASH_H

#include "main.h"

#define FLASH_BASE_ADDR			0x08000000			/* Flash基地址 */
#define	FLASH_SIZE				(64*1024)		/* Flash 容量 */

#define FLASH_IS_EQU		0   /* Flash内容和待写入的数据相等，不需要擦除和写操作 */
#define FLASH_REQ_WRITE		1	/* Flash不需要擦除，直接写 */
#define FLASH_REQ_ERASE		2	/* Flash需要先擦除,再写 */
#define FLASH_PARAM_ERR		3	/* 函数参数错误 */

extern uint16_t MyFLASH_Unlock(void);
extern uint16_t MyFLASH_Lock(void);
extern uint8_t MyFLASH_Erase(uint32_t start_Add,uint32_t end_Add);
extern uint8_t MyFLASH_Write(uint32_t address, uint8_t *data, uint32_t Len);
extern uint8_t *MyFLASH_Read (uint32_t address, uint8_t *data, uint32_t Len);

extern uint8_t bsp_ReadCpuFlash(uint32_t _ulFlashAddr, uint8_t *_ucpDst, uint32_t _ulSize);
extern uint8_t bsp_WriteCpuFlash(uint32_t _ulFlashAddr, uint8_t *_ucpSrc, uint32_t _ulSize);
extern uint8_t bsp_CmpCpuFlash(uint32_t _ulFlashAddr, uint8_t *_ucpBuf, uint32_t _ulSize);
extern uint8_t Write_UserMem_Flash(uint32_t _ulFlashAddr, uint8_t *_ucpSrc, uint32_t _ulSize);

#endif
