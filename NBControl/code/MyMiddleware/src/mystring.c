#include "mystring.h"
#include "math.h"
#include "string.h"


/*
	将bcd码转成16进制
	如0x86	转成 0x56
*/
uint8_t BcdToHex(uint8_t bcd) 
{
	return ((bcd>>4)*10 + (bcd&0x0f));
}

/*
	将16进制转成bcd码
	如0x0f	转成 0x15
*/
uint8_t HexToBcd(uint8_t hex) 
{
	return (((hex/10)<<4) +hex%10);
}



uint8_t CheckStr(uint8_t *buf, uint8_t str, uint16_t len)
{
	uint16_t i;
	for(i = 0; i<len; i++)
	{
		if(buf[i] != str)
			return 0;
	}
	return 1;
}


uint8_t CharToHex(uint8_t hex)
{
    if (hex > 47 && hex < 58)
    {
        return hex - 48;
    }
    if (hex > 64 && hex < 71)
    {
        return hex - 55;
    }
    if (hex > 96 && hex < 103)
    {
        return hex - 87;
    }
    return 0xff;
}


/*
  	buf中找str,如果找到返回位置，找不到返回0xffff
*/
uint16_t StrFind(uint8_t *buf, uint16_t len, uint8_t str)
{
	uint16_t i;
	for(i=0 ;i<len; i++)
	{
		if(buf[i] == str)
		{
		 	return i;
		}
	}
	return 0xffff;
}
void StrCopy(uint8_t *str1, uint8_t *str2, uint16_t len)
{
	uint16_t i;
	for(i=0;i<len;i++)
	{
		str1[i] = str2[i];	
	}

}
uint8_t Sum(unsigned char *Array, unsigned char Len)
{
    unsigned char tem=0;
    unsigned char x=0;
    for(x=0; x<Len; x++)
    {
        tem+=Array[x];
    }
    return tem;
}

uint16_t CRC16_calc(uint8_t *tmp_buf,uint32_t len) 
{
	uint16_t crc_tmp=0;
	uint8_t j;
	uint32_t i;

	for(i=0,crc_tmp=0xFFFF; i < len; i++)//做CRC校验
	{		
		crc_tmp ^= tmp_buf[i];
		for(j=0; j < 8; j++)
		{
			if(crc_tmp & 0x01)
			{
				crc_tmp = (crc_tmp>>1) ^ 0xA001;
			}											
			else
			{
				crc_tmp >>=1;
			}
		}
	}	
	return crc_tmp; 
}
/*
	第一次计算CRC时带入的记得是0XFFFF
*/
uint16_t ContinuousCRC16_calc(uint16_t crc, uint8_t *tmp_buf,uint32_t len) 
{
	uint16_t crc_tmp=crc;
	uint8_t j;
	uint32_t i;

	for(i=0,crc_tmp=crc; i < len; i++)//做CRC校验
	{		
		crc_tmp ^= tmp_buf[i];
		for(j=0; j < 8; j++)
		{
			if(crc_tmp & 0x01)
			{
				crc_tmp = (crc_tmp>>1) ^ 0xA001;
			}											
			else
			{
				crc_tmp >>=1;
			}
		}
	}	
	return crc_tmp; 
}


unsigned short CRC_CCITT(unsigned char* message, unsigned int len)
{
    int i, j;
    unsigned short crc_reg = 0xffff;
    unsigned short current;

    for (i = 0; i < len; i++)
    {
        current = message[i] << 8;

        for (j = 0; j < 8; j++)
        {
            if ((short)(crc_reg ^ current) < 0)
            {
                crc_reg = (crc_reg << 1) ^ 0x1021;
            }

            else
            {
                crc_reg <<= 1;
            }

            current <<= 1;
        }
    }

    return crc_reg;
}


void StrNegate(uint8_t *str)
{
	uint8_t tmp = 0;
	uint8_t i;
	
	tmp = *str;
	for(i=0; i<8; i++)
	{
		if(((tmp>>i)&0x01) == 0)
		{
			tmp |= ((0x01)<<i);
		}
		else
		{
			tmp &= ~((0x01)<<i);
		}
	}
	*str = tmp;
}
/*
	数字字符串转成uint32_t
	碰到非数字结束
	第一个就是非数字，返回0
	否则返回第几个
*/
uint8_t Str2Num(uint8_t *buf,uint8_t len,uint32_t *ret)
{
	uint8_t i;
	uint32_t num = 0;
	
	for(i=0; i<len; i++)
	{
		if((buf[i] < '0') || (buf[i] > '9'))
		{
			if(i==0)
				return 0;//第一个返回失败，没有数字
			else
			{
				*ret = num;
				return i;
			}
		}
		
		num = num*10 + buf[i]-'0';
	}
	*ret = num;
	return i;
}
/*
	 在buf中，获取第ucNum个str的位置
*/
uint16_t GetNStr(uint8_t *buf, uint16_t len, uint8_t str, uint8_t ucNum)
{
	uint8_t  i,j;
	
	//*,c1,c2,\r,\n
	for(i = 0,j=0;i<len;i++)
	{
		if(buf[i] == str)
		{
			j++;
			if(j == ucNum)
				return i;
		}		
	}
	return 0xffff;
}
/*
作用：  比较str1与str2是否一致，一致返回1，不一致返回0
参数：  str1、str2 字符指针
        len 比较的长度
*/
uint8_t StrComplate(uint8_t *str1, uint8_t *str2, uint16_t len)
{
    uint16_t i;
    for(i=0; i<len; i++)
    {
        if(str1[i] != str2[i])
        {
            return 0;
        }
    }
    return 1;
}

/*
  	buf中找str字符串,如果找到返回位置，找不到返回0xffff
*/
uint16_t StrFindString(uint8_t *buf, uint16_t len, uint8_t *str, uint8_t slen)
{
	uint16_t i;
	if(slen > len)
		return 0xffff;
	for(i=0 ;i<(len-slen+1); i++)
	{
		if(StrComplate(&buf[i], str, slen))
		{
			return i;
		}
	}
	return 0xffff;
}

/*	
Gdata:加速度采集三轴数据
Angle：转换后三轴角度
*/
void GValue2Angle(int *G_data, float *Angle)
{
	float Gdata[3];
	
	Gdata[0] = G_data[0];
	Gdata[1] = G_data[1];
	Gdata[2] = G_data[2];
	
	Angle[0] = atan(Gdata[0] / sqrt(Gdata[1]*Gdata[1] + Gdata[2]*Gdata[2]))*180/PI;

	Angle[1] = atan(Gdata[1] / sqrt(Gdata[0]*Gdata[0]+Gdata[2]*Gdata[2]))*180/PI;

	Angle[2] = atan( Gdata[2] / sqrt(Gdata[0]*Gdata[0] +Gdata[1]*Gdata[1]))*180/PI; 
	
	
	
	
}


void Swap(float *A, uint8_t i, uint8_t j)
{
    float temp = A[i];
    A[i] = A[j];
    A[j] = temp;
}

void BubbleSort(float *A, uint8_t n)
{
	uint8_t j = 0;
	uint8_t i = 0;
	
    for (j = 0; j < n - 1; j++)         // 每次最大元素就像气泡一样"浮"到数组的最后
    {
        for (i = 0; i < n - 1 - j; i++) // 依次比较相邻的两个元素,使较大的那个向后移
        {
            if (A[i] > A[i + 1])            // 如果条件改成A[i] >= A[i + 1],则变为不稳定的排序算法
            {
                Swap(A, i, i + 1);
            }
        }
    }
}


uint8_t DCstrIP_PORT_2_HEX(uint8_t *buf, uint8_t buf_len, uint8_t *ip, uint16_t *port)
{
						
	uint8_t i;
	uint8_t j;
	
	memset(ip,0,4);
	*port = 0;
	
	for(j=0,i=0; i< buf_len ;i++)
	{
		if((buf[i] >= '0') && (buf[i] <= '9'))
		{
			ip[j]= ip[j]*10+(buf[i]-0x30);
		}
		else if(buf[i] == '.') 
		{
			j++;
		}
		else if((buf[i] == '/') || (buf[i] == ':'))
		{
			uint16_t tmp16=0;
			for(i++; i< buf_len; i++)
			{								
				tmp16 = tmp16*10 + (buf[i]-0x30); 								
			}
			*port = tmp16;
			break;	
		}
	}
	return 1;
}
