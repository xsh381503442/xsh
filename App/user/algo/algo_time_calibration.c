/************************************************************
  Copyright (C), 2015? L.T.F Comm. E&T Co., Ltd.
  FileName: algo_time_calibration.c
  Author: hoopp       Version :1.0         Date:2018/05/23
  Description:     精密授时方案    
  Version:         // 版本信息
  Function List:   // 主要函数及其功能
  History:         // 历史修改记录
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
*由于没有仪器精确测量实际的XT频率，可以利用北斗授时功能测量时间误差，方法如下：
*1.开启北斗授时功能，授时成功后保存当前时间，检测到1pps中断后，将当前时间+1s（T1）设置到RTC；
*2.运行一天，再次开启北斗授时，在1pps中断时读取RTC时间（Ts）和当前北斗授时时间（T2），(Ts-T2)*1000000/(T2-T1) = PAdj；
*  这个PAdj即为上面第5条的PAdj；
*3.根据上面的方法进行补偿。
*4.运行一天，再一次开启北斗授时，在1pps中断时读取RTC时间，因为进行过补偿，这时候时间误差只有毫秒级，
*计算RTC时间和北斗时间的误差，将误差值存储到Flash，每天1点的时候加减掉这个误差再设置到RTC（确保每天只会校准一次）
********************************************/

static am_hal_rtc_time_t RTC_time_Calibration_T1;//第一次北斗授时后+1s
static am_hal_rtc_time_t RTC_time_Gps_Timing;//GPS授时后的时间 还没有赋给RTC_Time
static am_hal_rtc_time_t RTC_time_Timing;//GPS授时时的RTC时间

static int32_t Time_PAdj = 0;// Time_PAdj = (Ts-T2) *1000000/(T2-T1);
static int32_t Time_Adj = 0;//自适应值 应在-976~975，则CALXT = Time_Adj
static TimeAdjStr TimeAdj;
static TimeCalibrationStr TimeCalibration;
static TimeCalibrationStatus TimeStatus;
static uint8_t g_TimeCalibrationStatus = STATUS_FLASH_NO_ADJ_NO_TCALIBRATION;//精密时间状态
static bool IsGpsFirstTimingStatus = false;//是否是第一次gps授时 重启或者恢复出厂设置后都认为是第一次 用于记录第一次授时基准时间

static uint8_t s_is_open_difftime_status = STATUS_CLOSE_DIFFTIME;/*开启GPS准确授时任务后采集gps时间和rtc时间*/
static difftime_gps_str difftime_gps;

void RecordAndSavecalibrationValue(void);
static bool s_is_mcu_restart_and_need_set_time = false;//是否开机且需要重新授时
void set_s_is_mcu_restart_and_need_set_time(bool status)
{
	s_is_mcu_restart_and_need_set_time = status;
}
bool get_s_is_mcu_restart_and_need_set_time(void)
{
	return s_is_mcu_restart_and_need_set_time;
}
#if defined(WATCH_PRECISION_TIME_JUMP_TEST)
static uint32_t s_precision_time_jump_open_gps_count = 0;//精密授时时间跳变测试 打开GPS次数
static uint32_t s_precision_time_compare_exception_count = 0;//时间对比 时间误差大异常 计次
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
static uint8_t s_set_time_resource = SET_TIME_RESOURCE_BY_RESET;//默认重启授时
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
		//校准完成后,补偿时间以修改的时间为起点后24开始补偿
		//已保存calibration值时 仅保存修改日期的时间
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
/*设置精密时间校准状态*/
void Set_TimeCalibrationStatus(uint8_t status)
{
  g_TimeCalibrationStatus = status;
}
/*获取精密时间校准状态*/
uint8_t Get_TimeCalibrationStatus(void)
{
	return g_TimeCalibrationStatus;
}
/*设置精密时间校准的界面文字状态*/
void Set_TimeCalibrationHintStatus(TimeCalibrationStatus status)
{
  TimeStatus = status;
}
/*获取精密时间校准的界面文字状态*/
TimeCalibrationStatus Get_TimeCalibrationHintStatus(void)
{
	return TimeStatus;
}
/*设置精密时间校准的calibration值*/
void Set_TimeCalibrationStr(TimeCalibrationStr status)
{
  TimeCalibration = status;
}
/*获取精密时间校准的calibration值*/
TimeCalibrationStr Get_TimeCalibrationStr(void)
{
	return TimeCalibration;
}
/*设置精密时间校准的Adj值*/
void Set_TimeAdjStr(TimeAdjStr status)
{
  TimeAdj = status;
}
/*获取精密时间校准的Adj值*/
TimeAdjStr Get_TimeAdjStr(void)
{
	return TimeAdj;
}
/*设置授时后GPS时间*/
void Set_TimeGPSStr(am_hal_rtc_time_t status)
{
  RTC_time_Gps_Timing = status;
}
/*获取授时后GPS时间*/
am_hal_rtc_time_t Get_TimeGPSStr(void)
{
	return RTC_time_Gps_Timing;
}
/*设置授时后Rtc时间*/
void Set_TimeRTCStr(am_hal_rtc_time_t status)
{
  RTC_time_Timing = status;
}
/*获取授时后Rtc时间*/
am_hal_rtc_time_t Get_TimeRTCStr(void)
{
	return RTC_time_Timing;
}
/*设置第一次校准前授时的时间*/
void Set_T1GPSStr(am_hal_rtc_time_t status)
{
  RTC_time_Calibration_T1 = status;
}
/*获取第一次校准前授时的时间*/
am_hal_rtc_time_t Get_T1GPSStr(void)
{
	return RTC_time_Calibration_T1;
}
/*设置实时界面GPS开启状态*/
void set_open_difftime_status(uint8_t status)
{
	s_is_open_difftime_status = status;
}
/*获取实时界面GPS开启状态*/
uint8_t get_open_difftime_status(void)
{
	return s_is_open_difftime_status;
}
/*设置GPS更新下来的时间*/
void set_difftime_gps_time(difftime_gps_str status)
{
  difftime_gps = status;
}
/*获取GPS更新下来的时间*/
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
*5. Compute the adjustment value required in ppm as ((Fnom – Fmeas)*1000000)/Fmeas = PAdj
*6. Compute the adjustment value in steps as PAdj/(1000000/2^19) = PAdj/(0.9535) = Adj
*7. Compare Adj value with min/max range of -976 to 975
*8. If target Adj is within min and max, set CALXT = Adj
*9. Otherwise, the XT frequency is too low to be calibrated
*************************************************************/
/*初始化配置CALRC 针对外部晶振配置*/
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
/*用于配置开机后XT寄存器 校准完成后才能配置*/
void config_XT(void)
{
	if(g_TimeCalibrationStatus == STATUS_FLASH_HAS_ADJ_HAS_TCALIBRATION && TimeAdj.AdjValue != 0xFFFFFFFF)
	{
		CALRC_Config(TimeAdj.AdjValue);
	}
}
/*补偿CALRC
true:Adj配置成功,false:Adj配置不成功
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
		{//满足[-976,975] 及 GPS时间与T1时间不能超过2天TIME_DAY_MAX_MS,当时授时的RTC时间与GPS时间不能超过1分钟(暂定)
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


/*函数说明:保存Adj等参数到区域1中 TimeAdjStr*/
void SaveAdjData(void)
{
	dev_extFlash_enable();
	dev_extFlash_erase(TIME_CALIBRATION_ADJ_DATA_START_ADDRESS,sizeof(TimeAdjStr));
	dev_extFlash_write(TIME_CALIBRATION_ADJ_DATA_START_ADDRESS,(uint8_t*)(&TimeAdj),sizeof(TimeAdjStr));
	dev_extFlash_disable();
}
/*函数说明:保存Calibration等参数到区域2中 TimeCalibrationStr*/
void SaveCalibrationData(void)
{
	dev_extFlash_enable();
	dev_extFlash_erase(TIME_CALIBRATION_VALUE_DATA_START_ADDRESS,sizeof(TimeCalibrationStr));
	dev_extFlash_write(TIME_CALIBRATION_VALUE_DATA_START_ADDRESS,(uint8_t*)(&TimeCalibration),sizeof(TimeCalibrationStr));
	dev_extFlash_disable();
}
/*函数说明:保存Calibration状态等参数到区域3中 TimeCalibrationStatus*/
void SaveCalibrationStatusData(void)
{
	dev_extFlash_enable();
	dev_extFlash_erase(TIME_CALIBRATION_STSTUS_START_ADDRESS,sizeof(TimeCalibrationStatus));
	dev_extFlash_write(TIME_CALIBRATION_STSTUS_START_ADDRESS,(uint8_t*)(&TimeStatus),sizeof(TimeCalibrationStatus));
	dev_extFlash_disable();
}
/*
函数说明:获取flash中精密时间保存的相关参数及进行相关配置 及状态值 Adj值和Tcalibration值
        用于开启或重启后
返回值:获取的状态
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
	{//Adj有保存
		if(TimeCalibration.IsWrite == 1)
		{//TimeCalibration有保存
			flag = STATUS_FLASH_HAS_ADJ_HAS_TCALIBRATION;
		}
		else
		{
			flag = STATUS_FLASH_HAS_ADJ_NO_TCALIBRATION;
		}
	}
	else
	{//Adj没有保存
		if(TimeCalibration.IsWrite == 1)
		{//TimeCalibration有保存
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
/*函数说明:APP是否可以同步时间到手表
返回值:可以修改时间true,不可以false
*/
bool IsApptoWatchTime(rtc_time_t app,rtc_time_t watch)
{
	bool m_TimeFlag = false;
	
	g_TimeCalibrationStatus = ReadCalibrationValueAndConfigAdjValue();
	if(Get_IsInSportMode() == true && IsChange_ApptoWatch_TimeDiff(app,watch,TIME_DIFF_APP_WATCH) == false
		 && g_TimeCalibrationStatus < STATUS_FLASH_HAS_ADJ_HAS_TCALIBRATION)
	{//运动模式或稍后继续  且APP时间与watch时间差小于Tdiffapp时不能时间同步 校准过程中APP时间不能同步到WATCH
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
	{//运动模式或稍后继续 不能时间同步
		m_TimeFlag = false;
	}
	else
	{
		m_TimeFlag = true;
	}
	return m_TimeFlag;
}
#endif
/*判断时间是否有效*/
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

/*函数说明:根据补偿值设置RTC时间*/
static void SetRtcTime(void)
{
	struct tm time_c = {0};
	uint64_t t1 = 0,t2 = 0;
	rtc_time_t Ymdhms;
	am_hal_rtc_time_t m_before_time,m_now_time; //授时前RTC时间和要设置到RTC的时间
	uint32_t m_total_second = 0;//总秒数
	
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
	//设置RTC时间
	am_hal_rtc_time_set(&RTC_time);
	m_now_time = RTC_time;
	RenewOneDay(m_before_time,m_now_time,true);//这里开辟的空间的时间暂时忽略不计
#if defined(WATCH_GPS_OR_NO_GPS_RESOURCE_SET_TIME)
	set_s_set_time_resource(SET_TIME_RESOURCE_BY_CALIBRATION);//设置RTC时间来源--校准完成后补偿授时
#endif
	ALGO_TIME_CALIBRATION_PRINTF("[algo_time_calibration]: SetRtcTime -->m_now_time: %d-%02d-%02d %d:%02d:%02d.%02d0\n"
	 ,RTC_time.ui32Year,RTC_time.ui32Month,RTC_time.ui32DayOfMonth
	 ,RTC_time.ui32Hour,RTC_time.ui32Minute,RTC_time.ui32Second,RTC_time.ui32Hundredths);
}
/*函数说明:获取最后一次保存区域2中TimeCalibration时的时间,然后M小时(24h)后补偿
返回参数:是否补偿成功*/
bool TimeCalibrationCompensation(void)
{
	am_hal_rtc_time_t last_saveTime;
	int64_t diffTime = 0;//最后校准好的时间与新的RTC时间相比,如果大于24h,进行补偿
	
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
	{//状态ok后距离上次更改时间的24h后补偿 且flash中保存的时间+24h difftime在24点,浮动1分钟之间
		SetRtcTime();
		
		TimeCalibration.CalibrationSaveTime.Day ++;
		if(TimeCalibration.CalibrationSaveTime.Day > MonthMaxDay(TimeCalibration.CalibrationSaveTime.Year,TimeCalibration.CalibrationSaveTime.Month))
		{//天
			TimeCalibration.CalibrationSaveTime.Day -= MonthMaxDay(TimeCalibration.CalibrationSaveTime.Year,TimeCalibration.CalibrationSaveTime.Month);
			TimeCalibration.CalibrationSaveTime.Month ++;
			if(TimeCalibration.CalibrationSaveTime.Month > 12)
			{//月
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
/*函数说明:还原晶振配置 擦除保存区域1和区域2的数据及区域3的数据
	仅测试用或者上层发命令擦除精密时间数据用
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
	set_s_set_time_resource(SET_TIME_RESOURCE_BY_RESET);//设置RTC时间来源--重启
#endif
}
/*时间加1s后防止时间越位*/
void GetAddTime(am_hal_rtc_time_t *AddRtc)
{
	if( AddRtc->ui32Second >= 60 )
	{//秒 越位
		AddRtc->ui32Second -= 60;
		AddRtc->ui32Minute++;
		if( AddRtc->ui32Minute >= 60 )
		{//分 越位
			AddRtc->ui32Minute -= 60;
			AddRtc->ui32Hour++;
			if(AddRtc->ui32Hour >= 24)
			{//时
				AddRtc->ui32Hour -= 24;
				AddRtc->ui32DayOfMonth ++;
				if(AddRtc->ui32DayOfMonth > MonthMaxDay(AddRtc->ui32Year,AddRtc->ui32Month))
				{//天
					AddRtc->ui32DayOfMonth -= MonthMaxDay(AddRtc->ui32Year,AddRtc->ui32Month);
					AddRtc->ui32Month ++;
					if(AddRtc->ui32Month > 12)
					{//月
						AddRtc->ui32Month -= 12;
						AddRtc->ui32Year ++;
					}
				}
			}							
		}
	}
}
/*
函数说明:第一次校准 记录gps授时的时间
*/
void RecordT1(void)
{
	//记下T1
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
函数说明:第二次校准 记录gps授时的时间 
如果T2-T1<24h时,认为Adj校准有效,若Adj值处于[-976 ,975]则有效值
否则,第二次校准时间为第一次校准时间
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
	//获取Adj值并保存到区域1中
	if(m_Adj_Status && (diffTime >= TIME_DAY_MS && diffTime < TIME_DAY_MAX_MS))
	{//第一次和第二次时间差满足[24,48)之间确认有效
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
	{//大于48h 或者校准值异常 重新开始第一次
		TimeStatus.status = EXCEPTION_ADJ_TIMEOUT;
		RecordT1();
		ALGO_TIME_CALIBRATION_PRINTF("[algo_time_calibration]:TimeAdj did not saved!diffTime=0x%X\n",diffTime);
	}
	SaveCalibrationStatusData();
}
/*
函数说明:第三次校准 记录gps授时的时间 RTC_time_Calibration_Ts
判断Ts有效 第三次与第二次校准时间要满足相差24h以上
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

	//第三次校准后,计算出RTC_Time与GPS授时之间相差 该值用于补偿
	m_timediff = DiffTimeMs(RTC_time_Gps_Timing,RTC_time_Timing);//暂时不对该值作有效判断
	diffTime = DiffTimeMs(RTC_time_Gps_Timing,last_AdjTime);//当前时间与Adj之间保存的时间满足24h
	ALGO_TIME_CALIBRATION_PRINTF("[algo_time_calibration]:-before set-timeDiffValue=%d,diffTime=0x%x%x\n"
	   ,timeDiffValue,(uint32_t)(diffTime>>32),(uint32_t)diffTime);
	if((diffTime >= TIME_DAY_MS && diffTime < TIME_DAY_MAX_MS)
		  && g_TimeCalibrationStatus == STATUS_FLASH_HAS_ADJ_NO_TCALIBRATION)
	{//未保存calibration值时 保存补偿值及时间
		/*计算补偿值 以1小时为基数*/
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
	{//已保存calibration值时 仅保存修改日期的时间
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
	{//时间太长或太短 calibration值异常 刷新Adj中的时间为了下次校准
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
函数说明:在GPS授时中,精密授时业务处理
调用此函数前提:获取授时的时候的RTC时间及授时时间
*/
void TimeCalibrationInGps(void)
{
	g_TimeCalibrationStatus = ReadCalibrationValueAndConfigAdjValue();
	ALGO_TIME_CALIBRATION_PRINTF("[algo_time_calibration]:TimeCalibrationInGps g_TimeCalibrationStatus=%d,GpsFirstTimingStatus=%d\n",
					g_TimeCalibrationStatus,Get_GpsFirstTimingStatus());

		switch(g_TimeCalibrationStatus)
		{
			case STATUS_FLASH_NO_ADJ_NO_TCALIBRATION:
			case STATUS_FLASH_NO_ADJ_HAS_TCALIBRATION://异常情况 需要重新校准
				if(Get_GpsFirstTimingStatus() == false)
				{
					Set_GpsFirstTimingStatus(true);
					RecordT1();
					CALRC_Config(0);
				}
				else
				{//不是第一次则进行Adj校准配置及保存
#if defined(WATCH_GPS_OR_NO_GPS_RESOURCE_SET_TIME)
			if(get_s_set_time_resource() == SET_TIME_RESOURCE_BY_GPS)
			{//若授时前RTC时间是GPS设置的
				ALGO_TIME_CALIBRATION_PRINTF("[algo_time_calibration]:TimeCalibrationInGps --1--get_s_set_time_resource = SET_TIME_RESOURCE_BY_GPS\n");
				RecordAndSaveAdjValue();
			}
			else
			{//非GPS授时,须重新上一次GPS授时
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
				//重新校准calibrationValue
#if defined(WATCH_GPS_OR_NO_GPS_RESOURCE_SET_TIME)
			if(g_TimeCalibrationStatus == STATUS_FLASH_HAS_ADJ_NO_TCALIBRATION)
			{
				if(get_s_set_time_resource() == SET_TIME_RESOURCE_BY_GPS)
				{
					ALGO_TIME_CALIBRATION_PRINTF("[algo_time_calibration]:TimeCalibrationInGps --2--get_s_set_time_resource = SET_TIME_RESOURCE_BY_GPS\n");
					RecordAndSavecalibrationValue();
				}
				else
				{//非GPS授时,须重新上一次GPS授时
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
/*函数说明:是否设置时间到RTC中
条件:
1.授时界面需要授时
2.重启或断电等需要授时
3.完成精密校准的表开GPS可以授时
4.未完成精密校准的表开GPS时且满足区间大于24小时的可以授时
其他情况不授时*/
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
	{//处于第一次有效授时与第二次有效授时之间
		int64_t diffTime = 0;
		
		am_hal_rtc_time_get(&RTC_time);
		diffTime = DiffTimeMs(RTC_time,RTC_time_Calibration_T1);
		if(diffTime >= TIME_DAY_MS || (RTC_time_Calibration_T1.ui32Year == 0 && RTC_time_Calibration_T1.ui32Month == 0 && RTC_time_Calibration_T1.ui32DayOfMonth == 0))
		{//大于24h或者T1=0(清楚校准状态等)
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
	{//处于第二次有效授时与第三次有效授时之间
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
/*difftime_gps_str 初始化*/
static void init_difftime_gps_str(void)
{
	//显示的GPS时间初始化
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
/*开启GPS准确更新任务后采集gps时间和rtc时间*/
void open_difftime_rtc_gps_time(void)
{
	//开启GPS并获取GPS时间但不授时到RTC功能
	s_is_open_difftime_status = STATUS_OPEN_DIFFITME;

	init_difftime_gps_str();
	CreateGPSTask();
#if defined(WATCH_PRECISION_TIME_JUMP_TEST)
	s_precision_time_jump_open_gps_count ++;
#endif
}
/*关闭GPS准确授时任务后采集gps时间和rtc时间*/
void close_difftime_rtc_gps_time(void)
{
	//关闭GPS并获取GPS时间但不授时到RTC功能
	s_is_open_difftime_status = STATUS_CLOSE_DIFFTIME;
	CloseGPSTask();
#if defined(WATCH_PRECISION_TIME_JUMP_TEST)
	s_precision_time_jump_open_gps_count = 0;
	s_precision_time_compare_exception_count = 0;
#endif
}
/*获取GPS最新时间与rtc实时时间的差值 单位:毫秒*/
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


