#ifndef ALGO_SPORT_CYCLING_H
#define ALGO_SPORT_CYCLING_H

#include "algo_trackoptimizing.h"

//#define		STATIC_STATUS_CYCLING	1	//骑行静止状态
//#define		NONSTATIC_STATUS_CYCLING	0	//骑行非静止状态
#define		ACC_STATIC_DELAY_TIME	5	//判断骑行静止所需的缓冲时间
#define		AXIS_NUM 3	//算法中所需轴的数目

/**************************/
#define WATCH_STEP_ALGO
/*****************************/

#if defined WATCH_STEP_ALGO
#define		ACC_STATIC_SINGLE_THRESHOLD	100	//加速度三轴单个差值静止阈值
#define     ACC_STATIC_MEAN_THRESHOLD	200	//加速度三轴均值差值静止阈值
#define		SECOND_DATA_NUM_THREHOLD	(200*AXIS_NUM)	//一秒钟数据组数的阈值，暂设为200
#else
#define		ACC_STATIC_SINGLE_THRESHOLD	200	//加速度三轴单个差值静止阈值
#define     ACC_STATIC_MEAN_THRESHOLD	400	//加速度三轴均值差值静止阈值
#define		SECOND_DATA_NUM_THREHOLD	(400*AXIS_NUM)	//一秒钟数据组数的阈值，暂设为200
#endif

#define STATIC_JUDGMENT_AXIS 3	//静止判断所需的轴数
extern void algo_sport_cycling_init(void);
extern void algo_sport_cycling_start(int16_t* liv_acc_data, const uint16_t liv_i, const uint16_t len);
void staticjudgment_diff(int16_t* liv_acc_data);
void staticjudgment_cycling(void);
extern uint8_t staticjudgment_cycling_result(void);

#endif
