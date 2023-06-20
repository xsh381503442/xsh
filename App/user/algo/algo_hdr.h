#ifndef __ALGO_HDR_H__
#define __ALGO_HDR_H__

#include <stdint.h>
#include <stdbool.h>
//#define HEARTBUF_SIZE  101

#define HEARTBUF_SIZE  60


typedef struct {
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
	uint8_t hundredths;
} Countdown_time_t;
extern uint8_t g_HeartNum;	//ȡ������
extern uint8_t g_HeartBuf[HEARTBUF_SIZE];	//�������
extern uint8_t g_MaxHeart;	//�������
extern uint8_t g_HeartRemind ;	//����������ֵ

extern uint8_t get_hdr_max(void);
extern uint8_t  get_hdr_range(uint8_t hdr_value);
extern uint8_t get_hdr_range_color(uint8_t hdr_value);
extern void save_heartrate(uint8_t heart);
extern uint8_t get_10s_heartrate(void);
extern uint8_t cal_stable_heartrate(void);
extern uint8_t  get_VO2max_range(uint8_t VO2max_value,uint8_t male,uint8_t age);
extern uint8_t get_VO2max_range_color(uint8_t VO2maxRange);
extern void get_countdown_time(uint32_t value,Countdown_time_t *countdownTime);
#endif
