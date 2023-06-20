#ifndef _ALGO_TIME_CALIBRATION_H__
#define _ALGO_TIME_CALIBRATION_H__
#include "algo_config.h"
#include "bsp_rtc.h"
#include "com_data.h"
#include "watch_config.h"

#define TEST_ADJ_MAX_MS      60000  //ADJ校验时 时间误差不能大于1分钟 即 60000ms
#define TIME_DIFF_APP_WATCH   60  //app与watch时间之差 单位秒
#define TIME_DAY_MS         (86400000)//一天的毫秒数
#define TIME_DAY_MAX_MS     (7776000000)//90天的毫秒数
#define TIME_HOUR_MS        (3600000)//1小时的毫秒数
#define TIME_HOUR_BASE_CAL  (24) // 每24小时为一个周期补偿
//获取flash中精密时间保存的相关参数及进行相关配置 Adj值和Tcalibration值状态
#define STATUS_FLASH_NO_ADJ_NO_TCALIBRATION     (100)
#define STATUS_FLASH_NO_ADJ_HAS_TCALIBRATION    (101)
#define STATUS_FLASH_HAS_ADJ_NO_TCALIBRATION    (102)
#define STATUS_FLASH_HAS_ADJ_HAS_TCALIBRATION   (103)

#define EXCEPTION_NONE                         (0)   //精密时间校准初始状态 删除过
#define EXCEPTION_ADJ_VALUE_VALID              (1000)//精密时间校准ADJ值有效
#define EXCEPTION_ADJ_VALUE_INVALID            (1001)//精密时间校准ADJ值异常  值过大
#define EXCEPTION_CALIBRATION_VALUE_VALID      (1002)//精密时间校准Calibration值有效
#define EXCEPTION_CALIBRATION_VALUE_INVALID    (1003)//精密时间校准Calibration值异常 值过大
#define EXCEPTION_ADJ_TIMEOUT                  (1004)//精密时间校准ADJ时间超过额定区间
#define EXCEPTION_CALIBRATION_TIMEOUT          (1005)//精密时间校准Calibration时间超过额定区间
#define EXCEPTION_CALIBRATION_CORRECT          (1006)//精密时间补偿过时间
#define EXCEPTION_CALIBRATION_WRONG            (1007)//精密时间补偿时间时出错
#define EXCEPTION_MOUNT_GPS_CALIBRATION        (1008)//精密时间自动补偿中

#define STATUS_CLOSE_DIFFTIME   0  //关闭GPS获取GPS时间
#define STATUS_OPEN_DIFFITME    1  //开启GPS获取GPS时间
#define STATUS_OPEN_DIFFITME_COMPLETE    2  //开启GPS获取GPS时间成功
typedef struct
{
	am_hal_rtc_time_t m_time; //从gps上获取的时间
	int64_t gps_rtc_value;//gps与rtc相差的毫秒数
}difftime_gps_str;

#if defined(WATCH_GPS_OR_NO_GPS_RESOURCE_SET_TIME)
#define SET_TIME_RESOURCE_BY_RESET             (10)  //设置RTC时间来源-重启授时
#define SET_TIME_RESOURCE_BY_GPS               (11)  //设置RTC时间来源-GPS授时
#define SET_TIME_RESOURCE_BY_MANUAL            (12)  //设置RTC时间来源-手动授时
#define SET_TIME_RESOURCE_BY_BLE               (13)  //设置RTC时间来源-蓝牙授时
#define SET_TIME_RESOURCE_BY_CALIBRATION       (14)  //设置RTC时间来源-校准完成后补偿授时
extern uint8_t get_s_set_time_resource(void);
extern void set_s_set_time_resource(uint8_t resource);
#endif

#if defined(WATCH_PRECISION_TIME_CALIBRATION)
extern bool IsApptoWatchTime(rtc_time_t app,rtc_time_t watch);
extern void set_s_is_mcu_restart_and_need_set_time(bool status);
extern bool get_s_is_mcu_restart_and_need_set_time(void);
extern bool is_set_real_time_to_rtc(void);
#else
extern bool IsApptoWatchTime(void);
#endif
extern void CTimerB3TickHandler(void );
extern uint8_t ReadCalibrationValueAndConfigAdjValue(void);
extern void TimeCalibrationInGps(void);
extern bool TimeCalibrationCompensation(void);
extern void Reset_TimeCalibration(void);
extern bool IsTimeValid(TimeStr time);
extern void Set_GpsFirstTimingStatus(bool status);
extern void SaveCalibrationData(void);
extern void Set_TimeCalibrationStatus(uint8_t status);
extern uint8_t Get_TimeCalibrationStatus(void);
extern void Set_TimeCalibrationHintStatus(TimeCalibrationStatus status);
extern TimeCalibrationStatus Get_TimeCalibrationHintStatus(void);
extern void Set_TimeCalibrationStr(TimeCalibrationStr status);
extern TimeCalibrationStr Get_TimeCalibrationStr(void);
extern void Set_TimeAdjStr(TimeAdjStr status);
extern TimeAdjStr Get_TimeAdjStr(void);
extern void Set_TimeGPSStr(am_hal_rtc_time_t status);
extern am_hal_rtc_time_t Get_TimeGPSStr(void);
extern void Set_TimeRTCStr(am_hal_rtc_time_t status);
extern am_hal_rtc_time_t Get_TimeRTCStr(void);
extern void GetAddTime(am_hal_rtc_time_t *AddRtc);
extern void Set_T1GPSStr(am_hal_rtc_time_t status);
extern am_hal_rtc_time_t Get_T1GPSStr(void);
extern void config_XT(void);
extern void set_open_difftime_status(uint8_t status);
extern uint8_t get_open_difftime_status(void);
extern void open_difftime_rtc_gps_time(void);
extern void close_difftime_rtc_gps_time(void);
extern void set_difftime_gps_time(difftime_gps_str status);
extern difftime_gps_str get_difftime_gps_time(void);
extern void get_gps_rtc_ms_value(am_hal_rtc_time_t gps_time,am_hal_rtc_time_t rtc_time);

#if defined(WATCH_PRECISION_TIME_JUMP_TEST)
extern uint32_t get_precision_time_jump_open_gps_count(void);
extern uint32_t get_precision_time_compare_exception_count(void);
#endif

#endif
