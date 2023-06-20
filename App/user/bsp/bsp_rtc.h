#ifndef BSP_RTC_H
#define BSP_RTC_H
#include <stdint.h>
#include <time.h>
#include "bsp_config.h"
typedef struct rtc_time_struct
{ 
    uint8_t Year;
    uint8_t Month;
    uint8_t Day;
    uint8_t Hour;
    uint8_t Minute;
    uint8_t Second;
}__attribute__((packed))rtc_time_t; //时间结构体

extern am_hal_rtc_time_t RTC_time;
extern void bsp_rtc_init( void );
extern void bsp_rtc_interrupt_set( void );
extern uint8_t CaculateWeekDay(int y,int m, int d);
extern uint8_t Is_Leap_Year(uint16_t year);
extern uint32_t get_time_diff(rtc_time_t start, rtc_time_t end);
extern uint8_t MonthMaxDay(uint8_t year,uint8_t month);
extern int16_t DaysDiff(int y1,int m1,int d1,int y2,int m2,int d2);
extern int64_t DiffTimeMs(am_hal_rtc_time_t rtc1,am_hal_rtc_time_t rtc2);

extern bool IsChange_ApptoWatch_TimeDiff(rtc_time_t start, rtc_time_t end,uint32_t difftime);
extern int32_t Get_DiffTime(rtc_time_t start, rtc_time_t end);
extern unsigned int GetTimeSecond(rtc_time_t start);
extern rtc_time_t GetYmdhms(unsigned int msec_total);

extern rtc_time_t RtcToUtc(int32_t sec);

#if defined WATCH_TIMEZONE_SET
enum{
	TIMEZONE_MDY = 0U,
	TIMEZONE_HNL,
	TIMEZONE_ANC,
	TIMEZONE_LAX,
	TIMEZONE_DEN,
	TIMEZONE_CHI,
	TIMEZONE_NYC,
	TIMEZONE_CCS,
	TIMEZONE_SCL,
	TIMEZONE_YYT,
	TIMEZONE_RIO,
	TIMEZONE_FEN,
	TIMEZONE_PDL,
	TIMEZONE_ZERO,//零时区,伦敦时间
	TIMEZONE_PAR,
	TIMEZONE_CAI,
	TIMEZONE_MOW,
	TIMEZONE_THR,
	TIMEZONE_DXB,
	TIMEZONE_KBL,
	TIMEZONE_KHI,
	TIMEZONE_DEL,
	TIMEZONE_KTM,
	TIMEZONE_DAC,
	TIMEZONE_RGN,
	TIMEZONE_BKK,
	TIMEZONE_BJS,
	TIMEZONE_TYO,
	TIMEZONE_ADL,
	TIMEZONE_SYD,
	TIMEZONE_NOU,
	TIMEZONE_WLG,
};

#define TIME_ONE_HOUR_SECOND (3600)  //一个小时的总秒数

extern int32_t get_timezone_value(uint16_t country);
extern am_hal_rtc_time_t get_local_time(am_hal_rtc_time_t m_utc_old_time
	                          ,int16_t m_old_country,int16_t m_new_country);
#endif

#endif 
