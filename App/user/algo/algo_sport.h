#ifndef __ALGO_SPORT_H__
#define __ALGO_SPORT_H__

#include "algo_config.h"
#include <stdint.h>
#include "com_sport.h"
#include "watch_config.h"


/*距离计算结构:  
        lon  经度 ，有符号整型，实际经度X1000000
        lat  纬度 ，有符号整型，实际经度X1000000
        gps_status, gps状态，整型，0  gps状态无效   1  gps状态有效，
        pause_status, 暂停状态，整型，0  运动状态  1  暂停状态，2 停止状态
        time， 时间，整型，运动时长，单位:s
				pause_time, 时间，整型，间隔时长，单位：s
				up_value, 距离，整型，坡度上升值，单位：m
				down_value, 距离，整型，坡度下降值，单位：m
        stride，步长，整型，单位:cm
        steps，步数，整型，运动总步数
        activity_type，运动类型，整型，*/
typedef struct  __attribute__((packed))
{
    int32_t lon;
    int32_t lat;
    uint8_t gps_status;   
    uint8_t pause_status;
    uint32_t time;
	uint32_t pause_time;
    uint8_t stride;
    uint32_t steps;
	uint32_t up_value;
	uint32_t down_value;   
    uint8_t activity_type;
	
}DistanceInputStr;



/*
*函数：获取BMI
*输入: ·weight   体重,单位kg，整数
		height   身高，单位cm，整数
*返回值:BMI 
*备注  BMI = 体重/身高的平方    (kg/m2)
*/
float    GetBMI( unsigned char weight, unsigned char height );

/*
*函数：获取静息代谢率RMR
*输入: ·weight   体重,单位kg，整数
		height   身高，单位cm，整数
		age		 年龄，单位岁，整数
		sex		 性别，0女  1 男
*返回值:RMR  （kcal/min）
*备注  男性: RMR(卡路里/天)=(10x体重(kg))+(6.25x身高(cm))-(5x年龄(岁)) + 5 
	   女性: RMR(卡路里/天)=(10x体重(kg))+(6.25x身高(cm))-(5x年龄(岁)) - 161
基础代谢值 (kcal)= GetRMR() * 时间(分钟)

*/
float   GetRMR( unsigned char weight, unsigned char height, unsigned char age,unsigned char sex);

/*
*函数：计算日常计步能量
*输入: ·steps	 步数值
		weight	体重,单位kg，整数

*返回值:Energy  （cal） 整型
*备注  
日常计步能量消耗，单位 卡。

*/
unsigned int    CalculateDailyStepEnergy( unsigned int steps,unsigned char weight);

/*
*函数：计算徒步、健走能量消耗 
*输入: ·distance	 	距离， 单位  米， 整数
		weight			体重，单位，千克 整数

*返回值:Energy  （cal） 整型
*备注  
能量消耗，单位 卡.由于计算值存在小数点，为减少误差，建议每隔一分钟以上计算一次。

*/
unsigned int    CalculateWalkEnergy( unsigned int distance,unsigned char weight);


/*
*函数：计算跑步能量消耗 （包括所有类型的跑步）
*输入: ·distance	 	距离， 单位  米， 整数
		time			时间,单位 秒，整数，跑完distance这段距离所花的时间
		weight			体重，单位，千克 整数

*返回值:Energy  （cal） 整型
*备注  
跑步能量消耗，单位 卡.由于计算值存在小数点，为减少误差，建议每隔一分钟以上计算一次。

*/
unsigned int    CalculateRunningEnergy( unsigned int distance,unsigned int time,unsigned char weight);

/*
*函数：计算登山能量消耗 
*输入: ·heart	 	持续时间内平均心率，整数
		time		时间,单位 秒，整数
		weight		体重，单位，千克 整数
		age		 	年龄，单位岁，整数
		sex		 	性别，0女  1 男

*返回值:每分钟能量  （cal/min） 整型
*备注  
登山能量消耗，单位 卡/分钟.每分钟计算一次平均心率，代入函数后取得一分钟消耗的能量

*/
unsigned int    CalculateMountaineeringEnergy( unsigned char heart,unsigned int time,unsigned char weight, unsigned char age,unsigned char sex);

/*
*函数：计算骑行能量消耗 
*输入: ·speed	 	骑行速度，单位  米/小时，整数
		time		时间,单位 秒，整数
		weight		体重，单位，千克 整数


*返回值:能量   单位为卡
*备注  
骑行能量消耗
8	3
16	8
25	11
32	15

*/
unsigned int    CalculateCyclingEnergy( unsigned int speed,unsigned int time,unsigned char weight);

/*
*函数：计算游泳能量消耗 
*输入: ·distance	距离，单位  米，整数
		time		时间,单位 秒，整数
		sex		 	性别，0女  1 男

*返回值:能量   单位为卡
*备注  
游泳能量消耗，建议按趟数计算，输入单趟距离及时间获取单趟的卡路里
EE(kcal/m)=[0.603* pow(10,0.228v(m/s) )]/ 4.184
EE(kcal/m)=[0.360* pow(10,0.339v(m/s) )]/ 4.184
*/
unsigned int CalculateSwimmingEnergy( unsigned int distance, unsigned int time,unsigned char sex );


/*
*函数：计算VO2max
*输入: ·distance_in_12min	 	12分钟跑的最大距离，单位 m
		sex		 				性别，0女  1 男

*返回值:VO2max  （mL/kg·min） 浮点型
*备注  
最大摄氧量：手表有测试界面，在该界面提示下全力跑12分钟，测算距离后代入公式进行计算。
平时可以根据12分钟的最佳成绩代入计算。

*/
float  CalculateVO2max(unsigned int distance_in_12min, unsigned char sex );

/*
*函数：计算上升高度和下降高度
*输入: ·pressure	 	气压值，浮点型
		p_rising		上升高度指针，对应数据为整型，单位cm
		p_falling		下降高度指针，对应数据为整型，单位cm
*返回值:status		0 success   非零  fail
*备注  
在输入第一个气压值之前，需要输入0气压及上升高度和下降高度的基准值初始化函数
正常计算时输入气压值和上升高度及下降高度指针

*/

unsigned char CalculateAltitude( float pressure, unsigned short *p_rising,unsigned short *p_falling);


/*
*函数：计算距离
*输入:  lon  经度 ，有符号整型，实际经度X100000
        lat  纬度 ，有符号整型，实际经度X100000
        gps_status, gps状态，0 无效   1  有效
        time， 时间，整型，运动时长，单位:s
        stride，步长，整型，单位:cm
        steps，步数，整型，运动总步数
        activity_type，运动类型，整型，
	
*返回值:距离，单位:cm
*备注  

*/

extern uint32_t CalculateDistance(DistanceInputStr DistanceInput);

/*
*函数：计算速度
*输入:second 时长，单位 s
      start_distance 开始距离 ，单位 cm
      end_distance   结束距离 单位 cm
	
*返回值:速度，单位:速度10*km/h,配速s/km,垂直速度 10*m/h,泳速  s/100m
*备注  

*/

extern uint16_t CalculateSpeed(uint32_t second,uint32_t start_distance,uint32_t end_distance,uint8_t activity_type);

/*
*函数：计算步频
*输入:time 时间，单位 s, 
      steps 步数，time时间内走的步数
      	
*返回值:步频，单位:步/min
*备注  

*/
extern uint16_t CalculateFrequency(uint32_t time,uint32_t steps);


/*
*函数：计算步幅
*输入:distance 距离，单位: cm
      steps 步数，distance距离内走的步数
      	
*返回值:步幅，单位:cm
*备注  

*/
extern uint16_t CalculateStride(uint32_t distance,uint32_t steps);


/*
*函数：计算swolf
*输入:laps 趟数
      steps 划数
      time 时间，单位:s
*返回值:swolf,单位:实际X10倍
*备注  

*/
extern uint16_t CalculateSwolf(uint32_t steps,uint16_t laps,uint32_t time);


/*
*函数：计算心率参数:区间时间及最大心率,心率累加值
*输入:hdr_value 当前心率 
      Commondetail 运动公共数据结构
      hdrtotal,每秒心率累加指针，
*返回值:无
*备注  
计算心率落在哪个心率区间
示例:
   CalculateHeartrateZone(150,&ActivityData.ActivityDetails.CommonDetail，time,&hdrtotal);

*/
extern void CalculateHeartrateParam(uint8_t hdr_value,CommonDetailStr * Commondetail,uint32_t *hdrtotal,uint32_t *hdrtcount);
/*
*函数：计算心率参数:最大心率,心率累加值
*输入:hdr_value 当前心率 
      Commondetail 运动公共数据结构
      time，时长，整型
      hdrtotal,每秒心率累加指针，
      hdrtcount,累加次数指针
*返回值:无
*备注  
只计算最大心率值，不计算心率区间时间
示例:

*/
extern void CalculateHeartrateMax(uint8_t hdr_value,CommonDetailStr * Commondetail,uint32_t *hdrtotal,uint32_t *hdrtcount);

/*
*函数：计算踏频:区间时间及最大心率,心率累加值
*输入:cadValue 当前踏频
*返回值:cadTotal 踏频累计值
        cadCount 踏频次数
*备注
*/
extern void CalculateCadenceParam(uint32_t cadValue,uint32_t *cadTotal,uint32_t *cadCount);

/*
*函数：计算最佳配速
*输入:speed 当前配速 单位:s/km
      optimumspeed最佳速度指针，对应数据为整型，单位:s/km
    
	
*返回值:无
*备注 
示例:
CalculateOptimumSpeed(23,&ActivityData.ActivityDetails.RunningDetail.OptimumPace);

<<<<<<< .mine
*/

extern void CalculateOptimumSpeed(uint16_t speed,uint16_t *optimumspeed);

/*
*函数：计算最佳配速
*输入:speed 当前配速 单位:s/km
      optimumpace最佳配速指针，对应数据为整型，单位:s/km
    
	
*返回值:无
*备注  

*/
extern void CalculateOptimumPace(uint16_t speed,uint16_t *optimumpace);
/*
*函数：计算最高海拔
*输入:alt 当前海拔 单位:m
     MaxAlt最高海拔指针，单位m
    
	
*返回值:无
*备注  

*/
extern void CalculateMaxAlt(int16_t alt,int16_t *maxalt);

/*
*函数：计算恢复时间
*输入:distance 运动距离 单位:cm
      speed 平均速度，单位对应各运动类型
      type:运动类型
	
*返回值:recoverytime，单位:min
*备注  

*/
extern uint16_t CalculateRecoveryTime(uint32_t distance,CommonDetailStr Commondetail,uint32_t hdrtcount,uint8_t activity_type);


/*
*函数：计算有氧效果
*输入:HeartRateZone 心率区间数组指针  数据单位:分钟
      
	
*返回值:AerobicIntensity，百分数
*备注  
   有氧效果为 心率区间 2,3持续时间除以150分钟得到的百分比
   显示时除以10显示

*/
unsigned char GetAerobicIntensity( unsigned int *p_HeartRateZone );

/*
*函数：计算无氧效果
*输入:HeartRateZone 心率区间数组指针  数据单位:分钟
      
	
*返回值:AnaerobicIntensity，百分数
*备注  
   无氧效果为 心率区间 4,5持续时间除以75分钟得到的百分比
   显示时除以10显示

*/
unsigned char GetAnaerobicIntensity( unsigned int *p_HeartRateZone );

extern void trackoffset(int16_t * x1,int16_t *y1,int16_t x_offset,int16_t y_offset);
extern void trackzoom(int16_t * x1,int16_t *y1,uint16_t zoom0,uint16_t zoom1);
extern void trackrotate(int16_t * x1,int16_t *y1,uint16_t angle);
extern uint16_t CalculateFrequency_swimming(uint32_t time,uint32_t steps);
#endif

