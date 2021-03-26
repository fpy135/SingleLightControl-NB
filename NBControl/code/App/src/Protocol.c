#include "Protocol.h"
#include "mystring.h"
#include "string.h"

uint8_t send_data[256];

///**
//  * @brief  CRC16
//  * @param  None
//  * @retval None
//  */
//uint16_t CRC16_Calc(uint8_t *tmpbuf, uint8_t len)
//{
//    uint16_t crc_tmp=0;
//    uint8_t j;
//    uint8_t i;

//    crc_tmp = 0xFFFF;
//    for(i = 0; i < len; i++)
//    {
//        crc_tmp ^= tmpbuf[i];
//        for(j=0; j < 8; j++)
//        {
//            if(crc_tmp & 0x01)	//判断右移出的是不是1，如果是1则与多项式进行异或。
//            {
//                crc_tmp = (crc_tmp >> 1) ^ 0xA001;	//先将数据右移一位 与上面的多项式进行异或
//            }
//            else	//如果不是1，则直接移出
//            {
//                crc_tmp >>= 1;	//直接移出
//            }
//        }
//    }
//    return crc_tmp;
//}

uint8_t *Protocol_Pack(uint8_t sn, uint16_t type, uint32_t id, uint8_t cmd,uint16_t len, uint8_t *content)
{
    uint16_t CRCRes;
    uint8_t *pBuf;
    uint8_t i = 0;
	uint16_t temp_type = HALFWORD_Reverse(type);
    uint32_t temp_id = WORD_Reverse(id);
    send_data[0]  	= 0xAA;
    send_data[1]    = sn & 0x3f;
	send_data[2]  	= (uint8_t)(temp_type & 0xff);
    send_data[3]	= (uint8_t)((temp_type>>8) & 0xff);
    send_data[4]  	= (uint8_t)(temp_id & 0xff);
    send_data[5]	= (uint8_t)((temp_id>>8) & 0xff);
    send_data[6]   	= (uint8_t)((temp_id>>16) & 0xff);
    send_data[7]  	= (uint8_t)((temp_id>>24) & 0xff);
    send_data[8]   	= cmd;
    send_data[9]  	= len;
    memcpy(&send_data[10], content, len);

    pBuf = (uint8_t *)(&send_data);
    CRCRes = CRC16_calc(pBuf, 10+len);
//    RFProtocol.crc = (CRCRes << 8) | (CRCRes >> 8);
    send_data[10+len] = (uint8_t)(CRCRes & 0xff);
    send_data[10+len+1] = (uint8_t)((CRCRes>>8) & 0xff);
    return pBuf;
}

uint8_t *Protocol_Pack_BB(uint8_t sn, uint16_t type, uint32_t id, uint8_t cmd,uint16_t len, uint8_t *content)
{
    uint16_t CRCRes;
    uint8_t *pBuf;
    uint8_t i = 0;
	uint16_t temp_type = HALFWORD_Reverse(type);
    uint32_t temp_id = WORD_Reverse(id);
    send_data[0]  	= 0xBB;
    send_data[1]    = sn & 0x3f;
	send_data[2]  	= (uint8_t)(temp_type & 0xff);
    send_data[3]	= (uint8_t)((temp_type>>8) & 0xff);
    send_data[4]  	= (uint8_t)(temp_id & 0xff);
    send_data[5]	= (uint8_t)((temp_id>>8) & 0xff);
    send_data[6]   	= (uint8_t)((temp_id>>16) & 0xff);
    send_data[7]  	= (uint8_t)((temp_id>>24) & 0xff);
    send_data[8]   	= cmd;
    send_data[9]  	= len;
    memcpy(&send_data[10], content, len);

    pBuf = (uint8_t *)(&send_data);
    CRCRes = CRC16_calc(pBuf, 10+len);
//    RFProtocol.crc = (CRCRes << 8) | (CRCRes >> 8);
    send_data[10+len] = (uint8_t)(CRCRes & 0xff);
    send_data[10+len+1] = (uint8_t)((CRCRes>>8) & 0xff);
    return pBuf;
}

