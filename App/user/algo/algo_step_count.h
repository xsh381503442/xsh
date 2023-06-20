#ifndef _ALGO_STEP_COUNT_H__
#define _ALGO_STEP_COUNT_H__
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "algo_config.h"
#include "watch_config.h"
#define NO_USE_STEP_ALGO 0
#define USE_STEP_ALGO   1
#define STEP_PATTERN_DAILY 0 //日常模式
#define STEP_PATTERN_SPORT 1 //运动模式

#define SPEED_ZERO    0
#define SPEED_SLOW    1
#define SPEED_GENERAL 2
#define SPEED_NORMAL  3
#define SPEED_FAST    4
#define SPEED_RUN     5



typedef struct
{
  uint8_t  tag,next_num,temp_num;
	uint8_t remain_data_size;
	int wave_point[64];
	uint32_t amount;
	uint32_t passometer_mun;
	uint32_t step_num_sport;
	float step_next[48];
	float save_next_acc[24];
	float acc_temp[24];
	int last_point;
}step_data;

typedef struct
{

 uint32_t tmpcount,last_tmpcount,last_checkstep,tmp_checkstep;
 uint8_t tmptime ;
 uint8_t step_count_num;
 uint8_t step_count_time;
 int status_flag ;
}step_monitor_count;

typedef struct
 { 
 float acc_x[128];
 float acc_y[128];
 float acc_z[128];
 float acc_cpmplex[256]; 
 float acc_zero;
  uint16_t  get_data_flag,get_data_signle_len,get_data_sum,acc_position;
}acc_s;


typedef struct
{
uint8_t sign_flag;
int8_t sign;
float std_value,min_acc,min_time,max_time,limit_valley,value_valley,differ_wave_valley;
}pedo_data;
#if defined WATCH_RAISE_BRIGHT_SCREEN_ALGO
typedef struct
{
uint16_t num,num_neg;
float angle_initial,anglez_sum;
uint8_t screen_flag;
}screen_data;
#else
#endif
extern step_data  step_data_str;

extern float std_seconds_data (float *data,uint16_t len);
extern float mean_data(float *data,uint16_t len,uint8_t flag);
extern float max_min_data(float *data,uint16_t len,uint8_t flag);

extern void  step_amount_count(acc_s* acc,uint16_t size1,step_data *data,step_monitor_count *step);

extern uint8_t pedometer(acc_s* accp,uint8_t size,uint8_t size_seconds,step_data *data,step_monitor_count*step );

extern uint8_t cross_mean(acc_s* accp,float mean_c,uint8_t len,uint8_t flag);

extern int8_t sign_sum(float*data,uint16_t k,uint8_t flag,step_data*acc_str);
extern void step_count_entrance(uint16_t len,acc_s *acc_data);
extern int32_t get_data(float*data);

extern void set_step_status(uint8_t status);
extern uint8_t get_step_status(void);
extern void monitor_count_init(void);
//extern void store_step_Data(int32_t data,uint32_t count,int32_t mean,uint32_t len);
extern void init_steps(uint32_t steps,uint32_t steps_sport);
#if defined WATCH_RAISE_BRIGHT_SCREEN_ALGO
extern void brighten_screen_entrance(uint16_t len,acc_s *acc_data,uint16_t flag);
extern void brighten_screen_init(void);
#else 
#endif

#endif

