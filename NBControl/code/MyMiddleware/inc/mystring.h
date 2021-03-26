#ifndef __MYSTRING_H
#define __MYSTRING_H
#include "stdint.h"

#define HALFWORD_Reverse( x )	( (((x)&0x00ff)<<8) | (((x)&0xff00)>>8) )
#define WORD_Reverse(x) ((((x)&0xff)<<24)|(((x)&0xff00)<<8)|(((x)&0xff0000)>>8)|(((x)&0xff000000)>>24)) 

#define	PI	3.1415926535

extern uint8_t BcdToHex(uint8_t bcd); 
extern uint8_t HexToBcd(uint8_t hex); 
extern uint8_t CheckStr(uint8_t *buf, uint8_t str, uint16_t len);
extern uint8_t CharToHex(uint8_t hex);
extern uint16_t StrFind(uint8_t *buf, uint16_t len, uint8_t str);
extern uint8_t Sum(unsigned char *Array, unsigned char Len);
extern void StrNegate(uint8_t *str);
extern uint8_t StrComplate(uint8_t *str1, uint8_t *str2, uint16_t len);
extern uint16_t StrFindString(uint8_t *buf, uint16_t len, uint8_t *str, uint8_t slen);
extern void StrCopy(uint8_t *str1, uint8_t *str2, uint16_t len);
extern void GValue2Angle(int *G_data, float *Angle);
extern void BubbleSort(float *A, uint8_t n);
extern uint8_t DCstrIP_PORT_2_HEX(uint8_t *buf, uint8_t buf_len, uint8_t *ip, uint16_t *port);

extern uint16_t	GetNStr(uint8_t *buf, uint16_t len, uint8_t str, uint8_t ucNum);
extern uint8_t Str2Num(uint8_t *buf,uint8_t len,uint32_t *ret); 

extern uint16_t CRC16_calc(uint8_t *tmp_buf,uint32_t len);
extern uint16_t ContinuousCRC16_calc(uint16_t crc, uint8_t *tmp_buf,uint32_t len);
extern unsigned short CRC_CCITT(unsigned char* message, unsigned int len);

#endif




