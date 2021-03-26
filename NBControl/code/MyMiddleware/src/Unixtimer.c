#include "Unixtimer.h"
 
#define FOURYEARDAY (365+365+365+366)  //4年一个周期内的总天数（1970~2038不存在2100这类年份，故暂不优化）
#define TIMEZONE    8                  //北京时区调整 

static uint8_t month_day[12]={31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}; //平年 
static uint8_t Leap_month_day[12]={31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}; //闰年 
const uint16_t dayPerYear[4] = {365, 365, 365, 366};

rtc_time_t RtcTime;
uint32_t UnixTimsStamp;
 
// 判断是否是闰年 
// year: 需要判断的年 
// return：1：闰年
//	  	   0: 平年 
uint8_t isLeapYear(uint16_t year)
{
	uint8_t res=0;
	
	if(year%4 == 0) // 能够被4整除 
	{
		if((year%100 == 0) && (year%400 != 0))	//能够被100整除，但是不能够被400整除 
		{
			res = 0;
		}
		else
		{
			res =1;
		}
	}
	return res;
}
 
// 将Unix时间戳转换为北京时间
// unixTime: 需要判断的Unix时间戳 
// *tempBeijing:返回的北京时间
// return：none
// note：没对输入参数正确性做判断
void covUnixTimeStp2Beijing(uint32_t unixTime, rtc_time_t *tempBeijing)
{
    uint32_t totleDaynum=0, totleSecNum=0;
    uint16_t remainDayofYear;
    uint8_t *pr, tempYear=0;
    
 
    totleDaynum = unixTime/(24*60*60); //总天数(注意加括号)
    totleSecNum = unixTime%(24*60*60); //当天剩余的秒速
	if(totleSecNum>0)
		totleDaynum++;
    memset(tempBeijing, 0x00, sizeof(rtc_time_t));
    //1.计算哪一年
	totleDaynum = totleDaynum- 366-365-365;
    tempBeijing->ui8Year = 1973 + (totleDaynum/FOURYEARDAY)*4;
    remainDayofYear = totleDaynum%FOURYEARDAY;
    while(remainDayofYear >= dayPerYear[tempYear]){
        remainDayofYear -= dayPerYear[tempYear];
        tempBeijing->ui8Year++;
        tempYear++;
    }
    
    //2.计算哪一月的哪一天
    pr = isLeapYear(tempBeijing->ui8Year)?Leap_month_day:month_day;
    while(remainDayofYear > *(pr+tempBeijing->ui8Month))
    {
		remainDayofYear -= *(pr+tempBeijing->ui8Month);
        tempBeijing->ui8Month++;
    }
    tempBeijing->ui8Month++; //month
    tempBeijing->ui8DayOfMonth = remainDayofYear; //day
  
    //3.计算当天时间
    tempBeijing->ui8Hour = totleSecNum/3600;
    tempBeijing->ui8Minute = (totleSecNum%3600)/60; //error：变量搞错
    tempBeijing->ui8Second = (totleSecNum%3600)%60;
 
    //4.时区调整
    tempBeijing->ui8Hour +=TIMEZONE; 
    if(tempBeijing->ui8Hour>23){
        tempBeijing->ui8Hour -= 24;
        tempBeijing->ui8DayOfMonth++;
    }
}
 
// 将北京时间转换为Unix时间戳 
// year: 需要判断的年 
// return：Unix时间戳（从1970/1/1 00:00:00 到现在的秒数) 
// note：没对输入参数正确性做判断
uint32_t covBeijing2UnixTimeStp(rtc_time_t *beijingTime)
{
	uint32_t daynum=0, SecNum=0; //保存北京时间到起始时间的天数
	uint16_t tempYear=1970, tempMonth=0;
 
 
	//1.年的天数 
	while(tempYear < beijingTime->ui8Year) 
	{
		if(isLeapYear(tempYear)){
			daynum += 366;
		}
		else{
			daynum += 365;
		}
		tempYear++;
	}
	//2.月的天数
 	while(tempMonth < beijingTime->ui8Month-1) 
 	{
        if(isLeapYear(beijingTime->ui8Year)){ //闰年
            daynum += Leap_month_day[tempMonth];
        }
        else{
		    daynum += month_day[tempMonth];
        }
		tempMonth++;
	}
    //3.天数
	daynum += (beijingTime->ui8DayOfMonth-1);
 
    //4.时分秒
    SecNum = daynum*24*60*60; //s    
    SecNum += beijingTime->ui8Hour*60*60;    
    SecNum += beijingTime->ui8Minute*60;    
    SecNum += beijingTime->ui8Second;
 
    //5.时区调整
    SecNum -= TIMEZONE*60*60;
 
    return SecNum;
}
