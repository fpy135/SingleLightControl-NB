#ifndef BL6526B_H
#define BL6526B_H

#include "main.h"

#define use_software_spi		0

#if use_software_spi
	#define use_hardware_spi	0
#else
	#define use_hardware_spi	1
#endif

#if use_software_spi
/*   SPI4端口定义 */
#define BL6526B_1_SCLK_HIGH()		HAL_GPIO_WritePin(SPI2_SCK_GPIO_Port, SPI2_SCK_Pin, GPIO_PIN_SET)
#define BL6526B_1_SCLK_LOW()		HAL_GPIO_WritePin(SPI2_SCK_GPIO_Port, SPI2_SCK_Pin, GPIO_PIN_RESET)

#define BL6526B_1_MOSI_HIGH()		HAL_GPIO_WritePin(SPI2_MOSI_GPIO_Port, SPI2_MOSI_Pin, GPIO_PIN_SET)
#define BL6526B_1_MOSI_LOW()		HAL_GPIO_WritePin(SPI2_MOSI_GPIO_Port, SPI2_MOSI_Pin, GPIO_PIN_RESET)

#define BL6526B_1_MISO_IN			HAL_GPIO_ReadPin(SPI2_MISO_GPIO_Port,SPI2_MISO_Pin)


/*   SPI2端口定义 */
#define BL6526B_2_SCLK_HIGH()		HAL_GPIO_WritePin(SPI2_SCK_GPIO_Port, SPI2_SCK_Pin, GPIO_PIN_SET)
#define BL6526B_2_SCLK_LOW()		HAL_GPIO_WritePin(SPI2_SCK_GPIO_Port, SPI2_SCK_Pin, GPIO_PIN_RESET)

#define BL6526B_2_MOSI_HIGH()		HAL_GPIO_WritePin(SPI2_MOSI_GPIO_Port, SPI2_MOSI_Pin, GPIO_PIN_SET)
#define BL6526B_2_MOSI_LOW()		HAL_GPIO_WritePin(SPI2_MOSI_GPIO_Port, SPI2_MOSI_Pin, GPIO_PIN_RESET)

#define BL6526B_2_MISO_IN			HAL_GPIO_ReadPin(SPI2_MISO_GPIO_Port,SPI2_MISO_Pin)

#endif


/* 电能采集端口片选、复位、CF,IRQ端口表  */
#define BL6526B_1_CS_HIGH()   HAL_GPIO_WritePin(SPI2_CS_GPIO_Port, SPI2_CS_Pin, GPIO_PIN_SET)
#define BL6526B_1_CS_LOW()    HAL_GPIO_WritePin(SPI2_CS_GPIO_Port, SPI2_CS_Pin, GPIO_PIN_RESET)
#define BL6526B_1_RST_HIGH()  HAL_GPIO_WritePin(SPI2_RST_GPIO_Port, SPI2_RST_Pin, GPIO_PIN_SET)
#define BL6526B_1_RST_LOW()   HAL_GPIO_WritePin(SPI2_RST_GPIO_Port, SPI2_RST_Pin, GPIO_PIN_RESET)

#define BL6526B_1_CF_IN       GPIO_ReadInputDataBit(SPI2_CF_GPIO_Port,SPI2_CF_Pin)
#define BL6526B_1_IRQ_IN      GPIO_ReadInputDataBit(SPI2_IRQ_GPIO_Port,SPI2_IRQ_Pin)

/* 电能采集端口片选、复位、CF,IRQ端口表  */
#define BL6526B_2_CS_HIGH()   HAL_GPIO_WritePin(SPI2_CS_GPIO_Port, SPI2_CS_Pin, GPIO_PIN_SET)
#define BL6526B_2_CS_LOW()    HAL_GPIO_WritePin(SPI2_CS_GPIO_Port, SPI2_CS_Pin, GPIO_PIN_RESET)
#define BL6526B_2_RST_HIGH()  HAL_GPIO_WritePin(SPI2_RST_GPIO_Port, SPI2_RST_Pin, GPIO_PIN_SET)
#define BL6526B_2_RST_LOW()   HAL_GPIO_WritePin(SPI2_RST_GPIO_Port, SPI2_RST_Pin, GPIO_PIN_RESET)

#define BL6526B_2_CF_IN       GPIO_ReadInputDataBit(SPI2_CF_GPIO_Port,SPI2_CF_Pin)
#define BL6526B_2_IRQ_IN      GPIO_ReadInputDataBit(SPI2_IRQ_GPIO_Port,SPI2_IRQ_Pin)

typedef enum
{
	BL6526B_1 = 0,
	BL6526B_2 = 1,
}BL6526B_Type;

typedef struct
{
    uint32_t Power;//当前功率（视在功率）单位： 0.1W  传输值/1000为实际值
    uint32_t Current;//当前电流  单位：mA   传输值/1000为实际值
    uint32_t Voltage;//当前电压  单位：0.1V   传输值/10为实际值
    uint8_t Bl6526bState;//电量采集芯片的状态（过零、过流和过压三种状态）
    uint8_t RelayState;//继电器状态 开/关 两种状态
	uint8_t LedPwm;		//pwm调光值
    uint32_t Energy;//能量（视在能量） 单位： 0.01KW/h 传输值/100为实际值
}CollectionData; //stm32f103采集板 主要数据结构体

#pragma pack(push,1)
typedef struct
{
    uint32_t Voltage1;//当前电压  单位：mV   传输值/1000为实际值
    uint32_t Current1;//当前电流  单位：mA   传输值/1000为实际值
    uint32_t Voltage2	;//当前电压  单位：mV   传输值/1000为实际值
	uint32_t Current2;//当前电流  单位：mA   传输值/1000为实际值
}ElectricData_Type; //stm32f103采集板 主要数据结构体
#pragma pack(pop)

typedef struct
{
    uint32_t wrprot;			/* 写保护设置寄存器*/
    uint32_t mode;				/* 工作模式寄存器*/
    uint32_t gain;				/* 增益寄存器*/
    uint32_t mask;				/* 中断屏蔽寄存器*/
    uint32_t v_pklvl;			/* 电压峰值门限寄存器*/
    uint32_t i_pklvl;			/* 电流峰值门限寄存器*/
    uint32_t v_rms;			/* 电压有效值寄存器*/
    uint32_t i_rms;			/* 电流有效值寄存器*/
    uint32_t v_wave;			/* 电压波形寄存器*/
    uint32_t i_wave;			/* 电流波形寄存器*/
    uint32_t v_peak;			/* 电压瞬态峰值寄存器*/
    uint32_t i_peak;			/* 电流瞬态峰值寄存器*/
    uint32_t watt;				/* 平均有功功率寄存器*/
    uint32_t va;				/* 平均视在功率寄存器*/
    uint32_t pf;				/* 功率因子寄存器*/
    uint32_t vahr;				/* 视在能量寄存器*/
    uint32_t status;      		/* 中断状态寄存器*/
}BL6526BStruct;


/*前端增益调整 可编程增益放大器PAG 增益寄存器GAIN*/
#define PAGGain_1             				0x00
#define PAGGain_2             				0x01
#define PAGGain_4             				0x02
#define PAGGain_8             				0x03
#define PAGGain_16            				0x04
#define PAGGain_24            				0x05
#define PAGGain_32							0x06
#define PAGGain_48            				0x07


/************************************************************************/
/*  bl6526b register map     寄存器映射      0x3f= 0011 1111      bit6=1 write, bit6=0 read                       */
/************************************************************************/
//以上为电参量寄存器（内部写）
#define BL6526B_I_WAVE        				0X01    	//24bit电流波形寄存器，补码，刷新率14KHZ
#define BL6526B_V_WAVE						0X03		//24bit电压V波形寄存器，补码，刷新率14KHZ
#define BL6526B_LINE_WATTHR					0X04		//24bit线周期累计有功能量寄存器
#define BL6526B_I_RMS						0X05		//24bit电流有效值寄存器，刷新率10HZ
#define BL6526B_V_RMS						0X07		//24bit电压有效值寄存器，刷新率10HZ
#define BL6526B_PF							0X08		//24bit功率因子寄存器，刷新率10HZ
#define BL6526B_FREQ						0X09		//24bit线电压频率/周期寄存器
#define BL6526B_WATT						0X0A		//24bit平均有功功率寄存器，补码，刷新率10HZ
#define BL6526B_VA							0X0B		//24bit平均视在功率寄存器，刷新率10HZ
#define BL6526B_WATTHR						0X0C		//24bit有功能量寄存器
#define BL6526B_VAHR						0X0D		//24bit视在能量寄存器
#define BL6526B_PWAHR						0X0E		//24bit正功能量寄存器
#define BL6526B_NWAHR						0X0F		//24bit负功能量寄存器
#define BL6526B_I_PEAK						0X10		//24bit电流瞬态峰值寄存器，刷新率50HZ
#define BL6526B_V_PEAK						0X12		//24bit电压瞬态峰值寄存器，刷新率50HZ

//校表寄存器（外部写 0X3A除外）
#define BL6526B_MODE						0X14		//12bit工作模式寄存器
#define BL6526B_GAIN						0X15		//12bit增益寄存器
#define BL6526B_WA_CREEP					0X17		//24bit低12bit为：防潜动功率阈值寄存器（内部倍2^4，该值等于20ppm，最大FFF等于0.2%）
#define BL6526B_WA_REVP						0X18		//12bit 反向指示阈值寄存器（内部倍2^8，该值等于0.1%，最大FFF00等于3%）
#define BL6526B_WA_CFDIV					0X19		//12bit 有功CF缩放比例寄存器
#define BL6526B_WATTOS						0X1A		//12bit有功功率偏置校准寄存器，补码
#define BL6526B_WATTGN						0X1C		//12bit有功功率增益调整寄存器，补码
#define BL6526B_I_PHCAL						0X1E		//8bit电流通道相位校正寄存器（【7】为使能位，1.1us/1LSB,最大FF可调2.54°）
#define BL6526B_V_PHCAL						0X20		//8bit电压V通道相位校正寄存器（同上）
#define BL6526B_VAOS						0X21		//12bit视在功率偏置校准寄存器，补码
#define BL6526B_VAGN						0X22		//12bit视在功率增益调整寄存器，补码
#define BL6526B_I_RMSGN						0X23		//12bit电流有效值增益调整寄存器，补码
#define BL6526B_V_RMSGN						0X25		//12bit电压V有效值增益调整寄存器，补码
#define BL6526B_I_RMSOS						0X26		//12bit电流有效值偏置修正寄存器，补码，内部*4
#define BL6526B_V_RMSOS						0X28		//12bit电压V有效值偏置修正寄存器，补码，内部*4
#define BL6526B_RMS_CREEP					0X29		//12bit有效值小信号阈值寄存器，内部*4
#define BL6526B_WA_LOS						0X2A		//24bit有功小信号补偿寄存器，高12位保留，低12位补偿A相，补码
#define BL6526B_I_CHOS						0X2B		//12bit电流通道偏置调整寄存器，补码
#define BL6526B_V_CHOS						0X2D		//12bit电压V通道偏置调整寄存器，补码
#define BL6526B_I_CHGN						0X2E		//12bit电流通道增益调整寄存器，补码
#define BL6526B_V_CHGN						0X30		//12bit电压V通道增益调整寄存器，补码
#define BL6526B_LINECYC						0X31		//12bit线能量累加周期数寄存器
#define BL6526B_ZXTOUT						0X32		//16bit过零超时寄存器
#define BL6526B_SAGCYC						0X33		//8bit跌落线周期寄存器
#define BL6526B_SAGLVL						0X34		//12bit跌落电压阈值寄存器
#define BL6526B_I_PKLVL						0X35		//12bit电流A峰值门限寄存器
#define BL6526B_V_PKLVL						0X37		//12bit电压V峰值门限寄存器
#define BL6526B_AT_SEL						0X38		//16bit输出选择寄存器，详见 输出选择寄存器 说明
#define BL6526B_MASK						0X39		//12bit中断屏蔽寄存器，详见 中断屏蔽寄存器 说明
#define BL6526B_STATUS						0X3A		//12bit中断状态寄存器，详见 中断状态寄存器 说明

//特殊寄存器
#define BL6526B_READ						0X3B		//24bit 读出数据寄存器，记录上一次SPI读出的数据
#define BL6526B_WRITE						0X3C		//24bit 写入数据寄存器，记录上一次SPI写入的数据
#define BL6526B_CHKSUM						0X3D		//24bit 0x0111AF 校验寄存器，对多有可写校表寄存器的数值求和
#define BL6526B_WRPROT						0X3E		//8bit 写保护设置寄存器，写入 0x55 时，表示允许对可写寄存器写操作
#define BL6526B_Soft_nrst					0X3F		//24bit 当输入0x5A5A5A时，系统被reset，但需要打开写保护才能复位，复位数字部分电流。

extern CollectionData ElectricData;

extern void delay5us(uint16_t time);


extern void SPI_Configuration(void);
void BL6526B_Init(void);
extern uint32_t BL6526B_ReadData(BL6526B_Type bL6526b_type,uint8_t Cmd);
extern void BL6526B_WriteData(BL6526B_Type bL6526b_type,uint8_t Cmd,uint8_t byte1,uint8_t byte2,uint8_t byte3);
extern void SPI_SendByte(BL6526B_Type bL6526b_type,uint8_t byte);
extern uint8_t SPI_ReceiveByte(BL6526B_Type bL6526b_type);

extern void BL6526B_EXTI_Config(void);
extern void NVIC_Configuration(void);
extern void BL6526B_ProcessTask(CollectionData *electricdata);
extern void Bl6526bState_Process(BL6526BStruct *bl6526bdata,CollectionData *electricdata);


#endif

