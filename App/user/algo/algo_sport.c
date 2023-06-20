#include <math.h>
#include "drv_config.h"
#include "drv_ms5837.h"
#include "algo_sport.h"
#include "com_sport.h"
#include "com_data.h"
#include "algo_hdr.h"
#include "algo_trackoptimizing.h"



/*
*函数：获取BMI
*输入: ·weight   体重,单位kg，整数
		height   身高，单位cm，整数
*返回值:BMI 
*备注  BMI = 体重/身高的平方    (kg/m2)
*/
float    GetBMI( unsigned char weight, unsigned char height )
{
	if(weight == 0) weight = 60;
	if(height == 0) height = 170;
	float  val;
	val = height;
	val /= 100;  //单位转换为米
	return( ((float)weight)/(val*val) );	
}

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
float   GetRMR( unsigned char weight, unsigned char height, unsigned char age,unsigned char sex)
{
	if(weight == 0) weight = 60;
	if(height == 0) height = 170;
	if(age == 0) age = 25;
	
	return( (10*(unsigned short)weight+6.25*(double)height-5*(unsigned short)age+166*sex-161)/1440 );
}


/*
*函数：计算日常计步能量
*输入: ·steps	 步数值
		weight	体重,单位kg，整数

*返回值:Energy  （cal） 整型
*备注  
日常计步能量消耗，单位 卡。

*/
unsigned int    CalculateDailyStepEnergy( unsigned int steps,unsigned char weight)
{
	if(weight == 0) weight = 60;
	return( (unsigned int)(steps * (unsigned int)weight / 2) );
}

/*
*函数：计算徒步、健走能量消耗 
*输入: ·distance	 	距离， 单位  米， 整数
		weight			体重，单位，千克 整数

*返回值:Energy  （cal） 整型
*备注  
能量消耗，单位 卡.由于计算值存在小数点，为减少误差，建议每隔一分钟以上计算一次。

*/
unsigned int    CalculateWalkEnergy( unsigned int distance,unsigned char weight)
{
	if(weight == 0) weight = 60;
	return( (unsigned int)(0.8214*distance * weight ));
}


/*
*函数：计算跑步能量消耗 （包括所有类型的跑步）
*输入: ·distance	 	距离， 单位  米， 整数
		time			时间,单位 秒，整数，跑完distance这段距离所花的时间
		weight			体重，单位，千克 整数

*返回值:Energy  （cal） 整型
*备注  
跑步能量消耗，单位 卡.由于计算值存在小数点，为减少误差，建议每隔一分钟以上计算一次。

*/
unsigned int    CalculateRunningEnergy( unsigned int distance,unsigned int time,unsigned char weight)
{
	if(weight == 0) weight = 60;
	return( (unsigned int)( (0.9304*distance+ 0.1772 * time) * (float)weight ) );
}

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
unsigned int    CalculateMountaineeringEnergy( unsigned char heart,unsigned int time,unsigned char weight, unsigned char age,unsigned char sex)
{
    int32_t ret =0;
	if(weight == 0) weight = 60;
	if( sex )  //男性
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
unsigned int    CalculateCyclingEnergy( unsigned int speed,unsigned int time,unsigned char weight)
{
	unsigned int   k;
	
	//系数K计算，并乘以10倍
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
*函数：计算VO2max
*输入: ·distance_in_12min	 	12分钟跑的最大距离，单位 m
		sex		 				性别，0女  1 男

*返回值:VO2max  （mL/kg·min） 浮点型
*备注  
最大摄氧量：手表有测试界面，在该界面提示下全力跑12分钟，测算距离后代入公式进行计算。
平时可以根据12分钟的最佳成绩代入计算。

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
*函数：计算上升高度和下降高度
*输入: ·pressure	 	气压值，浮点型
		p_rising		上升高度指针，对应数据为整型，单位m
		p_falling		下降高度指针，对应数据为整型，单位m
*返回值:status		0 success   非零  fail
*备注  
在输入第一个气压值之前，需要输入0气压及上升高度和下降高度的基准值初始化函数
正常计算时输入气压值和上升高度及下降高度指针

*/

unsigned char CalculateAltitude( float pressure, unsigned short *p_rising,unsigned short *p_falling)
{
	static float   			trend_pressure = 0;  //上升下降转折点气压值
	//static unsigned int 	trend_altitude = 0;  //上升下降转折点高度值
	static unsigned int		last_rising = 0;
	static unsigned int		last_falling = 0;
	static  unsigned char 	pressure_trend = 1;  //转折状态标志 ，1 为 上升， 2 为下降
	static  float 			v_pressure[5] = {0,0,0,0,0};  //保存5个气压值
	unsigned char   		i;
	
	if( pressure == 0 )
	{
		trend_pressure = 0;
		//trend_altitude = 0; 
		last_rising = *p_rising;    //起始上升基准
		last_falling = *p_falling;	//起始下降基准
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
			if( pressure_trend == 1 )  //气压上升阶段
			{
				if( ( v_pressure[0] < v_pressure[1] ) && ( v_pressure[0] < v_pressure[2] ) )
				{
					pressure_trend = 2;//气压转换为下降阶段，高度为上升
					if( v_pressure[1] >= v_pressure[2] )
					{
						trend_pressure = v_pressure[1];
					}
					else
					{
						trend_pressure = v_pressure[2];
					}
					last_falling = *p_falling;	//起始下降基准
					*p_rising = *p_rising + ( unsigned int )( drv_ms5837_rel_altitude(v_pressure[0],trend_pressure) );
					last_rising = *p_rising;    //起始上升基准
				}
				else
				{
					if( v_pressure[0] > v_pressure[1] ) //气压持续上升，高度下降
					{
						*p_falling = last_falling + ( unsigned int )( drv_ms5837_rel_altitude(trend_pressure,v_pressure[0]));
					}
				}
					
			}
			else
			{
				if( ( v_pressure[0] > v_pressure[1] ) && ( v_pressure[0] > v_pressure[2] ) )
				{
					pressure_trend = 1;//气压转换为上升阶段，高度为下降
					if( v_pressure[1] <= v_pressure[2] )
					{
						trend_pressure = v_pressure[1];
					}
					else
					{
						trend_pressure = v_pressure[2];
					}
					last_rising = *p_rising;    //起始上升基准
					*p_falling = *p_falling + ( unsigned int )( drv_ms5837_rel_altitude(trend_pressure,v_pressure[0]) );
					last_falling = *p_falling;	//起始下降基准
				}
				else
				{
					if( v_pressure[0] < v_pressure[1] ) //气压持续下降，高度上升
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
*函数：计算距离
*输入:  lon  经度 ，有符号整型，实际经度X100000
        lat  纬度 ，有符号整型，实际经度X100000
        gps_status, gps状态，整型，0  gps状态无效   1  gps状态有效，
        pause_status, 暂停状态，整型，0  运动状态  1  暂停状态，
        time， 时间，整型，运动时长，单位:s
        stride，步长，整型，单位:cm
        steps，步数，整型，运动总步数
        activity_type，运动类型，整型，
	
*返回值:距离，单位:cm
*备注  

*/

uint32_t CalculateDistance(DistanceInputStr DistanceInput)
{

   uint32_t distance = 0;
	return distance;

}


/*
*函数：计算速度
*输入:second 时长，单位 s
      start_distance 开始距离 ，单位 cm
      end_distance   结束距离 单位 cm
      activity_type,活动类型，整型
	
*返回值:速度，单位:速度10*km/h,配速s/km,垂直速度 10*m/h,泳速  s/100m
*备注  

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
		   //速度10*km/h
		   speed = distance*36/second/100.f;
		}
		else if ((activity_type == ACT_WALK)||(activity_type == ACT_HIKING)||(activity_type == ACT_CROSSCOUNTRY)
            ||(activity_type == ACT_RUN) ||(activity_type == ACT_MARATHON)|| (activity_type == ACT_TRIATHLON_RUN)
		        ||(activity_type == ACT_INDOORRUN) || activity_type == ACT_TRAIN_PLAN_RUN)
		{
		    //配速s/km
			speed = second *100000.0/(double)distance;
		}
		else if (activity_type == ACT_CLIMB )
		{
		    //垂直速度 10*m/h
			speed = 10*distance*3600/second;
		}
		else if ((activity_type == ACT_SWIMMING)||(activity_type == ACT_TRIATHLON_SWIM))
		{
		   //泳速  s/100m
			speed = second*10000/distance;
		}
	}
	
	return speed;
}



/*
*函数：计算步频/划频
*输入:time 时间，单位 s, 
      steps 步数/划频，time时间内走的步数/划频
      	
*返回值:步频，单位:步/min
*备注  

*/
uint16_t CalculateFrequency(uint32_t time,uint32_t steps)
{
	uint16_t frequency;
    
    frequency  = 60*steps/time;
	return frequency;
}

/*
*函数：计算步幅
*输入:distance 距离，单位: cm
      steps 步数，distance距离内走的步数
      	
*返回值:步幅，单位:cm
*备注  

*/
uint16_t CalculateStride(uint32_t distance,uint32_t steps)
{
	uint16_t stride;
    
    stride  = distance/steps;
    
	return stride;
}



/*
*函数：计算swolf
*输入:laps 趟数
      steps 划数
      time 时间，单位:s
*返回值:swolf,单位:实际X10倍
*备注  

*/
uint16_t CalculateSwolf(uint32_t steps,uint16_t laps,uint32_t time)
{
	uint16_t swolf;
    
    swolf = 10*(steps + time)/laps;
    
	return swolf;
}

/*
*函数：计算心率参数:区间时间及最大心率,心率累加值
*输入:hdr_value 当前心率 
      Commondetail 运动公共数据结构
      time，时长，整型
      hdrtotal,每秒心率累加指针，
      hdrtcount,累加次数指针
*返回值:无
*备注  
计算心率落在哪个心率区间
示例:
   CalculateHeartrateZone(150,&ActivityData.ActivityDetails.CommonDetail，&hdrtotal);

*/
void CalculateHeartrateParam(uint8_t hdr_value,CommonDetailStr * Commondetail,uint32_t *hdrtotal,uint32_t *hdrtcount)
{
	uint8_t hdrRange = 0;
	
	//获取心率区间
	hdrRange = get_hdr_range(hdr_value);

    //计算心率区间时间
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

    //计算最大心率
    if(hdr_value > Commondetail->MaxHeart)
    {
        Commondetail->MaxHeart = hdr_value;

    }
    
    if(hdr_value != 0)
   {
        //计算心率累加值及次数
        (*hdrtotal) +=  hdr_value;

        (*hdrtcount)++;
   }
    
}

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
void CalculateHeartrateMax(uint8_t hdr_value,CommonDetailStr * Commondetail,uint32_t *hdrtotal,uint32_t *hdrtcount)
{

    //计算最大心率
    if(hdr_value > Commondetail->MaxHeart)
    {
        Commondetail->MaxHeart = hdr_value;

    }
    
    if(hdr_value != 0)
   {
        //计算心率累加值及次数
        (*hdrtotal) +=  hdr_value;

        (*hdrtcount)++;
   }
    
}


/*
*函数：计算踏频:区间时间及最大心率,心率累加值
*输入:cadValue 当前踏频
*返回值:cadTotal 踏频累计值
        cadCount 踏频次数
*备注
*/
void CalculateCadenceParam(uint32_t cadValue,uint32_t *cadTotal,uint32_t *cadCount)
{
    if(cadValue != 0)
	{
		//计算踏频累加值及次数
		(*cadTotal) += cadValue;

		(*cadCount)++;
	}
}

/*
*函数：计算最佳配速
*输入:speed 当前配速 单位:s/km
      optimumspeed最佳速度指针，对应数据为整型，单位:s/km
    
	
*返回值:无
*备注 
示例:
CalculateOptimumSpeed(23,&ActivityData.ActivityDetails.RunningDetail.OptimumPace);

*/


void CalculateOptimumSpeed(uint16_t speed,uint16_t *optimumspeed)
{    
    //速度
    if (speed > *optimumspeed)
    { 
       *optimumspeed = speed;
    }      	
}

/*
*函数：计算最佳配速
*输入:speed 当前配速 单位:s/km
      optimumpace最佳配速指针，对应数据为整型，单位:s/km
    
	
*返回值:无
*备注  

*/
void CalculateOptimumPace(uint16_t speed,uint16_t *optimumpace)
{

   if (*optimumpace == 0)
    {
        *optimumpace = speed;
    }
    //配速
	if ((speed < *optimumpace)&& (speed != 0))
    { 
        *optimumpace = speed;
    } 
	
}

/*
*函数：计算最高海拔
*输入:alt 当前海拔 单位:m
     MaxAlt最高海拔指针，单位m
    
	
*返回值:无
*备注  

*/
void CalculateMaxAlt(int16_t alt,int16_t *maxalt)
{

	if (alt > *maxalt)
    { 
        *maxalt = alt;
    } 
	
}


/*
*函数：计算恢复时间
*输入:distance 运动距离 单位:m
      Commondetail 运动公共数据结构
      hdrtcount,心率累加次数
      activity_type:运动类型
	
*返回值:recoverytime，单位:min
*备注  

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
*函数：计算有氧效果
*输入:HeartRateZone 心率区间数组指针  数据单位:s
      
	
*返回值:AerobicIntensity，百分数
*备注  
   有氧效果为 心率区间 2,3持续时间除以150分钟得到的百分比
   显示时除以10显示

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
*函数：计算无氧效果
*输入:HeartRateZone 心率区间数组指针  数据单位:s
      
	
*返回值:AnaerobicIntensity，百分数
*备注  
   无氧效果为 心率区间 4,5持续时间除以75分钟得到的百分比
   显示时除以10显示

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
*函数：计算点平移后的坐标
*输入:x0，y0 输入点的坐标
      x1，y1 平移后的点坐标指针
      x_offset，y_offset 平移量，坐标值增大时为正，坐标值减少为负
      
	
*返回值:无
*备注  

*/

void trackoffset(int16_t * x1,int16_t *y1,int16_t x_offset,int16_t y_offset)
{

   *x1 = *x1 + x_offset;
   *y1 = *y1 + y_offset;
}

/*
*函数：计算点缩放后的坐标
*输入:x0，y0 输入点的坐标
      x1，y1 缩放后的点坐标指针
      zoom0，zoom1 缩放前后的缩放等级
      
	
*返回值:无
*备注  
ZoomBase，缩放基准，最小缩放等级(1)时屏幕表示的距离 单位:米

*/

void trackzoom(int16_t * x1,int16_t *y1,uint16_t zoom0,uint16_t zoom1)
{


        *x1 = (*x1 -120) * zoom0/zoom1 +120;
  
        *y1 =(*y1 -120 )* zoom0/zoom1 +120;
 
   

}

/*
*函数：计算点旋转后的坐标
*输入:x0，y0 输入点的坐标
      x1，y1旋转后的点坐标指针
      angle 旋转角度，逆时针方向
      
	
*返回值:无
*备注  

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
