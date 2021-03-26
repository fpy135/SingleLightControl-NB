#include "IAP.h"
#include <string.h>
#include "StorageConfig.h"
#include "MyFlash.h"
#include "usart.h"
#include "mystring.h"
#include "iwdg.h"
#include "md5.h"

typedef void (*pFunction)(void);

/*
	更新区域    第一个页
	/----/	代码大小-4byte	
	/----/	CRC16-2Byte;更新标识-1byte;更新完成标识-1byte
	/----/	起始地址-4byte
	....
	/----	
	....		下发程序 - nbyte
	----/
*/




/*
	更新程序主流程
*/
void IAP_PRO(void);

/*
	更新程序
	返回：
		0   更新成功
		1		跟新失败
*/
static uint8_t UpData_Program(void);


/*
	检查备份，对比主程序和备份区的CRC是否相同
	如果不相同则备份主程序
*/
void CheckBackUp_Backup(void);

/*
	退回更新
*/
//static void rollback_Program(void);


#define PROM		1	//1	-	程序运行区
#define UPDATA		2	//2	- 更新区CRC
#define BACKUP		3	//3 - 备份区
#define CODEINFO	4	//4 - 程序信息区

/* 
	检查程序的CRC
	输入：
	1	-	程序运行区
	2	- 更新区CRC
	3 - 备份区
	输出：CRC
*/
//static uint16_t CheckCodeCRC(uint8_t sw,uint32_t len);


/*
	读取程序区flash
*/
static void IAP_Read_Program_Flash(uint32_t Startaddr, uint8_t *date, uint16_t len);
/*
	写程序区flash
*/
static void IAP_Write_Program_Flash(uint32_t Startaddr, uint8_t *date, uint16_t len);
/*
	读取程序信息区FLASH
*/
static void IAP_Read_CodeInfo_Flash(uint32_t Startaddr, uint8_t *date, uint16_t len);
/*
	写程序信息区FLASH
*/
void IAP_Write_CodeInfo_Flash(uint32_t Startaddr, uint8_t *date, uint16_t len);
/*
	读取更新区FLASH
*/
static void IAP_Read_Updata_Flash(uint32_t Startaddr, uint8_t *date, uint16_t len);
/*
	写更新区flash
*/
static void IAP_Write_Updata_Flash(uint32_t Startaddr, uint8_t *date, uint16_t len);
/*
	读取备份区FLASH
*/
static void IAP_Read_Backup_Flash(uint32_t Startaddr, uint8_t *date, uint16_t len);
/*
	写备份flash
*/
static void IAP_Write_Backup_Flash(uint32_t Startaddr, uint8_t *date, uint16_t len);

/*
擦除一个区域
输入：
	1	-	程序运行区
	2	- 更新区CRC
	3 - 备份区
*/
static void EraseArea(uint8_t sw);

/*
	擦除外部存储器一页
*/
static void EraseEXPage(uint32_t Startaddr);

/*
	擦除内部存储器一页
*/
static void ErasePage(uint32_t Startaddr,uint32_t Endadd);

#define	IAPBuf_SIZE	(1024*8)

uint8_t IAPBuf[IAPBuf_SIZE];//搬运程序buf

void _printUpInfo_boot(uint8_t *buff)
{
	uint8_t i;
	_myprintf("\r\n 文件长度:%u",WORD_Reverse(*(uint32_t *)&buff[CODE_LEN_ADDR]));
	_myprintf("\r\n 文件MD5:");
	for(i=0;i<16;i++)
	{
		_myprintf("%02x",buff[CODE_MD5_ADDR+i]);
	}
	_myprintf("\r\n 标识1:%u",buff[UPDATA_FLAG_ADDR]);
	_myprintf("\r\n 标识2:%u",buff[UPEDN_FLAG_ADDR]);
	_myprintf("\r\n 复位次数：%u\r\n",buff[RESETCOUNT_ADDR]);
}
/*
	更新程序主流程
*/
void IAP_PRO(void)
{
	uint8_t buff[CODE_INFO_LEN];
	uint8_t ret;
	
	IAP_Read_CodeInfo_Flash(UPDATA_INFO_ADDR,buff,CODE_INFO_LEN); // 读取更新区信息

	_printUpInfo_boot(buff);
	
	if(buff[UPDATA_FLAG_ADDR] == 1) // 有更新
	{
		_myprintf("\r\n 有更新数据！");

		//CheckBackUp_Backup(); // 检查程序备份，并备份程序   应用程序负责备份
		
		ret = UpData_Program();  //更新程序
		
		if(ret == 0) // 更新成功
		{
			buff[UPDATA_FLAG_ADDR] = 0;
			buff[UPEDN_FLAG_ADDR] = 0;
			buff[RESETCOUNT_ADDR] = 0;
			IAP_Write_CodeInfo_Flash(APP_INFO_ADDR,buff,CODE_INFO_LEN); // 写入标识
		}
		else // 更新失败了
		{
			if(ret == 1)  // 已经覆盖了程序CRC却计算出错 退回原程序
			{
				rollback_Program(); // 退回更新
				IAP_Read_CodeInfo_Flash(BACKUP_INFO_ADDR,buff,CODE_INFO_LEN); // 读取备份区信息
				IAP_Write_CodeInfo_Flash(APP_INFO_ADDR,buff,CODE_INFO_LEN); // 写入标识
				IAP_Read_CodeInfo_Flash(UPDATA_INFO_ADDR,buff,CODE_INFO_LEN); // 读取更新区信息
			}
			
			buff[UPEDN_FLAG_ADDR] = 0;
			buff[UPDATA_FLAG_ADDR] = 1;
			buff[RESETCOUNT_ADDR] = 0;
		}
		
//		EraseArea(CODEINFO);
		IAP_Write_CodeInfo_Flash(UPDATA_INFO_ADDR,buff,CODE_INFO_LEN); // 写入标识
	}
	else // 无更新
	{
		_myprintf("\r\n 无更新！");
	
		if(buff[UPEDN_FLAG_ADDR] == 1)  // 上一次更新失败  拷贝回原程序
		{
			if(buff[RESETCOUNT_ADDR] >= RESETCOUNT)
			{		
				_myprintf("\r\n 更新失败 退回更新！");
				rollback_Program(); // 退回更新
				buff[UPDATA_FLAG_ADDR] = 0;
				buff[RESETCOUNT_ADDR] = 0;
//				EraseArea(CODEINFO);
				IAP_Write_CodeInfo_Flash(UPDATA_INFO_ADDR,buff,CODE_INFO_LEN); // 写入标识
				IAP_Read_CodeInfo_Flash(BACKUP_INFO_ADDR,buff,CODE_INFO_LEN); // 读取备份区信息
				IAP_Write_CodeInfo_Flash(APP_INFO_ADDR,buff,CODE_INFO_LEN); // 写入标识
			}
			else
			{
				buff[RESETCOUNT_ADDR]++;
//				EraseArea(CODEINFO);
				IAP_Write_CodeInfo_Flash(UPDATA_INFO_ADDR,buff,CODE_INFO_LEN); // 写入标识
//				IAP_Write_CodeInfo_Flash(APP_INFO_ADDR,buff,CODE_INFO_LEN); // 写入标识
			}
		}
	}
	_myprintf("\r\n 跳转到主程序");
	/*
		跳转主程序
	*/
}
/*
	检查备份，对比主程序和备份区的CRC是否相同
	如果不相同则备份主程序
*/
void CheckBackUp_Backup(void)
{
//	uint16_t programcrc;
//	uint16_t codecrc;
	uint8_t backup_md5[16] = {0};
	uint8_t code_md5[16] = {0};
	uint8_t calculate_md5[16] = {0};
	uint8_t buff[CODE_INFO_LEN];
	uint8_t code_buff[CODE_INFO_LEN];
	uint8_t temp[4] = {0xff,0xff,0xff,0xff};
	uint32_t i;
	uint32_t lastnum;
	uint32_t write_pagenum;
	uint32_t code_size = CODE_SIZE;
	
	MD5_CTX md5;

	
	IAP_Read_CodeInfo_Flash(BACKUP_INFO_ADDR,buff,CODE_INFO_LEN);
	_myprintf("\r\n 原备份区信息↓");
	_printUpInfo_boot(buff);
	memcpy(backup_md5,&buff[CODE_MD5_ADDR],16);
	
	IAP_Read_CodeInfo_Flash(APP_INFO_ADDR,code_buff,CODE_INFO_LEN);
	_myprintf("\r\n 原程序区信息↓");
	_printUpInfo_boot(code_buff);
	memcpy(code_md5,&code_buff[CODE_MD5_ADDR],16);
	
	if((*(__IO uint32_t*)code_buff) == 0xffffffff)
	{
		code_size = CODE_SIZE;
		MD5Init(&md5);
		MD5Update(&md5,(uint8_t *)APP_INFO_ADDR,code_size);
		MD5Final(&md5,code_md5);
	}
	else
	{
		code_size = code_buff[CODE_LEN_ADDR]<<24|code_buff[CODE_LEN_ADDR+1]<<16|code_buff[CODE_LEN_ADDR+2]<<8|code_buff[CODE_LEN_ADDR+3];
	}
//	programcrc = CheckCodeCRC_Boot(PROM,CODE_SIZE); // 计算程序存储区内的CRC 
//	codecrc = (buff[CODE_MD5_ADDR] << 8) | buff[CODE_MD5_ADDR+1];
//	_myprintf("\r\n\r\n 当前运行程序CRC：0x%02x",programcrc);
	
//	if((codecrc != programcrc) ||(codecrc == 0xffff ) ||(codecrc == 0x0000) ) // CRC 对不上需要备份主程序
	// md5 对不上需要备份主程序
	if(StrComplate(backup_md5,code_md5,16) == 0 || StrFindString(backup_md5,16,(uint8_t *)temp,4) != 0xffff)
	{
			EraseArea(BACKUP); // 擦除备份区代码

			_myprintf("\r\n提示：备份区MD5不正确需要重新备份程序");
			/*
				需要写入的扇区数量  程序大小/ 页大小 = 页的写入量
				程序大小以 byte为单位
			*/
			write_pagenum = code_size/IAPBuf_SIZE;  // 大小和扇区需要对齐 尽量不要出现余数
			lastnum = code_size%IAPBuf_SIZE;  // 内部flash 不能读超出范围否则会出现非法访问。
			_myprintf("\r\n 备份扇区数%d.%d",write_pagenum,lastnum);
			for(i=0;i<write_pagenum;i++) // 将内部的程序写入外部备份区 // i+1 外部FLASH 第一个区4K 是用于存储更新区信息的
			{
				IAP_Write_Backup_Flash(BACKUP_ADDR+i*IAPBuf_SIZE,(uint8_t *)(APP_START_ADDR+i*IAPBuf_SIZE),IAPBuf_SIZE); 
				if(!(i%50))
					_myprintf("\r\n");
				_myprintf("<");
			}
			
			if(lastnum !=0) // 写入剩余的数据
				IAP_Write_Backup_Flash(BACKUP_ADDR+i*IAPBuf_SIZE,(uint8_t *)(APP_START_ADDR+write_pagenum*IAPBuf_SIZE),lastnum); 
			MD5Init(&md5);
			MD5Update(&md5,(uint8_t *)BACKUP_ADDR,code_size);
			MD5Final(&md5,calculate_md5);
			_myprintf("\r\n 数据复制完成");
			buff[CODE_LEN_ADDR] =   (code_size>>24)&0xff;
			buff[CODE_LEN_ADDR+1] = (code_size>>16)&0xff;
			buff[CODE_LEN_ADDR+2] = (code_size>>8)&0xff;
			buff[CODE_LEN_ADDR+3] = (uint8_t)code_size;
			memcpy(&buff[CODE_MD5_ADDR],calculate_md5,16);
			IAP_Write_CodeInfo_Flash(BACKUP_INFO_ADDR,buff,CODE_INFO_LEN); // 写入信息
			if(StrFindString(code_md5,16,(uint8_t *)temp,4) != 0xffff)
				IAP_Write_CodeInfo_Flash(BACKUP_INFO_ADDR,buff,CODE_INFO_LEN); // 写入信息
			IAP_Write_CodeInfo_Flash(APP_INFO_ADDR,buff,CODE_INFO_LEN); // 写入信息
			IAP_Read_CodeInfo_Flash(BACKUP_INFO_ADDR,buff,CODE_INFO_LEN);
			_myprintf("\r\n 当前备份区信息↓");
			_printUpInfo_boot(buff);
	}
	/* */
}
/*
	更新程序
	返回：
		0   更新成功
		1		更新失败
		2		外部程序存储区内的数据有误未更新
*/
static uint8_t UpData_Program(void)
{
	uint8_t buff[CODE_INFO_LEN];
	uint32_t len;
	uint16_t upcrc,crc,i,pagenum;
	uint32_t lastnum;
	uint8_t updata_md5[16] = {0};
	uint8_t calculate_md5[16] = {0};
	MD5_CTX md5;
	MD5Init(&md5);

	IAP_Read_Updata_Flash(UPDATA_INFO_ADDR,buff,CODE_INFO_LEN);
	_myprintf("\r\n 更新区信息↓");
	_printUpInfo_boot(buff);
	len = buff[CODE_LEN_ADDR]<<24|buff[CODE_LEN_ADDR+1]<<16|buff[CODE_LEN_ADDR+2]<<8|buff[CODE_LEN_ADDR+3];
//	upcrc = buff[CODE_MD5_ADDR]<<8|buff[CODE_MD5_ADDR+1];
	memcpy(updata_md5,&buff[CODE_MD5_ADDR],16);
	
	if(len == 0xffffffff)
	{
		_myprintf("\r\n ERROR:更新区数据长度不对:0x%08x",len);
		return 2;
	}
//	crc = CheckCodeCRC_Boot(UPDATA,len); // 取得更新区的CRC
	MD5Update(&md5,(uint8_t *)UPDATA_ADDR,len);
	MD5Final(&md5,calculate_md5);
	_myprintf("\r\n 更新区计算md5:");
	for(i=0;i<16;i++)
	{
		_myprintf("%02x",calculate_md5[i]);
	}
//	if(crc!= upcrc) // 存储的CRC和FLASH里计算的CRC不一样
	if(StrComplate(updata_md5,calculate_md5,16) == 0)
	{
		_myprintf("\r\n ERROR:更新信息区里md5和FLASH读出计算的md5不相同");
		return 2;
	}
	else // CRC正确写入程序存储区
	{
		_myprintf("\r\n 正在擦除整个APP程序区");
		EraseArea(PROM);	//擦除整个APP程序区
		/*
			写入大小 = len/ 单次写入大小  = 写入的次数 
		*/
		pagenum = len/IAPBuf_SIZE;  // 8K 
		if(len%IAPBuf_SIZE)
			pagenum++;
		lastnum = len%IAPBuf_SIZE;
		_myprintf("\r\n 写入包数%d.%d",pagenum,lastnum);
		
		for(i=0;i<pagenum;i++) /* 写入代码*/
		{
			IAP_Read_Updata_Flash(UPDATA_ADDR+i*IAPBuf_SIZE, IAPBuf,IAPBuf_SIZE); 
//			PageAddres	=	GerPageStartAddr(APP_START_ADDR+i*FLASH_PAGE_SIZE);//计算页数	
//			ErasePage(PageAddres); // 擦除该页
			HAL_IWDG_Refresh(&hiwdg);
			IAP_Write_Program_Flash(APP_START_ADDR+i*IAPBuf_SIZE,IAPBuf,IAPBuf_SIZE); 
			if(!(i%50))
				_myprintf("\r\n");
			_myprintf(">");
		}
		
//		upcrc = CheckCodeCRC_Boot(PROM,len); // 计算程序存储区内的CRC 
		memset(calculate_md5,0,16);
		MD5Init(&md5);
		MD5Update(&md5,(uint8_t *)APP_START_ADDR,len);
		MD5Final(&md5,calculate_md5);
		if(StrComplate(updata_md5,calculate_md5,16) == 0)
		{
			_myprintf("\r\n ERROR:更新后的md5和更新信息区里md5不同");
			return 1;
		}
	}
	/*
		写入app的程序信息
	*/
	return 0;
}
/*
	退回更新
*/
 void rollback_Program(void)
{
	uint8_t buff[CODE_INFO_LEN];
	uint8_t databuff[CODE_INFO_LEN];
	uint16_t programcrc,codecrc;
	uint8_t backup_md5[16] = {0};
	uint8_t calculate_md5[16] = {0};
	uint16_t i,pagenum;
	uint32_t lastnum;
	uint32_t len;
	uint32_t PageAddres;
	MD5_CTX md5;
	MD5Init(&md5);	
	
	IAP_Read_CodeInfo_Flash(BACKUP_INFO_ADDR,buff,CODE_INFO_LEN);
	_myprintf("\r\n 备份区内的数据信息↓");
	_printUpInfo_boot(buff);
	
	memcpy(backup_md5,&buff[CODE_MD5_ADDR],16);
	len = buff[CODE_LEN_ADDR]<<24|buff[CODE_LEN_ADDR+1]<<16|buff[CODE_LEN_ADDR+2]<<8|buff[CODE_LEN_ADDR+3];
	if(len == 0xffffffff)
		return;
	IAP_Read_Backup_Flash(BACKUP_ADDR, databuff,8);
	if ( ((*(__IO uint32_t*)databuff) & 0x2FF00000 ) != 0x20000000 )
	{
		_myprintf("\r\n 备份区内不是正确的程序:0x%08x",((*(__IO uint32_t*)databuff) & 0x2FF00000 ));
		return;
	}
	
	MD5Update(&md5,(uint8_t *)BACKUP_ADDR,len);
	MD5Final(&md5,calculate_md5);
	
	_myprintf("\r\n 计算备份区内的程序MD5：");
	for(i=0;i<16;i++)
	{
		_myprintf("%02x",calculate_md5[i]);
	}
	
	
	
	  //||(!(codecrc == 0xffff ) ||(codecrc == 0x0000)) 
	if(StrComplate(backup_md5,calculate_md5,16) == 1) // CRC对上就可以将程序拷回
	{
		_myprintf("\r\n crc正确复制备份区内程序到程序运行区");
		_myprintf("\r\n 正在擦除整个APP程序区");
		EraseArea(PROM);	//擦除整个APP程序区
		pagenum = len/IAPBuf_SIZE;
		lastnum = len%IAPBuf_SIZE;
		if(len%IAPBuf_SIZE)  // 正常是1K对其的
			pagenum++;
		_myprintf("\r\n 写入包数%d.%d",pagenum,lastnum);
		for(i=0;i<pagenum;i++) /* 写入代码*/
		{
			IAP_Read_Backup_Flash(BACKUP_ADDR+i*IAPBuf_SIZE, IAPBuf,IAPBuf_SIZE); 
			HAL_IWDG_Refresh(&hiwdg);
			
			IAP_Write_Program_Flash(APP_START_ADDR+i*IAPBuf_SIZE,IAPBuf,IAPBuf_SIZE); 
			if(!(i%50))
				_myprintf("\r\n");
			_myprintf(">");
			//FlashWritNWithoutErasing(PageAddres,IAPBuf,1024);  //写该页数据		
		}
	}
}
/* 
	检查程序的CRC
	输入：
	1	-	程序运行区
	2	- 更新区CRC
	3 - 备份区
	输出：CRC
*/
uint16_t CheckCodeCRC_Boot(uint8_t sw,uint32_t len)
{
	uint16_t crc16=0xffff;
	uint32_t lastnum;
	uint32_t i,pagemun;
		switch(sw)
		{
			case PROM:
			if(len>iFLASH_SIZE)
				return 0;
			
					crc16 = CRC16_calc((uint8_t *)APP_START_ADDR,len);
					pagemun = len/IAPBuf_SIZE;	
					lastnum = len%IAPBuf_SIZE;
					_myprintf("\r\n 计算数量%d.%d,计算CRC：0x%02x",pagemun,lastnum,crc16);
				break;
			case UPDATA:
			case BACKUP:
				if(len>iFLASH_SIZE) // 超过正常程序大小
					return 0;
					pagemun = len/IAPBuf_SIZE;
					lastnum = len%IAPBuf_SIZE;
					_myprintf("\r\n 计算数量%d.%d",pagemun,lastnum);
					for(i=0;i<pagemun;i++)
					{
						if(sw == 2)
							IAP_Read_Updata_Flash(UPDATA_ADDR+i*IAPBuf_SIZE, IAPBuf,IAPBuf_SIZE); 
						else if(sw == 3)
							IAP_Read_Updata_Flash(BACKUP_ADDR+i*IAPBuf_SIZE, IAPBuf,IAPBuf_SIZE);
						
						crc16 = ContinuousCRC16_calc(crc16,IAPBuf,IAPBuf_SIZE);
						
						if(!(i%50))
							_myprintf("\r\n");
						_myprintf("*");
					}
					if(lastnum!=0)
					{
						if(sw == 2)
							IAP_Read_Updata_Flash(UPDATA_ADDR+(pagemun)*IAPBuf_SIZE, IAPBuf,IAPBuf_SIZE); 
						else if(sw == 3)
							IAP_Read_Updata_Flash(BACKUP_ADDR+(pagemun)*IAPBuf_SIZE, IAPBuf,IAPBuf_SIZE);
							
						crc16 = ContinuousCRC16_calc(crc16,IAPBuf,lastnum);
					}
				break;
		}
	return crc16;
}


/*
	擦除内部存储器一页
	@输入：擦除地址 
*/
static void ErasePage(uint32_t Startaddr,uint32_t Endadd)
{
	MyFLASH_Erase(Startaddr,Endadd);//擦除该页 
}
/*
	擦除外部存储器一页
	@输入：擦除的地址
*/
//static void EraseEXPage(uint32_t Startaddr)
//{
//		W25XXEraseSector(Startaddr);
//}


/*
	读取程序区flash
*/
static void IAP_Read_Program_Flash(uint32_t Startaddr, uint8_t *date, uint16_t len)
{
	bsp_ReadCpuFlash(Startaddr, date, len);
}
/*
	写程序区flash
*/
static void IAP_Write_Program_Flash(uint32_t Startaddr, uint8_t *date, uint16_t len)
{
	bsp_WriteCpuFlash(Startaddr,date,len);  //写该页数据		
}

/*
	读取程序信息区FLASH
*/
static void IAP_Read_CodeInfo_Flash(uint32_t Startaddr, uint8_t *date, uint16_t len)
{
	bsp_ReadCpuFlash(Startaddr, date, len);
}

/*
	写程序信息区FLASH
*/
void IAP_Write_CodeInfo_Flash(uint32_t Startaddr, uint8_t *date, uint16_t len)
{
	uint8_t app_info_data[CODE_INFO_LEN];
	uint8_t backup_info_data[CODE_INFO_LEN];
	uint8_t updata_info_data[CODE_INFO_LEN];
	
	IAP_Read_CodeInfo_Flash(APP_INFO_ADDR,app_info_data,CODE_INFO_LEN);
	IAP_Read_CodeInfo_Flash(BACKUP_INFO_ADDR,backup_info_data,CODE_INFO_LEN);
	IAP_Read_CodeInfo_Flash(UPDATA_INFO_ADDR,updata_info_data,CODE_INFO_LEN);
	EraseArea(CODEINFO);
	if(Startaddr == APP_INFO_ADDR)
		StrCopy(app_info_data,date,len);
	else if(Startaddr == BACKUP_INFO_ADDR)
		StrCopy(backup_info_data,date,len);
	else if(Startaddr == UPDATA_INFO_ADDR)
		StrCopy(updata_info_data,date,len);
	bsp_WriteCpuFlash(APP_INFO_ADDR,app_info_data,CODE_INFO_LEN);
	bsp_WriteCpuFlash(BACKUP_INFO_ADDR,backup_info_data,CODE_INFO_LEN);
	bsp_WriteCpuFlash(UPDATA_INFO_ADDR,updata_info_data,CODE_INFO_LEN);
} 

/*
	读取更新区FLASH
*/
static void IAP_Read_Updata_Flash(uint32_t Startaddr, uint8_t *date, uint16_t len)
{
	bsp_ReadCpuFlash(Startaddr, date, len);
}

/*
	写更新区flash
*/
static void IAP_Write_Updata_Flash(uint32_t Startaddr, uint8_t *date, uint16_t len)
{
	bsp_WriteCpuFlash(Startaddr,date,len);  //写该页数据		
}

/*
	读取备份区FLASH
*/
static void IAP_Read_Backup_Flash(uint32_t Startaddr, uint8_t *date, uint16_t len)
{
	bsp_ReadCpuFlash(Startaddr, date, len);
}

/*
	写备份flash
*/
static void IAP_Write_Backup_Flash(uint32_t Startaddr, uint8_t *date, uint16_t len)
{
	bsp_WriteCpuFlash(Startaddr,date,len);  //写该页数据		
}

/*
擦除一个区域
输入：
	1	-	程序运行区
	2	- 更新区CRC
	3 - 备份区
*/
static void EraseArea(uint8_t sw)
{
	switch(sw)
	{
		case PROM: //256k  
			MyFLASH_Erase(APP_START_ADDR,APP_START_ADDR+256*1024-1);
		break;
		case BACKUP: //256k  
			MyFLASH_Erase(BACKUP_ADDR,BACKUP_ADDR+256*1024-1);
		break;
		case UPDATA: //256k  
			MyFLASH_Erase(UPDATA_ADDR,UPDATA_ADDR+256*1024-1);
		break;
		case CODEINFO:	//64k
			MyFLASH_Erase(CODE_INFO_ADDR,CODE_INFO_ADDR+64*1024-1);
		break;
		default:break;
	}
}

void IAP_Jump(uint32_t AppAddr)
{
	uint32_t JumpAddress;
	pFunction Jump_To_Application;

	if(((*(__IO uint32_t*)(AppAddr+4))&0xFF000000)==0x08000000)	//判断是否为0X08XXXXXX.
	{
		if ( ( (*(__IO uint32_t*)AppAddr) & 0x2FF00000 ) == 0x20000000 )
		{ 
			JumpAddress = *(__IO uint32_t*)(AppAddr + 4);
			/* Jump to user application */
			Jump_To_Application = (pFunction)JumpAddress;
			//NVIC_SetVectorTable(0x00000000,AppAddr-0x00000000);
			/* Initialize user application's Stack Pointer */
			__set_MSP(*(__IO uint32_t*)AppAddr);
			_myprintf("\r\njump to app\r\n");
			
			HAL_UART_DeInit(&huart5);

			HAL_DeInit();
			__HAL_RCC_GPIOA_CLK_DISABLE();
			__HAL_RCC_GPIOB_CLK_DISABLE();
			__HAL_RCC_GPIOC_CLK_DISABLE();
			__HAL_RCC_GPIOD_CLK_DISABLE();
			__HAL_RCC_GPIOE_CLK_DISABLE();
			__HAL_RCC_GPIOF_CLK_DISABLE();
			__HAL_RCC_GPIOH_CLK_DISABLE();
			__HAL_RCC_UART5_CLK_DISABLE();			__disable_irq();
			/* Jump to application */
			Jump_To_Application();
		}
	}
}
