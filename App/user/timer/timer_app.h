#ifndef __TIMER_APP_H__
#define __TIMER_APP_H__

#include <stdbool.h>
#include <stdint.h>
#include "algo_hdr.h"
#include "watch_config.h"
#define COUNTDOWN_COUNTER_MS_12_MINUTE (720000)//12分钟倒计时
#define COUNTDOWN_COUNTER_MS_30_MINUTE (1800000)//30分钟倒计时
#define TIMER_INTERVAL_MS_1000   1000  //1s定时间隔
#define TIMER_INTERVAL_MS_LOCK  (59000)//59s锁屏
#define TIMER_INTERVAL_MS_UNLOCK_HINT (3000)//解锁错误3s自动返回定时
#define TIMER_INTERVAL_MS_VO2MAX  (1000)   //用于界面刷新
#define TIMER_INTERVAL_MS_VO2MAX_COUNTDOWN (1000)//用于倒计时
#define TIMER_INTERVAL_MS_TOOL 	(2000)//用于工具界面超时
#define TIMER_INTERVAL_MS_PRECISION_TIME_JUMP_TEST  (30000)//用于精密授时时间跳变测试
#define TIMER_EARLY_END_VO2MAX   (600000)//600000ms 测试10分钟
#define TIMER_LINGHANG_BEIDOU_SEND_WAIT_MS  (1000)//领航消息等待
#define TIMER_INTERVAL_MS_AUTO_SET_UTC_TIMEOUT  (90000)//用于自动授时 90s超时
#define TIMER_EARLY_END_LACTATE  (1200000)//1200000ms 测试20分钟
#define TIMER_DETECT_LEAP_SECOND  (1000)//用于GPS授时时 闰秒检测
#define TIMER_SPORT_RECORD_DETAIL  (120000)//120s运动总结详情界面超时处理 无任何按键操作,则自动返回待机界面
#define TIMER_PRESSURE_DETECTION   (30000)//30s气压检测提醒
#define TIMER_AUTO_BACK_HOME       (600000)//10分钟无任何操作（无按键操作）返回待机界面
#define TIMER_COUNTDOWN_LEAP_SECOND  (1000)//1s闰秒模式 倒计时用

#define TIMER_ID_COUNTDOWN     1  //倒计时
#define TIMER_ID_HDR           2  //心率
#define TIMER_ID_VO2MAX        3  //最大摄氧量12分钟倒计时
#define TIMER_ID_LOCK          4  //自动锁屏
#define TIMER_ID_UNLOCK_HINT     5  //解锁错误3s自动返回定时
#define TIMER_ID_VO2MAX_COUNTDOWN  6
#define TIMER_ID_STEP_CLIBRATION    7 //步长校准界面刷新
#define TIMER_ID_TOOL    			8 //工具界面2S超时
#define TIMER_ID_PRECISION_TIME_JUMP_TEST  	9//精密授时时间跳变定时器
#define TIMER_ID_PAUSE 10	 //暂停界面定时器
#define TIMER_ID_LINGHANG_BEIDOU_SEND_WAIT  	11//领航消息发送等待定时器
#define TIMER_ID_AUTO_SET_UTC_TIMEOUT  	12//精密授时时间跳变定时器
#define TIMER_ID_DETECT_LEAP_SECOND     13//用于GPS授时时 闰秒检测
#define TIMER_ID_SPORT_RECORD_DETAIL     14//120s运动总结详情界面超时处理 无任何按键操作,则自动返回待机界面
#define TIMER_ID_PRESSURE_DETECTION       15//领航气压检测提醒
#define TIMER_ID_AUTO_BACK_HOME           16//10分钟无任何操作（无按键操作）返回待机界面
#define TIMER_ID_COUNTDOWN_LEAP_SECOND         18//1s闰秒模式 倒计时用

#define TIMER_TICKS_TO_WAIT  100

#define DISTINGUISH_VO2     (1)  //最大摄氧量界面进入
#define DISTINGUISH_LACTATE (2)  //乳酸阈界面进入
#define DISTINGUISH_STEP    (3)  //计步校准界面进入

#define DETECT_LEAP_SECOND_MAX_TIME  (13*60000)//闰秒检测时间最长  13分钟
extern Countdown_time_t time_countdown;;
extern uint32_t get_steps_countdown_mile(void);
extern float get_steps_countdown_min_km(void);
extern void timer_app_init(void);
extern void timer_app_uninit(void);
extern void timer_app_countdown_start(void);
extern void timer_app_countdown_stop(bool is_isr);
extern void timer_app_hdr_start(void);
extern void timer_app_hdr_stop(bool is_isr);
extern void timer_app_VO2max_start(void);
extern void timer_app_VO2max_stop(bool is_isr);
extern void timer_app_VO2max_Countdown_start(uint32_t time);
extern void timer_app_VO2max_Countdown_stop(bool is_isr);
extern void timer_app_LOCK_start(void);
extern void timer_app_LOCK_stop(bool is_isr);
extern void timer_app_UNLOCK_HINT_start(void);
extern void timer_app_UNLOCK_HINT_stop(bool is_isr);
extern void timer_app_step_calibration_start(void);
extern void timer_app_step_calibration_stop(bool is_isr);
extern void timer_app_tool_start(void);
extern void timer_app_tool_stop(bool is_isr);
#if defined WATCH_SIM_SPORT || defined WATCH_COM_SPORT
extern void timer_app_pasue_start(void);
extern void timer_app_pasue_stop(bool is_isr);
#endif
#if defined COD
extern void timer_rt_cod_start(void);
extern void timer_rt_cod_stop(bool is_isr);
extern void timer_cod_sensor_start(void);
extern void timer_cod_sensor_stop(bool is_isr);
#endif
#ifdef WATCH_COD_BSP
extern void timer_lcd_start(void);
#endif
#ifdef WATCH_LCD_ESD
extern void timer_read_lcd_start(void);
#endif

extern uint8_t get_average_time_hdr(void);
extern void InitDistanceAlgo_VO2maxLactate(void);
extern void UninitVO2maxMeasing(void);
extern uint32_t total_hdr_value,total_hdr_count,total_hdr_VO2max_value,total_hdr_VO2max_count;
extern uint8_t Distinguish_VO2_Lactate;
extern uint8_t Average_Hdr_Value;
extern uint32_t mile_minute_lactate;
extern uint32_t g_VO2maxLactate_SpeedTotal,g_VO2maxLactate_Count;
extern uint32_t Step_VO2CalInit,Step_VO2maxOrCalibration,StepDiff_VO2maxOrCalibration;
extern bool CalibrationStep_GPSFlag;
extern uint32_t calibration_step,calibration_mile;
#if defined(WATCH_PRECISION_TIME_JUMP_TEST)
extern void timer_app_precision_time_jump_test_start(void);
extern void timer_app_precision_time_jump_test_stop(bool is_isr);
#endif
extern void  timer_app_vice_pasue_start(uint32_t num,uint8_t flag);
extern void timer_app_vice_pasue_stop(bool is_isr);

#if defined(WATCH_NOT_LEAP_SECOND_SET_TIME)
/*闰秒检测定时器 开始*/
extern void timer_app_detect_leap_second_start(void);
extern void timer_app_detect_leap_second_stop(bool is_isr);
#endif
extern void timer_app_sport_record_detail_start(void);
extern void timer_app_sport_record_detail_stop(bool is_isr);

#if defined(WATCH_AUTO_BACK_HOME)
extern void timer_app_auto_back_home_start(void);
extern void timer_app_auto_back_home_stop(bool is_isr);
#endif

#endif
