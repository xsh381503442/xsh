#ifndef ALGO_TRACKOPTIMIZING_H
#define	ALGO_TRACKOPTIMIZING_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "com_sport.h"
#include "algo_sport.h"
#include "algo_sport_cycling.h"

#define     R_EARTH 6371004      //地球半径，单位米

#define     PI 3.1415926

#define     PERSEC_STEP_SIZE 5 //计算步频时的秒数
#define     PERSEC_SPEED_SIZE 7 //计算秒速度时的秒数

#define     DELDRIFT_POINTS_SIZE 7     

#define     SMOOTH_POINTS_SIZE 8   //平滑算法所需的点数

#define     OPTIMIZING_POINTS_SIZE  (DELDRIFT_POINTS_SIZE+SMOOTH_POINTS_SIZE-2)   //21  优化算法总的所需点数

#define     OPTIMIZING_RESULT_POINTS_SIZE  SMOOTH_POINTS_SIZE/2    // 4 一次性轨迹优化结果的大小

#define     STEPSCOMP_SIZE 5   //计步时保存步数的存储大小

#define		MAX_STEPS_THRESHOLD	10	//单位时间内步数最大上限值，10步，防止异常步数

#define		STATIC_STEPS_THRESHOLD	5 //静止标志的步数阈值

#define		SPEED_DELAY_TIME	3	//速度延迟显示的时间，单位是s

#define		SPEED_MINLIMIT 0.28f  //骑行类与步行类的速度下阈值
/*步行类速度上限阈值:5.5km/h.1.53m/s

奔跑速度上限阈值5.5m/s

*/


/*#define     WALK_SPEED_MAXLIMIT_ZERO 0.5f
#define     WALK_SPEED_MAXLIMIT_SLOW 1.32f
#define     WALK_SPEED_MAXLIMIT_GRNERAL 1.98f
#define     WALK_SPEED_MAXLIMIT_NORMAL 2.74f
#define     WALK_SPEED_MAXLIMIT_FAST 3.96f
#define     RUN_SPEED_MAXLIMIT 8.33f*/	//奔跑类的速度上阈值

#define     WALK_SPEED_MAXLIMIT_ZERO 0.5f
#define     WALK_SPEED_MAXLIMIT_SLOW 1.35f
#define     WALK_SPEED_MAXLIMIT_GRNERAL 2.05f
#define     WALK_SPEED_MAXLIMIT_NORMAL 2.85f
#define     WALK_SPEED_MAXLIMIT_FAST 4.05f
#define     RUN_SPEED_MAXLIMIT 8.33f	//奔跑类的速度上阈值


#define     CYCLING_SPEED_MAXLIMIT 27.78f	//骑行类的速度上阈值，单位是m/s

#define			DELAY_SMOOTH_ARGUMENT 0.4f  //瞬时速度滞后滤波的权重取值

#define     STEP_STRIDE_MAX_SPEED 5 //校准步长设定的单位时间内的最大距离

#define		NOGPS_MAX_DISTANCE_FOOT 12 //步行类，掉星或者其他原因造成GPS邻近两点间的距离参数，单位米

#define		NOGPS_MAX_DISTANCE_CYCLE 24 //骑行类，掉星或者其他原因造成GPS邻近两点间的距离参数，单位米

#define		GPS_NORMAL 0 //GPS没有异常

#define		GPS_DRIFT_EXCEPTION 1 //GPS漂移异常

#define		NOGPS_DIST_EXCEPTION 2 //GPS掉星距离出现异常

#define		SPORT_FLAG_STATIS 0 //静止状态

#define		SPORT_FLAG_MOVE 1 //非静止状态

#define		TIME_THS    20     //步频计算的时间间隔

#define		SPORTS_STATUS_END 2	//嵌入式端规定，结束状态时，暂停标志符为2

#define   PAUSE_STATUS 1 	//暂停状态时，标志为1

#define   NON_PAUSE_STATUS 0 	//非暂停状态时，标志为0

#define   DOWN_GPS_FLAG 0 	//GPS不在线标志

#define   UP_GPS_FLAG 1 	//GPS在线标志

#define   REMOVE_START_GPS_SUM 3 	//去除起始点的GPS数量

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
  
    double data[REMOVE_NOISE_DATA_SUM][2];     //存储需处理的的经（下标0）、纬度（下标1）
   
    uint8_t pausestatus[REMOVE_NOISE_DATA_SUM];  //存储暂停状态，用于计算距离与规划轨迹输出
    int32_t middle_data_raw[2];
    uint8_t data_number;
	uint32_t data_sum;   //数组有效元素的个数
}gps_data_rn;

typedef struct __attribute__((packed))
{
  
    double data[SMOOTH_DATA_SUM][2];     //存储需处理的的经（下标0）、纬度（下标1）
    uint8_t pausestatus[SMOOTH_DATA_SUM];  //存储暂停状态，用于计算距离与规划轨迹输出
    uint8_t data_number;
	uint32_t data_sum;   //数组有效元素的个数
	uint32_t data_tsum;  
	uint8_t output_data_sum;
	uint8_t data_smdata_get_flag;
}gps_data_sm;


typedef struct __attribute__((packed))
{
  
    int32_t data[OUTPUT_DATA_SUM][2];     //存储需处理的的经（下标0）、纬度（下标1）
    uint8_t output_data_sum;
}gps_data_out;


typedef struct  __attribute__((packed))
{
	 double data_lat;//纬度
	 double data_lon;//经度
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
    int32_t Longitude;	//优化算法处理后输出的经度
    int32_t Latitude;		//优化算法处理后的纬度
	uint8_t pausestatus;  //存储暂停状态，用于计算距离与规划轨迹输出
	
	
}TrackOptimizingResultDataStr;


typedef struct __attribute__((packed))
{
	  int32_t steps[STEPSCOMP_SIZE];
	  uint8_t rear;
	  uint8_t size;
}StepsCompStr;  //计步时步数累积的过程，一般累积五秒计算一次

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
	uint32_t inst_step[TIME_THS];  //计算实时步频的数组
}step_queue;


extern StepsCompStr StepsComp;  //存储步数

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
