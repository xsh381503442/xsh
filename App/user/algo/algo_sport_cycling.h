#ifndef ALGO_SPORT_CYCLING_H
#define ALGO_SPORT_CYCLING_H

#include "algo_trackoptimizing.h"

//#define		STATIC_STATUS_CYCLING	1	//���о�ֹ״̬
//#define		NONSTATIC_STATUS_CYCLING	0	//���зǾ�ֹ״̬
#define		ACC_STATIC_DELAY_TIME	5	//�ж����о�ֹ����Ļ���ʱ��
#define		AXIS_NUM 3	//�㷨�����������Ŀ

/**************************/
#define WATCH_STEP_ALGO
/*****************************/

#if defined WATCH_STEP_ALGO
#define		ACC_STATIC_SINGLE_THRESHOLD	100	//���ٶ����ᵥ����ֵ��ֹ��ֵ
#define     ACC_STATIC_MEAN_THRESHOLD	200	//���ٶ������ֵ��ֵ��ֹ��ֵ
#define		SECOND_DATA_NUM_THREHOLD	(200*AXIS_NUM)	//һ����������������ֵ������Ϊ200
#else
#define		ACC_STATIC_SINGLE_THRESHOLD	200	//���ٶ����ᵥ����ֵ��ֹ��ֵ
#define     ACC_STATIC_MEAN_THRESHOLD	400	//���ٶ������ֵ��ֵ��ֹ��ֵ
#define		SECOND_DATA_NUM_THREHOLD	(400*AXIS_NUM)	//һ����������������ֵ������Ϊ200
#endif

#define STATIC_JUDGMENT_AXIS 3	//��ֹ�ж����������
extern void algo_sport_cycling_init(void);
extern void algo_sport_cycling_start(int16_t* liv_acc_data, const uint16_t liv_i, const uint16_t len);
void staticjudgment_diff(int16_t* liv_acc_data);
void staticjudgment_cycling(void);
extern uint8_t staticjudgment_cycling_result(void);

#endif
