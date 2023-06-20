#include "bsp_rtc.h"
#include <stdint.h>



#if DEBUG_ENABLED == 1 && BSP_RTC_LOG_ENABLED == 1
	#define BSP_RTC_LOG_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define BSP_RTC_LOG_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define BSP_RTC_LOG_WRITESTRING(...)
	#define BSP_RTC_LOG_PRINTF(...)		        
#endif

am_hal_rtc_time_t RTC_time;

void bsp_rtc_init( void )
{
	am_hal_rtc_time_t hal_time;

//	//RTCʱ��ѡ��Ϊ�ⲿ32.768kHz����
//	am_hal_rtc_osc_select(AM_HAL_RTC_OSC_XT);
//	// Enable the RTC.
//	//
//	am_hal_rtc_osc_enable();

	//
	// The RTC is initialized from an arbitrary date.
	//
	hal_time.ui32Hour = 12;
	hal_time.ui32Minute = 1;
	hal_time.ui32Second = 0;
	hal_time.ui32Hundredths = 0;
	hal_time.ui32DayOfMonth = 1;
	hal_time.ui32Month = 1;
	hal_time.ui32Year = 20;
	hal_time.ui32Weekday = CaculateWeekDay(hal_time.ui32Year,hal_time.ui32Month,hal_time.ui32DayOfMonth);
	hal_time.ui32Century = 0;
	hal_time.ui32Hundredths = 0;

	am_hal_rtc_time_set(&hal_time);

	bsp_rtc_interrupt_set();
	BSP_RTC_LOG_PRINTF("[bsp_rtc]: RTC_Init!\n");
}

void bsp_rtc_interrupt_set( void )
{


	am_hal_rtc_alarm_interval_set(AM_HAL_RTC_ALM_RPT_MIN);
	am_hal_rtc_int_clear(AM_HAL_RTC_INT_ALM);
	am_hal_rtc_int_enable(AM_HAL_RTC_INT_ALM);

	//
	// Enable the RTC interrupt in the NVIC.
	//
//	am_hal_interrupt_priority_set(AM_HAL_INTERRUPT_CLKGEN,AM_HAL_INTERRUPT_PRIORITY(4));
//	am_hal_interrupt_enable(AM_HAL_INTERRUPT_CLKGEN);

}
//�������ڼ�������
uint8_t CaculateWeekDay(int y,int m, int d)
{
	int iWeek = 0;
	
	y += 2000;
	
  if(m==1||m==2) 
	{
		m+=12;
		y--;
  }
    iWeek = (d + 1 + 2*m + 3*(m+1)/5 + y + y/4 - y/100 + y/400)%7;
	
	return iWeek;
}

//�Ƿ������� 1-�� 0-��
uint8_t Is_Leap_Year(uint16_t year)
{			
	year += 2000;
	
	if(year%4==0) //�����ܱ�4����
	{ 
		if(year%100==0) 
		{ 
			if(year%400==0)
			{
				return 1;//�����00��β,��Ҫ�ܱ�400���� 	
			}				
			else 
			{
				return 0;  
			}				
		}
		else 
		{
			return 1; 
		}			
	}
	else 
	{
		return 0;
	}		
}

//������������֮����������
uint32_t get_time_diff(rtc_time_t start, rtc_time_t end)
{
	struct tm time_c = {0};
	time_t t1,t2;
	uint32_t tmp;
	
	time_c.tm_year = 2000 - 1900 + start.Year;
	time_c.tm_mon  = start.Month - 1;
	time_c.tm_mday = start.Day;
	time_c.tm_hour = start.Hour;
	time_c.tm_min  = start.Minute;
	time_c.tm_sec  = start.Second;
	t1 = mktime(&time_c);
	
	time_c.tm_year = 2000 - 1900 + end.Year;
	time_c.tm_mon  = end.Month - 1;
	time_c.tm_mday = end.Day;
	time_c.tm_hour = end.Hour;
	time_c.tm_min  = end.Minute;
	time_c.tm_sec  = end.Second;
	t2 = mktime(&time_c);
	
	if(t2 > t1)
	{
		tmp = t2 - t1;
	}
	else
	{
		tmp = t1 - t2;
	}
	
	return tmp;
}

uint8_t MonthMaxDay(uint8_t year,uint8_t month)
{
	uint8_t day = 0;
	uint16_t m_year = year + 2000;
	
	//����ĳ�µ��������
	switch(month)
	{
		case 1: 
		case 3: 
		case 5: 
		case 7: 
		case 8: 
		case 10: 
		case 12: 
			day = 31;
			break;
		case 4: 
		case 6: 
		case 9: 
		case 11: 
			day = 30;
			break;
		case 2: 
			if((m_year % 4 == 0 && m_year % 100 != 0) || m_year % 400 ==0 )//�ж��Ƿ�������
			{
				day = 29;
			}
			else
			{
				day = 28;
			}
			break;
		default:
			break;
	}
	
	return day;
}

/* �ж�ĳ�����ڴ����(y��1��1��)������(y��m��d��)������ */
static uint16_t Days(int y,int m,int d)
{
 int days[]={0,31,28,31,30,31,30,31,31,30,31,30,31};
 int i;
 int sum = 0 ;  /* �������� */
 if(Is_Leap_Year(y)) /* ���Ϊ����,2���� 29 �� */
  days[2] = 29 ;
 for(i=0;i<m;i++)
  sum = sum +days[i] ;
 sum = sum + d - 1 ;
  return sum;
}
/*
 �ж���������֮�������.
 ����һ:�� y1,�� m1,�� d1;
 ����һ:�� y2,�� m2,�� d2;
*/
int16_t DaysDiff(int y1,int m1,int d1,int y2,int m2,int d2)
{
 int16_t s1,s2; /* �����������ڴ�����������ڵ����� */
 int16_t count; /* �����������֮��Ĳ�ֵ */
 int16_t sum=0; /*                        */
 int16_t t,t1,t2;
 if(y1==y2)
 {
  s1 = Days(y1,m1,d1);
  s2 = Days(y2,m2,d2);
  return (s2-s1);  
 }
 else if(y1>y2)
 {
	 return -1;
 }
 else
 {
  count = y2 - y1 ;
  if(count == 1)
  {
   t2 = Days(y2,m2,d2);
   t1 = Days(y1,12,31) - Days(y1,m1,d1);
   return (t1+t2+count);
  }
  else
  {   
   for(t = y1+1;t<y2;t++)
    sum = sum + Days(t,12,31);
   t2 = Days(y2,m2,d2);
   t1 = Days(y1,12,31) - Days(y1,m1,d1);
   return (sum+t1+t2+count);
  }
 }
}
/*��������֮��ĺ����ֵ*/
int64_t DiffTimeMs(am_hal_rtc_time_t rtc1,am_hal_rtc_time_t rtc2)
{
	rtc_time_t rtc_time_t1,rtc_time_t2;
	int64_t T1 = 0,T2 = 0;
	int64_t ms_time = 0;
	
	rtc_time_t1.Year = rtc1.ui32Year;
	rtc_time_t1.Month = rtc1.ui32Month;
	rtc_time_t1.Day = rtc1.ui32DayOfMonth;
	rtc_time_t1.Hour = rtc1.ui32Hour;
	rtc_time_t1.Minute = rtc1.ui32Minute;
	rtc_time_t1.Second = rtc1.ui32Second;
	
	rtc_time_t2.Year = rtc2.ui32Year;
	rtc_time_t2.Month = rtc2.ui32Month;
	rtc_time_t2.Day = rtc2.ui32DayOfMonth;
	rtc_time_t2.Hour = rtc2.ui32Hour;
	rtc_time_t2.Minute = rtc2.ui32Minute;
	rtc_time_t2.Second = rtc2.ui32Second;

	T1 = GetTimeSecond(rtc_time_t1); //s
	T2 = GetTimeSecond(rtc_time_t2); //s
	ms_time = (T1*1000+(int64_t)(rtc1.ui32Hundredths * 10)) - (T2*1000+(int64_t)(rtc2.ui32Hundredths * 10));

	return ms_time;
}

/*��ȡ app�·���ʱ�����ֱ�ǰʱ��Ĳ�ֵ �����ֵ����1���ӷ���true,���򷵻�false
  ���߷�������ͬʱ����true,����false
*/
bool IsChange_ApptoWatch_TimeDiff(rtc_time_t start, rtc_time_t end,uint32_t difftime)
{
	struct tm time_c = {0};
	time_t t1,t2;
	uint32_t tmp;
	
	time_c.tm_year = 2000 - 1900 + start.Year;
	time_c.tm_mon  = start.Month - 1;
	time_c.tm_mday = start.Day;
	time_c.tm_hour = start.Hour;
	time_c.tm_min  = start.Minute;
	time_c.tm_sec  = start.Second;
	t1 = mktime(&time_c);
	
	time_c.tm_year = 2000 - 1900 + end.Year;
	time_c.tm_mon  = end.Month - 1;
	time_c.tm_mday = end.Day;
	time_c.tm_hour = end.Hour;
	time_c.tm_min  = end.Minute;
	time_c.tm_sec  = end.Second;
	t2 = mktime(&time_c);
	
	if(t2 > t1)
	{
		tmp = t2 - t1;
	}
	else
	{
		tmp = t1 - t2;
	}
	if(tmp < difftime)
	{//difftime=60s �ݶ�
		if(start.Minute != end.Minute)
		{//��������ͬ,Ҳ��Ҫ�޸�ʱ��
			return true;
		}
		else
		{//����60s,����������ͬ��,���޸�ʱ��
			return false;
		}
	}
	else
	{//����60s,��Ҫ�ı�ʱ��
		return true;
	}
}
//������������֮����������
int32_t Get_DiffTime(rtc_time_t start, rtc_time_t end)
{
	struct tm time_c = {0};
	time_t t1,t2;
	int32_t tmp;
	
	time_c.tm_year = 2000 - 1900 + start.Year;
	time_c.tm_mon  = start.Month - 1;
	time_c.tm_mday = start.Day;
	time_c.tm_hour = start.Hour;
	time_c.tm_min  = start.Minute;
	time_c.tm_sec  = start.Second;
	t1 = mktime(&time_c);
	
	time_c.tm_year = 2000 - 1900 + end.Year;
	time_c.tm_mon  = end.Month - 1;
	time_c.tm_mday = end.Day;
	time_c.tm_hour = end.Hour;
	time_c.tm_min  = end.Minute;
	time_c.tm_sec  = end.Second;
	t2 = mktime(&time_c);
	
	tmp = t1 -t2;
	
	return tmp;
}
/*����ʱ�� ������ʱ���� ���������*/
unsigned int GetTimeSecond(rtc_time_t start)
{
	struct tm time_c = {0};
	time_t t1;
	
	time_c.tm_year = 2000 - 1900 + start.Year;
	time_c.tm_mon  = start.Month - 1;
	time_c.tm_mday = start.Day;
	time_c.tm_hour = start.Hour;
	time_c.tm_min  = start.Minute;
	time_c.tm_sec  = start.Second;

	t1 = mktime(&time_c);

	return t1;
}
/*�������� ����ʱ�� ������ʱ����*/
rtc_time_t GetYmdhms(unsigned int msec_total)
{
	rtc_time_t Ymdhms;
	struct tm *ptr;

  ptr = localtime(&msec_total);
	
	Ymdhms.Year = ptr->tm_year + 1900 - 2000;
	Ymdhms.Month = ptr->tm_mon + 1;
	Ymdhms.Day = ptr->tm_mday;
	Ymdhms.Hour = ptr->tm_hour;
	Ymdhms.Minute = ptr->tm_min;
	Ymdhms.Second = ptr->tm_sec;
	
	return Ymdhms;
}

/*
 rtcʱ��תutcʱ��
 �������Ϊʱ���������
*/
rtc_time_t RtcToUtc(int32_t sec)
{
 
	rtc_time_t Ymdhms,utc_time;
	uint32_t t1;
	
	am_hal_rtc_time_get(&RTC_time);
	Ymdhms.Year = RTC_time.ui32Year;
	Ymdhms.Month = RTC_time.ui32Month;
	Ymdhms.Day = RTC_time.ui32DayOfMonth;
	Ymdhms.Hour = RTC_time.ui32Hour;
	Ymdhms.Minute = RTC_time.ui32Minute;
	Ymdhms.Second = RTC_time.ui32Second;

	t1 = GetTimeSecond(Ymdhms);

	utc_time = GetYmdhms(t1 - sec);

	return utc_time;

}
#if defined WATCH_TIMEZONE_SET

/*
����˵��:���ݲ�ͬ�Ĺ���,������ù�������ʱ����UTC��0ʱ��֮���ֵ������
���:����  
     TIMEZONE_ZERO 0ʱ��
		 TIMEZONE_BJS 8ʱ��
����ֵ:��UCT 0ʱ���������
*/
int32_t get_timezone_value(uint16_t country)
{
	float value = 0;
	int32_t m_timezone_second = 0;//ʱ����0ʱ����ֵ������
	
	switch(country)
	{
	case TIMEZONE_MDY:
		value = -11;
		break;
	case TIMEZONE_HNL:
		value = -10;
		break;
	case TIMEZONE_ANC:
		value = -9;
		break;
	case TIMEZONE_LAX:
		value = -8;
		break;
	case TIMEZONE_DEN:
		value = -7;
		break;
	case TIMEZONE_CHI:
		value = -6;
		break;
	case TIMEZONE_NYC:
		value = -5;
		break;
	case TIMEZONE_CCS:
		value = -4.5;
		break;
	case TIMEZONE_SCL:
		value = -4;
		break;
	case TIMEZONE_YYT:
		value = -3.5;
		break;
	case TIMEZONE_RIO:
		value = -3;
		break;
	case TIMEZONE_FEN:
		value = -2;
		break;
	case TIMEZONE_PDL:
		value = -1;
		break;
	case TIMEZONE_ZERO://��ʱ��,�׶�ʱ��
		value = 0 ;
		break;
	case TIMEZONE_PAR:
		value = 1;
		break;
	case TIMEZONE_CAI:
		value = 2;
		break;
	case TIMEZONE_MOW:
		value = 3;
		break;
	case TIMEZONE_THR:
		value = 3.5;
		break;
	case TIMEZONE_DXB:
		value = 4;
		break;
	case TIMEZONE_KBL:
		value = 4.5;
		break;
	case TIMEZONE_KHI:
		value = 5;
		break;
	case TIMEZONE_DEL:
		value = 5.5;
		break;
	case TIMEZONE_KTM:
		value = 5.75;
		break;
	case TIMEZONE_DAC:
		value = 6;
		break;
	case TIMEZONE_RGN:
		value = 6.5;
		break;
	case TIMEZONE_BKK:
		value = 7;
		break;
	case TIMEZONE_BJS:
		value = 8;
		break;
	case TIMEZONE_TYO:
		value = 9;
		break;
	case TIMEZONE_ADL:
		value = 9.5;
		break;
	case TIMEZONE_SYD:
		value = 10;
		break;
	case TIMEZONE_NOU:
		value = 11;
		break;
	case TIMEZONE_WLG:
		value = 12;
		break;
		default://Ĭ�ϱ���ʱ��
			value = 8;
			break;
	}
	m_timezone_second = (int32_t)(value * TIME_ONE_HOUR_SECOND);
	
	return m_timezone_second;
}
/*
����˵��:��ȡ����ʱ��
���:m_old_country  ����ʱ��ǰ����ʱ��
		 m_new_country  ����ʱ���󵱵�ʱ��
     m_utc_old_time: m_old_country�ĵ���ʱ��
���ز���:����ʱ����ĵ���ʱ��
*/
am_hal_rtc_time_t get_local_time(am_hal_rtc_time_t m_utc_old_time
                                ,int16_t m_old_country
                                ,int16_t m_new_country)
{
	am_hal_rtc_time_t m_local_time;
	rtc_time_t m_rtc_time_utc,m_rtc_time_local;
	unsigned int m_utc_second = 0;
	
	m_rtc_time_utc.Year = m_utc_old_time.ui32Year;
	m_rtc_time_utc.Month = m_utc_old_time.ui32Month;
	m_rtc_time_utc.Day = m_utc_old_time.ui32DayOfMonth;
	m_rtc_time_utc.Hour = m_utc_old_time.ui32Hour;
	m_rtc_time_utc.Minute = m_utc_old_time.ui32Minute;
	m_rtc_time_utc.Second = m_utc_old_time.ui32Second;
	//��ȡutc������
	m_utc_second = GetTimeSecond(m_rtc_time_utc);
	//��ǰʱ�������UTC-0ʱ����������
	m_utc_second -=  get_timezone_value(m_old_country);
	//ת�ɽ�Ҫ���õ�ʱ����������
	m_utc_second +=  get_timezone_value(m_new_country);
	
	//�����������ʱ����
	m_rtc_time_local = GetYmdhms(m_utc_second);
	
	m_local_time.ui32Hour = m_rtc_time_local.Hour;
	m_local_time.ui32Minute = m_rtc_time_local.Minute;
	m_local_time.ui32Second = m_rtc_time_local.Second;
	m_local_time.ui32DayOfMonth = m_rtc_time_local.Day;
	m_local_time.ui32Month = m_rtc_time_local.Month;
	m_local_time.ui32Year = m_rtc_time_local.Year;
	m_local_time.ui32Weekday = CaculateWeekDay(m_rtc_time_local.Year
	                            ,m_rtc_time_local.Month,m_rtc_time_local.Day);
	m_local_time.ui32Hundredths = 0;
	m_local_time.ui32Century = 0;
	
	return m_local_time;
}





#endif


