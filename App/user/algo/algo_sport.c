#include <math.h>
#include "drv_config.h"
#include "drv_ms5837.h"
#include "algo_sport.h"
#include "com_sport.h"
#include "com_data.h"
#include "algo_hdr.h"
#include "algo_trackoptimizing.h"



/*
*��������ȡBMI
*����: ��weight   ����,��λkg������
		height   ��ߣ���λcm������
*����ֵ:BMI 
*��ע  BMI = ����/��ߵ�ƽ��    (kg/m2)
*/
float    GetBMI( unsigned char weight, unsigned char height )
{
	if(weight == 0) weight = 60;
	if(height == 0) height = 170;
	float  val;
	val = height;
	val /= 100;  //��λת��Ϊ��
	return( ((float)weight)/(val*val) );	
}

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
float   GetRMR( unsigned char weight, unsigned char height, unsigned char age,unsigned char sex)
{
	if(weight == 0) weight = 60;
	if(height == 0) height = 170;
	if(age == 0) age = 25;
	
	return( (10*(unsigned short)weight+6.25*(double)height-5*(unsigned short)age+166*sex-161)/1440 );
}


/*
*�����������ճ��Ʋ�����
*����: ��steps	 ����ֵ
		weight	����,��λkg������

*����ֵ:Energy  ��cal�� ����
*��ע  
�ճ��Ʋ��������ģ���λ ����

*/
unsigned int    CalculateDailyStepEnergy( unsigned int steps,unsigned char weight)
{
	if(weight == 0) weight = 60;
	return( (unsigned int)(steps * (unsigned int)weight / 2) );
}

/*
*����������ͽ���������������� 
*����: ��distance	 	���룬 ��λ  �ף� ����
		weight			���أ���λ��ǧ�� ����

*����ֵ:Energy  ��cal�� ����
*��ע  
�������ģ���λ ��.���ڼ���ֵ����С���㣬Ϊ����������ÿ��һ�������ϼ���һ�Ρ�

*/
unsigned int    CalculateWalkEnergy( unsigned int distance,unsigned char weight)
{
	if(weight == 0) weight = 60;
	return( (unsigned int)(0.8214*distance * weight ));
}


/*
*�����������ܲ��������� �������������͵��ܲ���
*����: ��distance	 	���룬 ��λ  �ף� ����
		time			ʱ��,��λ �룬����������distance��ξ���������ʱ��
		weight			���أ���λ��ǧ�� ����

*����ֵ:Energy  ��cal�� ����
*��ע  
�ܲ��������ģ���λ ��.���ڼ���ֵ����С���㣬Ϊ����������ÿ��һ�������ϼ���һ�Ρ�

*/
unsigned int    CalculateRunningEnergy( unsigned int distance,unsigned int time,unsigned char weight)
{
	if(weight == 0) weight = 60;
	return( (unsigned int)( (0.9304*distance+ 0.1772 * time) * (float)weight ) );
}

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
unsigned int    CalculateMountaineeringEnergy( unsigned char heart,unsigned int time,unsigned char weight, unsigned char age,unsigned char sex)
{
    int32_t ret =0;
	if(weight == 0) weight = 60;
	if( sex )  //����
	{
	    ret = (630.9*heart+198.8*weight+201.7*age-55096.9)/4.184;		
	}
	else
	{
	    ret = (447.2*heart-126.3*weight+74.0*age-20402.2)/4.184;	
	}

    if (ret < 0)
    {
        ret = 0;
    }

    return (unsigned int) ret;
}

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
unsigned int    CalculateCyclingEnergy( unsigned int speed,unsigned int time,unsigned char weight)
{
	unsigned int   k;
	
	//ϵ��K���㣬������10��
	if( speed < 8000  ) //
	{
		k = (speed * 3) / 800;  	
	}
	else if( speed < 16000  )
	{
		k = ( ( speed-8000 )* 5) / 800 + 30;
	}
	else if( speed < 25000  )
	{
		k = ( ( speed-16000 )* 3) / 900 + 80;
	}
	else if( speed < 32000  )
	{
		k = ( ( speed-25000 )* 4) / 700 + 110;
	}
	else
	{
		k = 150;
	}
	
	return( ( (unsigned int)weight) * time * k / 36 );
}

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
unsigned int CalculateSwimmingEnergy( unsigned int distance, unsigned int time,unsigned char sex )
{
	double speed;
	speed = ((float)distance) / time;
	if( sex )
	{
		return( (unsigned int)(603.0 * pow(10,0.228*speed ) * ((float)distance) / 4.184) );
	}
	else
	{
		return( (unsigned int)(360.0 * pow(10,0.339*speed ) * ((float)distance) / 4.184) );
	}
}	


/*
*����������VO2max
*����: ��distance_in_12min	 	12�����ܵ������룬��λ m
		sex		 				�Ա�0Ů  1 ��

*����ֵ:VO2max  ��mL/kg��min�� ������
*��ע  
������������ֱ��в��Խ��棬�ڸý�����ʾ��ȫ����12���ӣ�����������빫ʽ���м��㡣
ƽʱ���Ը���12���ӵ���ѳɼ�������㡣

*/
float  CalculateVO2max(unsigned int distance_in_12min, unsigned char sex )
{
	if( sex )
	{
		return( 0.021 * distance_in_12min - 6.447 );
	}
	else
	{
		return( 0.027 * distance_in_12min - 15.478 );
	}
}

/*
*���������������߶Ⱥ��½��߶�
*����: ��pressure	 	��ѹֵ��������
		p_rising		�����߶�ָ�룬��Ӧ����Ϊ���ͣ���λm
		p_falling		�½��߶�ָ�룬��Ӧ����Ϊ���ͣ���λm
*����ֵ:status		0 success   ����  fail
*��ע  
�������һ����ѹֵ֮ǰ����Ҫ����0��ѹ�������߶Ⱥ��½��߶ȵĻ�׼ֵ��ʼ������
��������ʱ������ѹֵ�������߶ȼ��½��߶�ָ��

*/

unsigned char CalculateAltitude( float pressure, unsigned short *p_rising,unsigned short *p_falling)
{
	static float   			trend_pressure = 0;  //�����½�ת�۵���ѹֵ
	//static unsigned int 	trend_altitude = 0;  //�����½�ת�۵�߶�ֵ
	static unsigned int		last_rising = 0;
	static unsigned int		last_falling = 0;
	static  unsigned char 	pressure_trend = 1;  //ת��״̬��־ ��1 Ϊ ������ 2 Ϊ�½�
	static  float 			v_pressure[5] = {0,0,0,0,0};  //����5����ѹֵ
	unsigned char   		i;
	
	if( pressure == 0 )
	{
		trend_pressure = 0;
		//trend_altitude = 0; 
		last_rising = *p_rising;    //��ʼ������׼
		last_falling = *p_falling;	//��ʼ�½���׼
		for( i = 0;i < 5;i++ )
		{
			v_pressure[i] = 0;
		}
		pressure_trend = 1;		
	}
	else if( ( pressure >= 300) && ( pressure <= 1200 ) )
	{
		if( trend_pressure == 0 )
		{
			trend_pressure = pressure;
		//	trend_altitude = *p_rising;
            v_pressure[0] = pressure;
		}
		else
		{
			for( i = 4;i > 0;i-- )
			{
				v_pressure[i] = v_pressure[i-1];
			}
			v_pressure[0] = pressure;
			if( pressure_trend == 1 )  //��ѹ�����׶�
			{
				if( ( v_pressure[0] < v_pressure[1] ) && ( v_pressure[0] < v_pressure[2] ) )
				{
					pressure_trend = 2;//��ѹת��Ϊ�½��׶Σ��߶�Ϊ����
					if( v_pressure[1] >= v_pressure[2] )
					{
						trend_pressure = v_pressure[1];
					}
					else
					{
						trend_pressure = v_pressure[2];
					}
					last_falling = *p_falling;	//��ʼ�½���׼
					*p_rising = *p_rising + ( unsigned int )( drv_ms5837_rel_altitude(v_pressure[0],trend_pressure) );
					last_rising = *p_rising;    //��ʼ������׼
				}
				else
				{
					if( v_pressure[0] > v_pressure[1] ) //��ѹ�����������߶��½�
					{
						*p_falling = last_falling + ( unsigned int )( drv_ms5837_rel_altitude(trend_pressure,v_pressure[0]));
					}
				}
					
			}
			else
			{
				if( ( v_pressure[0] > v_pressure[1] ) && ( v_pressure[0] > v_pressure[2] ) )
				{
					pressure_trend = 1;//��ѹת��Ϊ�����׶Σ��߶�Ϊ�½�
					if( v_pressure[1] <= v_pressure[2] )
					{
						trend_pressure = v_pressure[1];
					}
					else
					{
						trend_pressure = v_pressure[2];
					}
					last_rising = *p_rising;    //��ʼ������׼
					*p_falling = *p_falling + ( unsigned int )( drv_ms5837_rel_altitude(trend_pressure,v_pressure[0]) );
					last_falling = *p_falling;	//��ʼ�½���׼
				}
				else
				{
					if( v_pressure[0] < v_pressure[1] ) //��ѹ�����½����߶�����
					{
						*p_rising = last_rising + ( unsigned int )( drv_ms5837_rel_altitude(v_pressure[0],trend_pressure));
					}
				}
			}
		}			
	}
	else
	{
		return( 1 );
	}
	return( 0 );
}


/*
*�������������
*����:  lon  ���� ���з������ͣ�ʵ�ʾ���X100000
        lat  γ�� ���з������ͣ�ʵ�ʾ���X100000
        gps_status, gps״̬�����ͣ�0  gps״̬��Ч   1  gps״̬��Ч��
        pause_status, ��ͣ״̬�����ͣ�0  �˶�״̬  1  ��ͣ״̬��
        time�� ʱ�䣬���ͣ��˶�ʱ������λ:s
        stride�����������ͣ���λ:cm
        steps�����������ͣ��˶��ܲ���
        activity_type���˶����ͣ����ͣ�
	
*����ֵ:���룬��λ:cm
*��ע  

*/

uint32_t CalculateDistance(DistanceInputStr DistanceInput)
{

   uint32_t distance = 0;
	return distance;

}


/*
*�����������ٶ�
*����:second ʱ������λ s
      start_distance ��ʼ���� ����λ cm
      end_distance   �������� ��λ cm
      activity_type,����ͣ�����
	
*����ֵ:�ٶȣ���λ:�ٶ�10*km/h,����s/km,��ֱ�ٶ� 10*m/h,Ӿ��  s/100m
*��ע  

*/

uint16_t CalculateSpeed(uint32_t second,uint32_t start_distance,uint32_t end_distance,uint8_t activity_type)
{
	uint32_t distance;
	uint16_t speed = 0;
	
	if(end_distance <= start_distance)
	{
		speed = 0;
	}
	else
	{
		distance = end_distance - start_distance;
		if ((activity_type == ACT_CYCLING) || (activity_type == ACT_TRIATHLON_CYCLING))
		{
		   //�ٶ�10*km/h
		   speed = distance*36/second/100.f;
		}
		else if ((activity_type == ACT_WALK)||(activity_type == ACT_HIKING)||(activity_type == ACT_CROSSCOUNTRY)
            ||(activity_type == ACT_RUN) ||(activity_type == ACT_MARATHON)|| (activity_type == ACT_TRIATHLON_RUN)
		        ||(activity_type == ACT_INDOORRUN) || activity_type == ACT_TRAIN_PLAN_RUN)
		{
		    //����s/km
			speed = second *100000.0/(double)distance;
		}
		else if (activity_type == ACT_CLIMB )
		{
		    //��ֱ�ٶ� 10*m/h
			speed = 10*distance*3600/second;
		}
		else if ((activity_type == ACT_SWIMMING)||(activity_type == ACT_TRIATHLON_SWIM))
		{
		   //Ӿ��  s/100m
			speed = second*10000/distance;
		}
	}
	
	return speed;
}



/*
*���������㲽Ƶ/��Ƶ
*����:time ʱ�䣬��λ s, 
      steps ����/��Ƶ��timeʱ�����ߵĲ���/��Ƶ
      	
*����ֵ:��Ƶ����λ:��/min
*��ע  

*/
uint16_t CalculateFrequency(uint32_t time,uint32_t steps)
{
	uint16_t frequency;
    
    frequency  = 60*steps/time;
	return frequency;
}

/*
*���������㲽��
*����:distance ���룬��λ: cm
      steps ������distance�������ߵĲ���
      	
*����ֵ:��������λ:cm
*��ע  

*/
uint16_t CalculateStride(uint32_t distance,uint32_t steps)
{
	uint16_t stride;
    
    stride  = distance/steps;
    
	return stride;
}



/*
*����������swolf
*����:laps ����
      steps ����
      time ʱ�䣬��λ:s
*����ֵ:swolf,��λ:ʵ��X10��
*��ע  

*/
uint16_t CalculateSwolf(uint32_t steps,uint16_t laps,uint32_t time)
{
	uint16_t swolf;
    
    swolf = 10*(steps + time)/laps;
    
	return swolf;
}

/*
*�������������ʲ���:����ʱ�估�������,�����ۼ�ֵ
*����:hdr_value ��ǰ���� 
      Commondetail �˶��������ݽṹ
      time��ʱ��������
      hdrtotal,ÿ�������ۼ�ָ�룬
      hdrtcount,�ۼӴ���ָ��
*����ֵ:��
*��ע  
�������������ĸ���������
ʾ��:
   CalculateHeartrateZone(150,&ActivityData.ActivityDetails.CommonDetail��&hdrtotal);

*/
void CalculateHeartrateParam(uint8_t hdr_value,CommonDetailStr * Commondetail,uint32_t *hdrtotal,uint32_t *hdrtcount)
{
	uint8_t hdrRange = 0;
	
	//��ȡ��������
	hdrRange = get_hdr_range(hdr_value);

    //������������ʱ��
	switch(hdrRange)
	{	
		case 1:
			Commondetail->HeartRateZone[0]++;
			break;
		case 2:
			Commondetail->HeartRateZone[1]++;
			break;
		case 3:
			Commondetail->HeartRateZone[2]++;
			break;
		case 4:
			Commondetail->HeartRateZone[3]++;
			break;
		case 5:
			Commondetail->HeartRateZone[4]++;
			break;
		default:
			break;
      }

    //�����������
    if(hdr_value > Commondetail->MaxHeart)
    {
        Commondetail->MaxHeart = hdr_value;

    }
    
    if(hdr_value != 0)
   {
        //���������ۼ�ֵ������
        (*hdrtotal) +=  hdr_value;

        (*hdrtcount)++;
   }
    
}

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
void CalculateHeartrateMax(uint8_t hdr_value,CommonDetailStr * Commondetail,uint32_t *hdrtotal,uint32_t *hdrtcount)
{

    //�����������
    if(hdr_value > Commondetail->MaxHeart)
    {
        Commondetail->MaxHeart = hdr_value;

    }
    
    if(hdr_value != 0)
   {
        //���������ۼ�ֵ������
        (*hdrtotal) +=  hdr_value;

        (*hdrtcount)++;
   }
    
}


/*
*����������̤Ƶ:����ʱ�估�������,�����ۼ�ֵ
*����:cadValue ��ǰ̤Ƶ
*����ֵ:cadTotal ̤Ƶ�ۼ�ֵ
        cadCount ̤Ƶ����
*��ע
*/
void CalculateCadenceParam(uint32_t cadValue,uint32_t *cadTotal,uint32_t *cadCount)
{
    if(cadValue != 0)
	{
		//����̤Ƶ�ۼ�ֵ������
		(*cadTotal) += cadValue;

		(*cadCount)++;
	}
}

/*
*�����������������
*����:speed ��ǰ���� ��λ:s/km
      optimumspeed����ٶ�ָ�룬��Ӧ����Ϊ���ͣ���λ:s/km
    
	
*����ֵ:��
*��ע 
ʾ��:
CalculateOptimumSpeed(23,&ActivityData.ActivityDetails.RunningDetail.OptimumPace);

*/


void CalculateOptimumSpeed(uint16_t speed,uint16_t *optimumspeed)
{    
    //�ٶ�
    if (speed > *optimumspeed)
    { 
       *optimumspeed = speed;
    }      	
}

/*
*�����������������
*����:speed ��ǰ���� ��λ:s/km
      optimumpace�������ָ�룬��Ӧ����Ϊ���ͣ���λ:s/km
    
	
*����ֵ:��
*��ע  

*/
void CalculateOptimumPace(uint16_t speed,uint16_t *optimumpace)
{

   if (*optimumpace == 0)
    {
        *optimumpace = speed;
    }
    //����
	if ((speed < *optimumpace)&& (speed != 0))
    { 
        *optimumpace = speed;
    } 
	
}

/*
*������������ߺ���
*����:alt ��ǰ���� ��λ:m
     MaxAlt��ߺ���ָ�룬��λm
    
	
*����ֵ:��
*��ע  

*/
void CalculateMaxAlt(int16_t alt,int16_t *maxalt)
{

	if (alt > *maxalt)
    { 
        *maxalt = alt;
    } 
	
}


/*
*����������ָ�ʱ��
*����:distance �˶����� ��λ:m
      Commondetail �˶��������ݽṹ
      hdrtcount,�����ۼӴ���
      activity_type:�˶�����
	
*����ֵ:recoverytime����λ:min
*��ע  

*/
uint16_t CalculateRecoveryTime(uint32_t distance,CommonDetailStr Commondetail,uint32_t hdrtcount,uint8_t activity_type)
{
    uint16_t recoverytime = 0;

	if ((activity_type == ACT_CYCLING)||(activity_type == ACT_TRIATHLON_CYCLING)||(activity_type == ACT_CROSSCOUNTRY)
            ||(activity_type == ACT_RUN) ||(activity_type == ACT_MARATHON)|| (activity_type == ACT_TRIATHLON_RUN)
            ||(activity_type == ACT_INDOORRUN) || (activity_type == ACT_TRAIN_PLAN_RUN))
		       
	{

		recoverytime = distance * 0.24 * (1 + (Commondetail.HeartRateZone[1] +Commondetail.HeartRateZone[2] + Commondetail.HeartRateZone[3]+ Commondetail.HeartRateZone[4])/(float)hdrtcount);

	}
	else if ((activity_type == ACT_WALK)||(activity_type == ACT_HIKING)||(activity_type == ACT_CLIMB))
	{

		recoverytime = distance * 0.1008 * (1 + (Commondetail.HeartRateZone[1] +Commondetail.HeartRateZone[2] + Commondetail.HeartRateZone[3]+ Commondetail.HeartRateZone[4])/(float)hdrtcount);
	}
    

    return recoverytime;
	
}

/*
*��������������Ч��
*����:HeartRateZone ������������ָ��  ���ݵ�λ:s
      
	
*����ֵ:AerobicIntensity���ٷ���
*��ע  
   ����Ч��Ϊ �������� 2,3����ʱ�����150���ӵõ��İٷֱ�
   ��ʾʱ����10��ʾ

*/
unsigned char GetAerobicIntensity( unsigned int *p_HeartRateZone )
{
	unsigned int AerobicTime;
	
	AerobicTime = p_HeartRateZone[1] + p_HeartRateZone[2];
	if( AerobicTime >= (150*60) )
	{
		return( 100 );
	}
	else
	{
		return( (unsigned char )(AerobicTime * 100 / (150*60)) );
	}
}

/*
*��������������Ч��
*����:HeartRateZone ������������ָ��  ���ݵ�λ:s
      
	
*����ֵ:AnaerobicIntensity���ٷ���
*��ע  
   ����Ч��Ϊ �������� 4,5����ʱ�����75���ӵõ��İٷֱ�
   ��ʾʱ����10��ʾ

*/
unsigned char GetAnaerobicIntensity( unsigned int *p_HeartRateZone )
{
	unsigned int AnaerobicTime;
	
	AnaerobicTime = p_HeartRateZone[3] + p_HeartRateZone[4];
	if( AnaerobicTime >= (75*60) )
	{
		return( 100 );
	}
	else
	{
		return( (unsigned char )(AnaerobicTime * 100 / (75*60)) );
	}
}

/*
*�����������ƽ�ƺ������
*����:x0��y0 ����������
      x1��y1 ƽ�ƺ�ĵ�����ָ��
      x_offset��y_offset ƽ����������ֵ����ʱΪ��������ֵ����Ϊ��
      
	
*����ֵ:��
*��ע  

*/

void trackoffset(int16_t * x1,int16_t *y1,int16_t x_offset,int16_t y_offset)
{

   *x1 = *x1 + x_offset;
   *y1 = *y1 + y_offset;
}

/*
*��������������ź������
*����:x0��y0 ����������
      x1��y1 ���ź�ĵ�����ָ��
      zoom0��zoom1 ����ǰ������ŵȼ�
      
	
*����ֵ:��
*��ע  
ZoomBase�����Ż�׼����С���ŵȼ�(1)ʱ��Ļ��ʾ�ľ��� ��λ:��

*/

void trackzoom(int16_t * x1,int16_t *y1,uint16_t zoom0,uint16_t zoom1)
{


        *x1 = (*x1 -120) * zoom0/zoom1 +120;
  
        *y1 =(*y1 -120 )* zoom0/zoom1 +120;
 
   

}

/*
*�������������ת�������
*����:x0��y0 ����������
      x1��y1��ת��ĵ�����ָ��
      angle ��ת�Ƕȣ���ʱ�뷽��
      
	
*����ֵ:��
*��ע  

*/

void trackrotate(int16_t * x1,int16_t *y1,uint16_t angle)
{

    *x1 = (*x1 - 120) *cos(angle) + (*y1 - 120) *sin(angle) +120;
    *y1 = -(*x1 -120) *sin(angle) + (*y1 - 120) *cos(angle) +120;

}


uint16_t CalculateFrequency_swimming(uint32_t time,uint32_t steps)
{
		uint16_t frequency;
    
    frequency  = round(60.0*steps/time);
	return frequency;
}
