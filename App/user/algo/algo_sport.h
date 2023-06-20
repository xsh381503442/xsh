#ifndef __ALGO_SPORT_H__
#define __ALGO_SPORT_H__

#include "algo_config.h"
#include <stdint.h>
#include "com_sport.h"
#include "watch_config.h"


/*�������ṹ:  
        lon  ���� ���з������ͣ�ʵ�ʾ���X1000000
        lat  γ�� ���з������ͣ�ʵ�ʾ���X1000000
        gps_status, gps״̬�����ͣ�0  gps״̬��Ч   1  gps״̬��Ч��
        pause_status, ��ͣ״̬�����ͣ�0  �˶�״̬  1  ��ͣ״̬��2 ֹͣ״̬
        time�� ʱ�䣬���ͣ��˶�ʱ������λ:s
				pause_time, ʱ�䣬���ͣ����ʱ������λ��s
				up_value, ���룬���ͣ��¶�����ֵ����λ��m
				down_value, ���룬���ͣ��¶��½�ֵ����λ��m
        stride�����������ͣ���λ:cm
        steps�����������ͣ��˶��ܲ���
        activity_type���˶����ͣ����ͣ�*/
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
*��������ȡBMI
*����: ��weight   ����,��λkg������
		height   ��ߣ���λcm������
*����ֵ:BMI 
*��ע  BMI = ����/��ߵ�ƽ��    (kg/m2)
*/
float    GetBMI( unsigned char weight, unsigned char height );

/*
*��������ȡ��Ϣ��л��RMR
*����: ��weight   ����,��λkg������
		height   ��ߣ���λcm������
		age		 ���䣬��λ�꣬����
		sex		 �Ա�0Ů  1 ��
*����ֵ:RMR  ��kcal/min��
*��ע  ����: RMR(��·��/��)=(10x����(kg))+(6.25x���(cm))-(5x����(��)) + 5 
	   Ů��: RMR(��·��/��)=(10x����(kg))+(6.25x���(cm))-(5x����(��)) - 161
������лֵ (kcal)= GetRMR() * ʱ��(����)

*/
float   GetRMR( unsigned char weight, unsigned char height, unsigned char age,unsigned char sex);

/*
*�����������ճ��Ʋ�����
*����: ��steps	 ����ֵ
		weight	����,��λkg������

*����ֵ:Energy  ��cal�� ����
*��ע  
�ճ��Ʋ��������ģ���λ ����

*/
unsigned int    CalculateDailyStepEnergy( unsigned int steps,unsigned char weight);

/*
*����������ͽ���������������� 
*����: ��distance	 	���룬 ��λ  �ף� ����
		weight			���أ���λ��ǧ�� ����

*����ֵ:Energy  ��cal�� ����
*��ע  
�������ģ���λ ��.���ڼ���ֵ����С���㣬Ϊ����������ÿ��һ�������ϼ���һ�Ρ�

*/
unsigned int    CalculateWalkEnergy( unsigned int distance,unsigned char weight);


/*
*�����������ܲ��������� �������������͵��ܲ���
*����: ��distance	 	���룬 ��λ  �ף� ����
		time			ʱ��,��λ �룬����������distance��ξ���������ʱ��
		weight			���أ���λ��ǧ�� ����

*����ֵ:Energy  ��cal�� ����
*��ע  
�ܲ��������ģ���λ ��.���ڼ���ֵ����С���㣬Ϊ����������ÿ��һ�������ϼ���һ�Ρ�

*/
unsigned int    CalculateRunningEnergy( unsigned int distance,unsigned int time,unsigned char weight);

/*
*�����������ɽ�������� 
*����: ��heart	 	����ʱ����ƽ�����ʣ�����
		time		ʱ��,��λ �룬����
		weight		���أ���λ��ǧ�� ����
		age		 	���䣬��λ�꣬����
		sex		 	�Ա�0Ů  1 ��

*����ֵ:ÿ��������  ��cal/min�� ����
*��ע  
��ɽ�������ģ���λ ��/����.ÿ���Ӽ���һ��ƽ�����ʣ����뺯����ȡ��һ�������ĵ�����

*/
unsigned int    CalculateMountaineeringEnergy( unsigned char heart,unsigned int time,unsigned char weight, unsigned char age,unsigned char sex);

/*
*���������������������� 
*����: ��speed	 	�����ٶȣ���λ  ��/Сʱ������
		time		ʱ��,��λ �룬����
		weight		���أ���λ��ǧ�� ����


*����ֵ:����   ��λΪ��
*��ע  
������������
8	3
16	8
25	11
32	15

*/
unsigned int    CalculateCyclingEnergy( unsigned int speed,unsigned int time,unsigned char weight);

/*
*������������Ӿ�������� 
*����: ��distance	���룬��λ  �ף�����
		time		ʱ��,��λ �룬����
		sex		 	�Ա�0Ů  1 ��

*����ֵ:����   ��λΪ��
*��ע  
��Ӿ�������ģ����鰴�������㣬���뵥�˾��뼰ʱ���ȡ���˵Ŀ�·��
EE(kcal/m)=[0.603* pow(10,0.228v(m/s) )]/ 4.184
EE(kcal/m)=[0.360* pow(10,0.339v(m/s) )]/ 4.184
*/
unsigned int CalculateSwimmingEnergy( unsigned int distance, unsigned int time,unsigned char sex );


/*
*����������VO2max
*����: ��distance_in_12min	 	12�����ܵ������룬��λ m
		sex		 				�Ա�0Ů  1 ��

*����ֵ:VO2max  ��mL/kg��min�� ������
*��ע  
������������ֱ��в��Խ��棬�ڸý�����ʾ��ȫ����12���ӣ�����������빫ʽ���м��㡣
ƽʱ���Ը���12���ӵ���ѳɼ�������㡣

*/
float  CalculateVO2max(unsigned int distance_in_12min, unsigned char sex );

/*
*���������������߶Ⱥ��½��߶�
*����: ��pressure	 	��ѹֵ��������
		p_rising		�����߶�ָ�룬��Ӧ����Ϊ���ͣ���λcm
		p_falling		�½��߶�ָ�룬��Ӧ����Ϊ���ͣ���λcm
*����ֵ:status		0 success   ����  fail
*��ע  
�������һ����ѹֵ֮ǰ����Ҫ����0��ѹ�������߶Ⱥ��½��߶ȵĻ�׼ֵ��ʼ������
��������ʱ������ѹֵ�������߶ȼ��½��߶�ָ��

*/

unsigned char CalculateAltitude( float pressure, unsigned short *p_rising,unsigned short *p_falling);


/*
*�������������
*����:  lon  ���� ���з������ͣ�ʵ�ʾ���X100000
        lat  γ�� ���з������ͣ�ʵ�ʾ���X100000
        gps_status, gps״̬��0 ��Ч   1  ��Ч
        time�� ʱ�䣬���ͣ��˶�ʱ������λ:s
        stride�����������ͣ���λ:cm
        steps�����������ͣ��˶��ܲ���
        activity_type���˶����ͣ����ͣ�
	
*����ֵ:���룬��λ:cm
*��ע  

*/

extern uint32_t CalculateDistance(DistanceInputStr DistanceInput);

/*
*�����������ٶ�
*����:second ʱ������λ s
      start_distance ��ʼ���� ����λ cm
      end_distance   �������� ��λ cm
	
*����ֵ:�ٶȣ���λ:�ٶ�10*km/h,����s/km,��ֱ�ٶ� 10*m/h,Ӿ��  s/100m
*��ע  

*/

extern uint16_t CalculateSpeed(uint32_t second,uint32_t start_distance,uint32_t end_distance,uint8_t activity_type);

/*
*���������㲽Ƶ
*����:time ʱ�䣬��λ s, 
      steps ������timeʱ�����ߵĲ���
      	
*����ֵ:��Ƶ����λ:��/min
*��ע  

*/
extern uint16_t CalculateFrequency(uint32_t time,uint32_t steps);


/*
*���������㲽��
*����:distance ���룬��λ: cm
      steps ������distance�������ߵĲ���
      	
*����ֵ:��������λ:cm
*��ע  

*/
extern uint16_t CalculateStride(uint32_t distance,uint32_t steps);


/*
*����������swolf
*����:laps ����
      steps ����
      time ʱ�䣬��λ:s
*����ֵ:swolf,��λ:ʵ��X10��
*��ע  

*/
extern uint16_t CalculateSwolf(uint32_t steps,uint16_t laps,uint32_t time);


/*
*�������������ʲ���:����ʱ�估�������,�����ۼ�ֵ
*����:hdr_value ��ǰ���� 
      Commondetail �˶��������ݽṹ
      hdrtotal,ÿ�������ۼ�ָ�룬
*����ֵ:��
*��ע  
�������������ĸ���������
ʾ��:
   CalculateHeartrateZone(150,&ActivityData.ActivityDetails.CommonDetail��time,&hdrtotal);

*/
extern void CalculateHeartrateParam(uint8_t hdr_value,CommonDetailStr * Commondetail,uint32_t *hdrtotal,uint32_t *hdrtcount);
/*
*�������������ʲ���:�������,�����ۼ�ֵ
*����:hdr_value ��ǰ���� 
      Commondetail �˶��������ݽṹ
      time��ʱ��������
      hdrtotal,ÿ�������ۼ�ָ�룬
      hdrtcount,�ۼӴ���ָ��
*����ֵ:��
*��ע  
ֻ�����������ֵ����������������ʱ��
ʾ��:

*/
extern void CalculateHeartrateMax(uint8_t hdr_value,CommonDetailStr * Commondetail,uint32_t *hdrtotal,uint32_t *hdrtcount);

/*
*����������̤Ƶ:����ʱ�估�������,�����ۼ�ֵ
*����:cadValue ��ǰ̤Ƶ
*����ֵ:cadTotal ̤Ƶ�ۼ�ֵ
        cadCount ̤Ƶ����
*��ע
*/
extern void CalculateCadenceParam(uint32_t cadValue,uint32_t *cadTotal,uint32_t *cadCount);

/*
*�����������������
*����:speed ��ǰ���� ��λ:s/km
      optimumspeed����ٶ�ָ�룬��Ӧ����Ϊ���ͣ���λ:s/km
    
	
*����ֵ:��
*��ע 
ʾ��:
CalculateOptimumSpeed(23,&ActivityData.ActivityDetails.RunningDetail.OptimumPace);

<<<<<<< .mine
*/

extern void CalculateOptimumSpeed(uint16_t speed,uint16_t *optimumspeed);

/*
*�����������������
*����:speed ��ǰ���� ��λ:s/km
      optimumpace�������ָ�룬��Ӧ����Ϊ���ͣ���λ:s/km
    
	
*����ֵ:��
*��ע  

*/
extern void CalculateOptimumPace(uint16_t speed,uint16_t *optimumpace);
/*
*������������ߺ���
*����:alt ��ǰ���� ��λ:m
     MaxAlt��ߺ���ָ�룬��λm
    
	
*����ֵ:��
*��ע  

*/
extern void CalculateMaxAlt(int16_t alt,int16_t *maxalt);

/*
*����������ָ�ʱ��
*����:distance �˶����� ��λ:cm
      speed ƽ���ٶȣ���λ��Ӧ���˶�����
      type:�˶�����
	
*����ֵ:recoverytime����λ:min
*��ע  

*/
extern uint16_t CalculateRecoveryTime(uint32_t distance,CommonDetailStr Commondetail,uint32_t hdrtcount,uint8_t activity_type);


/*
*��������������Ч��
*����:HeartRateZone ������������ָ��  ���ݵ�λ:����
      
	
*����ֵ:AerobicIntensity���ٷ���
*��ע  
   ����Ч��Ϊ �������� 2,3����ʱ�����150���ӵõ��İٷֱ�
   ��ʾʱ����10��ʾ

*/
unsigned char GetAerobicIntensity( unsigned int *p_HeartRateZone );

/*
*��������������Ч��
*����:HeartRateZone ������������ָ��  ���ݵ�λ:����
      
	
*����ֵ:AnaerobicIntensity���ٷ���
*��ע  
   ����Ч��Ϊ �������� 4,5����ʱ�����75���ӵõ��İٷֱ�
   ��ʾʱ����10��ʾ

*/
unsigned char GetAnaerobicIntensity( unsigned int *p_HeartRateZone );

extern void trackoffset(int16_t * x1,int16_t *y1,int16_t x_offset,int16_t y_offset);
extern void trackzoom(int16_t * x1,int16_t *y1,uint16_t zoom0,uint16_t zoom1);
extern void trackrotate(int16_t * x1,int16_t *y1,uint16_t angle);
extern uint16_t CalculateFrequency_swimming(uint32_t time,uint32_t steps);
#endif

