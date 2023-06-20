#ifndef __TIMER_APP_H__
#define __TIMER_APP_H__

#include <stdbool.h>
#include <stdint.h>
#include "algo_hdr.h"
#include "watch_config.h"
#define COUNTDOWN_COUNTER_MS_12_MINUTE (720000)//12���ӵ���ʱ
#define COUNTDOWN_COUNTER_MS_30_MINUTE (1800000)//30���ӵ���ʱ
#define TIMER_INTERVAL_MS_1000   1000  //1s��ʱ���
#define TIMER_INTERVAL_MS_LOCK  (59000)//59s����
#define TIMER_INTERVAL_MS_UNLOCK_HINT (3000)//��������3s�Զ����ض�ʱ
#define TIMER_INTERVAL_MS_VO2MAX  (1000)   //���ڽ���ˢ��
#define TIMER_INTERVAL_MS_VO2MAX_COUNTDOWN (1000)//���ڵ���ʱ
#define TIMER_INTERVAL_MS_TOOL 	(2000)//���ڹ��߽��泬ʱ
#define TIMER_INTERVAL_MS_PRECISION_TIME_JUMP_TEST  (30000)//���ھ�����ʱʱ���������
#define TIMER_EARLY_END_VO2MAX   (600000)//600000ms ����10����
#define TIMER_LINGHANG_BEIDOU_SEND_WAIT_MS  (1000)//�캽��Ϣ�ȴ�
#define TIMER_INTERVAL_MS_AUTO_SET_UTC_TIMEOUT  (90000)//�����Զ���ʱ 90s��ʱ
#define TIMER_EARLY_END_LACTATE  (1200000)//1200000ms ����20����
#define TIMER_DETECT_LEAP_SECOND  (1000)//����GPS��ʱʱ ������
#define TIMER_SPORT_RECORD_DETAIL  (120000)//120s�˶��ܽ�������泬ʱ���� ���κΰ�������,���Զ����ش�������
#define TIMER_PRESSURE_DETECTION   (30000)//30s��ѹ�������
#define TIMER_AUTO_BACK_HOME       (600000)//10�������κβ������ް������������ش�������
#define TIMER_COUNTDOWN_LEAP_SECOND  (1000)//1s����ģʽ ����ʱ��

#define TIMER_ID_COUNTDOWN     1  //����ʱ
#define TIMER_ID_HDR           2  //����
#define TIMER_ID_VO2MAX        3  //���������12���ӵ���ʱ
#define TIMER_ID_LOCK          4  //�Զ�����
#define TIMER_ID_UNLOCK_HINT     5  //��������3s�Զ����ض�ʱ
#define TIMER_ID_VO2MAX_COUNTDOWN  6
#define TIMER_ID_STEP_CLIBRATION    7 //����У׼����ˢ��
#define TIMER_ID_TOOL    			8 //���߽���2S��ʱ
#define TIMER_ID_PRECISION_TIME_JUMP_TEST  	9//������ʱʱ�����䶨ʱ��
#define TIMER_ID_PAUSE 10	 //��ͣ���涨ʱ��
#define TIMER_ID_LINGHANG_BEIDOU_SEND_WAIT  	11//�캽��Ϣ���͵ȴ���ʱ��
#define TIMER_ID_AUTO_SET_UTC_TIMEOUT  	12//������ʱʱ�����䶨ʱ��
#define TIMER_ID_DETECT_LEAP_SECOND     13//����GPS��ʱʱ ������
#define TIMER_ID_SPORT_RECORD_DETAIL     14//120s�˶��ܽ�������泬ʱ���� ���κΰ�������,���Զ����ش�������
#define TIMER_ID_PRESSURE_DETECTION       15//�캽��ѹ�������
#define TIMER_ID_AUTO_BACK_HOME           16//10�������κβ������ް������������ش�������
#define TIMER_ID_COUNTDOWN_LEAP_SECOND         18//1s����ģʽ ����ʱ��

#define TIMER_TICKS_TO_WAIT  100

#define DISTINGUISH_VO2     (1)  //����������������
#define DISTINGUISH_LACTATE (2)  //�����н������
#define DISTINGUISH_STEP    (3)  //�Ʋ�У׼�������

#define DETECT_LEAP_SECOND_MAX_TIME  (13*60000)//������ʱ���  13����
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
/*�����ⶨʱ�� ��ʼ*/
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
