#ifndef __LED_APP_H
#define __LED_APP_H

#include "main.h"

#define HARDWARE_PWM		0

//#define Led0_Write(x)				HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, x)
#define PWM1_Write(x)				HAL_GPIO_WritePin(PWM1_GPIO_Port, PWM1_Pin, x)

#define REL_OFF		GPIO_PIN_SET
#define REL_ON		GPIO_PIN_RESET

#define REL1_Write(x)				HAL_GPIO_WritePin(REL_EN_GPIO_Port, REL_EN_Pin, x)

#define Led_Switch_Brightness(x) 	LedPwm = x

#define LED_SUMMER_START			4	//led策略夏令时4月开始
#define LED_SUMMER_END				9	///led策略夏令时9月结束

#define LED_SHOW_CONTINUE_TIME		(600*10)	//10分钟

typedef enum{
	LEDOFF	= 0,
	LEDON	= 1,
}LedStatus_Type;

#pragma pack(push,1)
typedef struct
{
    uint8_t  start_hour;
    uint8_t  start_min;
    uint16_t phase1_time;
    uint8_t  phase1_Pwm;
	uint16_t phase2_time;
    uint8_t  phase2_Pwm;
    uint16_t phase3_time;
    uint8_t  phase3_Pwm;
	uint16_t phase4_time;
    uint8_t  phase4_Pwm;
}TimeControl_Type;

typedef struct
{
	uint8_t start_sec1;
    uint8_t start_min1;
	uint8_t start_hour1;
    uint8_t led_status1;
    uint8_t phase1_Pwm1;
	uint8_t start_sec2;
    uint8_t start_min2;
	uint8_t start_hour2;
    uint8_t led_status2;
    uint8_t phase1_Pwm2;
	uint8_t start_sec3;
    uint8_t start_min3;
	uint8_t start_hour3;
    uint8_t led_status3;
    uint8_t phase1_Pwm3;
	uint8_t start_sec4;
    uint8_t start_min4;
	uint8_t start_hour4;
    uint8_t led_status4;
    uint8_t phase1_Pwm4;
	uint8_t start_sec5;
    uint8_t start_min5;
	uint8_t start_hour5;
    uint8_t led_status5;
    uint8_t phase1_Pwm5;
	uint8_t	count;		//策略个数
}LoraTimeControl_Type;
#pragma pack(pop)

extern TimeControl_Type TimeControl_Data;
extern LoraTimeControl_Type LoraTimeControl_Data;	//时控模式结构体

extern uint8_t LedPwm;
extern uint8_t targetBrightness;
extern uint8_t LedStatus;
extern uint8_t ledshowFlag;

extern uint8_t Get_TimeControl_Data(TimeControl_Type *timecontrol_data);
extern void Write_TimeControl_Data(TimeControl_Type *timecontrol_data);
extern void Write_LoraTimeControl_Data(LoraTimeControl_Type *loratimecontrol_data);
extern void Get_LoraTimeControl_Data(LoraTimeControl_Type *loratimecontrol_data);

extern void Write_LED_Data(uint8_t *ledstatus,uint8_t *ledpwm);
extern void Get_LED_Data(uint8_t *ledstatus,uint8_t *ledpwm);

extern void Led_Pwm_Control(void);
extern void Switch_Brightness(void);
extern void Led_pwm_change(uint8_t ledpwm);

extern void CreatLedTask(void);

#endif
