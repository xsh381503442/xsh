#ifndef ALGO_TRACKOPTIMIZING_H
#define	ALGO_TRACKOPTIMIZING_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "com_sport.h"
#include "algo_sport.h"
#include "algo_sport_cycling.h"

#define     R_EARTH 6371004      //����뾶����λ��

#define     PI 3.1415926

#define     PERSEC_STEP_SIZE 5 //���㲽Ƶʱ������
#define     PERSEC_SPEED_SIZE 7 //�������ٶ�ʱ������

#define     DELDRIFT_POINTS_SIZE 7     

#define     SMOOTH_POINTS_SIZE 8   //ƽ���㷨����ĵ���

#define     OPTIMIZING_POINTS_SIZE  (DELDRIFT_POINTS_SIZE+SMOOTH_POINTS_SIZE-2)   //21  �Ż��㷨�ܵ��������

#define     OPTIMIZING_RESULT_POINTS_SIZE  SMOOTH_POINTS_SIZE/2    // 4 һ���Թ켣�Ż�����Ĵ�С

#define     STEPSCOMP_SIZE 5   //�Ʋ�ʱ���沽���Ĵ洢��С

#define		MAX_STEPS_THRESHOLD	10	//��λʱ���ڲ����������ֵ��10������ֹ�쳣����

#define		STATIC_STEPS_THRESHOLD	5 //��ֹ��־�Ĳ�����ֵ

#define		SPEED_DELAY_TIME	3	//�ٶ��ӳ���ʾ��ʱ�䣬��λ��s

#define		SPEED_MINLIMIT 0.28f  //�������벽������ٶ�����ֵ
/*�������ٶ�������ֵ:5.5km/h.1.53m/s

�����ٶ�������ֵ5.5m/s

*/


/*#define     WALK_SPEED_MAXLIMIT_ZERO 0.5f
#define     WALK_SPEED_MAXLIMIT_SLOW 1.32f
#define     WALK_SPEED_MAXLIMIT_GRNERAL 1.98f
#define     WALK_SPEED_MAXLIMIT_NORMAL 2.74f
#define     WALK_SPEED_MAXLIMIT_FAST 3.96f
#define     RUN_SPEED_MAXLIMIT 8.33f*/	//��������ٶ�����ֵ

#define     WALK_SPEED_MAXLIMIT_ZERO 0.5f
#define     WALK_SPEED_MAXLIMIT_SLOW 1.35f
#define     WALK_SPEED_MAXLIMIT_GRNERAL 2.05f
#define     WALK_SPEED_MAXLIMIT_NORMAL 2.85f
#define     WALK_SPEED_MAXLIMIT_FAST 4.05f
#define     RUN_SPEED_MAXLIMIT 8.33f	//��������ٶ�����ֵ


#define     CYCLING_SPEED_MAXLIMIT 27.78f	//��������ٶ�����ֵ����λ��m/s

#define			DELAY_SMOOTH_ARGUMENT 0.4f  //˲ʱ�ٶ��ͺ��˲���Ȩ��ȡֵ

#define     STEP_STRIDE_MAX_SPEED 5 //У׼�����趨�ĵ�λʱ���ڵ�������

#define		NOGPS_MAX_DISTANCE_FOOT 12 //�����࣬���ǻ�������ԭ�����GPS�ڽ������ľ����������λ��

#define		NOGPS_MAX_DISTANCE_CYCLE 24 //�����࣬���ǻ�������ԭ�����GPS�ڽ������ľ����������λ��

#define		GPS_NORMAL 0 //GPSû���쳣

#define		GPS_DRIFT_EXCEPTION 1 //GPSƯ���쳣

#define		NOGPS_DIST_EXCEPTION 2 //GPS���Ǿ�������쳣

#define		SPORT_FLAG_STATIS 0 //��ֹ״̬

#define		SPORT_FLAG_MOVE 1 //�Ǿ�ֹ״̬

#define		TIME_THS    20     //��Ƶ�����ʱ����

#define		SPORTS_STATUS_END 2	//Ƕ��ʽ�˹涨������״̬ʱ����ͣ��־��Ϊ2

#define   PAUSE_STATUS 1 	//��ͣ״̬ʱ����־Ϊ1

#define   NON_PAUSE_STATUS 0 	//����ͣ״̬ʱ����־Ϊ0

#define   DOWN_GPS_FLAG 0 	//GPS�����߱�־

#define   UP_GPS_FLAG 1 	//GPS���߱�־

#define   REMOVE_START_GPS_SUM 3 	//ȥ����ʼ���GPS����

#define   REMOVE_NOISE_DATA_SUM 3

#define   SMOOTH_DATA_SUM 7

#define   SMOOTH_DATA_TSUM 7

#define   KF_DATA_SUM  7

#define   REMOVE_NOISE_DATA_DISTANCE_VALUE 1.0

#define   START_STEP_ARRAY_SUM 7

#define  OUTPUT_DATA_SUM 7

#define  OUTPUT_DATA_SUM_PRE 7

#define  DISCARD_SM_DATA 0

#define  GET_SM_DATA 1


typedef struct __attribute__((packed))
{
  
    double data[REMOVE_NOISE_DATA_SUM][2];     //�洢�账��ĵľ����±�0����γ�ȣ��±�1��
   
    uint8_t pausestatus[REMOVE_NOISE_DATA_SUM];  //�洢��ͣ״̬�����ڼ��������滮�켣���
    int32_t middle_data_raw[2];
    uint8_t data_number;
	uint32_t data_sum;   //������ЧԪ�صĸ���
}gps_data_rn;

typedef struct __attribute__((packed))
{
  
    double data[SMOOTH_DATA_SUM][2];     //�洢�账��ĵľ����±�0����γ�ȣ��±�1��
    uint8_t pausestatus[SMOOTH_DATA_SUM];  //�洢��ͣ״̬�����ڼ��������滮�켣���
    uint8_t data_number;
	uint32_t data_sum;   //������ЧԪ�صĸ���
	uint32_t data_tsum;  
	uint8_t output_data_sum;
	uint8_t data_smdata_get_flag;
}gps_data_sm;


typedef struct __attribute__((packed))
{
  
    int32_t data[OUTPUT_DATA_SUM][2];     //�洢�账��ĵľ����±�0����γ�ȣ��±�1��
    uint8_t output_data_sum;
}gps_data_out;


typedef struct  __attribute__((packed))
{
	 double data_lat;//γ��
	 double data_lon;//����
}gps_data_use_for_rn;

typedef struct __attribute__((packed))

{
  uint8_t cal_dis_start;
  float ndistance_step;
  float hdistance_step;
  float sdistance_gps;
  float distance_sum;
  double middle_gps_data[2];

}gps_data_distance;

typedef struct __attribute__((packed))
{
    int32_t Longitude;	//�Ż��㷨���������ľ���
    int32_t Latitude;		//�Ż��㷨������γ��
	uint8_t pausestatus;  //�洢��ͣ״̬�����ڼ��������滮�켣���
	
	
}TrackOptimizingResultDataStr;


typedef struct __attribute__((packed))
{
	  int32_t steps[STEPSCOMP_SIZE];
	  uint8_t rear;
	  uint8_t size;
}StepsCompStr;  //�Ʋ�ʱ�����ۻ��Ĺ��̣�һ���ۻ��������һ��

typedef struct 
	{
    float drift_dst[10];
    float ori_dst[DELDRIFT_POINTS_SIZE];
    int32_t drift_last_lon;       //drift speed last longitude
    int32_t drift_last_lat;       //drift speed last latitude
    int32_t ori_last_lon;
    int32_t ori_last_lat;
    uint8_t ori_dst_head;
    uint8_t ori_count;
    float speed;
}instant_speed_t;

typedef struct __attribute__((packed))
{   uint8_t temp_sequence;
	uint32_t front;
	uint32_t rear;
	uint32_t inst_step[TIME_THS];  //����ʵʱ��Ƶ������
}step_queue;


extern StepsCompStr StepsComp;  //�洢����

float GpsDistance(int32_t  LonA, int32_t  LatA, int32_t  LonB, int32_t  LatB);

//float gps_distance_f(double  LonA, double  LatA, double  LonB, double  LatB);

extern void TrackOptimizingInit(void);
extern void TrackOptimizingDataAccept(DistanceInputStr DistanceInput);
extern void PhysicalParamCalculateInit(void);
extern void PhysicalParamCalDataAccept(int32_t Lon, int32_t Lat, uint32_t steps, uint32_t stepdiff);
extern void PhyParamCalculateDistSpeed(void);

uint8_t gps_exception(void);
void nogps_maxgap_init(uint8_t flag);
void sport_over_or_restart();

uint8_t StaticJudgment(void); 
void SportsCalculateDistSpeed(void);

extern void MeanSmoothDataAccept(const int32_t Lon, const int32_t Lat, const uint8_t liv_pausestatus,uint8_t frontS);


void MeanSmooth(void);

void SpeedOptimizingDataCollection(int32_t org_lon, int32_t org_lat);
void NoGPSSpeedParamInit(void);
void minuteMeanSpeedCalculate(void);

extern uint32_t DistanceResult(void);
extern uint16_t SpeedResult(void);
extern uint16_t MeanSpeedResult(void);
extern uint32_t StepStrideResult(void);
void steps_freq(void);
extern uint16_t get_steps_freq(void);
extern void track_bubble_sort(float *ac, uint16_t n);
extern float gps_raw_data_process_rn(gps_data_rn*data_rn);
extern void gps_raw_data_rn_receive();
extern void gps_rn_data_sm_receive();
extern float gps_sm_data_distance_receive();


#endif
