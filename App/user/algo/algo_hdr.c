#include "algo_hdr.h"
#include "com_data.h"
#include "drv_lcd.h"
#include "algo_config.h"
#include "drv_heartrate.h"
#include "font_config.h"

#if DEBUG_ENABLED == 1 && ALGO_HDR_LOG_ENABLED == 1
	#define ALGO_HDR_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define ALGO_HDR_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define ALGO_HDR_WRITESTRING(...)
	#define ALGO_HDR_PRINTF(...)		        
#endif

uint8_t g_HeartNum = 0;	//ȡ������
uint8_t g_HeartBuf[HEARTBUF_SIZE] = {0};	//�������
uint8_t g_MaxHeart = 0;	//�������
uint8_t g_HeartRemind = 0;	//����������ֵ

//������ʻ�ȡ����
uint8_t get_hdr_max(void)
{
	uint8_t hdrMax = 0,age = 0;
	if(SetValue.AppSet.Age < 120)//����������
	{
		age = SetValue.AppSet.Age;
	}
	else
	{
		age = 100;
	}
	#ifdef COD 
	hdrMax = 220 - age;
	#else
	hdrMax = 210 -(uint8_t)(age/2) - (uint8_t)(0.11*SetValue.AppSet.Weight);
    if (SetValue.AppSet.Sex)
    {
        hdrMax +=4;
    }

    if(hdrMax < 40)
    {
        hdrMax = 40;

    }
	#endif 
	return hdrMax;

}
/*
*��������������ֵ�õ���������
*����: ����ֵ
*����ֵ:���������(1~5),0�������50%,����50%������ֵ
*��ע���������������䷶Χ����algo
*������� = 205.8 - 0.685*����
*
*����1 50%~60%    ����
*����2 60%~70%    ȼ֬
*����3 70%~80%    ǿ�ķ�
*����4 80%~90%    ������
*����5 90%~100%   �����˶�
*/
uint8_t  get_hdr_range(uint8_t hdr_value)
{
  uint8_t hdrMax = 0,range = 0;

  	hdrMax = get_hdr_max();
	#ifdef COD 
	g_HeartRemind = SetValue.AppSet.htr_zone_five_min;;
	
	if(hdr_value < ((hdrMax - SetValue.rest_heart)* 0.5 +SetValue.rest_heart))
	{
		range = 0;
	}
    else if(hdr_value >= ((hdrMax - SetValue.rest_heart)* 0.5 + SetValue.rest_heart) && hdr_value < ((hdrMax - SetValue.rest_heart)* 0.6 + SetValue.rest_heart))
	{
		range = 1;
	}
	else if(hdr_value >= ((hdrMax - SetValue.rest_heart)* 0.6 + SetValue.rest_heart) && hdr_value < ((hdrMax - SetValue.rest_heart)* 0.7 + SetValue.rest_heart))
	{
		range = 2;
	}
	else if(hdr_value >= ((hdrMax - SetValue.rest_heart)* 0.7 + SetValue.rest_heart) && hdr_value < ((hdrMax - SetValue.rest_heart)* 0.8 + SetValue.rest_heart))
	{
		range = 3;
	}
	else if(hdr_value >= ((hdrMax - SetValue.rest_heart)* 0.8 + SetValue.rest_heart) && hdr_value < ((hdrMax - SetValue.rest_heart)* 0.9 + SetValue.rest_heart))
	{
		range = 4;
	}
	else
	{
		range = 5;
	}
	
	#else
    #if defined WATCH_SIM_SPORT
	g_HeartRemind = SetValue.AppSet.htr_zone_five_min;
	#else
    g_HeartRemind = hdrMax * 0.9;
	#endif

	if(hdr_value < (hdrMax * 0.5))
	{
		range = 0;
	}
  else if(hdr_value >= (hdrMax * 0.5) && hdr_value < (hdrMax * 0.6))
	{
		range = 1;
	}
	else if(hdr_value >= (hdrMax * 0.6) && hdr_value < (hdrMax * 0.7))
	{
		range = 2;
	}
	else if(hdr_value >= (hdrMax * 0.7) && hdr_value < (hdrMax * 0.8))
	{
		range = 3;
	}
	else if(hdr_value >= (hdrMax * 0.8) && hdr_value < (hdrMax * 0.9))
	{
		range = 4;
	}
	else
	{
		range = 5;
	}
	#endif
	return range;
}
/*
*����:��ȡ����ֵ��ʾ����ɫֵ
*����:����ֵ
*���:��ɫֵ
*��ע:
*����0 0~ 50%                 ǳ��ɫ        (LCD_LIGHTGRAY)
*����1 50%~60%    �������    ��ɫ 	        (LCD_CYAN)
*����2 60%~70%    ����ȼ֬    ʸ������   	(LCD_CYAN)
*����3 70%~80%    ��������    ����   	    (LCD_GREEN)
*����4 80%~90%    ��������    ��³ɫ  	    (LCD_PERU)
*����5 90%~100%   ���޳��    ��ɫ   	    (LCD_RED)
*/
uint8_t get_hdr_range_color(uint8_t hdr_value)
{
	uint8_t hdrColor = 0,hdrRange = 0;
	
	//��ȡ��������
	hdrRange = get_hdr_range(hdr_value);
	switch(hdrRange)
	{
		case 0:
			hdrColor = LCD_GRAY;
			break;
		case 1:
			hdrColor = LCD_SPRINGGREEN;
			break;
		case 2:
			hdrColor = LCD_MEDIUMBLUE;
			break;
		case 3:
			hdrColor = LCD_FORESTGREEN;
			break;
		case 4:
			hdrColor = LCD_BROWN;
			break;
		case 5:
			hdrColor = LCD_MAROON;
			break;
		default:
			break;
	}
	return hdrColor;
}
/**********************������������۱�***************************
*����ֵ:����0-5 �ֱ�����¡��С������š�����
*��	  13-19��	     20-29��	 30-39��	    40-49��	   50-59��	    60-69��
*����	��56	        ��52.5	      ��49.5	      ��48.1     	��45.4	     ��44.3
*��	  51-55.9	    46.5-52.4	 45.0-49.4	 43.8-48.0	 41.0-45.3	 36.5-44.2
*��	  45.2-50.9	  42.5-46.4	 41.0-44.9	 39.0-43.7	 35.8-40.9	 32.2-36.4
*��	  38.4-45.1	  36.5-42.4	 35.5-40.9	 33.6-38.9	 31.0-35.7	 26.1-32.1
*��	  35.0-38.3	  33.0-36.4	 31.5-35.4	 30.2-33.5	 26.1-30.9	 20.5-26.0
*��	   <35	      <33	        <31.5	      <30.2	<      26.1	      <20.5

*Ů	  13-19��	    20-29��	    30-39��	    40-49��	    50-59��	  60-69��
*����	��42	        ��41	        ��40.1	      ��37.0	      ��35.8	    ��31.5
*��	  39.0-41.9	  37.0-40.9	  35.7-40.0	  32.9-36.9	 31.5-35.7	30.3-31.4
*��	  35.0-38.9	  33.0-36.9	  31.5-35.6	  29.0-32.8	 27.0-31.4	24.5-30.2
*��	  31.0-34.9	  29.0-32.9	  27.0-31.4	  24.5-28.9	 22.8-26.9	20.2-24.4
*��	  25.0-30.9	  23.6-28.9	  22.8-26.9	  21.0-24.4	 20.2-22.7	17.5-20.1
*��	  <25	        <23.6	      <22.8	      <21.0	     <20.2	    <17.5
*
* �������������ʱ�����ͽ������������
************************************************/
uint8_t  get_VO2max_range(uint8_t VO2max_value,uint8_t male,uint8_t age)
{
  uint8_t range = 0;
	if(male == 1)//man
	{
		if(age <= 19)
		{
			if(VO2max_value >= 56)
			{
				range = 5;
			}
			else if(VO2max_value >= 51 && VO2max_value <= 55.9)
			{
				range = 4;
			}
			else if(VO2max_value >= 45.2 && VO2max_value <= 50.9)
			{
				range = 3;
			}
			else if(VO2max_value >= 38.4 && VO2max_value <= 45.1)
			{
				range = 2;
			}
			else if(VO2max_value >= 35.0 && VO2max_value <= 38.3)
			{
				range = 1;
			}
			else if(VO2max_value < 35)
			{
				range = 0;
			}
		}
		else if(age >= 20 && age <= 29)
		{
			if(VO2max_value >= 52.5)
			{
				range = 5;
			}
			else if(VO2max_value >= 46.5 && VO2max_value <= 52.4)
			{
				range = 4;
			}
			else if(VO2max_value >= 42.5 && VO2max_value <= 46.4)
			{
				range = 3;
			}
			else if(VO2max_value >= 36.5 && VO2max_value <= 42.4)
			{
				range = 2;
			}
			else if(VO2max_value >= 33.0 && VO2max_value <= 36.4)
			{
				range = 1;
			}
			else if(VO2max_value < 33)
			{
				range = 0;
			}
		}
		else if(age >= 30 && age <= 39)
		{
			if(VO2max_value >= 49.5)
			{
				range = 5;
			}
			else if(VO2max_value >= 45.0 && VO2max_value <= 49.4)
			{
				range = 4;
			}
			else if(VO2max_value >= 41.0 && VO2max_value <= 44.9)
			{
				range = 3;
			}
			else if(VO2max_value >= 35.5 && VO2max_value <= 40.9)
			{
				range = 2;
			}
			else if(VO2max_value >= 31.5 && VO2max_value <= 35.4)
			{
				range = 1;
			}
			else if(VO2max_value < 31.5)
			{
				range = 0;
			}
		}
		else if(age >= 40 && age <= 49)
		{
			if(VO2max_value >= 48.1)
			{
				range = 5;
			}
			else if(VO2max_value >= 43.8 && VO2max_value <= 48.0)
			{
				range = 4;
			}
			else if(VO2max_value >= 39.0 && VO2max_value <= 43.7)
			{
				range = 3;
			}
			else if(VO2max_value >= 33.6 && VO2max_value <= 38.9)
			{
				range = 2;
			}
			else if(VO2max_value >= 30.2 && VO2max_value <= 33.5)
			{
				range = 1;
			}
			else if(VO2max_value < 30.2)
			{
				range = 0;
			}
		}
		else if(age >= 50 && age <= 59)
		{
			if(VO2max_value >= 45.4)
			{
				range = 5;
			}
			else if(VO2max_value >= 41.0 && VO2max_value <= 45.3)
			{
				range = 4;
			}
			else if(VO2max_value >= 35.8 && VO2max_value <= 40.9)
			{
				range = 3;
			}
			else if(VO2max_value >= 31.0 && VO2max_value <= 35.7)
			{
				range = 2;
			}
			else if(VO2max_value >= 26.1 && VO2max_value <= 30.9)
			{
				range = 1;
			}
			else if(VO2max_value < 26.1)
			{
				range = 0;
			}
		}
		else if(age >= 60)
		{
			if(VO2max_value >= 44.3)
			{
				range = 5;
			}
			else if(VO2max_value >= 36.5 && VO2max_value <= 44.2)
			{
				range = 4;
			}
			else if(VO2max_value >= 32.2 && VO2max_value <= 36.4)
			{
				range = 3;
			}
			else if(VO2max_value >= 26.1 && VO2max_value <= 32.1)
			{
				range = 2;
			}
			else if(VO2max_value >= 20.5 && VO2max_value <= 26.0)
			{
				range = 1;
			}
			else if(VO2max_value < 20.5)
			{
				range = 0;
			}
		}

	}
	else
	{//woman
		if(age <= 19)
		{
			if(VO2max_value >= 42)
			{
				range = 5;
			}
			else if(VO2max_value >= 39.0 && VO2max_value <= 41.9)
			{
				range = 4;
			}
			else if(VO2max_value >= 35.0 && VO2max_value <= 38.9)
			{
				range = 3;
			}
			else if(VO2max_value >= 31.0 && VO2max_value <= 44.9)
			{
				range = 2;
			}
			else if(VO2max_value >= 25.0 && VO2max_value <= 30.9)
			{
				range = 1;
			}
			else if(VO2max_value < 25)
			{
				range = 0;
			}
		}
		else if(age >= 20 && age <= 29)
		{
			if(VO2max_value >= 41)
			{
				range = 5;
			}
			else if(VO2max_value >= 37.0 && VO2max_value <= 40.9)
			{
				range = 4;
			}
			else if(VO2max_value >= 33.0 && VO2max_value <= 36.9)
			{
				range = 3;
			}
			else if(VO2max_value >= 29.0 && VO2max_value <= 32.9)
			{
				range = 2;
			}
			else if(VO2max_value >= 23.6 && VO2max_value <= 28.9)
			{
				range = 1;
			}
			else if(VO2max_value < 23.6)
			{
				range = 0;
			}
		}
		else if(age >= 30 && age <= 39)
		{
			if(VO2max_value >= 40.1)
			{
				range = 5;
			}
			else if(VO2max_value >= 35.7 && VO2max_value <= 40.0)
			{
				range = 4;
			}
			else if(VO2max_value >= 31.5 && VO2max_value <= 35.6)
			{
				range = 3;
			}
			else if(VO2max_value >= 27.0 && VO2max_value <= 31.4)
			{
				range = 2;
			}
			else if(VO2max_value >= 22.8 && VO2max_value <= 26.9)
			{
				range = 1;
			}
			else if(VO2max_value < 22.8)
			{
				range = 0;
			}
		}
		else if(age >= 40 && age <= 49)
		{
			if(VO2max_value >= 37.0)
			{
				range = 5;
			}
			else if(VO2max_value >= 32.9 && VO2max_value <= 36.9)
			{
				range = 4;
			}
			else if(VO2max_value >= 29.0 && VO2max_value <= 32.8)
			{
				range = 3;
			}
			else if(VO2max_value >= 24.5 && VO2max_value <= 28.9)
			{
				range = 2;
			}
			else if(VO2max_value >= 21.0 && VO2max_value <= 24.4)
			{
				range = 1;
			}
			else if(VO2max_value < 21.0)
			{
				range = 0;
			}
		}
		else if(age >= 50 && age <= 59)
		{
			if(VO2max_value >= 35.8)
			{
				range = 5;
			}
			else if(VO2max_value >= 31.5 && VO2max_value <= 35.7)
			{
				range = 4;
			}
			else if(VO2max_value >= 27.0 && VO2max_value <= 31.4)
			{
				range = 3;
			}
			else if(VO2max_value >= 22.8 && VO2max_value <= 26.9)
			{
				range = 2;
			}
			else if(VO2max_value >= 20.2 && VO2max_value <= 22.7)
			{
				range = 1;
			}
			else if(VO2max_value < 20.2)
			{
				range = 0;
			}
		}
		else if(age >= 60)
		{
			if(VO2max_value >= 31.5)
			{
				range = 5;
			}
			else if(VO2max_value >= 30.3 && VO2max_value <= 31.4)
			{
				range = 4;
			}
			else if(VO2max_value >= 24.5 && VO2max_value <= 30.2)
			{
				range = 3;
			}
			else if(VO2max_value >= 20.2 && VO2max_value <= 24.4)
			{
				range = 2;
			}
			else if(VO2max_value >= 17.5 && VO2max_value <= 20.1)
			{
				range = 1;
			}
			else if(VO2max_value < 17.5)
			{
				range = 0;
			}
		}
	}

	
	return range;
}
uint8_t get_VO2max_range_color(uint8_t VO2maxRange)
{
	uint8_t VO2maxColor = 0;
	
	switch(VO2maxRange)
	{
		case 0:
			VO2maxColor = LCD_RED;
			break;
		case 1:
			//VO2maxColor = LCD_PALEVIOLETRED;
			VO2maxColor = LCD_CRIMSON;
			break;
		case 2:
			//VO2maxColor = LCD_CHOCOLATE;
			VO2maxColor = LCD_DEEPPINK;
			break;
		case 3:
			//VO2maxColor = LCD_GREEN;
			VO2maxColor = LCD_PERU;
			break;
		case 4:
			//VO2maxColor = LCD_DARKTURQUOISE;
			VO2maxColor = LCD_CORNFLOWERBLUE;
			break;
		case 5:
			//VO2maxColor = LCD_DEEPPINK;
			VO2maxColor = LCD_CYAN;
			break;
		default:
			break;
	}
	return VO2maxColor;
}
/*
*����:��������ֵ��������g_HeartBuf[HEARTBUF_SIZE]
*����:����ֵ
*���:
*��ע:
**/
void save_heartrate(uint8_t heart)
{
	uint8_t i;
	
	if(heart > g_MaxHeart)
	{
		g_MaxHeart = heart;
	}
	//����ȡ������ֵ���뻺����
	if(g_HeartNum == 0)
	{
		//�տ�ʼ����ʱ������Ϊ0��ֵ
		if(drv_getHdrValue() != 0)
		{
			g_HeartBuf[g_HeartNum++] = heart;
		}
	}
	else if(g_HeartNum < HEARTBUF_SIZE)
	{
		g_HeartBuf[g_HeartNum++] = heart;
	}
	else
	{
		for (i = 0; i < HEARTBUF_SIZE - 1; i++)
		{
			g_HeartBuf[i] = g_HeartBuf[i+1];
		}
		g_HeartBuf[HEARTBUF_SIZE-1] = heart;
	}
}
//��ȡǰ10s������
uint8_t get_10s_heartrate(void)
{

	uint8_t value = 0;
	uint8_t i;
	uint16_t accvalue = 0;

	if (g_HeartNum < 10)
	{
		for (i = 0; i < g_HeartNum; i++)
		{
			accvalue += g_HeartBuf[i];
		}
		value = accvalue/g_HeartNum;

	}
	else
	{
		for (i = g_HeartNum -10; i < g_HeartNum; i++)
		{
			accvalue += g_HeartBuf[i];
		}

		value = accvalue/10;
	}

	return value;

}

/*�����ȶ�����*/
uint8_t cal_stable_heartrate(void)
{
	uint32_t value = 0;
	uint8_t t = 0,hdrStableValue = 0;
	for(uint8_t i = 0;i < HEARTBUF_SIZE-1;i++)
	{
		for(uint8_t j = 0;j < HEARTBUF_SIZE-1-i;j++)
		{
			if(g_HeartBuf[j] > g_HeartBuf[j+1])
			{
				t = g_HeartBuf[j];
				g_HeartBuf[j] = g_HeartBuf[j+1];
				g_HeartBuf[j+1] = t;
			}
		}
	}
	//ȥ�������Сֵ��10��
	for(uint8_t i = 10;i < HEARTBUF_SIZE-1-10;i++)
	{
		value += g_HeartBuf[i];
	}
	hdrStableValue = (int)(value/(HEARTBUF_SIZE-1-20));
	
	return hdrStableValue;
}
/*����ʱ
*���:����ʱֵ����λms 
*/
void get_countdown_time(uint32_t value,Countdown_time_t *countdownTime)
{
	uint32_t h = 0, m = 0, s = 0, ms = 0;
 
	h = value / 3600000;
	m = (value - h*3600000)/60000;
	s = (value - h*3600000 - m*60000)/1000;
	ms = (value - h*3600000 - m*60000)%1000;
	
	countdownTime->hour = h;
	countdownTime->minute = m;
	countdownTime->second = s;
	countdownTime->hundredths = ms/10;
}


















