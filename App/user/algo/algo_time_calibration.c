/************************************************************
  Copyright (C), 2015? L.T.F Comm. E&T Co., Ltd.
  FileName: algo_time_calibration.c
  Author: hoopp       Version :1.0         Date:2018/05/23
  Description:     ������ʱ����    
  Version:         // �汾��Ϣ
  Function List:   // ��Ҫ�������书��
  History:         // ��ʷ�޸ļ�¼
  <author>  <time>    <version >   <desc>
***********************************************************/
#include "algo_time_calibration.h"
#include "task_gps.h"
#include "task_sport.h"
#include "task_display.h"
#include "bsp_rtc.h"
#include "bsp_timer.h"
#include "drv_extflash.h"


#if DEBUG_ENABLED == 1 && ALGO_TIME_CALIBRATION_LOG_ENABLED == 1
	#define ALGO_TIME_CALIBRATION_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define ALGO_TIME_CALIBRATION_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define ALGO_TIME_CALIBRATION_WRITESTRING(...)
	#define ALGO_TIME_CALIBRATION_PRINTF(...)		        
#endif

/**************************************
*����û��������ȷ����ʵ�ʵ�XTƵ�ʣ��������ñ�����ʱ���ܲ���ʱ�����������£�
*1.����������ʱ���ܣ���ʱ�ɹ��󱣴浱ǰʱ�䣬��⵽1pps�жϺ󣬽���ǰʱ��+1s��T1�����õ�RTC��
*2.����һ�죬�ٴο���������ʱ����1pps�ж�ʱ��ȡRTCʱ�䣨Ts���͵�ǰ������ʱʱ�䣨T2����(Ts-T2)*1000000/(T2-T1) = PAdj��
*  ���PAdj��Ϊ�����5����PAdj��
*3.��������ķ������в�����
*4.����һ�죬��һ�ο���������ʱ����1pps�ж�ʱ��ȡRTCʱ�䣬��Ϊ���й���������ʱ��ʱ�����ֻ�к��뼶��
*����RTCʱ��ͱ���ʱ����������ֵ�洢��Flash��ÿ��1���ʱ��Ӽ��������������õ�RTC��ȷ��ÿ��ֻ��У׼һ�Σ�
********************************************/

static am_hal_rtc_time_t RTC_time_Calibration_T1;//��һ�α�����ʱ��+1s
static am_hal_rtc_time_t RTC_time_Gps_Timing;//GPS��ʱ���ʱ�� ��û�и���RTC_Time
static am_hal_rtc_time_t RTC_time_Timing;//GPS��ʱʱ��RTCʱ��

static int32_t Time_PAdj = 0;// Time_PAdj = (Ts-T2) *1000000/(T2-T1);
static int32_t Time_Adj = 0;//����Ӧֵ Ӧ��-976~975����CALXT = Time_Adj
static TimeAdjStr TimeAdj;
static TimeCalibrationStr TimeCalibration;
static TimeCalibrationStatus TimeStatus;
static uint8_t g_TimeCalibrationStatus = STATUS_FLASH_NO_ADJ_NO_TCALIBRATION;//����ʱ��״̬
static bool IsGpsFirstTimingStatus = false;//�Ƿ��ǵ�һ��gps��ʱ �������߻ָ��������ú���Ϊ�ǵ�һ�� ���ڼ�¼��һ����ʱ��׼ʱ��

static uint8_t s_is_open_difftime_status = STATUS_CLOSE_DIFFTIME;/*����GPS׼ȷ��ʱ�����ɼ�gpsʱ���rtcʱ��*/
static difftime_gps_str difftime_gps;

void RecordAndSavecalibrationValue(void);
static bool s_is_mcu_restart_and_need_set_time = false;//�Ƿ񿪻�����Ҫ������ʱ
void set_s_is_mcu_restart_and_need_set_time(bool status)
{
	s_is_mcu_restart_and_need_set_time = status;
}
bool get_s_is_mcu_restart_and_need_set_time(void)
{
	return s_is_mcu_restart_and_need_set_time;
}
#if defined(WATCH_PRECISION_TIME_JUMP_TEST)
static uint32_t s_precision_time_jump_open_gps_count = 0;//������ʱʱ��������� ��GPS����
static uint32_t s_precision_time_compare_exception_count = 0;//ʱ��Ա� ʱ�������쳣 �ƴ�
uint32_t get_precision_time_jump_open_gps_count(void)
{
	return s_precision_time_jump_open_gps_count;
}
uint32_t get_precision_time_compare_exception_count(void)
{
	return s_precision_time_compare_exception_count;
}
#endif

#if defined(WATCH_GPS_OR_NO_GPS_RESOURCE_SET_TIME)
static uint8_t s_set_time_resource = SET_TIME_RESOURCE_BY_RESET;//Ĭ��������ʱ
uint8_t get_s_set_time_resource(void)
{
	return s_set_time_resource;
}
void set_s_set_time_resource(uint8_t resource)
{
	s_set_time_resource = resource;

	g_TimeCalibrationStatus = ReadCalibrationValueAndConfigAdjValue();
	if(g_TimeCalibrationStatus == STATUS_FLASH_HAS_ADJ_HAS_TCALIBRATION)
	{
		//У׼��ɺ�,����ʱ�����޸ĵ�ʱ��Ϊ����24��ʼ����
		//�ѱ���calibrationֵʱ �������޸����ڵ�ʱ��
		am_hal_rtc_time_get(&RTC_time);
		TimeCalibration.CalibrationSaveTime.Year = RTC_time.ui32Year;
		TimeCalibration.CalibrationSaveTime.Month = RTC_time.ui32Month;
		TimeCalibration.CalibrationSaveTime.Day = RTC_time.ui32DayOfMonth;
		TimeCalibration.CalibrationSaveTime.Hour = RTC_time.ui32Hour;
		TimeCalibration.CalibrationSaveTime.Minute = RTC_time.ui32Minute;
		TimeCalibration.CalibrationSaveTime.Second = RTC_time.ui32Second;
		TimeCalibration.CalibrationSaveTime.Msecond = RTC_time.ui32Hundredths;
		SaveCalibrationData();
		ALGO_TIME_CALIBRATION_PRINTF("[algo_time_calibration]:set_s_set_time_resource = %d,-->STATUS_FLASH_HAS_ADJ_HAS_TCALIBRATION\n",resource);
	}
	else
	{
		ALGO_TIME_CALIBRATION_PRINTF("[algo_time_calibration]:set_s_set_time_resource = %d,-->NOT STATUS_FLASH_HAS_ADJ_HAS_TCALIBRATION\n",resource);
	}
}
#endif
void Set_GpsFirstTimingStatus(bool status)
{
	IsGpsFirstTimingStatus = status;
}
bool Get_GpsFirstTimingStatus(void)
{
	return IsGpsFirstTimingStatus;
}
/*���þ���ʱ��У׼״̬*/
void Set_TimeCalibrationStatus(uint8_t status)
{
  g_TimeCalibrationStatus = status;
}
/*��ȡ����ʱ��У׼״̬*/
uint8_t Get_TimeCalibrationStatus(void)
{
	return g_TimeCalibrationStatus;
}
/*���þ���ʱ��У׼�Ľ�������״̬*/
void Set_TimeCalibrationHintStatus(TimeCalibrationStatus status)
{
  TimeStatus = status;
}
/*��ȡ����ʱ��У׼�Ľ�������״̬*/
TimeCalibrationStatus Get_TimeCalibrationHintStatus(void)
{
	return TimeStatus;
}
/*���þ���ʱ��У׼��calibrationֵ*/
void Set_TimeCalibrationStr(TimeCalibrationStr status)
{
  TimeCalibration = status;
}
/*��ȡ����ʱ��У׼��calibrationֵ*/
TimeCalibrationStr Get_TimeCalibrationStr(void)
{
	return TimeCalibration;
}
/*���þ���ʱ��У׼��Adjֵ*/
void Set_TimeAdjStr(TimeAdjStr status)
{
  TimeAdj = status;
}
/*��ȡ����ʱ��У׼��Adjֵ*/
TimeAdjStr Get_TimeAdjStr(void)
{
	return TimeAdj;
}
/*������ʱ��GPSʱ��*/
void Set_TimeGPSStr(am_hal_rtc_time_t status)
{
  RTC_time_Gps_Timing = status;
}
/*��ȡ��ʱ��GPSʱ��*/
am_hal_rtc_time_t Get_TimeGPSStr(void)
{
	return RTC_time_Gps_Timing;
}
/*������ʱ��Rtcʱ��*/
void Set_TimeRTCStr(am_hal_rtc_time_t status)
{
  RTC_time_Timing = status;
}
/*��ȡ��ʱ��Rtcʱ��*/
am_hal_rtc_time_t Get_TimeRTCStr(void)
{
	return RTC_time_Timing;
}
/*���õ�һ��У׼ǰ��ʱ��ʱ��*/
void Set_T1GPSStr(am_hal_rtc_time_t status)
{
  RTC_time_Calibration_T1 = status;
}
/*��ȡ��һ��У׼ǰ��ʱ��ʱ��*/
am_hal_rtc_time_t Get_T1GPSStr(void)
{
	return RTC_time_Calibration_T1;
}
/*����ʵʱ����GPS����״̬*/
void set_open_difftime_status(uint8_t status)
{
	s_is_open_difftime_status = status;
}
/*��ȡʵʱ����GPS����״̬*/
uint8_t get_open_difftime_status(void)
{
	return s_is_open_difftime_status;
}
/*����GPS����������ʱ��*/
void set_difftime_gps_time(difftime_gps_str status)
{
  difftime_gps = status;
}
/*��ȡGPS����������ʱ��*/
difftime_gps_str get_difftime_gps_time(void)
{
	return difftime_gps;
}
/*******************************************************
*0. Write "0x47" to the CLKKEY register to enable access to CLK_GEN registers
*1. Set the CALXT register field to 0 toinsure calibrationis not occurring
*2. Select the XT oscillator by setting the REG_CLKGEN_OCTRL_OSEL bit to 0.
*3. Select the XT or a division of it on a CLKOUT pad.
*4. Measure the frequency Fmeas at the CLKOUT pad.
*5. Compute the adjustment value required in ppm as ((Fnom �C Fmeas)*1000000)/Fmeas = PAdj
*6. Compute the adjustment value in steps as PAdj/(1000000/2^19) = PAdj/(0.9535) = Adj
*7. Compare Adj value with min/max range of -976 to 975
*8. If target Adj is within min and max, set CALXT = Adj
*9. Otherwise, the XT frequency is too low to be calibrated
*************************************************************/
/*��ʼ������CALRC ����ⲿ��������*/
static void CALRC_Config(int32_t xt_value)
{
	//
	// Unlock the clock control register.
	//
	AM_REG(CLKGEN, CLKKEY) = AM_REG_CLKGEN_CLKKEY_KEYVAL;

	//
	// Set the CALXT field to 0.
	//
	AM_REG(CLKGEN,CALXT) = xt_value;

	//
	// Lock the clock configuration registers.
	//
	AM_REG(CLKGEN, CLKKEY) = 0;
}
/*�������ÿ�����XT�Ĵ��� У׼��ɺ��������*/
void config_XT(void)
{
	if(g_TimeCalibrationStatus == STATUS_FLASH_HAS_ADJ_HAS_TCALIBRATION && TimeAdj.AdjValue != 0xFFFFFFFF)
	{
		CALRC_Config(TimeAdj.AdjValue);
	}
}
/*����CALRC
true:Adj���óɹ�,false:Adj���ò��ɹ�
*/
bool Compute_Adj(void)
{
	int64_t T1_T2,Ts_T2;//ms

	T1_T2 = DiffTimeMs(RTC_time_Gps_Timing,RTC_time_Calibration_T1);
	Ts_T2 = DiffTimeMs(RTC_time_Gps_Timing,RTC_time_Timing);
	
	if((T1_T2) < TIME_DAY_MAX_MS  && (T1_T2 >= TIME_DAY_MS))
	{
		if(T1_T2 != 0)
		{
			Time_PAdj = (Ts_T2) * 1000000/(T1_T2);
		}
		else
		{
			Time_PAdj = 0;
		}
		
		Time_Adj = Time_PAdj / (0.9535);

		ALGO_TIME_CALIBRATION_PRINTF("[algo_time_calibration]:Compute_Adj RTC_time_Gps_Timing:%d-%02d-%02d %d:%02d:%02d.%02d0\n"
					,RTC_time_Gps_Timing.ui32Year,RTC_time_Gps_Timing.ui32Month,RTC_time_Gps_Timing.ui32DayOfMonth
					,RTC_time_Gps_Timing.ui32Hour,RTC_time_Gps_Timing.ui32Minute,RTC_time_Gps_Timing.ui32Second,RTC_time_Gps_Timing.ui32Hundredths);
		ALGO_TIME_CALIBRATION_PRINTF("[algo_time_calibration]:Compute_Adj RTC_time_Calibration_T1:%d-%02d-%02d %d:%02d:%02d.%02d0\n"
					,RTC_time_Calibration_T1.ui32Year,RTC_time_Calibration_T1.ui32Month,RTC_time_Calibration_T1.ui32DayOfMonth
					,RTC_time_Calibration_T1.ui32Hour,RTC_time_Calibration_T1.ui32Minute,RTC_time_Calibration_T1.ui32Second,RTC_time_Calibration_T1.ui32Hundredths);
		ALGO_TIME_CALIBRATION_PRINTF("[algo_time_calibration]:Compute_Adj RTC_time_Timing:%d-%02d-%02d %d:%02d:%02d.%02d0\n"
					,RTC_time_Timing.ui32Year,RTC_time_Timing.ui32Month,RTC_time_Timing.ui32DayOfMonth
					,RTC_time_Timing.ui32Hour,RTC_time_Timing.ui32Minute,RTC_time_Timing.ui32Second,RTC_time_Timing.ui32Hundredths);
		ALGO_TIME_CALIBRATION_PRINTF("[algo_time_calibration]:T1_T2=0x%X,Ts_T2=0x%X,Time_PAdj=%d,Time_Adj=%d\n",T1_T2,Ts_T2,Time_PAdj,Time_Adj);
		if(Time_PAdj >= -976 && Time_PAdj <= 975)
		{//����[-976,975] �� GPSʱ����T1ʱ�䲻�ܳ���2��TIME_DAY_MAX_MS,��ʱ��ʱ��RTCʱ����GPSʱ�䲻�ܳ���1����(�ݶ�)
			//set CALRC = Time_Adj
			CALRC_Config(Time_Adj);
			ALGO_TIME_CALIBRATION_PRINTF("[algo_time_calibration]:Time_Adj is valid!Time_Adj=%d\n",Time_Adj);
			return true;
		}
		else
		{
			ALGO_TIME_CALIBRATION_PRINTF("[algo_time_calibration]:Time_Adj is invalid!Time_Adj=%d\n",Time_Adj);
			return false;
		}
	}
	else
	{
		return false;
	}
}


/*����˵��:����Adj�Ȳ���������1�� TimeAdjStr*/
void SaveAdjData(void)
{
	dev_extFlash_enable();
	dev_extFlash_erase(TIME_CALIBRATION_ADJ_DATA_START_ADDRESS,sizeof(TimeAdjStr));
	dev_extFlash_write(TIME_CALIBRATION_ADJ_DATA_START_ADDRESS,(uint8_t*)(&TimeAdj),sizeof(TimeAdjStr));
	dev_extFlash_disable();
}
/*����˵��:����Calibration�Ȳ���������2�� TimeCalibrationStr*/
void SaveCalibrationData(void)
{
	dev_extFlash_enable();
	dev_extFlash_erase(TIME_CALIBRATION_VALUE_DATA_START_ADDRESS,sizeof(TimeCalibrationStr));
	dev_extFlash_write(TIME_CALIBRATION_VALUE_DATA_START_ADDRESS,(uint8_t*)(&TimeCalibration),sizeof(TimeCalibrationStr));
	dev_extFlash_disable();
}
/*����˵��:����Calibration״̬�Ȳ���������3�� TimeCalibrationStatus*/
void SaveCalibrationStatusData(void)
{
	dev_extFlash_enable();
	dev_extFlash_erase(TIME_CALIBRATION_STSTUS_START_ADDRESS,sizeof(TimeCalibrationStatus));
	dev_extFlash_write(TIME_CALIBRATION_STSTUS_START_ADDRESS,(uint8_t*)(&TimeStatus),sizeof(TimeCalibrationStatus));
	dev_extFlash_disable();
}
/*
����˵��:��ȡflash�о���ʱ�䱣�����ز���������������� ��״ֵ̬ Adjֵ��Tcalibrationֵ
        ���ڿ�����������
����ֵ:��ȡ��״̬
*/
uint8_t ReadCalibrationValueAndConfigAdjValue(void)
{
	uint8_t flag = STATUS_FLASH_NO_ADJ_NO_TCALIBRATION;
	
	dev_extFlash_enable();
	dev_extFlash_read(TIME_CALIBRATION_ADJ_DATA_START_ADDRESS,(uint8_t*)(&TimeAdj),sizeof(TimeAdjStr));
	dev_extFlash_read(TIME_CALIBRATION_VALUE_DATA_START_ADDRESS,(uint8_t*)(&TimeCalibration),sizeof(TimeCalibrationStr));
	dev_extFlash_read(TIME_CALIBRATION_STSTUS_START_ADDRESS,(uint8_t*)(&TimeStatus),sizeof(TimeCalibrationStatus));
	dev_extFlash_disable();
	
	if(TimeAdj.IsWrite == 1)
	{//Adj�б���
		if(TimeCalibration.IsWrite == 1)
		{//TimeCalibration�б���
			flag = STATUS_FLASH_HAS_ADJ_HAS_TCALIBRATION;
		}
		else
		{
			flag = STATUS_FLASH_HAS_ADJ_NO_TCALIBRATION;
		}
	}
	else
	{//Adjû�б���
		if(TimeCalibration.IsWrite == 1)
		{//TimeCalibration�б���
			flag = STATUS_FLASH_NO_ADJ_HAS_TCALIBRATION;
		}
		else
		{
			flag = STATUS_FLASH_NO_ADJ_NO_TCALIBRATION;
		}
	}
	return flag;
}
#if defined(WATCH_PRECISION_TIME_CALIBRATION)
/*����˵��:APP�Ƿ����ͬ��ʱ�䵽�ֱ�
����ֵ:�����޸�ʱ��true,������false
*/
bool IsApptoWatchTime(rtc_time_t app,rtc_time_t watch)
{
	bool m_TimeFlag = false;
	
	g_TimeCalibrationStatus = ReadCalibrationValueAndConfigAdjValue();
	if(Get_IsInSportMode() == true && IsChange_ApptoWatch_TimeDiff(app,watch,TIME_DIFF_APP_WATCH) == false
		 && g_TimeCalibrationStatus < STATUS_FLASH_HAS_ADJ_HAS_TCALIBRATION)
	{//�˶�ģʽ���Ժ����  ��APPʱ����watchʱ���С��Tdiffappʱ����ʱ��ͬ�� У׼������APPʱ�䲻��ͬ����WATCH
		m_TimeFlag = false;
	}
	else
	{
		m_TimeFlag = true;
	}
	return m_TimeFlag;
}
#else
bool IsApptoWatchTime(void)
{
	bool m_TimeFlag = false;
	
	if(Get_IsInSportMode() == true)
	{//�˶�ģʽ���Ժ���� ����ʱ��ͬ��
		m_TimeFlag = false;
	}
	else
	{
		m_TimeFlag = true;
	}
	return m_TimeFlag;
}
#endif
/*�ж�ʱ���Ƿ���Ч*/
bool IsTimeValid(TimeStr time)
{
	if(time.Year == 0xFF || time.Month == 0xFF || time.Day == 0xFF
		 || time.Hour == 0xFF || time.Minute == 0xFF || time.Second == 0xFF || time.Msecond == 0xFF)
	{
		return false;
	}
	else
	{
		return true;
	}
}

/*����˵��:���ݲ���ֵ����RTCʱ��*/
static void SetRtcTime(void)
{
	struct tm time_c = {0};
	uint64_t t1 = 0,t2 = 0;
	rtc_time_t Ymdhms;
	am_hal_rtc_time_t m_before_time,m_now_time; //��ʱǰRTCʱ���Ҫ���õ�RTC��ʱ��
	uint32_t m_total_second = 0;//������
	
	am_hal_rtc_time_get(&m_before_time);
	time_c.tm_year = 2000 - 1900 + m_before_time.ui32Year;
	time_c.tm_mon  = m_before_time.ui32Month-1;
	time_c.tm_mday = m_before_time.ui32DayOfMonth;
	time_c.tm_hour = m_before_time.ui32Hour;
	time_c.tm_min  = m_before_time.ui32Minute;
	time_c.tm_sec  = m_before_time.ui32Second;
	t1 = (uint64_t)(mktime(&time_c))*1000 + (uint64_t)(m_before_time.ui32Hundredths*10);//ms
	t2 = (uint64_t)((int64_t)(t1) + (int64_t)(TimeCalibration.CalibrationValue));
	m_total_second = (uint32_t)(t2/1000);

	Ymdhms = GetYmdhms(m_total_second);
	RTC_time.ui32Year = Ymdhms.Year;
	RTC_time.ui32Month = Ymdhms.Month;
	RTC_time.ui32DayOfMonth = Ymdhms.Day;
	RTC_time.ui32Hour = Ymdhms.Hour;
	RTC_time.ui32Minute = Ymdhms.Minute;
	RTC_time.ui32Second = Ymdhms.Second;
	RTC_time.ui32Hundredths = t2%1000/10;
	RTC_time.ui32Weekday = CaculateWeekDay(Ymdhms.Year,Ymdhms.Month,Ymdhms.Day);
	RTC_time.ui32Century = 0;
	//����RTCʱ��
	am_hal_rtc_time_set(&RTC_time);
	m_now_time = RTC_time;
	RenewOneDay(m_before_time,m_now_time,true);//���￪�ٵĿռ��ʱ����ʱ���Բ���
#if defined(WATCH_GPS_OR_NO_GPS_RESOURCE_SET_TIME)
	set_s_set_time_resource(SET_TIME_RESOURCE_BY_CALIBRATION);//����RTCʱ����Դ--У׼��ɺ󲹳���ʱ
#endif
	ALGO_TIME_CALIBRATION_PRINTF("[algo_time_calibration]: SetRtcTime -->m_now_time: %d-%02d-%02d %d:%02d:%02d.%02d0\n"
	 ,RTC_time.ui32Year,RTC_time.ui32Month,RTC_time.ui32DayOfMonth
	 ,RTC_time.ui32Hour,RTC_time.ui32Minute,RTC_time.ui32Second,RTC_time.ui32Hundredths);
}
/*����˵��:��ȡ���һ�α�������2��TimeCalibrationʱ��ʱ��,Ȼ��MСʱ(24h)�󲹳�
���ز���:�Ƿ񲹳��ɹ�*/
bool TimeCalibrationCompensation(void)
{
	am_hal_rtc_time_t last_saveTime;
	int64_t diffTime = 0;//���У׼�õ�ʱ�����µ�RTCʱ�����,�������24h,���в���
	
	g_TimeCalibrationStatus = ReadCalibrationValueAndConfigAdjValue();
	
	last_saveTime.ui32Year = TimeCalibration.CalibrationSaveTime.Year;
	last_saveTime.ui32Month = TimeCalibration.CalibrationSaveTime.Month;
	last_saveTime.ui32DayOfMonth = TimeCalibration.CalibrationSaveTime.Day;
	last_saveTime.ui32Hour = TimeCalibration.CalibrationSaveTime.Hour;
	last_saveTime.ui32Minute = TimeCalibration.CalibrationSaveTime.Minute;
	last_saveTime.ui32Second = TimeCalibration.CalibrationSaveTime.Second;
	last_saveTime.ui32Hundredths = TimeCalibration.CalibrationSaveTime.Msecond;

	am_hal_rtc_time_get(&RTC_time);
	diffTime = DiffTimeMs(RTC_time,last_saveTime);
#if defined(DEBUG_TASK_GPS)
	ALGO_TIME_CALIBRATION_PRINTF("[algo_time_calibration]:calibration 24h before--->last_saveTime:%d-%02d-%02d %d:%02d:%02d.%02d0 diffTime=%d,%d,%d\n"
	 ,last_saveTime.ui32Year,last_saveTime.ui32Month,last_saveTime.ui32DayOfMonth
	 ,last_saveTime.ui32Hour,last_saveTime.ui32Minute,last_saveTime.ui32Second,last_saveTime.ui32Hundredths
	 ,(int32_t)diffTime,g_TimeCalibrationStatus,IsTimeValid(TimeCalibration.CalibrationSaveTime));
	ALGO_TIME_CALIBRATION_PRINTF("[algo_time_calibration]:calibration 24h before--->RTC_time:%d-%02d-%02d %d:%02d:%02d.%02d0,diffTime=%x-%x\n"
	 ,RTC_time.ui32Year,RTC_time.ui32Month,RTC_time.ui32DayOfMonth
	 ,RTC_time.ui32Hour,RTC_time.ui32Minute,RTC_time.ui32Second,RTC_time.ui32Hundredths
	 ,(int32_t)(diffTime>>32),(int32_t)(diffTime));
#endif
	if((g_TimeCalibrationStatus == STATUS_FLASH_HAS_ADJ_HAS_TCALIBRATION)
		 && (IsTimeValid(TimeCalibration.CalibrationSaveTime) == true) && (diffTime >= TIME_DAY_MS) && (diffTime < TIME_DAY_MS + 60000))
	{//״̬ok������ϴθ���ʱ���24h�󲹳� ��flash�б����ʱ��+24h difftime��24��,����1����֮��
		SetRtcTime();
		
		TimeCalibration.CalibrationSaveTime.Day ++;
		if(TimeCalibration.CalibrationSaveTime.Day > MonthMaxDay(TimeCalibration.CalibrationSaveTime.Year,TimeCalibration.CalibrationSaveTime.Month))
		{//��
			TimeCalibration.CalibrationSaveTime.Day -= MonthMaxDay(TimeCalibration.CalibrationSaveTime.Year,TimeCalibration.CalibrationSaveTime.Month);
			TimeCalibration.CalibrationSaveTime.Month ++;
			if(TimeCalibration.CalibrationSaveTime.Month > 12)
			{//��
				TimeCalibration.CalibrationSaveTime.Month -= 12;
				TimeCalibration.CalibrationSaveTime.Year ++;
			}
		}
		TimeStatus.status = EXCEPTION_CALIBRATION_CORRECT;
		SaveCalibrationData();
		SaveCalibrationStatusData();
		ALGO_TIME_CALIBRATION_PRINTF("[algo_time_calibration]:calibration 24h later:%d-%02d-%02d %d:%02d:%02d.%02d0\n",
	  TimeCalibration.CalibrationSaveTime.Year,TimeCalibration.CalibrationSaveTime.Month,TimeCalibration.CalibrationSaveTime.Day,
	  TimeCalibration.CalibrationSaveTime.Hour,TimeCalibration.CalibrationSaveTime.Minute,TimeCalibration.CalibrationSaveTime.Second,
	  TimeCalibration.CalibrationSaveTime.Msecond);
		return true;
	}
	else
	{
//		TimeStatus.status = EXCEPTION_CALIBRATION_WRONG;
//		SaveCalibrationStatusData();
		return false;
	}
}
/*����˵��:��ԭ�������� ������������1������2�����ݼ�����3������
	�������û����ϲ㷢�����������ʱ��������
*/
void Reset_TimeCalibration(void)
{
	CALRC_Config(0);
	g_TimeCalibrationStatus = STATUS_FLASH_NO_ADJ_NO_TCALIBRATION;
	TimeStatus.status = EXCEPTION_NONE;
	Set_GpsFirstTimingStatus(false);
	dev_extFlash_enable();
	dev_extFlash_erase(TIME_CALIBRATION_ADJ_DATA_START_ADDRESS,sizeof(TimeAdjStr));
	dev_extFlash_erase(TIME_CALIBRATION_VALUE_DATA_START_ADDRESS,sizeof(TimeCalibrationStr));
	dev_extFlash_erase(TIME_CALIBRATION_STSTUS_START_ADDRESS,sizeof(TimeCalibrationStatus));
	dev_extFlash_disable();
	memset(&RTC_time_Calibration_T1,0,sizeof(RTC_time_Calibration_T1));
#if defined(WATCH_GPS_OR_NO_GPS_RESOURCE_SET_TIME)
	set_s_set_time_resource(SET_TIME_RESOURCE_BY_RESET);//����RTCʱ����Դ--����
#endif
}
/*ʱ���1s���ֹʱ��Խλ*/
void GetAddTime(am_hal_rtc_time_t *AddRtc)
{
	if( AddRtc->ui32Second >= 60 )
	{//�� Խλ
		AddRtc->ui32Second -= 60;
		AddRtc->ui32Minute++;
		if( AddRtc->ui32Minute >= 60 )
		{//�� Խλ
			AddRtc->ui32Minute -= 60;
			AddRtc->ui32Hour++;
			if(AddRtc->ui32Hour >= 24)
			{//ʱ
				AddRtc->ui32Hour -= 24;
				AddRtc->ui32DayOfMonth ++;
				if(AddRtc->ui32DayOfMonth > MonthMaxDay(AddRtc->ui32Year,AddRtc->ui32Month))
				{//��
					AddRtc->ui32DayOfMonth -= MonthMaxDay(AddRtc->ui32Year,AddRtc->ui32Month);
					AddRtc->ui32Month ++;
					if(AddRtc->ui32Month > 12)
					{//��
						AddRtc->ui32Month -= 12;
						AddRtc->ui32Year ++;
					}
				}
			}							
		}
	}
}
/*
����˵��:��һ��У׼ ��¼gps��ʱ��ʱ��
*/
void RecordT1(void)
{
	//����T1
	RTC_time_Calibration_T1 = RTC_time_Gps_Timing;

	ALGO_TIME_CALIBRATION_PRINTF("[algo_time_calibration]:RecordT1:%d-%02d-%02d %d:%02d:%02d.%02d0\n",
	  RTC_time_Calibration_T1.ui32Year,RTC_time_Calibration_T1.ui32Month,RTC_time_Calibration_T1.ui32DayOfMonth,
	  RTC_time_Calibration_T1.ui32Hour,RTC_time_Calibration_T1.ui32Minute,RTC_time_Calibration_T1.ui32Second,
	  RTC_time_Calibration_T1.ui32Hundredths);
	ALGO_TIME_CALIBRATION_PRINTF("[algo_time_calibration]:RTC_time:%d-%02d-%02d %d:%02d:%02d.%02d0\n",
	  RTC_time.ui32Year,RTC_time.ui32Month,RTC_time.ui32DayOfMonth,
	  RTC_time.ui32Hour,RTC_time.ui32Minute,RTC_time.ui32Second,
	  RTC_time.ui32Hundredths);
}
/*
����˵��:�ڶ���У׼ ��¼gps��ʱ��ʱ�� 
���T2-T1<24hʱ,��ΪAdjУ׼��Ч,��Adjֵ����[-976 ,975]����Чֵ
����,�ڶ���У׼ʱ��Ϊ��һ��У׼ʱ��
*/
void RecordAndSaveAdjValue(void)
{
	int64_t diffTime = 0;
	bool m_Adj_Status = false;
	ALGO_TIME_CALIBRATION_PRINTF("[algo_time_calibration]:RecordT2:%d-%02d-%02d %d:%02d:%02d.%02d0\n",
	  RTC_time_Gps_Timing.ui32Year,RTC_time_Gps_Timing.ui32Month,RTC_time_Gps_Timing.ui32DayOfMonth,
	  RTC_time_Gps_Timing.ui32Hour,RTC_time_Gps_Timing.ui32Minute,RTC_time_Gps_Timing.ui32Second,
	  RTC_time_Gps_Timing.ui32Hundredths);
	ALGO_TIME_CALIBRATION_PRINTF("[algo_time_calibration]:RecordTs:%d-%02d-%02d %d:%02d:%02d.%02d0\n",
	  RTC_time_Timing.ui32Year,RTC_time_Timing.ui32Month,RTC_time_Timing.ui32DayOfMonth,
	  RTC_time_Timing.ui32Hour,RTC_time_Timing.ui32Minute,RTC_time_Timing.ui32Second,
	  RTC_time_Timing.ui32Hundredths);
	
	diffTime = DiffTimeMs(RTC_time,RTC_time_Calibration_T1);
	m_Adj_Status = Compute_Adj();
	ALGO_TIME_CALIBRATION_PRINTF("[algo_time_calibration]:RecordAndSaveAdjValue diffTime=0x%X,m_Adj_Status=%d\n",
	       diffTime,m_Adj_Status);
	//��ȡAdjֵ�����浽����1��
	if(m_Adj_Status && (diffTime >= TIME_DAY_MS && diffTime < TIME_DAY_MAX_MS))
	{//��һ�κ͵ڶ���ʱ�������[24,48)֮��ȷ����Ч
		TimeAdj.IsWrite = 1;
		TimeAdj.GpsFirstTime.Year = RTC_time_Gps_Timing.ui32Year;
		TimeAdj.GpsFirstTime.Month = RTC_time_Gps_Timing.ui32Month;
		TimeAdj.GpsFirstTime.Day = RTC_time_Gps_Timing.ui32DayOfMonth;
		TimeAdj.GpsFirstTime.Hour = RTC_time_Gps_Timing.ui32Hour;
		TimeAdj.GpsFirstTime.Minute = RTC_time_Gps_Timing.ui32Minute;
		TimeAdj.GpsFirstTime.Second = RTC_time_Gps_Timing.ui32Second;
		TimeAdj.GpsFirstTime.Msecond = RTC_time_Gps_Timing.ui32Hundredths;
		TimeAdj.AdjValue = Time_Adj;
		TimeAdj.AdjSaveTime.Year = RTC_time_Timing.ui32Year;
		TimeAdj.AdjSaveTime.Month = RTC_time_Timing.ui32Month;
		TimeAdj.AdjSaveTime.Day = RTC_time_Timing.ui32DayOfMonth;
		TimeAdj.AdjSaveTime.Hour = RTC_time_Timing.ui32Hour;
		TimeAdj.AdjSaveTime.Minute = RTC_time_Timing.ui32Minute;
		TimeAdj.AdjSaveTime.Second = RTC_time_Timing.ui32Second;
		TimeAdj.AdjSaveTime.Msecond = RTC_time_Timing.ui32Hundredths;
		
		g_TimeCalibrationStatus = STATUS_FLASH_NO_ADJ_NO_TCALIBRATION;
		TimeStatus.status = EXCEPTION_ADJ_VALUE_VALID;
		SaveAdjData();
		ALGO_TIME_CALIBRATION_PRINTF("[algo_time_calibration]:TimeAdj saved!diffTime=0x%X\n",diffTime);
	}
	else if(m_Adj_Status && (diffTime >= TIME_DAY_MAX_MS || diffTime < TIME_DAY_MS))
	{
		TimeStatus.status = EXCEPTION_ADJ_TIMEOUT;
		ALGO_TIME_CALIBRATION_PRINTF("[algo_time_calibration]:TimeAdj did not saved!EXCEPTION_ADJ_TIMEOUT diffTime=0x%X\n",diffTime);
		RecordT1();
	}
	else if(m_Adj_Status == false)
	{
		TimeStatus.status = EXCEPTION_ADJ_VALUE_INVALID;
		ALGO_TIME_CALIBRATION_PRINTF("[algo_time_calibration]:TimeAdj did not saved!EXCEPTION_ADJ_VALUE_INVALID diffTime=0x%X\n",diffTime);
		RecordT1();
	}
	else
	{//����48h ����У׼ֵ�쳣 ���¿�ʼ��һ��
		TimeStatus.status = EXCEPTION_ADJ_TIMEOUT;
		RecordT1();
		ALGO_TIME_CALIBRATION_PRINTF("[algo_time_calibration]:TimeAdj did not saved!diffTime=0x%X\n",diffTime);
	}
	SaveCalibrationStatusData();
}
/*
����˵��:������У׼ ��¼gps��ʱ��ʱ�� RTC_time_Calibration_Ts
�ж�Ts��Ч ��������ڶ���У׼ʱ��Ҫ�������24h����
*/
void RecordAndSavecalibrationValue(void)
{
	am_hal_rtc_time_t last_AdjTime;
	int64_t diffTime = 0,timeDiffValue = 0,m_timediff = 0;

	ALGO_TIME_CALIBRATION_PRINTF("[algo_time_calibration]:RTC_time_Gps_Timing:%d-%02d-%02d %d:%02d:%02d.%02d0\n",
	  RTC_time_Gps_Timing.ui32Year,RTC_time_Gps_Timing.ui32Month,RTC_time_Gps_Timing.ui32DayOfMonth,
	  RTC_time_Gps_Timing.ui32Hour,RTC_time_Gps_Timing.ui32Minute,RTC_time_Gps_Timing.ui32Second,
	  RTC_time_Gps_Timing.ui32Hundredths);
	ALGO_TIME_CALIBRATION_PRINTF("[algo_time_calibration]:RTC_time_Timing:%d-%02d-%02d %d:%02d:%02d.%02d0\n",
	  RTC_time_Timing.ui32Year,RTC_time_Timing.ui32Month,RTC_time_Timing.ui32DayOfMonth,
	  RTC_time_Timing.ui32Hour,RTC_time_Timing.ui32Minute,RTC_time_Timing.ui32Second,
	  RTC_time_Timing.ui32Hundredths);
	
	last_AdjTime.ui32Year = TimeAdj.GpsFirstTime.Year;
	last_AdjTime.ui32Month = TimeAdj.GpsFirstTime.Month;
	last_AdjTime.ui32DayOfMonth = TimeAdj.GpsFirstTime.Day;
	last_AdjTime.ui32Hour = TimeAdj.GpsFirstTime.Hour;
	last_AdjTime.ui32Minute = TimeAdj.GpsFirstTime.Minute;
	last_AdjTime.ui32Second = TimeAdj.GpsFirstTime.Second;
	last_AdjTime.ui32Hundredths = TimeAdj.GpsFirstTime.Msecond;

	//������У׼��,�����RTC_Time��GPS��ʱ֮����� ��ֵ���ڲ���
	m_timediff = DiffTimeMs(RTC_time_Gps_Timing,RTC_time_Timing);//��ʱ���Ը�ֵ����Ч�ж�
	diffTime = DiffTimeMs(RTC_time_Gps_Timing,last_AdjTime);//��ǰʱ����Adj֮�䱣���ʱ������24h
	ALGO_TIME_CALIBRATION_PRINTF("[algo_time_calibration]:-before set-timeDiffValue=%d,diffTime=0x%x%x\n"
	   ,timeDiffValue,(uint32_t)(diffTime>>32),(uint32_t)diffTime);
	if((diffTime >= TIME_DAY_MS && diffTime < TIME_DAY_MAX_MS)
		  && g_TimeCalibrationStatus == STATUS_FLASH_HAS_ADJ_NO_TCALIBRATION)
	{//δ����calibrationֵʱ ���油��ֵ��ʱ��
		/*���㲹��ֵ ��1СʱΪ����*/
		timeDiffValue = (m_timediff / (diffTime / TIME_HOUR_MS)) * TIME_HOUR_BASE_CAL;
		
		ALGO_TIME_CALIBRATION_PRINTF("[algo_time_calibration]:-set-timeDiffValue=%d,diffTime=0x%x%x\n"
					,timeDiffValue,(uint32_t)(diffTime>>32),(uint32_t)diffTime);
		TimeCalibration.IsWrite = 1;
		TimeCalibration.GpsSecondTime.Year = RTC_time_Gps_Timing.ui32Year;
		TimeCalibration.GpsSecondTime.Month = RTC_time_Gps_Timing.ui32Month;
		TimeCalibration.GpsSecondTime.Day = RTC_time_Gps_Timing.ui32DayOfMonth;
		TimeCalibration.GpsSecondTime.Hour = RTC_time_Gps_Timing.ui32Hour;
		TimeCalibration.GpsSecondTime.Minute = RTC_time_Gps_Timing.ui32Minute;
		TimeCalibration.GpsSecondTime.Second = RTC_time_Gps_Timing.ui32Second;
		TimeCalibration.GpsSecondTime.Msecond = RTC_time_Gps_Timing.ui32Hundredths;
		TimeCalibration.CalibrationValue = timeDiffValue;
		TimeCalibration.CalibrationSaveTime.Year = RTC_time_Timing.ui32Year;
		TimeCalibration.CalibrationSaveTime.Month = RTC_time_Timing.ui32Month;
		TimeCalibration.CalibrationSaveTime.Day = RTC_time_Timing.ui32DayOfMonth;
		TimeCalibration.CalibrationSaveTime.Hour = RTC_time_Timing.ui32Hour;
		TimeCalibration.CalibrationSaveTime.Minute = RTC_time_Timing.ui32Minute;
		TimeCalibration.CalibrationSaveTime.Second = RTC_time_Timing.ui32Second;
		TimeCalibration.CalibrationSaveTime.Msecond = RTC_time_Timing.ui32Hundredths;
		
		TimeStatus.status = EXCEPTION_CALIBRATION_VALUE_VALID;
		SaveCalibrationData();
		ALGO_TIME_CALIBRATION_PRINTF("[algo_time_calibration]:TimeCalibration saved!g_TimeCalibrationStatus=%d,timeDiffValue=0x%X,diffTime=0x%X,EXCEPTION_CALIBRATION_VALUE_VALID\n",
		 g_TimeCalibrationStatus,timeDiffValue,diffTime);
	}
	else if((diffTime >= TIME_DAY_MS && diffTime < TIME_DAY_MAX_MS)
 		&& g_TimeCalibrationStatus == STATUS_FLASH_HAS_ADJ_HAS_TCALIBRATION)
	{//�ѱ���calibrationֵʱ �������޸����ڵ�ʱ��
		TimeCalibration.CalibrationSaveTime.Year = RTC_time_Gps_Timing.ui32Year;
		TimeCalibration.CalibrationSaveTime.Month = RTC_time_Gps_Timing.ui32Month;
		TimeCalibration.CalibrationSaveTime.Day = RTC_time_Gps_Timing.ui32DayOfMonth;
		TimeCalibration.CalibrationSaveTime.Hour = RTC_time_Gps_Timing.ui32Hour;
		TimeCalibration.CalibrationSaveTime.Minute = RTC_time_Gps_Timing.ui32Minute;
		TimeCalibration.CalibrationSaveTime.Second = RTC_time_Gps_Timing.ui32Second;
		TimeCalibration.CalibrationSaveTime.Msecond = RTC_time_Gps_Timing.ui32Hundredths;
		SaveCalibrationData();
		TimeStatus.status = EXCEPTION_MOUNT_GPS_CALIBRATION;
		ALGO_TIME_CALIBRATION_PRINTF("[algo_time_calibration]:TimeCalibration saved!g_TimeCalibrationStatus=%d,timeDiffValue=0x%X,diffTime=0x%X,EXCEPTION_MOUNT_GPS_CALIBRATION\n",
		 g_TimeCalibrationStatus,timeDiffValue,diffTime);
	}
	else
	{//ʱ��̫����̫�� calibrationֵ�쳣 ˢ��Adj�е�ʱ��Ϊ���´�У׼
		TimeAdj.AdjSaveTime.Year = RTC_time_Gps_Timing.ui32Year;
		TimeAdj.AdjSaveTime.Month = RTC_time_Gps_Timing.ui32Month;
		TimeAdj.AdjSaveTime.Day = RTC_time_Gps_Timing.ui32DayOfMonth;
		TimeAdj.AdjSaveTime.Hour = RTC_time_Gps_Timing.ui32Hour;
		TimeAdj.AdjSaveTime.Minute = RTC_time_Gps_Timing.ui32Minute;
		TimeAdj.AdjSaveTime.Second = RTC_time_Gps_Timing.ui32Second;
		TimeAdj.AdjSaveTime.Msecond = RTC_time_Gps_Timing.ui32Hundredths;
		SaveAdjData();
		TimeStatus.status = EXCEPTION_CALIBRATION_VALUE_INVALID;
		ALGO_TIME_CALIBRATION_PRINTF("[algo_time_calibration]:TimeCalibration saved!g_TimeCalibrationStatus=%d,timeDiffValue=0x%X,diffTime=0x%X,EXCEPTION_CALIBRATION_VALUE_INVALID\n",
		 g_TimeCalibrationStatus,timeDiffValue,diffTime);
	}
	SaveCalibrationStatusData();
}
/*
����˵��:��GPS��ʱ��,������ʱҵ����
���ô˺���ǰ��:��ȡ��ʱ��ʱ���RTCʱ�估��ʱʱ��
*/
void TimeCalibrationInGps(void)
{
	g_TimeCalibrationStatus = ReadCalibrationValueAndConfigAdjValue();
	ALGO_TIME_CALIBRATION_PRINTF("[algo_time_calibration]:TimeCalibrationInGps g_TimeCalibrationStatus=%d,GpsFirstTimingStatus=%d\n",
					g_TimeCalibrationStatus,Get_GpsFirstTimingStatus());

		switch(g_TimeCalibrationStatus)
		{
			case STATUS_FLASH_NO_ADJ_NO_TCALIBRATION:
			case STATUS_FLASH_NO_ADJ_HAS_TCALIBRATION://�쳣��� ��Ҫ����У׼
				if(Get_GpsFirstTimingStatus() == false)
				{
					Set_GpsFirstTimingStatus(true);
					RecordT1();
					CALRC_Config(0);
				}
				else
				{//���ǵ�һ�������AdjУ׼���ü�����
#if defined(WATCH_GPS_OR_NO_GPS_RESOURCE_SET_TIME)
			if(get_s_set_time_resource() == SET_TIME_RESOURCE_BY_GPS)
			{//����ʱǰRTCʱ����GPS���õ�
				ALGO_TIME_CALIBRATION_PRINTF("[algo_time_calibration]:TimeCalibrationInGps --1--get_s_set_time_resource = SET_TIME_RESOURCE_BY_GPS\n");
				RecordAndSaveAdjValue();
			}
			else
			{//��GPS��ʱ,��������һ��GPS��ʱ
				Set_GpsFirstTimingStatus(true);
				RecordT1();
				CALRC_Config(0);
				ALGO_TIME_CALIBRATION_PRINTF("[algo_time_calibration]:TimeCalibrationInGps --1--get_s_set_time_resource = %d\n",get_s_set_time_resource());
			}
#else
			RecordAndSaveAdjValue();
#endif
				}
				break;
			case STATUS_FLASH_HAS_ADJ_NO_TCALIBRATION:
			case STATUS_FLASH_HAS_ADJ_HAS_TCALIBRATION:
				//����У׼calibrationValue
#if defined(WATCH_GPS_OR_NO_GPS_RESOURCE_SET_TIME)
			if(g_TimeCalibrationStatus == STATUS_FLASH_HAS_ADJ_NO_TCALIBRATION)
			{
				if(get_s_set_time_resource() == SET_TIME_RESOURCE_BY_GPS)
				{
					ALGO_TIME_CALIBRATION_PRINTF("[algo_time_calibration]:TimeCalibrationInGps --2--get_s_set_time_resource = SET_TIME_RESOURCE_BY_GPS\n");
					RecordAndSavecalibrationValue();
				}
				else
				{//��GPS��ʱ,��������һ��GPS��ʱ
					RecordAndSaveAdjValue();
					ALGO_TIME_CALIBRATION_PRINTF("[algo_time_calibration]:TimeCalibrationInGps --2--get_s_set_time_resource = %d\n",get_s_set_time_resource());
				}
			}
			else
			{
				RecordAndSavecalibrationValue();
				ALGO_TIME_CALIBRATION_PRINTF("[algo_time_calibration]:TimeCalibrationInGps --3--get_s_set_time_resource = %d\n",get_s_set_time_resource());
			}
#else
				RecordAndSavecalibrationValue();
#endif
				break;
			default:
				break;
		}

}
/*����˵��:�Ƿ�����ʱ�䵽RTC��
����:
1.��ʱ������Ҫ��ʱ
2.������ϵ����Ҫ��ʱ
3.��ɾ���У׼�ı�GPS������ʱ
4.δ��ɾ���У׼�ı�GPSʱ�������������24Сʱ�Ŀ�����ʱ
�����������ʱ*/
bool is_set_real_time_to_rtc(void)
{
	if((ScreenState == DISPLAY_SCREEN_UTC_GET))
	{
		ALGO_TIME_CALIBRATION_PRINTF("[algo_time_calibration]:is_set_real_time_to_rtc-->DISPLAY_SCREEN_UTC_GET\n");
		return true;
	}
	if(get_s_is_mcu_restart_and_need_set_time() == true)
	{
		ALGO_TIME_CALIBRATION_PRINTF("[algo_time_calibration]:is_set_real_time_to_rtc-->mcu restart!!\n");
		return true;
	}
	if(Get_TimeCalibrationStatus() == STATUS_FLASH_HAS_ADJ_HAS_TCALIBRATION)
	{
		ALGO_TIME_CALIBRATION_PRINTF("[algo_time_calibration]:is_set_real_time_to_rtc-->STATUS_FLASH_HAS_ADJ_HAS_TCALIBRATION!!\n");
		return true;
	}
	else if(Get_TimeCalibrationStatus() == STATUS_FLASH_NO_ADJ_NO_TCALIBRATION
		 || Get_TimeCalibrationStatus() == STATUS_FLASH_NO_ADJ_HAS_TCALIBRATION)
	{//���ڵ�һ����Ч��ʱ��ڶ�����Ч��ʱ֮��
		int64_t diffTime = 0;
		
		am_hal_rtc_time_get(&RTC_time);
		diffTime = DiffTimeMs(RTC_time,RTC_time_Calibration_T1);
		if(diffTime >= TIME_DAY_MS || (RTC_time_Calibration_T1.ui32Year == 0 && RTC_time_Calibration_T1.ui32Month == 0 && RTC_time_Calibration_T1.ui32DayOfMonth == 0))
		{//����24h����T1=0(���У׼״̬��)
			ALGO_TIME_CALIBRATION_PRINTF("[algo_time_calibration]:is_set_real_time_to_rtc-->STATUS_FLASH_NO_ADJ_NO_TCALIBRATION---diffTime >= TIME_DAY_MS!!\n");
			return true;
		}
		else
		{
			ALGO_TIME_CALIBRATION_PRINTF("[algo_time_calibration]:is_set_real_time_to_rtc-->STATUS_FLASH_NO_ADJ_NO_TCALIBRATION---diffTime < TIME_DAY_MS!!\n");
			return false;
		}
	}
	else if(Get_TimeCalibrationStatus() == STATUS_FLASH_HAS_ADJ_NO_TCALIBRATION)
	{//���ڵڶ�����Ч��ʱ���������Ч��ʱ֮��
		am_hal_rtc_time_get(&RTC_time);
		am_hal_rtc_time_t m_last_AdjTime;
		int64_t diffTime = 0;
		
		m_last_AdjTime.ui32Year = TimeAdj.GpsFirstTime.Year;
		m_last_AdjTime.ui32Month = TimeAdj.GpsFirstTime.Month;
		m_last_AdjTime.ui32DayOfMonth = TimeAdj.GpsFirstTime.Day;
		m_last_AdjTime.ui32Hour = TimeAdj.GpsFirstTime.Hour;
		m_last_AdjTime.ui32Minute = TimeAdj.GpsFirstTime.Minute;
		m_last_AdjTime.ui32Second = TimeAdj.GpsFirstTime.Second;
		m_last_AdjTime.ui32Hundredths = TimeAdj.GpsFirstTime.Msecond;

		diffTime = DiffTimeMs(RTC_time,m_last_AdjTime);
		
		if(diffTime >= TIME_DAY_MS)
		{
			ALGO_TIME_CALIBRATION_PRINTF("[algo_time_calibration]:is_set_real_time_to_rtc-->STATUS_FLASH_HAS_ADJ_NO_TCALIBRATION---diffTime >= TIME_DAY_MS!!\n");
			return true;
		}
		else
		{
			ALGO_TIME_CALIBRATION_PRINTF("[algo_time_calibration]:is_set_real_time_to_rtc-->STATUS_FLASH_HAS_ADJ_NO_TCALIBRATION---diffTime < TIME_DAY_MS!!\n");
			return false;
		}
	}
	
	return false;
}
/*difftime_gps_str ��ʼ��*/
static void init_difftime_gps_str(void)
{
	//��ʾ��GPSʱ���ʼ��
	difftime_gps.m_time.ui32Year = 0xFF;
	difftime_gps.m_time.ui32Month = 0xFF;
	difftime_gps.m_time.ui32DayOfMonth = 0xFF;
	difftime_gps.m_time.ui32Hour = 0xFF;
	difftime_gps.m_time.ui32Minute = 0xFF;
	difftime_gps.m_time.ui32Second = 0xFF;
	difftime_gps.m_time.ui32Hundredths = 0xFF;
	difftime_gps.m_time.ui32Weekday = 0xFF;
	difftime_gps.m_time.ui32Century = 0xFF;
	difftime_gps.gps_rtc_value = 0;
}
/*����GPS׼ȷ���������ɼ�gpsʱ���rtcʱ��*/
void open_difftime_rtc_gps_time(void)
{
	//����GPS����ȡGPSʱ�䵫����ʱ��RTC����
	s_is_open_difftime_status = STATUS_OPEN_DIFFITME;

	init_difftime_gps_str();
	CreateGPSTask();
#if defined(WATCH_PRECISION_TIME_JUMP_TEST)
	s_precision_time_jump_open_gps_count ++;
#endif
}
/*�ر�GPS׼ȷ��ʱ�����ɼ�gpsʱ���rtcʱ��*/
void close_difftime_rtc_gps_time(void)
{
	//�ر�GPS����ȡGPSʱ�䵫����ʱ��RTC����
	s_is_open_difftime_status = STATUS_CLOSE_DIFFTIME;
	CloseGPSTask();
#if defined(WATCH_PRECISION_TIME_JUMP_TEST)
	s_precision_time_jump_open_gps_count = 0;
	s_precision_time_compare_exception_count = 0;
#endif
}
/*��ȡGPS����ʱ����rtcʵʱʱ��Ĳ�ֵ ��λ:����*/
void get_gps_rtc_ms_value(am_hal_rtc_time_t gps_time,am_hal_rtc_time_t rtc_time)
{
	difftime_gps.gps_rtc_value = DiffTimeMs(gps_time,rtc_time);
	difftime_gps.m_time.ui32Year = gps_time.ui32Year;
	difftime_gps.m_time.ui32Month = gps_time.ui32Month;
	difftime_gps.m_time.ui32DayOfMonth = gps_time.ui32DayOfMonth;
	difftime_gps.m_time.ui32Hour = gps_time.ui32Hour;
	difftime_gps.m_time.ui32Minute = gps_time.ui32Minute;
	difftime_gps.m_time.ui32Second = gps_time.ui32Second;
	difftime_gps.m_time.ui32Hundredths = gps_time.ui32Hundredths;
	difftime_gps.m_time.ui32Weekday = gps_time.ui32Weekday;
	difftime_gps.m_time.ui32Century = gps_time.ui32Century;
	ALGO_TIME_CALIBRATION_PRINTF("[algo_time_calibration]:get_gps_rtc_ms_value gps_rtc_value=%d\n",(int32_t)difftime_gps.gps_rtc_value);
#if defined(WATCH_PRECISION_TIME_JUMP_TEST)
	if((int32_t)difftime_gps.gps_rtc_value > 500 || (int32_t)difftime_gps.gps_rtc_value <= -500)
	{
		s_precision_time_compare_exception_count ++;
	}
#endif
}


