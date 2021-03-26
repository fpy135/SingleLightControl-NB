#include "bl6526b.h"
#include "SysConfig.h"
#include "math.h"
#include "main.h"
#include "bsp_uart.h"
#include "bsp_spi.h"

BL6526BStruct bl6526b1data;
BL6526BStruct bl6526b2data;

CollectionData ElectricData = {0};		/*将IRMT_S1结构体的值都初始化为0*/

extern uint16_t FLASH_HF33F_Relay;					/*FLASH中存放的掉电前的继电器状态*/
void bsp_SpiGpio_Init(void);

//extern uint8_t BL6526B_Data_Write[];

//粗延时函数，微秒
void delay5us(uint16_t time)
{
   uint16_t i=0;  
   while(time--)
   {
      i=50;  //自己定义
      while(i--) ;    
   }
}

/*******************************************************************************
** 函数名称: SPI1_Configuration
** 功能描述：
** 函数说明：SPI1初始化
** 作　  者:
** 日　  期:
*******************************************************************************/
void SPI_Configuration(void)
{
    BL6526B_1_CS_HIGH();
	BL6526B_1_RST_HIGH();
	BL6526B_2_CS_HIGH();
	BL6526B_2_RST_HIGH();
}

/*******************************************************************************
** 函数名称: BL6526B_Init
** 功能描述：
** 函数说明：BL6526B_Init初始化
** 作　  者:
** 日　  期:
*******************************************************************************/
void BL6526B_Init(void)
{
#if use_software_spi
	bsp_SpiGpio_Init();
#endif
#if use_hardware_spi
	MX_SPI2_Init();
	SPI2_SetSpeed(SPI_BAUDRATEPRESCALER_256); //设置时钟,高速模式
	SPI2_ReadWriteByte(0Xff);                           //启动传输
#endif
    SPI_Configuration();
    BL6526B_1_RST_LOW();
	BL6526B_2_RST_LOW();
    HAL_Delay(10);
    BL6526B_1_RST_HIGH();
	BL6526B_2_RST_HIGH();
    HAL_Delay(10);
	BL6526B_WriteData(BL6526B_1,(BL6526B_WRPROT | 0x40),0x00,0x00,0x55);	// 8bits 关闭写保护
    BL6526B_WriteData(BL6526B_1,(BL6526B_MODE | 0x40),0x00,0x00,0x00);		//12bits 使用高通滤波器，能量累加模式采用绝对值累加
																			//使用功率阈值比较方式防潜动，关闭定时防潜方式
    BL6526B_WriteData(BL6526B_1,(BL6526B_GAIN | 0x40),0x00,0x00,0x03);		//12bits 电流增益8倍，电压1倍增益
    BL6526B_WriteData(BL6526B_1,(BL6526B_MASK | 0x40),0x00,0x00,0x28);		//12bits 中断屏蔽寄存器，过压，过流中断，
    BL6526B_WriteData(BL6526B_1,(BL6526B_V_PKLVL | 0x40),0x00,0x03,0xEA);	//12bits 240V左右会过压
    BL6526B_WriteData(BL6526B_1,(BL6526B_I_PKLVL | 0x40),0x00,0x03,0xE8);	//12bits 5A电流左右会过流
	BL6526B_WriteData(BL6526B_1,(BL6526B_WRPROT | 0x40),0x00,0x00,0x00);	// 8bits 开启写保护
	
//	BL6526B_WriteData(BL6526B_2,(BL6526B_WRPROT | 0x40),0x00,0x00,0x55);	// 8bits 关闭写保护
//    BL6526B_WriteData(BL6526B_2,(BL6526B_MODE | 0x40),0x00,0x00,0x00);		//12bits 使用高通滤波器，能量累加模式采用绝对值累加
//																			//使用功率阈值比较方式防潜动，关闭定时防潜方式
//    BL6526B_WriteData(BL6526B_2,(BL6526B_GAIN | 0x40),0x00,0x00,0x03);		//12bits 电流增益8倍，电压1倍增益
//    BL6526B_WriteData(BL6526B_2,(BL6526B_MASK | 0x40),0x00,0x00,0x28);		//12bits 中断屏蔽寄存器，过压，过流中断，
//    BL6526B_WriteData(BL6526B_2,(BL6526B_V_PKLVL | 0x40),0x00,0x03,0xEA);	//12bits 240V左右会过压
//    BL6526B_WriteData(BL6526B_2,(BL6526B_I_PKLVL | 0x40),0x00,0x03,0xE8);	//12bits 5A电流左右会过流
//	BL6526B_WriteData(BL6526B_2,(BL6526B_WRPROT | 0x40),0x00,0x00,0x00);	// 8bits 开启写保护
}

#if use_software_spi

void bsp_SpiGpio_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	
	__HAL_RCC_GPIOB_CLK_ENABLE();
	
	 /*Configure GPIO pin : PtPin */
	GPIO_InitStruct.Pin = SPI2_MISO_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(SPI2_MISO_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : PtPin */
	GPIO_InitStruct.Pin = SPI2_MOSI_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(SPI2_MOSI_GPIO_Port, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin = SPI2_SCK_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(SPI2_SCK_GPIO_Port, &GPIO_InitStruct);
	
	HAL_GPIO_WritePin(GPIOB, SPI2_SCK_Pin|SPI2_CS_Pin|SPI2_MISO_Pin|SPI2_MOSI_Pin, GPIO_PIN_RESET);
}

/*******************************************************************************
** 函数名称: BL6526B_ReadData
** 功能描述：往里面传一个CMD = 0X00 + Address
** 函数说明：BL6526B_ReadData从BL6526B里面读数据出来 24bit数据
** 作　  者:
** 日　  期:
*******************************************************************************/
uint32_t BL6526B_ReadData(BL6526B_Type bL6526b_type, uint8_t Cmd)
{
    uint8_t byte[3];
    uint8_t i,j;
    uint32_t data = 0;

	if(bL6526b_type == BL6526B_1)
	{
		delay5us(1);
		BL6526B_1_CS_HIGH();
		delay5us(10);
		BL6526B_1_MOSI_LOW();
		BL6526B_1_SCLK_LOW();
		delay5us(1);
		BL6526B_1_CS_LOW();
		delay5us(1);
	}
	else if(bL6526b_type == BL6526B_2)
	{
		delay5us(1);
		BL6526B_2_CS_HIGH();
		delay5us(10);
		BL6526B_2_MOSI_LOW();
		BL6526B_2_SCLK_LOW();
		delay5us(1);
		BL6526B_2_CS_LOW();
		delay5us(1);
	}		
	//先往地址里面写一个字节 告诉BL6526B的哪个寄存器 
	SPI_SendByte(bL6526b_type,Cmd);
	delay5us(10);

	if(bL6526b_type == BL6526B_1)
	{
		//开始接收3个字节数据
		for(j=0;j<3;j++)
		{
			for(i=0;i<8;i++)
			{
				BL6526B_1_SCLK_HIGH();
				delay5us(1);
				BL6526B_1_SCLK_LOW();
				delay5us(1);
				byte[j]<<=1;
				if(BL6526B_1_MISO_IN)
					byte[j]++;  
				delay5us(1);
			}
		}
		delay5us(1);
		BL6526B_1_CS_HIGH();
		delay5us(1);
		BL6526B_1_SCLK_LOW();
		BL6526B_1_MOSI_LOW();
		delay5us(1);
	}
	else if(bL6526b_type == BL6526B_2)
	{
		//开始接收3个字节数据
		for(j=0;j<3;j++)
		{
			for(i=0;i<8;i++)
			{
				BL6526B_2_SCLK_HIGH();
				delay5us(1);
				BL6526B_2_SCLK_LOW();
				delay5us(1);
				byte[j]<<=1;
				if(BL6526B_2_MISO_IN)
					byte[j]++;  
				delay5us(1);
			}
		}
		delay5us(1);
		BL6526B_2_CS_HIGH();
		delay5us(1);
		BL6526B_2_SCLK_LOW();
		BL6526B_2_MOSI_LOW();
		delay5us(1);
	}
    data = (data|byte[0])<<8;
    data = (data|byte[1])<<8;
    data = (data|byte[2]);
    return data ;

}

/*******************************************************************************
** 函数名称: BL6526B_WriteData
** 功能描述：往里面传一个CMD = 0X00 + Address
** 函数说明：BL6526B_WriteData
** 作　  者:
** 日　  期:
*******************************************************************************/
void BL6526B_WriteData(BL6526B_Type bL6526b_type,uint8_t Cmd,uint8_t byte1,uint8_t byte2,uint8_t byte3)
{
	
	if(bL6526b_type == BL6526B_1)
	{
		BL6526B_1_MOSI_LOW();
		BL6526B_1_SCLK_LOW();
		delay5us(1);
		BL6526B_1_CS_LOW();
	}
	else if(bL6526b_type == BL6526B_2)
	{
		BL6526B_2_MOSI_LOW();
		BL6526B_2_SCLK_LOW();
		delay5us(1);
		BL6526B_2_CS_LOW();
	}
	delay5us(3);
	//先往地址里面写一个字节 告诉BL6526B的哪个寄存器 
	SPI_SendByte(bL6526b_type,Cmd);
	SPI_SendByte(bL6526b_type,byte1);
	SPI_SendByte(bL6526b_type,byte2);
	SPI_SendByte(bL6526b_type,byte3);

	
	if(bL6526b_type == BL6526B_1)
	{
		BL6526B_1_MOSI_LOW();
		BL6526B_1_SCLK_LOW();
		delay5us(3);
		BL6526B_1_CS_HIGH();
	}
	else if(bL6526b_type == BL6526B_2)
	{
		BL6526B_2_MOSI_LOW();
		BL6526B_2_SCLK_LOW();
		delay5us(3);
		BL6526B_2_CS_HIGH();
	}
}
/*******************************************************************************
** 函数名称: SPI_SendByte
** 功能描述：
** 函数说明：发送一个字节到BL6526B
** 作　  者:
** 日　  期:
*******************************************************************************/
void SPI_SendByte(BL6526B_Type bL6526b_type,uint8_t byte)
{
    uint8_t i;
	if(bL6526b_type == BL6526B_1)
	{
		for(i=0;i<8;i++)
		{
			BL6526B_1_SCLK_LOW();
			if(byte&0x80)
				BL6526B_1_MOSI_HIGH();
			else
				BL6526B_1_MOSI_LOW();
			byte<<=1; 	
			delay5us(1);
			BL6526B_1_SCLK_HIGH(); //时钟拉高 产生上升沿 一个bit位就发走了
			delay5us(1);
		}
		BL6526B_1_SCLK_LOW();
	}
	else if(bL6526b_type == BL6526B_2)
	{
		for(i=0;i<8;i++)
		{
			BL6526B_2_SCLK_LOW();
			if(byte&0x80)
				BL6526B_2_MOSI_HIGH();
			else
				BL6526B_2_MOSI_LOW();
			byte<<=1; 	
			delay5us(1);
			BL6526B_2_SCLK_HIGH(); //时钟拉高 产生上升沿 一个bit位就发走了
			delay5us(1);
		}
		BL6526B_2_SCLK_LOW();
	}
}

/*******************************************************************************
** 函数名称: SPI3_ReceiveByte
** 功能描述：
** 函数说明：接收到一个字节
** 作　  者:
** 日　  期:
*******************************************************************************/
uint8_t SPI_ReceiveByte(BL6526B_Type bL6526b_type)
{
    uint8_t i,byte = 0;
	if(bL6526b_type == BL6526B_1)
	{
		for(i=0;i<8;i++)
		{
			BL6526B_1_SCLK_LOW();
			byte<<=1;
			if(BL6526B_1_MISO_IN)
				byte++;  
			BL6526B_1_SCLK_HIGH(); 
		}
		BL6526B_1_SCLK_LOW();
	}
	else if(bL6526b_type == BL6526B_1)
	{
		for(i=0;i<8;i++)
		{
			BL6526B_2_SCLK_LOW();
			byte<<=1;
			if(BL6526B_2_MISO_IN)
				byte++;  
			BL6526B_2_SCLK_HIGH(); 
		}
		BL6526B_2_SCLK_LOW();
	}
	return byte;
}
#endif

#if use_hardware_spi
/*******************************************************************************
** 函数名称: BL6526B_ReadData
** 功能描述：往里面传一个CMD = 0X00 + Address
** 函数说明：BL6526B_ReadData从BL6526B里面读数据出来 24bit数据
** 作　  者:
** 日　  期:
*******************************************************************************/

uint32_t BL6526B_ReadData(BL6526B_Type bL6526b_type, uint8_t Cmd)
{
    uint8_t byte[3];
    uint8_t j;
    uint32_t data = 0;

	if(bL6526b_type == BL6526B_1)
	{
		delay5us(1);
		BL6526B_1_CS_HIGH();
		delay5us(10);
		BL6526B_1_CS_LOW();
		delay5us(1);
	}
	else if(bL6526b_type == BL6526B_2)
	{
		delay5us(1);
		BL6526B_2_CS_HIGH();
		delay5us(10);
		BL6526B_2_CS_LOW();
		delay5us(1);
	}		
	//先往地址里面写一个字节 告诉BL6526B的哪个寄存器 
	SPI_SendByte(bL6526b_type,Cmd);
	delay5us(10);

	if(bL6526b_type == BL6526B_1)
	{
		//开始接收3个字节数据
		for(j=0;j<3;j++)
		{
			byte[j] = SPI_ReceiveByte(BL6526B_1);
		}
	}
	else if(bL6526b_type == BL6526B_2)
	{
		//开始接收3个字节数据
		for(j=0;j<3;j++)
		{
			byte[j] = SPI_ReceiveByte(BL6526B_2);
		}
	}
    data = (data|byte[0])<<8;
    data = (data|byte[1])<<8;
    data = (data|byte[2]);
    return data ;

}

/*******************************************************************************
** 函数名称: BL6526B_WriteData
** 功能描述：往里面传一个CMD = 0X00 + Address
** 函数说明：BL6526B_WriteData
** 作　  者:
** 日　  期:
*******************************************************************************/
void BL6526B_WriteData(BL6526B_Type bL6526b_type,uint8_t Cmd,uint8_t byte1,uint8_t byte2,uint8_t byte3)
{
	
	if(bL6526b_type == BL6526B_1)
	{
		BL6526B_1_CS_LOW();
	}
	else if(bL6526b_type == BL6526B_2)
	{
		BL6526B_2_CS_LOW();
	}
	delay5us(3);
	//先往地址里面写一个字节 告诉BL6526B的哪个寄存器 
	SPI_SendByte(bL6526b_type,Cmd);
	SPI_SendByte(bL6526b_type,byte1);
	SPI_SendByte(bL6526b_type,byte2);
	SPI_SendByte(bL6526b_type,byte3);

	
	if(bL6526b_type == BL6526B_1)
	{
		delay5us(3);
		BL6526B_1_CS_HIGH();
	}
	else if(bL6526b_type == BL6526B_2)
	{
		delay5us(3);
		BL6526B_2_CS_HIGH();
	}
}
/*******************************************************************************
** 函数名称: SPI_SendByte
** 功能描述：
** 函数说明：发送一个字节到BL6526B
** 作　  者:
** 日　  期:
*******************************************************************************/
void SPI_SendByte(BL6526B_Type bL6526b_type,uint8_t byte)
{
	if(bL6526b_type == BL6526B_1)
	{
		SPI2_ReadWriteByte(byte);
	}
	else if(bL6526b_type == BL6526B_2)
	{
		SPI2_ReadWriteByte(byte);
	}
}

/*******************************************************************************
** 函数名称: SPI3_ReceiveByte
** 功能描述：
** 函数说明：接收到一个字节
** 作　  者:
** 日　  期:
*******************************************************************************/
uint8_t SPI_ReceiveByte(BL6526B_Type bL6526b_type)
{
    uint8_t byte = 0;
	if(bL6526b_type == BL6526B_1)
	{
		byte = SPI2_ReadWriteByte(0xFF);
	}
	else if(bL6526b_type == BL6526B_1)
	{
		byte = SPI2_ReadWriteByte(0xFF);
	}
	return byte;
}
#endif

void BL6526B_ProcessTask(CollectionData *electricdata)
{
	BL6526BStruct bl6526bdata;
    float v,i,p,e;
    float sum = 0.0;
    uint8_t x = 0;
    v=i=p=e=0.0;

//    BL6526B_WriteData(BL6526B_1,(BL6526B_WRPROT | 0x40),0x00,0x00,0x55);		// 8bits 关闭写保护
//    BL6526B_WriteData(BL6526B_1,(BL6526B_MODE | 0x40),0x00,0x00,0x00);		//12bits 使用高通滤波器，能量累加模式采用绝对值累加
//																	//使用功率阈值比较方式防潜动，关闭定时防潜方式
//    BL6526B_WriteData(BL6526B_1,(BL6526B_GAIN | 0x40),0x00,0x00,0x03);		//12bits 电流增益8倍，电压1倍增益
//    BL6526B_WriteData(BL6526B_1,(BL6526B_MASK | 0x40),0x00,0x00,0x28);		//12bits 中断屏蔽寄存器，过压，过流中断，
//    BL6526B_WriteData(BL6526B_1,(BL6526B_V_PKLVL | 0x40),0x00,0x03,0xEA);		//12bits 240V左右会过压
//    BL6526B_WriteData(BL6526B_1,(BL6526B_I_PKLVL | 0x40),0x00,0x03,0xE8);		//12bits 5A电流左右会过流

    bl6526bdata.status = (BL6526B_ReadData(BL6526B_1,BL6526B_STATUS));		/* 中断状态寄存器*/

    bl6526bdata.wrprot = (BL6526B_ReadData(BL6526B_1,BL6526B_WRPROT));		/* 写保护设置寄存器*/

    bl6526bdata.mode = (BL6526B_ReadData(BL6526B_1,BL6526B_MODE));			/* 工作模式寄存器*/

    bl6526bdata.gain = (BL6526B_ReadData(BL6526B_1,BL6526B_GAIN));			/* 增益寄存器*/

    bl6526bdata.mask = (BL6526B_ReadData(BL6526B_1,BL6526B_MASK));			/* 中断屏蔽寄存器*/

    bl6526bdata.v_pklvl = (BL6526B_ReadData(BL6526B_1,BL6526B_V_PKLVL));		/* 电压峰值门限寄存器*/

    bl6526bdata.i_pklvl = (BL6526B_ReadData(BL6526B_1,BL6526B_I_PKLVL));		/* 电流峰值门限寄存器*/

    bl6526bdata.v_rms = (BL6526B_ReadData(BL6526B_1,BL6526B_V_RMS));			/* 电压有效值寄存器*/

    bl6526bdata.i_rms = (BL6526B_ReadData(BL6526B_1,BL6526B_I_RMS));			/* 电流有效值寄存器*/

//    bl6526bdata.v_wave = (BL6526B_ReadData(BL6526B_1,BL6526B_V_WAVE));		/* 电压波形寄存器*/
//    myprintf("BL6526B1_V_WAVE = %d\r\n", bl6526bdata.v_wave);

//    bl6526bdata.i_wave = (BL6526B_ReadData(BL6526B_1,BL6526B_I_WAVE));		/* 电流波形寄存器*/
//    myprintf("BL6526B1_I_WAVE = %d\r\n", bl6526bdata.i_wave);

//    bl6526bdata.v_peak = (BL6526B_ReadData(BL6526B_1,BL6526B_V_PEAK));		/* 电压瞬态峰值寄存器*/
//    myprintf("BL6526B1_V_PEAK = %d\r\n", bl6526bdata.v_peak);

//    bl6526bdata.i_peak = (BL6526B_ReadData(BL6526B_1,BL6526B_I_PEAK));		/* 电流瞬态峰值寄存器*/
//    myprintf("BL6526B1_I_PEAK = %d\r\n", bl6526bdata.i_peak);

//    bl6526bdata.watt = (BL6526B_ReadData(BL6526B_1,BL6526B_WATT));			/* 平均有功功率寄存器*/
//    myprintf("BL6526B1_WATT = %d\r\n", bl6526bdata.watt);

//    bl6526bdata.va = (BL6526B_ReadData(BL6526B_1,BL6526B_VA));				/* 平均视在功率寄存器*/
//    myprintf("BL6526B1_VA = %d\r\n", bl6526bdata.va);

//    bl6526bdata.pf = (BL6526B_ReadData(BL6526B_1,BL6526B_PF));				/* 功率因子寄存器*/
//    for(x = 1; x <= 23; x++)
//    {
//        sum += ((float)pow(2,x-24)) * (bl6526bdata.pf>>(x-1) & (0x01));
//    }
//    myprintf("BL6526B1_PF = %f\r\n", sum);

    bl6526bdata.vahr = (BL6526B_ReadData(BL6526B_1,BL6526B_VAHR));			/* 视在能量寄存器*/

    v = bl6526bdata.v_rms / 12335.58;								/* 12335.58是实际测试中220V 每v的寄存器值 */
    i = bl6526bdata.i_rms / 636789.47;								/* 555914.9是实际测试中1A 每ma的寄存器值 */
    p = v * i;
    e = bl6526bdata.vahr / 20253000.0;

    if (((uint32_t)(i * 1000)) < 10)								//电流小于0.01A 则将其设置为0
        electricdata->Current = 0;
    else
        electricdata->Current = (uint32_t)(i * 1000);				//单位0.001mA

    if (((uint32_t)(v * 1000)) < 1000)								//电压小于1V 则将其设置为0
        electricdata->Voltage = 0;
    else
        electricdata->Voltage = (uint32_t)(v * 1000);				//单位0.001mV

    if (((uint32_t)(p * 1000)) < 1000)								//功率小于1W 则将其设置为0
        electricdata->Power = 0;
    else
        electricdata->Power = (uint32_t)(p * 10);

    electricdata->Energy = (uint32_t)(e * 100);


//    myprintf("--------------------------------------\r\n");			/*分割*/
//    myprintf("BL6526B1_STATUS = 0X%02X\r\n", bl6526bdata.status);
//    myprintf("BL6526B1_WRPROT = 0x%02X\r\n", bl6526bdata.wrprot);
//    myprintf("BL6526B1_MODE = 0x%02X\r\n", bl6526bdata.mode);
//    myprintf("BL6526B1_GAIN = 0x%02X\r\n", bl6526bdata.gain);
//    myprintf("BL6526B1_MASK = 0x%02X\r\n", bl6526bdata.mask);
//    myprintf("BL6526B1V_PKLVL = 0x%02X\r\n", bl6526bdata.v_pklvl);
//    myprintf("BL6526B1_I_PKLVL = 0x%02X\r\n", bl6526bdata.i_pklvl);
//    myprintf("BL6526B1_V_RMS = %d\r\n", bl6526bdata.v_rms);
//    myprintf("BL6526B1_I_RMS = %d\r\n", bl6526bdata.i_rms);
//    myprintf("BL6526B1_VAHR = %d\r\n", bl6526bdata.vahr);
//    myprintf("V1 = %d mV,I1 = %d mA,P = %d mW\n",electricdata->Voltage,electricdata->Current,(uint32_t)(p * 1000));
//		myprintf("V = %.1fV,A = %.2fA,P = %.2fW,E = %f° \n",v,i,p,e);
//    myprintf("--------------------------------------\r\n");			/*分割*/
	Bl6526bState_Process(&bl6526bdata,electricdata);								/*设置电量采集芯片状态(正常,过流,过压,过零)*/

}

/*******************************************************************************
** 函数名称: NVIC_Configuration
** 功能描述：
** 函数说明：配置嵌套向量中断控制器NVIC
** 作　  者:
** 日　  期:
*******************************************************************************/
void NVIC_Configuration(void)
{
//  NVIC_InitTypeDef NVIC_InitStructure;
//
//  /* 配置NVIC为优先级组1 */
//  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
//
//  /* 配置中断源：按键1 */
//  NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
//  /* 配置抢占优先级 */
//  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
//  /* 配置子优先级 */
//  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
//  /* 使能中断通道 */
//  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//  NVIC_Init(&NVIC_InitStructure);

}
/*******************************************************************************
** 函数名称: BL6526B_EXTI_Config
** 功能描述：
** 函数说明：配置 IO为EXTI中断口，并设置中断优先级
** 作　  者:
** 日　  期:
*******************************************************************************/
void BL6526B_EXTI_Config(void)
{
//	GPIO_InitTypeDef GPIO_InitStructure;
//	EXTI_InitTypeDef EXTI_InitStructure;

//	/*开启按键GPIO口的时钟*/
//	RCC_APB2PeriphClockCmd((RCC_APB2Periph_GPIOB|RCC_APB2Periph_AFIO),ENABLE);			//因为要用到AFIO_EXTICRx这个寄存器
//
//	/* 配置 NVIC 中断*/
//	NVIC_Configuration();
//
///*--------------------------BL6526B的配置-----------------------------*/
//	/* 选择按键用到的GPIO */
//  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
//  /* 配置为浮空输入 */
//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
//  GPIO_Init(GPIOB, &GPIO_InitStructure);

//	/* 选择EXTI的信号源 */
//  GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource10);			//将GPIOB连接到EXIT_line10,通过配置AFIO_EXTICRx这个寄存器
//  EXTI_InitStructure.EXTI_Line = EXTI_Line10;
//
//	/* EXTI为中断模式 */
//  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
//	/* 下降沿中断 */
//  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
//  /* 使能中断 */
//  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
//  EXTI_Init(&EXTI_InitStructure);
}

/*******************************************************************************
** 函数名称: EXTI15_10_IRQHandler
** 功能描述：
** 函数说明：BL6526B中断处理函数
** 作　  者:
** 日　  期:
*******************************************************************************/
//void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
//{
//	uint32_t temp;
//    //确保是否产生了EXTI Line中断
//	if(GPIO_Pin == GPIO_PIN_4)
//	{
//		temp = (BL6526B_ReadData(BL6526B_2,BL6526B_STATUS));
//		if (bl6526b2data.status & 0x20)
//		{
//				_myprintf("\n******************BL6526B电压过载********************\n");
////				BL6526B_WriteData((BL6526B_STATUS | 0x40),0x00,0x00,0x00);
////				IRMT_S1.Bl6526bState = 0X02;
////				temp = (BL6526B_ReadData(BL6526B_STATUS));
////				_myprintf("BL6526B_STATUS = 0x%X\n",temp);
//		}
//		if (bl6526b2data.status & 0x08)
//		{
//				_myprintf("\n******************BL6526B电流过载********************\n");
//		}
//	}
//}

/*******************************************************************************
** 函数名称: Bl6526bState_Process
** 功能描述：
** 函数说明：设置电量采集芯片状态(正常,过流,过压,过零)函数
** 作　  者:
** 日　  期:
*******************************************************************************/
void Bl6526bState_Process(BL6526BStruct *bl6526bdata,CollectionData *electricdata)
{
    if (bl6526bdata->status & 0x28)
    {
        electricdata->Bl6526bState = 0X04;
    }
    else if (bl6526bdata->status & 0x08)
    {
        electricdata->Bl6526bState = 0X02;
    }
	else if (bl6526bdata->status & 0x20)
    {
        electricdata->Bl6526bState = 0X01;
    }
    else
    {
        electricdata->Bl6526bState = 0X00;
    }

    switch(electricdata->Bl6526bState)
    {
    case 0x00:		//正常
        myprintf("\n******************BL6526B正常********************\n");;
        break;

    case 0x01:		//过压
        myprintf("\n******************BL6526B电流过压********************\n");
        break;

    case 0x02:		//过流
        myprintf("\n******************BL6526B电流过流********************\n");
        break;

    case 0x04:		//同时过压过流
        myprintf("\n******************BL6526B电流过压过流********************\n");
        break;
    }

}
