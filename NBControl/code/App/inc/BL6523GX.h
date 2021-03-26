#ifndef __BL6523GX_H
#define	__BL6523GX_H

#include "main.h"

#define BL6523Send				UART2Write

#define BL6523GX_1_NRST_HIGH()  HAL_GPIO_WritePin(BL_NRST_GPIO_Port, BL_NRST_Pin, GPIO_PIN_SET)
#define BL6523GX_1_NRST_LOW()   HAL_GPIO_WritePin(BL_NRST_GPIO_Port, BL_NRST_Pin, GPIO_PIN_RESET)

#define BL6523GX_1_CF_IN       GPIO_ReadInputDataBit(BL_CF_GPIO_Port,BL_CF_Pin)

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
#define BL6523G_CF_WATTHR      				0X01    	//24bitCF 脉冲能量
#define BL6523G_LINE_WATTHR					0X04		//24bit线周期累计有功能量寄存器
#define BL6523G_IA_RMS						0X05		//24bit电流A有效值寄存器，刷新率2.5HZ
#define BL6523G_IB_RMS						0X06		//24bit电流B有效值寄存器，刷新率2.5HZ
#define BL6523G_V_RMS						0X07		//24bit电压有效值寄存器，刷新率2.5HZ
#define BL6523G_PF							0X08		//24bit功率因子寄存器，刷新率2.5HZ
#define BL6523G_FREQ						0X09		//24bit线电压频率/周期寄存器
#define BL6523G_A_WATT						0X0A		//24bit电流A平均有功功率寄存器，补码，刷新率2.5HZ
#define BL6523G_VA							0X0B		//24bit平均视在功率寄存器，刷新率2.5HZ
#define BL6523G_COUNTER_CF					0X0C		//24BIT CF 脉冲个数
#define BL6523G_WATTHR						0X0D		//24bit B 或 A+B 通道有功能量寄存器
#define BL6523G_COUNTER_CFP					0X0E		//24bit正向 CF 脉冲个数
#define BL6523G_COUNTER_CFN					0X0F		//24bit反向 CF 脉冲个数
//#define BL6523G_VAHR						0X0D		//24bit视在能量寄存器
//#define BL6523G_PWAHR						0X0E		//24bit正功能量寄存器
//#define BL6523G_NWAHR						0X0F		//24bit负功能量寄存器
//#define BL6523G_I_PEAK						0X10		//24bit电流瞬态峰值寄存器，刷新率50HZ
#define BL6523G_V_PEAK						0X12		//24bit电压瞬态峰值寄存器，刷新率100HZ
#define BL6523G_B_WATT						0X13		//24bit电流 B 通道计量的平均有功功率寄存器，补码，刷新率 2.5Hz

//校表寄存器（外部写 0X3A除外）
#define BL6523G_MODE						0X14		//12bit工作模式寄存器
#define BL6523G_GAIN						0X15		//12bit增益寄存器
#define BL6523G_FAULTLVL					0X16		//12bit电流或两相功率不平衡屏蔽阈值寄存器（内部倍 2^8）
#define BL6523G_WA_CREEP					0X17		//24bit低12bit为：防潜动功率阈值寄存器（内部倍2^4，该值等于20ppm，最大FFF等于0.2%）
//#define BL6523G_WA_REVP						0X18		//12bit 反向指示阈值寄存器（内部倍2^8，该值等于0.1%，最大FFF00等于3%）
#define BL6523G_WA_CFDIV					0X19		//12bit 有功CF缩放比例寄存器
#define BL6523G_A_WATTOS					0X1A		//16bit A 通道有功功率偏置校准寄存器，补码
#define BL6523G_B_WATTOS					0X1B		//16bit B 通道有功功率偏置校准寄存器，补码
#define BL6523G_A_WATTGN					0X1C		//16bit A 通道有功功率增益调整寄存器，补码
#define BL6523G_B_WATTGN					0X1D		//16bit B 通道有功功率增益调整寄存器，补码
#define BL6523G_FREQ_SEL					0X1E		//24bit模拟电路频率控制寄存器。客户无需使用。
#define BL6523G_BG_CTRL						0X1F		//22bit模拟电路控制寄存器
#define BL6523G_V_PHCAL						0X20		//24bit电压V通道相位校正寄存器
														//1、低 8 位[7:0]为电流 A 通道相位校正寄存器 IA_PHCAL
														//2、中 8 位[15:8]为电流 B 通道相位校正寄存器 IB_PHCAL
														//3、高 8 位[23:16]为电压 V 通道相位校正寄存器 V_PHCAL
//#define BL6523G_VAOS						0X21		//12bit视在功率偏置校准寄存器，补码
#define BL6523G_BG_CTRL2					0X22		//17bit
//#define BL6523G_VAGN						0X22		//12bit视在功率增益调整寄存器，补码
//#define BL6523G_I_RMSGN						0X23		//12bit电流有效值增益调整寄存器，补码
//#define BL6523G_V_RMSGN						0X25		//12bit电压V有效值增益调整寄存器，补码
#define BL6523G_IA_RMSOS					0X26		//16bit电流A有效值偏置修正寄存器，补码，内部*4
#define BL6523G_IB_RMSOS					0X27		//16bit电流B有效值偏置修正寄存器，补码，内部*4
//#define BL6523G_V_RMSOS						0X28		//16bit电压V有效值偏置修正寄存器，补码，内部*4
#define BL6523G_ CHKSUM_ADJ					0X29		//24bit校验和调整
#define BL6523G_WA_LOS						0X2A		//24bit有功小信号补偿寄存器，高 12 位补偿 B 相，低12位补偿A相，补码
//#define BL6523G_I_CHOS						0X2B		//12bit电流通道偏置调整寄存器，补码
//#define BL6523G_V_CHOS						0X2D		//12bit电压V通道偏置调整寄存器，补码
#define BL6523G_IA_CHGN						0X2E		//16bit电流A通道增益调整寄存器，补码
#define BL6523G_IB_CHGN						0X2F		//16bit电流B通道增益调整寄存器，补码
#define BL6523G_V_CHGN						0X30		//16bit电压V通道增益调整寄存器，补码
#define BL6523G_LINECYC						0X31		//12bit线能量累加周期数寄存器
//#define BL6523G_ZXTOUT						0X32		//16bit过零超时寄存器
#define BL6523G_SAGCYC						0X33		//8bit跌落线周期寄存器
#define BL6523G_SAGLVL						0X34		//12bit跌落电压阈值寄存器
//#define BL6523G_I_PKLVL						0X35		//12bit电流A峰值门限寄存器
#define BL6523G_V_PKLVL						0X37		//12bit电压V峰值门限寄存器
#define BL6523G_AT_SEL						0X38		//16bit输出选择寄存器，详见 输出选择寄存器 说明
#define BL6523G_MASK						0X39		//12bit中断屏蔽寄存器，详见 中断屏蔽寄存器 说明
#define BL6523G_STATUS						0X3A		//16bit中断状态寄存器，详见 中断状态寄存器 说明

//特殊寄存器
#define BL6523G_READ						0X3B		//24bit 读出数据寄存器，记录上一次SPI读出的数据
#define BL6523G_WRITE						0X3C		//24bit 写入数据寄存器，记录上一次SPI写入的数据
#define BL6523G_CHKSUM						0X3D		//24bit 0x0111AF 校验寄存器，对多有可写校表寄存器的数值求和
#define BL6523G_WRPROT						0X3E		//8bit 写保护设置寄存器，写入 0x55 时，表示允许对可写寄存器写操作
#define BL6523G_Soft_nrst					0X3F		//24bit 当输入0x5A5A5A时，系统被reset，但需要打开写保护才能复位，复位数字部分电流。

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

typedef struct
{
    uint32_t wrprot;			/* 写保护设置寄存器*/
    uint32_t mode;				/* 工作模式寄存器*/
    uint32_t gain;				/* 增益寄存器*/
    uint32_t mask;				/* 中断屏蔽寄存器*/
    uint32_t v_pklvl;			/* 电压峰值门限寄存器*/
    uint32_t i_pklvl;			/* 电流峰值门限寄存器*/
    uint32_t v_rms;			/* 电压有效值寄存器*/
    uint32_t ia_rms;			/* 电流A有效值寄存器*/
    uint32_t ib_rms;			/* 电流B有效值寄存器*/
    uint32_t v_wave;			/* 电压波形寄存器*/
    uint32_t i_wave;			/* 电流波形寄存器*/
    uint32_t v_peak;			/* 电压瞬态峰值寄存器*/
    uint32_t i_peak;			/* 电流瞬态峰值寄存器*/
    uint32_t watt;				/* 平均有功功率寄存器*/
    uint32_t va;				/* 平均视在功率寄存器*/
    uint32_t pf;				/* 功率因子寄存器*/
    uint32_t vahr;				/* 视在能量寄存器*/
    uint32_t status;      		/* 中断状态寄存器*/
}BL6523GXStruct;

#pragma pack(push,1)
typedef struct
{
    uint32_t Voltage1;//当前电压  单位：mV   传输值/1000为实际值
    uint32_t Current1;//当前电流  单位：mA   传输值/1000为实际值
    uint32_t Energy1;//当前电压  单位：mV   传输值/1000为实际值
}ElectricData_Type; //stm32f103采集板 主要数据结构体
#pragma pack(pop)

extern CollectionData ElectricData;

extern uint8_t BL6523GX_CheckSelf(void);

extern void BL6523GX_Init(void);
extern void BL6523GX_ProcessTask(CollectionData *electricdata);

#endif
