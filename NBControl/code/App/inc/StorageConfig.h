#ifndef __STORAGECONFIG_H
#define __STORAGECONFIG_H


#define CODE_SIZE 			(256*1024)  //程序存储区内的代码大小 byte

/***************************************
flash 地址规划配置		1M flash

在这里要注意FLASH_ProgramWord函数的参数 ： 
Address 必须为4的整倍数，否则写入报错：
FLASH_ERROR_PROGRAM（写入错误 查了很久也没找到关于这个错误的描述，花费了不少时间）
**************************************/
//															//Boot区64k
//#define CODE_INFO_ADDR			(0x08010000)		//程序信息区	64k
//#define APP_START_ADDR 			(0x08020000)		//APP起始地址	256k	
//#define BACKUP_ADDR 			(0x08060000)		//程序备份区	256k
//#define UPDATA_ADDR 			(0x080A0000)		//程序下载区	256k
#define USER_DATA_ADDR			(0x0800FC00)		//用户存储段地址 1k

//程序信息扇区
#define CODE_INFO_LEN			(23)
#define APP_INFO_ADDR			(CODE_INFO_ADDR)
#define BACKUP_INFO_ADDR		(CODE_INFO_ADDR+1024*20)
#define UPDATA_INFO_ADDR		(CODE_INFO_ADDR+1024*40)


//用户扇区
#define USER_SECTOR_USE_LEN		(16+16+24+26)
#define DEVICETYPE_ADDR			(USER_DATA_ADDR)
#define DEVICEID_ADDR			(DEVICETYPE_ADDR + 4)
#define IDSTATUS_ADDR			(DEVICEID_ADDR+4)
#define LEDSTATUS_ADDR			(IDSTATUS_ADDR+4)

#define TIMECONTROL_ADDR		(LEDSTATUS_ADDR+4)		//时间策略14字节 但为了4字节对齐，改为16字节

#define DeviceIP_ADDR			(TIMECONTROL_ADDR+16)
#define DeviceNetmask_ADDR		(DeviceIP_ADDR+4)
#define DeviceGw_ADDR			(DeviceNetmask_ADDR+4)
#define ServerIP_ADDR			(DeviceGw_ADDR+4)
#define ServerPort_ADDR			(ServerIP_ADDR+4)
#define DHCP_Enable_ADDR		(ServerPort_ADDR+4)			//24字节

#define LORATIMECONTROL_ADDR	(DHCP_Enable_ADDR+4)		//26字节

#endif
