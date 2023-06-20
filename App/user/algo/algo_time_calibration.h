#ifndef _ALGO_TIME_CALIBRATION_H__
#define _ALGO_TIME_CALIBRATION_H__
#include "algo_config.h"
#include "bsp_rtc.h"
#include "com_data.h"
#include "watch_config.h"

#define TEST_ADJ_MAX_MS      60000  //ADJУ��ʱ ʱ�����ܴ���1���� �� 60000ms
#define TIME_DIFF_APP_WATCH   60  //app��watchʱ��֮�� ��λ��
#define TIME_DAY_MS         (86400000)//һ��ĺ�����
#define TIME_DAY_MAX_MS     (7776000000)//90��ĺ�����
#define TIME_HOUR_MS        (3600000)//1Сʱ�ĺ�����
#define TIME_HOUR_BASE_CAL  (24) // ÿ24СʱΪһ�����ڲ���
//��ȡflash�о���ʱ�䱣�����ز���������������� Adjֵ��Tcalibrationֵ״̬
#define STATUS_FLASH_NO_ADJ_NO_TCALIBRATION     (100)
#define STATUS_FLASH_NO_ADJ_HAS_TCALIBRATION    (101)
#define STATUS_FLASH_HAS_ADJ_NO_TCALIBRATION    (102)
#define STATUS_FLASH_HAS_ADJ_HAS_TCALIBRATION   (103)

#define EXCEPTION_NONE                         (0)   //����ʱ��У׼��ʼ״̬ ɾ����
#define EXCEPTION_ADJ_VALUE_VALID              (1000)//����ʱ��У׼ADJֵ��Ч
#define EXCEPTION_ADJ_VALUE_INVALID            (1001)//����ʱ��У׼ADJֵ�쳣  ֵ����
#define EXCEPTION_CALIBRATION_VALUE_VALID      (1002)//����ʱ��У׼Calibrationֵ��Ч
#define EXCEPTION_CALIBRATION_VALUE_INVALID    (1003)//����ʱ��У׼Calibrationֵ�쳣 ֵ����
#define EXCEPTION_ADJ_TIMEOUT                  (1004)//����ʱ��У׼ADJʱ�䳬�������
#define EXCEPTION_CALIBRATION_TIMEOUT          (1005)//����ʱ��У׼Calibrationʱ�䳬�������
#define EXCEPTION_CALIBRATION_CORRECT          (1006)//����ʱ�䲹����ʱ��
#define EXCEPTION_CALIBRATION_WRONG            (1007)//����ʱ�䲹��ʱ��ʱ����
#define EXCEPTION_MOUNT_GPS_CALIBRATION        (1008)//����ʱ���Զ�������

#define STATUS_CLOSE_DIFFTIME   0  //�ر�GPS��ȡGPSʱ��
#define STATUS_OPEN_DIFFITME    1  //����GPS��ȡGPSʱ��
#define STATUS_OPEN_DIFFITME_COMPLETE    2  //����GPS��ȡGPSʱ��ɹ�
typedef struct
{
	am_hal_rtc_time_t m_time; //��gps�ϻ�ȡ��ʱ��
	int64_t gps_rtc_value;//gps��rtc���ĺ�����
}difftime_gps_str;

#if defined(WATCH_GPS_OR_NO_GPS_RESOURCE_SET_TIME)
#define SET_TIME_RESOURCE_BY_RESET             (10)  //����RTCʱ����Դ-������ʱ
#define SET_TIME_RESOURCE_BY_GPS               (11)  //����RTCʱ����Դ-GPS��ʱ
#define SET_TIME_RESOURCE_BY_MANUAL            (12)  //����RTCʱ����Դ-�ֶ���ʱ
#define SET_TIME_RESOURCE_BY_BLE               (13)  //����RTCʱ����Դ-������ʱ
#define SET_TIME_RESOURCE_BY_CALIBRATION       (14)  //����RTCʱ����Դ-У׼��ɺ󲹳���ʱ
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
