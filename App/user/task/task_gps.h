#ifndef _TASK_GPS_H__
#define _TASK_GPS_H__

#include "drv_ublox.h"
#include "watch_config.h"
#define BUFFER_GPS_LEN_MAX  (1200)
#define BUFFER_I2C_REAS_LEN_MAX (255)

#define TIME_NAV_GPS_NONE               (0)//闰秒检测无效
#define TIME_NAV_GPS_SEND_AND_INVALID   (1)//发送检测闰秒但仍然无效
#define TIME_NAV_GPS_VALID            (2)//发送检测闰秒且收到闰秒有效


#define TIME_GET_LEAP_SECOND_STATUS_NONE  (0)//未获取到闰秒
#define TIME_GET_LEAP_SECOND_STATUS_FIRST (1)//第一次接收闰秒成功
#define TIME_GET_LEAP_SECOND_STATUS_SECOND_WAIT (2)//第二次等待接收闰秒成功
#define TIME_GET_LEAP_SECOND_STATUS_SECOND (3)//第二次接收闰秒成功

extern void CreateGPSTask(void);
extern void CloseGPSTask(void);
extern void TaskGPSTime(void* pvParameter);
extern void ublox_init(void);

extern void gps_value_get(GPSMode *p_gps);

extern void gps_value_clear(void);
//获取经度
extern int32_t GetGpsLon(void);
//获取纬度
extern int32_t GetGpsLat(void);
//获取gps状态

extern bool GetGpsStatus (void);
extern uint16_t GetGpspdop (void);
extern int32_t GetGpsAlt(void);


#if defined STORE_ALT_TEST_VERTION
extern int32_t GetGpsAlt(void);
extern void GetGpsSn(uint8_t *sn);
extern uint16_t GetGpsHdop(void);
#endif

extern bool GetGPSScreenStatus(void);
extern uint32_t g_VO2max_Step;//最大摄氧量的步数
#if defined(WATCH_PRECISION_TIME_CALIBRATION)
extern uint32_t get_s_reset_gpsrmc_count(void);
extern uint8_t s_is_open_nav_time_gps;
#endif

#endif
