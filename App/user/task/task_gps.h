#ifndef _TASK_GPS_H__
#define _TASK_GPS_H__

#include "drv_ublox.h"
#include "watch_config.h"
#define BUFFER_GPS_LEN_MAX  (1200)
#define BUFFER_I2C_REAS_LEN_MAX (255)

#define TIME_NAV_GPS_NONE               (0)//��������Ч
#define TIME_NAV_GPS_SEND_AND_INVALID   (1)//���ͼ�����뵫��Ȼ��Ч
#define TIME_NAV_GPS_VALID            (2)//���ͼ���������յ�������Ч


#define TIME_GET_LEAP_SECOND_STATUS_NONE  (0)//δ��ȡ������
#define TIME_GET_LEAP_SECOND_STATUS_FIRST (1)//��һ�ν�������ɹ�
#define TIME_GET_LEAP_SECOND_STATUS_SECOND_WAIT (2)//�ڶ��εȴ���������ɹ�
#define TIME_GET_LEAP_SECOND_STATUS_SECOND (3)//�ڶ��ν�������ɹ�

extern void CreateGPSTask(void);
extern void CloseGPSTask(void);
extern void TaskGPSTime(void* pvParameter);
extern void ublox_init(void);

extern void gps_value_get(GPSMode *p_gps);

extern void gps_value_clear(void);
//��ȡ����
extern int32_t GetGpsLon(void);
//��ȡγ��
extern int32_t GetGpsLat(void);
//��ȡgps״̬

extern bool GetGpsStatus (void);
extern uint16_t GetGpspdop (void);
extern int32_t GetGpsAlt(void);


#if defined STORE_ALT_TEST_VERTION
extern int32_t GetGpsAlt(void);
extern void GetGpsSn(uint8_t *sn);
extern uint16_t GetGpsHdop(void);
#endif

extern bool GetGPSScreenStatus(void);
extern uint32_t g_VO2max_Step;//����������Ĳ���
#if defined(WATCH_PRECISION_TIME_CALIBRATION)
extern uint32_t get_s_reset_gpsrmc_count(void);
extern uint8_t s_is_open_nav_time_gps;
#endif

#endif
