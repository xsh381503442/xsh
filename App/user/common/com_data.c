#include "com_config.h"
#include "com_data.h"

#include "bsp_rtc.h"

#include "drv_extflash.h"
#include "drv_lsm6dsl.h"

#include "am_bootloader.h"

#include "task_ble.h"
#include "task_timer.h"
#include "task_sport.h"
#include "task_step.h"
#include "task_gps.h"

#include "gui_home.h"
#include "gui_sport.h"
#include "gui_step.h"
#include "gui_tool_calendar.h"

#include "com_sport.h"
#include "com_dial.h"
#include "algo_sport.h"
#include "algo_step_count.h"

#include "lib_error.h"
#include "lib_app_data.h"
#include "mode_power_off.h"
#include "algo_time_calibration.h"
#include "gui_system.h"

#if DEBUG_ENABLED == 1 && COM_DATA_LOG_ENABLED == 1
	#define COM_DATA_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define COM_DATA_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define COM_DATA_WRITESTRING(...)
	#define COM_DATA_PRINTF(...)		        
#endif
#define READ_DAY_DATA    23//��ȡ�ճ�/� 0��������


SetValueStr SetValue = {0};
DayMainDataStr  DayMainData = {0};
UploadStr Upload[2];//���ϴ��Ĳ���  ���� ����
DownloadStr Download[3];//APP�´��Ĳ��� �����������
TrainPlanDownStr TrainPlanDownloadSettingStr;
AmbientDataStr AmbientData;
extern uint32_t weekValue_sport[7],weekValue_common[7];
#ifdef COD 

_all_day_steps all_day_steps[7];
uint32_t last_store_com_step = 0 ; //
uint32_t last_store_run_step = 0 ;
uint32_t last_store_walk_step = 0 ;
uint32_t last_store_cal = 0 ;
uint32_t last_store_dis = 0 ;

uint32_t last_sync_com_step = 0 ; //
uint32_t last_sync_run_step = 0 ;
uint32_t last_sync_walk_step = 0 ;
uint32_t last_sync_cal = 0 ;
uint32_t last_sync_dis = 0 ;

#endif 
uint32_t StartSteps =0;
uint8_t last_basedheartrate =0;//ǰһ���������ֵ
/*��ȡ��ǰ�ܲ���*/
uint32_t Get_TotalStep(void)
{
	DayMainData.CommonSteps = get_filt_stepcount();
	return (DayMainData.CommonSteps + DayMainData.SportSteps);
}
/*��ȡ��ǰ�ܿ�·��*/
uint32_t Get_TotalEnergy(void)
{
	DayMainData.CommonEnergy = Get_CommonEnergy();
	return (DayMainData.CommonEnergy + DayMainData.SportEnergy);
}


/*��ȡ��ǰ�����*/
uint32_t Get_TotalDistance(void)
{
	DayMainData.CommonDistance = DayMainData.CommonSteps * Get_StepStride();
	return (DayMainData.CommonDistance + DayMainData.SportDistance);
}
/*��ȡ��ǰ�ճ���·��
�ֱ����ճ��Ʋ��������������˽���0�㵽��ǰʱ���������Ļ�����лֵ
*/
uint32_t Get_CommonEnergy(void)
{
#ifdef COD 

	uint32_t calories = 0;
	calories = CalculateDailyStepEnergy(DayMainData.CommonSteps,SetValue.AppSet.Weight);//��λ��
    return calories;
	
#else
	uint32_t calories = 0;
	if((RTC_time.ui32Hour==0)&&(RTC_time.ui32Minute==0))
		{
          calories = 24*60*GetRMR(SetValue.AppSet.Weight,SetValue.AppSet.Height,SetValue.AppSet.Age,SetValue.AppSet.Sex)*1000
                   + CalculateDailyStepEnergy(DayMainData.CommonSteps,SetValue.AppSet.Weight);//��λ��
	 }
	else
		{
	calories = (RTC_time.ui32Hour*60+RTC_time.ui32Minute)
								*GetRMR(SetValue.AppSet.Weight,SetValue.AppSet.Height,
										SetValue.AppSet.Age,SetValue.AppSet.Sex)*1000
                + CalculateDailyStepEnergy(DayMainData.CommonSteps,SetValue.AppSet.Weight);//��λ��
		}
	return calories;
#endif
}

#ifdef COD 

void all_day_steps_reinit(void)
{
	COM_DATA_PRINTF("%s\r\n",__func__);
	last_store_com_step =  get_filt_stepcount();
	last_store_run_step = SetValue.run_step;
	last_store_walk_step = SetValue.walk_step;
	last_store_cal = Get_TotalEnergy()/100;
	last_store_dis = Get_TotalDistance()/10;
}

void all_day_steps_clear(void)
{
	COM_DATA_PRINTF("%s\r\n",__func__);
	last_store_com_step =  0;
	last_store_run_step = 0;
	last_store_walk_step = 0;
	last_store_cal = 0;
	last_store_dis = 0;
	last_sync_com_step = 0 ; 
	last_sync_run_step = 0 ;
	last_sync_walk_step = 0 ;
	last_sync_cal = 0 ;
	last_sync_dis = 0 ;
}


void Store_all_day_steps(void)
{	
	COM_DATA_PRINTF("%s\r\n",__func__);
	uint16_t com_step_tmp,run_step_tmp,walk_step_tmp,cal_tmp,dis_tmp;
 	uint16_t com_step,run_step,walk_step,cal,dis;
	
	if (get_filt_stepcount() > last_store_com_step)
	{
		com_step_tmp = get_filt_stepcount() - last_store_com_step;
	}
	else
	{
		com_step_tmp= 0;
	}
	if ( SetValue.run_step > last_store_run_step)
	{
		run_step_tmp = SetValue.run_step  - last_store_run_step;
	}
	else
	{
		run_step_tmp= 0;
	}
	if (SetValue.walk_step  >last_store_walk_step)
	{
		walk_step_tmp = SetValue.walk_step  - last_store_walk_step;
	}
	else
	{
		walk_step_tmp = 0;
	}
	if ((Get_TotalEnergy()/100) > last_store_cal)
	{
		cal_tmp = Get_TotalEnergy()/100 - last_store_cal;
	}
	else
	{
		cal_tmp = 0;
	}
	if ((Get_TotalDistance()/10) > last_store_dis)
	{
		dis_tmp = Get_TotalDistance()/10 - last_store_dis;
	}
	else
	{
		dis_tmp= 0;
	}

	//��С��ת��
	com_step = (com_step_tmp << 8) |(com_step_tmp >> 8);
	run_step = (run_step_tmp << 8) |(run_step_tmp >> 8);
	walk_step = (walk_step_tmp << 8) |(walk_step_tmp >> 8);
	cal = (cal_tmp << 8) |(cal_tmp >> 8);
	dis = (dis_tmp << 8) |(dis_tmp >> 8);
	
	uint8_t index = DayMainData.Index;
	
	if (SetValue.all_day_step_address <ALL_DAY_STEPS_SIZE )
	{
		dev_extFlash_enable();
		dev_extFlash_write(ALL_DAY_STEPS_START_ADDRESS+ ALL_DAY_STEPS_SIZE*index+ SetValue.all_day_step_address,(uint8_t*)(&com_step),2);
		dev_extFlash_write(ALL_DAY_STEPS_START_ADDRESS+ ALL_DAY_STEPS_SIZE*index+ SetValue.all_day_step_address+2,(uint8_t*)(&run_step),2);
		dev_extFlash_write(ALL_DAY_STEPS_START_ADDRESS+ ALL_DAY_STEPS_SIZE*index+ SetValue.all_day_step_address+4,(uint8_t*)(&walk_step),2);
		dev_extFlash_write(ALL_DAY_STEPS_START_ADDRESS+ ALL_DAY_STEPS_SIZE*index+ SetValue.all_day_step_address+6,(uint8_t*)(&cal),2);
		dev_extFlash_write(ALL_DAY_STEPS_START_ADDRESS+ ALL_DAY_STEPS_SIZE*index+ SetValue.all_day_step_address+8,(uint8_t*)(&dis),2);
		dev_extFlash_disable();
		SetValue.all_day_step_address+= (10);
	}

	all_day_steps_reinit();
}

void Store_all_day_time(void)
{	
	COM_DATA_PRINTF("%s\r\n",__func__);
	uint8_t flag[5] = {0xfe,0xfe,0xfe,0xfe,0xfe};

	uint8_t year,month,day,hour,min;

	year = RTC_time.ui32Year;
	month = RTC_time.ui32Month;
	day = RTC_time.ui32DayOfMonth;
	hour = RTC_time.ui32Hour;
	min = RTC_time.ui32Minute;
	
	uint8_t index = DayMainData.Index;
	
	if (SetValue.all_day_step_address <ALL_DAY_STEPS_SIZE )
	{
		dev_extFlash_enable();
		dev_extFlash_write(ALL_DAY_STEPS_START_ADDRESS+ ALL_DAY_STEPS_SIZE*index+ SetValue.all_day_step_address,(uint8_t*)flag,5);
		dev_extFlash_write(ALL_DAY_STEPS_START_ADDRESS+ ALL_DAY_STEPS_SIZE*index+ SetValue.all_day_step_address+5,(uint8_t*)(&year),1);
		dev_extFlash_write(ALL_DAY_STEPS_START_ADDRESS+ ALL_DAY_STEPS_SIZE*index+ SetValue.all_day_step_address+6,(uint8_t*)(&month),1);
		dev_extFlash_write(ALL_DAY_STEPS_START_ADDRESS+ ALL_DAY_STEPS_SIZE*index+ SetValue.all_day_step_address+7,(uint8_t*)(&day),1);
		dev_extFlash_write(ALL_DAY_STEPS_START_ADDRESS+ ALL_DAY_STEPS_SIZE*index+ SetValue.all_day_step_address+8,(uint8_t*)(&hour),1);
		dev_extFlash_write(ALL_DAY_STEPS_START_ADDRESS+ ALL_DAY_STEPS_SIZE*index+ SetValue.all_day_step_address+9,(uint8_t*)(&min),1);
		dev_extFlash_disable();
		SetValue.all_day_step_address+= (10);	
	}

}

void read_all_day_steps(void)
{
	_all_day_steps temp_all_day_steps[7];
	_all_day_steps tmp;
	uint32_t temp_day[7];
	uint32_t temp_value;
	memset(temp_all_day_steps,0,sizeof(temp_all_day_steps));

	for (uint8_t i = 0;i< 7;i++)
	{	
	
		dev_extFlash_enable();
		dev_extFlash_read(ALL_DAY_STEPS_START_ADDRESS+ ALL_DAY_STEPS_SIZE*i+5,(uint8_t*)&temp_all_day_steps[i],3);
		
		if (temp_all_day_steps[i].year != 0xFF)
		{
			temp_day[i] = DaysDiff(20,1,1,temp_all_day_steps[i].year,temp_all_day_steps[i].month,temp_all_day_steps[i].day);
	   	    temp_all_day_steps[i].addr = ALL_DAY_STEPS_START_ADDRESS+ ALL_DAY_STEPS_SIZE *i;
			#if 1
			for(uint16_t j = 0;j< 409;j++)
			{
				dev_extFlash_read(ALL_DAY_STEPS_START_ADDRESS+ ALL_DAY_STEPS_SIZE*i+10*j,(uint8_t*)&temp_value,4);
				if (temp_value == 0xFFFFFFFF)
				{
				temp_all_day_steps[i].size = 10 *j;
				break;
				}
			}
			#endif
		}
		else
		{
			temp_day[i]=0x7FFFFFFF;
		}
	
		dev_extFlash_disable();

		COM_DATA_PRINTF("read temp_all_day_steps %d,%d,%d,addr:%x,%x\r\n",temp_all_day_steps[i].year,temp_all_day_steps[i].month,temp_all_day_steps[i].day,temp_all_day_steps[i].addr,temp_all_day_steps[i].size);
	}
	
	//����������
	 for (uint8_t i = 0; i < 7; i++) {        
		for (uint8_t j = 0; j < 7 - i - 1; j++) {              
			if (temp_day[j] > temp_day[j+1]) {                 
				memcpy(&tmp,&temp_all_day_steps[j],sizeof(_all_day_steps));
				memcpy(&temp_all_day_steps[j],&temp_all_day_steps[j+1],sizeof(_all_day_steps));
				memcpy(&temp_all_day_steps[j+1],&tmp,sizeof(_all_day_steps));
				temp_value = temp_day[j];
				temp_day[j] = temp_day[j+1];
				temp_day[j+1] = temp_value;
				
			}          
		}
	} 
	#if DEBUG_ENABLED == 1 
	for (uint8_t i = 0;i< 7;i++)
	{
		COM_DATA_PRINTF("order temp_all_day_steps Y:%dM:%dD:%d,addr:%x,s:%x\r\n",temp_all_day_steps[i].year,temp_all_day_steps[i].month,temp_all_day_steps[i].day,temp_all_day_steps[i].addr,temp_all_day_steps[i].size);	
	}
	#endif
	memcpy(all_day_steps,temp_all_day_steps,sizeof(_all_day_steps)*7);

}

void delete_all_day_steps(void)
{
	COM_DATA_PRINTF("%s\r\n",__func__);
	dev_extFlash_enable();
	for (uint8_t i = 0;i< 7;i++)
	{	
		dev_extFlash_erase(ALL_DAY_STEPS_START_ADDRESS+ ALL_DAY_STEPS_SIZE*i,ALL_DAY_STEPS_SIZE);	
	}
	dev_extFlash_disable();
	
	SetValue.all_day_step_address = 0;
	all_day_steps_reinit();
	Store_all_day_time();
}

#if 0
void delete_sport_data(void)
{
	
	//��Act_Stop_Time���㣬��Ϊɾ����׼��
	rtc_time_t stop_time = {0};
	uint32_t stop_time_offset = 8;
	dev_extFlash_enable();
	for (uint8_t day = 0;day< 7;day++)
	{	
		for (uint8_t num = 0; num < DAY_ACTIVITY_MAX_NUM; num++)
		{
			dev_extFlash_write( DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*day+ACTIVITY_DATA_START_ADDRESS + 
																ACTIVITY_DAY_DATA_SIZE*num + stop_time_offset ,(uint8_t*)(&stop_time),sizeof(rtc_time_t));
		}
	}
	#if DEBUG_ENABLED == 1 && COM_DATA_LOG_ENABLED == 1
	ActivityDataStr activitydata;
	dev_extFlash_read( DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE +ACTIVITY_DATA_START_ADDRESS+ACTIVITY_DAY_DATA_SIZE,(uint8_t*)(&activitydata),sizeof(ActivityDataStr));
	COM_DATA_PRINTF("delete_sport_data StopY:%dD:%dS:%d,Start %d\r\n",activitydata.Act_Stop_Time.Year,activitydata.Act_Stop_Time.Day,activitydata.Act_Stop_Time.Second,activitydata.Act_Start_Time.Year);
	#endif
	dev_extFlash_disable();

}
#endif

#endif 
/*
����˵��:��ȡ���� ����У׼ʱ,������ߵ�45%,��У׼��,��У׼����
*/
uint32_t Get_StepStride(void)
{
	uint32_t m_default_stride = 77;
	if(SetValue.IsFirstCalibrationStep == 1)
	{
		if(SetValue.AppSet.Height*45/100 == 0)
		{
			return m_default_stride;
		}
		else
		{
			return (SetValue.AppSet.Height*45/100);
		}
		
	}
	else
	{
		if(SetValue.AppGet.CalibrationStride == 0)
		{
			return m_default_stride;
		}
		else
		{
			return SetValue.AppGet.CalibrationStride;
		}
	}
}
void Store_SetData (void)
{
	//���¼���CRC
	//SetValue.crc = am_bootloader_fast_crc32((uint8_t*)(&SetValue.crc)+sizeof(SetValue.crc), sizeof(SetValueStr)-sizeof(SetValue.crc));	
	//��������
	dev_extFlash_enable();
	dev_extFlash_erase(SET_VALUE_ADDRESS,sizeof(SetValueStr));
	dev_extFlash_write(SET_VALUE_ADDRESS,(uint8_t*)(&SetValue),sizeof(SetValueStr));
	dev_extFlash_disable();
}

void Store_AmbientData(void)
{	
	//���滷���������
	dev_extFlash_enable();
	dev_extFlash_erase(AMBIENT_DATA_START_ADDRESS,sizeof(AmbientDataStr));
	dev_extFlash_write(AMBIENT_DATA_START_ADDRESS,(uint8_t*)(&AmbientData),sizeof(AmbientDataStr));
	dev_extFlash_disable();
}

//�������  ��ֹ�������岻��
void ClearStepCount(void)
{
	drv_lsm6dsl_acc_init();
	am_util_delay_ms(100);
	drv_lsm6dsl_clear_stepcount();
	#if defined (WATCH_STEP_ALGO)
	#else
	drv_lsm6dsl_enable_pedometer();
	#endif
	am_util_delay_ms(100);
	#if defined (WATCH_STEP_ALGO)
	set_filt_stepcount(0,0);
	drv_lsm6dsl_acc_fifo_init();
	#else
	set_filt_stepcount(0,0);
	move_int = 1;
	#endif 
	
}
//0�����㲽��
void TimetoClearStepCount(void)
{
	
	if(IsSportMode(ScreenState) != true)
	{
		//�����˶�ģʽ�£����³�ʼ���Ʋ�
		//����ǰ�Ȼ���
		DayMainData.CommonSteps = 0;
		DayMainData.SportSteps = 0;
		ClearStepCount();
	}
	else
	{
		//���˶�ģʽ�£����Ļ���
		//ActivityData.Activity_Index = 1;
	}
	
}
void AmbientData_Init(void)
{
	//��ʼ����������
	memset(&AmbientData, 0, sizeof(AmbientDataStr));
	
	Store_AmbientData();
}

void AmbientData_Read(void)
{
	//��ȡ��������
	dev_extFlash_enable();
	dev_extFlash_read(AMBIENT_DATA_START_ADDRESS, (uint8_t*)(&AmbientData), sizeof(AmbientDataStr));
	dev_extFlash_disable();
	
	if((AmbientData.AltitudeNum >= AMBIENT_DATA_MAX_NUM) 
		|| (AmbientData.PressureNum >= AMBIENT_DATA_MAX_NUM)
		|| (AmbientData.AltitudeSum > AMBIENT_DATA_MAX_NUM)
		|| (AmbientData.PressureSum > AMBIENT_DATA_MAX_NUM))
	{
		AmbientData_Init();
	}

//	AmbientData.PressureNum = 89;
//	AmbientData.PressureSum = 90;
//	AmbientData.AltitudeNum = 89;
//	AmbientData.AltitudeSum = 90;
//	
//	uint16_t start = 10055;
//	
//	for(uint8_t i = 0; i < 90; i++)
//	{
//		AmbientData.PressureData[i] = start;
//		if(i < 40)
//		{
//			start += 10;
//		}
//		else if(i < 60)
//		{
//			start -= 5; 
//		}
//		else if(i < 80)
//		{
//			start++;
//		}
//		else
//		{
//			start -= 5;
//		}
//	}
}

/*
*������ �Ƚ�Ҫ�޸ĵ�ʱ��͵�ǰʱ�䣬�ж��Ƿ��¿��洢�ռ�
*����: before_time ����ʱ��ǰ��RTCʱ��
       now_time    �������õ�RTC��ʱ��
		Act_Num_Clear		 	1�������ڸ����ã�0���ڲ���flash������һ���ռ�

*����ֵ:��
*��ע  
am_hal_rtc_time_set(&RTC_time);�����ڿ��ռ�֮ǰ���� ���⿪�ռ���ɵ�ʱ��
����޸�ʱ�������뵱ǰ���ڲ�һ�£������¿��ռ�
�������һ������Ҫ���¿��ռ䣬������Ҫ���Сʱ�������Сʱ��С�Ļ�������ͬСʱ�����洢ÿСʱ����
*/
void RenewOneDay( am_hal_rtc_time_t before_time,am_hal_rtc_time_t now_time,uint8_t Act_Num_Clear)
{
	uint8_t   clear_flag = 0;
	
	clear_flag = Act_Num_Clear;  //û���ã�ֻ���ú���������ǰ��ʽ����
	
	if(Act_Num_Clear == 1)
	{
		if( ( now_time.ui32DayOfMonth != before_time.ui32DayOfMonth ) || ( now_time.ui32Month != before_time.ui32Month )
						|| ( now_time.ui32Year != before_time.ui32Year ) )
		{//��ͬ��
			SetValue.CurrentAddress.ChangedHour = 0xFF;
			clear_flag = 1;
		}
		else
		{//ͬһ��
			if( before_time.ui32Hour > now_time.ui32Hour )
			{//�޸ĵ�ʱ����ֱ�ʵ��ʱ��С ���޸�ʱ�䵽�ֱ�ʱ��֮���ֹ�����ճ�����,�ظ�д��������쳣
				SetValue.CurrentAddress.ChangedHour = before_time.ui32Hour;
				Store_SetData();
				clear_flag = 0;			
			}
			else
			{//�޸ĵ�ʱ������ֱ�ʱ��ʱ�䲻��Ӱ��洢
				SetValue.CurrentAddress.ChangedHour = 0xFF;
				Store_SetData();
				clear_flag = 0;
			}
		}
	}
	else if(Act_Num_Clear == 0)
	{
		SetValue.CurrentAddress.ChangedHour = 0xFF;
		clear_flag = 1;
	}
	
	DayMainData.CommonEnergy = Get_CommonEnergy();//���¼���һ�λ�����·��
	if(clear_flag > 0 )
	{//��ͬ�� ��Ҫ����flash,���м�����ͬ���ڵ��ճ������ʱ������������ۼ�
			SetValue.CurrentAddress.ChangedHour = 0xFF;
			DayMainData.Year = now_time.ui32Year;
			DayMainData.Month = now_time.ui32Month;
			DayMainData.Date = now_time.ui32DayOfMonth;

			DayMainData.Index = SetValue.CurrentAddress.Index + 1;
		
			SetValue.IsAllowBaseHDR = 1;//ÿ��0�㿪ʼ���������������
			SetValue.TrainPlanHintFlag = 0;//ѵ���ƻ����ڷ��˶�ģʽ�ǳ�����ʱ�����Ҫ����

			if( DayMainData.Index >= 7 )
			{
				DayMainData.Index = 0;
			}
			SetValue.CurrentAddress.Index = DayMainData.Index;
			
			SetValue.CurrentAddress.IndexActivity = 0;
			
			SetValue.TrainRecordIndex[DayMainData.Index].ActivityNum = 0;

			dev_extFlash_enable();
			dev_extFlash_erase(DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*DayMainData.Index,DAY_MAIN_DATA_SIZE);
			dev_extFlash_write(DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*SetValue.CurrentAddress.Index,(uint8_t*)(&DayMainData),4);
			dev_extFlash_disable();

			#ifdef COD
			SetValue.all_day_step_address = 0;
			dev_extFlash_enable();
			dev_extFlash_erase(ALL_DAY_STEPS_START_ADDRESS+ALL_DAY_STEPS_SIZE*DayMainData.Index,ALL_DAY_STEPS_SIZE);
			dev_extFlash_disable();
			all_day_steps_reinit();
			Store_all_day_time();
			#endif 
			
			Store_SetData();
		}
		//���»�ȡ�޸ĺ�����ڵ�ѵ���ƻ����ݣ��ݷ�����
		if(get_trainplan_valid_flag())
		{//��ѵ���ƻ�ͬ��
			Get_TrainPlanDayRecord();
		}
#if defined WACTH_THEME_APP_DOWNLOAD
		//�Զ���ʱ����Ҫ�ڶ�ȡ�½��ձ��̵���Ч	
		if((before_time.ui32Year != now_time.ui32Year) || (before_time.ui32Month != now_time.ui32Month) 
			|| (before_time.ui32DayOfMonth != now_time.ui32DayOfMonth))
		{
			com_dial_init();
		}
#endif
	}

//static int toVal(char *pcAsciiStr)
//{
//	int iRetVal = 0;
//	iRetVal += pcAsciiStr[1] - '0';
//	iRetVal += pcAsciiStr[0] == ' ' ? 0 : (pcAsciiStr[0] - '0') * 10;
//	return iRetVal;
//}

//static char *pcMonth[] =
//{
//    "January",
//    "February",
//    "March",
//    "April",
//    "May",
//    "June",
//    "July",
//    "August",
//    "September",
//    "October",
//    "November",
//    "December",
//    "Invalid month"
//};

//static int mthToIndex(char *pcMon)
//{
//    int idx;
//    for (idx = 0; idx < 12; idx++)
//    {
//        if ( am_util_string_strnicmp(pcMonth[idx], pcMon, 3) == 0 )
//        {
//            return idx+1;
//        }
//    }
//    return 12;
//}
/*�ۼ�ʱ�䵼��Խλ
  ���ڿ�����10s*/
void AddTimetoOffSide(am_hal_rtc_time_t *AddRtc)
{
	AddRtc->ui32Second += 10;
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
static void DayMainData_Init(bool set_rtc)
{
	COM_DATA_PRINTF("[com_data]:DayMainData_Init--PowerOffHour=%d,CommonSteps=%d,SportSteps=%d,drv_lsm6dsl_get_stepcount()=%d\n",
	SetValue.CurrentAddress.PowerOffHour,DayMainData.CommonSteps,DayMainData.SportSteps,drv_lsm6dsl_get_stepcount());
	am_hal_rtc_time_get(&RTC_time);
	dev_extFlash_enable();
	dev_extFlash_read(DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*SetValue.CurrentAddress.Index,(uint8_t*)(&DayMainData),4);
	dev_extFlash_disable();

    StartSteps = 0;

#ifdef WATCH_STEP_ALGO
	set_step_status(USE_STEP_ALGO);
		
#else
	set_step_status(NO_USE_STEP_ALGO);
#endif

    if((SetValue.CurrentAddress.PowerOffHour & 0x80)&&(DayMainData.CommonSteps == 0)&&(DayMainData.SportSteps == 0)
			 &&(DayMainData.BasedHeartrate == 0) && (drv_lsm6dsl_get_stepcount() != 0))
    {//�����ػ� �����λ,���ݻָ�
				DayMainData.CommonSteps = SetValue.PowerOffMianData.CommonSteps;//�Ʋ���������ֵ����
        DayMainData.CommonEnergy = SetValue.PowerOffMianData.CommonEnergy;
        DayMainData.CommonDistance = SetValue.PowerOffMianData.CommonDistance;
        DayMainData.SportSteps = SetValue.PowerOffMianData.SportSteps;
        DayMainData.SportEnergy = SetValue.PowerOffMianData.SportEnergy;
        DayMainData.SportDistance = SetValue.PowerOffMianData.SportDistance;
				DayMainData.BasedHeartrate = SetValue.PowerOffMianData.BasedHeartrate;
				DayMainData.VO2MaxMeasuring = SetValue.PowerOffMianData.VO2MaxMeasuring;
				DayMainData.LTBmpMeasuring = SetValue.PowerOffMianData.LTBmpMeasuring;
				DayMainData.LTSpeedMeasuring = SetValue.PowerOffMianData.LTSpeedMeasuring;
				COM_DATA_PRINTF("[com_data]:DayMainData_Init-1-DayMainData.BasedHeartrate =%d,DayMainData.CommonSteps=%d,SportSteps=%d\n",
			    DayMainData.BasedHeartrate,DayMainData.CommonSteps,SetValue.PowerOffMianData.SportSteps );
                StartSteps = DayMainData.CommonSteps + DayMainData.SportSteps -drv_lsm6dsl_get_stepcount();			
			
				set_filt_stepcount(DayMainData.CommonSteps, DayMainData.SportSteps);
				
				
    }
		 else if((SetValue.CurrentAddress.PowerOffHour & 0x80)&&(DayMainData.CommonSteps == 0)&&(DayMainData.SportSteps == 0)
			 &&(DayMainData.BasedHeartrate == 0) && (drv_lsm6dsl_get_stepcount() == 0))
		 {//�ϵ縴λ,���ݻָ�
			DayMainData.CommonSteps = SetValue.PowerOffMianData.CommonSteps;//�Ʋ����������� ���ݻָ�
		  DayMainData.CommonEnergy = SetValue.PowerOffMianData.CommonEnergy;
			DayMainData.CommonDistance = SetValue.PowerOffMianData.CommonDistance;
			DayMainData.SportSteps = SetValue.PowerOffMianData.SportSteps;
			DayMainData.SportEnergy = SetValue.PowerOffMianData.SportEnergy;
			DayMainData.SportDistance = SetValue.PowerOffMianData.SportDistance;
			DayMainData.BasedHeartrate = SetValue.PowerOffMianData.BasedHeartrate;
			DayMainData.VO2MaxMeasuring = SetValue.PowerOffMianData.VO2MaxMeasuring;
			DayMainData.LTBmpMeasuring = SetValue.PowerOffMianData.LTBmpMeasuring;
			DayMainData.LTSpeedMeasuring = SetValue.PowerOffMianData.LTSpeedMeasuring;
			StartSteps = DayMainData.CommonSteps + DayMainData.SportSteps;
			
			set_filt_stepcount(DayMainData.CommonSteps, DayMainData.SportSteps);
			
			COM_DATA_PRINTF("[com_data]:DayMainData_Init-2-DayMainData.BasedHeartrate =%d,DayMainData.CommonSteps=%d\n",
			    DayMainData.BasedHeartrate,DayMainData.CommonSteps );
		 }
//	if(!set_rtc)
//	{
//		RTC_time.ui32Hour = toVal(&__TIME__[0]);
//		RTC_time.ui32Minute = toVal(&__TIME__[3]);
//		RTC_time.ui32Second = toVal(&__TIME__[6]);
//		RTC_time.ui32DayOfMonth = toVal(&__DATE__[4]);
//		RTC_time.ui32Month = mthToIndex(&__DATE__[0]);
//		RTC_time.ui32Year = toVal(&__DATE__[9]);
//		RTC_time.ui32Weekday = am_util_time_computeDayofWeek(RTC_time.ui32Year+2000, RTC_time.ui32Month, RTC_time.ui32DayOfMonth);
//		RTC_time.ui32Century = 0;
//		RTC_time.ui32Hundredths = 0;
//		am_hal_rtc_time_set(&RTC_time);
//	}

	if(RTC_time.ui32Weekday != am_util_time_computeDayofWeek(RTC_time.ui32Year+2000, RTC_time.ui32Month, RTC_time.ui32DayOfMonth))
	{
		//ʱ�䶪ʧ
		RTC_time.ui32Year = SetValue.PowerOffTime.Year;
		
		//��
		if((SetValue.PowerOffTime.Month > 0) && (SetValue.PowerOffTime.Month <= 12))
		{
			RTC_time.ui32Month = SetValue.PowerOffTime.Month;
		}
		else
		{
			RTC_time.ui32Month = 1;
		}
		
		//��
		if((SetValue.PowerOffTime.Day > 0) && (SetValue.PowerOffTime.Day <= MonthMaxDay(RTC_time.ui32Year, RTC_time.ui32Month)))
		{
			RTC_time.ui32DayOfMonth = SetValue.PowerOffTime.Day;
		}
		else
		{
			RTC_time.ui32DayOfMonth = 1;
		}
		
		if(SetValue.CurrentAddress.PowerOffHour & 0x80)
		{
			//�����ػ�״̬��
			RTC_time.ui32Hour = SetValue.PowerOffTime.Hour;
			RTC_time.ui32Minute = SetValue.PowerOffTime.Minute;
			RTC_time.ui32Second = SetValue.PowerOffTime.Second;
			
			if( RTC_time.ui32Hour < 23 )
			{
				//����8S��λ��2S����ʱ��
				AddTimetoOffSide(&RTC_time);
			}
		}
		else
		{
			//�쳣�ػ���ʱ��ֻ�ָܻ�Ϊ�ϸ�����ʱ��
			if( (SetValue.CurrentAddress.PowerOffHour & 0x7F) < 24 )
			{
				RTC_time.ui32Hour = SetValue.CurrentAddress.PowerOffHour & 0x7F;
			}
			else
			{
				RTC_time.ui32Hour = 12;
			}
			RTC_time.ui32Minute = 1;
			RTC_time.ui32Second = 0;	
		}
		RTC_time.ui32Hundredths = 0;
		RTC_time.ui32Weekday = CaculateWeekDay(RTC_time.ui32Year,RTC_time.ui32Month,RTC_time.ui32DayOfMonth);
		RTC_time.ui32Century = 0;
		RTC_time.ui32Hundredths = 0;
		am_hal_rtc_time_set(&RTC_time);
	}
	
	if( ( RTC_time.ui32DayOfMonth != DayMainData.Date ) || ( RTC_time.ui32Month != DayMainData.Month ) || (RTC_time.ui32Year != DayMainData.Year ) )
	{ //���ڲ���
		COM_DATA_PRINTF("[com_data]:DayMainData_Init Date need reconfiguration!\n");
		DayMainData.Year = RTC_time.ui32Year;
		DayMainData.Month = RTC_time.ui32Month;
		DayMainData.Date = RTC_time.ui32DayOfMonth;
		DayMainData.CommonDistance = 0;
		DayMainData.CommonEnergy = 0;
		DayMainData.CommonSteps = 0;
		DayMainData.BasedHeartrate = 0;
		DayMainData.Pressure = 0;
		DayMainData.SportDistance = 0;
		DayMainData.SportEnergy = 0;
		DayMainData.SportSteps = 0;
		DayMainData.Temprature = 0;
//		DayMainData.VO2MaxMeasuring = 0;
//		DayMainData.LTBmpMeasuring = 0;
//		DayMainData.LTSpeedMeasuring = 0;
		memset(DayMainData.Reserved,0,sizeof(DayMainData.Reserved));
		DayMainData.Index = SetValue.CurrentAddress.Index + 1;
		
		//��Ʋ�
		ClearStepCount();
		
		SetValue.IsAllowBaseHDR = 1;//ÿ��0�㿪ʼ���������������
		SetValue.TrainPlanHintFlag = 0;//ѵ���ƻ����ڷ��˶�ģʽ�ǳ�����ʱ�����Ҫ����
		if( DayMainData.Index >= 7 )
		{
			DayMainData.Index = 0;
		}
		SetValue.CurrentAddress.Index = DayMainData.Index;
		SetValue.CurrentAddress.IndexActivity = 0;
		SetValue.TrainRecordIndex[DayMainData.Index].ActivityNum = 0;
		SetValue.CurrentAddress.ChangedHour = 0xFF;
		SetValue.CurrentAddress.PowerOffHour = RTC_time.ui32Hour;
		
		dev_extFlash_enable();
		dev_extFlash_erase(DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*DayMainData.Index,sizeof(DayMainDataStr));
		dev_extFlash_write(DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*SetValue.CurrentAddress.Index,(uint8_t*)(&DayMainData),4);
		dev_extFlash_disable();

		#ifdef COD
		SetValue.run_step = 0;
	    SetValue.walk_step = 0;
		SetValue.all_day_step_address = 0;
		dev_extFlash_enable();
		dev_extFlash_erase(ALL_DAY_STEPS_START_ADDRESS+ALL_DAY_STEPS_SIZE*DayMainData.Index,ALL_DAY_STEPS_SIZE);
		dev_extFlash_disable();
		#endif 
	}
	else
	{
	  SetValue.CurrentAddress.PowerOffHour = RTC_time.ui32Hour;
		COM_DATA_PRINTF("[com_data]:DayMainData_Init date is right!\n");
	}
	#ifdef  COD 
	all_day_steps_reinit();
	Store_all_day_time();
	#endif
}

/*��ȡ�ճ���·��ճ��������ճ�����*/
void Get_DayCommonDataData(void)
{
	DayMainData.CommonSteps = get_filt_stepcount();
	DayMainData.CommonEnergy = Get_CommonEnergy();
	DayMainData.CommonDistance = DayMainData.CommonSteps * Get_StepStride();
}
static void Write_ExFlash_DayMainData(uint8_t  HourIndex)
{
	//��ȡ�ճ�����
	Get_DayCommonDataData();
	
	dev_extFlash_enable();
	dev_extFlash_write( DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*DayMainData.Index+DAY_COMMON_STEPS_OFFSET+DAY_COMMON_STEPS_LENGTH*HourIndex,
										 (uint8_t*)(&DayMainData.CommonSteps),DAY_COMMON_STEPS_LENGTH);
	dev_extFlash_write( DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*DayMainData.Index+DAY_COMMON_ENERGY_OFFSET+DAY_COMMON_ENERGY_LENGTH*HourIndex,
										 (uint8_t*)(&DayMainData.CommonEnergy),DAY_COMMON_ENERGY_LENGTH);
	dev_extFlash_write(DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*DayMainData.Index+DAY_COMMON_DISTANCE_OFFSET+DAY_COMMON_DISTANCE_LENGTH*HourIndex,
										 (uint8_t*)(&DayMainData.CommonDistance),DAY_COMMON_DISTANCE_LENGTH);
	dev_extFlash_write( DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*DayMainData.Index+DAY_SPORTS_STEPS_OFFSET+DAY_SPORTS_STEPS_LENGTH*HourIndex,
										 (uint8_t*)(&DayMainData.SportSteps),DAY_SPORTS_STEPS_LENGTH);
	dev_extFlash_write( DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*DayMainData.Index+DAY_SPORTS_ENERGY_OFFSET+DAY_SPORTS_ENERGY_LENGTH*HourIndex,
										 (uint8_t*)(&DayMainData.SportEnergy),DAY_SPORTS_ENERGY_LENGTH);
	dev_extFlash_write(DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*DayMainData.Index+DAY_SPORTS_DISTANCE_OFFSET+DAY_SPORTS_DISTANCE_LENGTH*HourIndex,
										 (uint8_t*)(&DayMainData.SportDistance),DAY_SPORTS_DISTANCE_LENGTH);			 
	dev_extFlash_write(DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*DayMainData.Index+DAY_BASE_HDR_OFFSET+DAY_BASE_HDR_LENGTH*HourIndex,
										 (uint8_t*)(&DayMainData.BasedHeartrate),DAY_BASE_HDR_LENGTH);								 
	dev_extFlash_write(DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*DayMainData.Index+DAY_TEMPRATURE_OFFSET+DAY_TEMPRATURE_LENGTH*HourIndex,
										 (uint8_t*)(&DayMainData.Temprature),DAY_TEMPRATURE_LENGTH);
	dev_extFlash_write(DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*DayMainData.Index+DAY_PRESSURE_OFFSET+DAY_PRESSURE_LENGTH*HourIndex,
										 (uint8_t*)(&DayMainData.Pressure),DAY_PRESSURE_LENGTH);
	dev_extFlash_write(DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*DayMainData.Index+DAY_VO2MAXMEASURING_OFFSET+DAY_VO2MAXMEASURING_LENGTH*HourIndex,
										 (uint8_t*)(&DayMainData.VO2MaxMeasuring),DAY_VO2MAXMEASURING_LENGTH);								 
	dev_extFlash_write(DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*DayMainData.Index+DAY_LTSPEEDMEASURING_OFFSET+DAY_LTSPEEDMEASURING_LENGTH*HourIndex,
										 (uint8_t*)(&DayMainData.LTSpeedMeasuring),DAY_LTSPEEDMEASURING_LENGTH);
	dev_extFlash_write(DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*DayMainData.Index+DAY_LTBMPMEASURING_OFFSET+DAY_LTBMPMEASURING_LENGTH*HourIndex,
										 (uint8_t*)(&DayMainData.LTBmpMeasuring),DAY_LTBMPMEASURING_LENGTH);

	dev_extFlash_disable();
}
/*
���ܣ��洢ÿ�������ݣ�������ǰ�ܲ�������ǰ�ܿ�·���ǰ�ܾ��룬������á�
������HourIndex   Сʱ�����ţ��洢����Сʱ����Ϊ24����ÿ��8���ֽڣ�1��ʱ��0�����Դ�����
������
*/
static void Store_DayMainData( uint8_t  HourIndex )
{
	if( SetValue.CurrentAddress.ChangedHour == 0xFF )
	{
		Write_ExFlash_DayMainData(HourIndex);
	}
	else
	{
		if( SetValue.CurrentAddress.ChangedHour <= HourIndex )
		{
			Write_ExFlash_DayMainData(HourIndex);
			SetValue.CurrentAddress.ChangedHour = 0xFF;
		}
	}

}

void StoreDayMainData(void)
{
	if( RTC_time.ui32Minute == 0 ) //����
	{
		if( RTC_time.ui32Hour == 0 )  //0��
		{
			COM_DATA_PRINTF("[com_data]:store Store_DayMainData -- 00:00!\n");
			Store_DayMainData(23);	
		
			TimetoClearStepCount();
	
			DayMainData.Year = RTC_time.ui32Year;
			DayMainData.Month = RTC_time.ui32Month;
			DayMainData.Date = RTC_time.ui32DayOfMonth;
			DayMainData.CommonDistance = 0;
			DayMainData.CommonEnergy = 0;
			DayMainData.SportDistance = 0;
			DayMainData.SportEnergy = 0;
			DayMainData.BasedHeartrate = 0;

			DayMainData.Index = SetValue.CurrentAddress.Index + 1;							
			if( DayMainData.Index >= 7 )
			{
				DayMainData.Index = 0;
			}
			SetValue.CurrentAddress.Index = DayMainData.Index;
			SetValue.CurrentAddress.IndexActivity = 0;
			SetValue.TrainRecordIndex[DayMainData.Index].ActivityNum = 0;

			dev_extFlash_enable();
			//ÿ����������2��Sector
			dev_extFlash_erase(DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*DayMainData.Index,DAY_MAIN_DATA_SIZE);
			dev_extFlash_write( DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*DayMainData.Index,(uint8_t*)(&DayMainData),4);
			dev_extFlash_disable();
			#ifdef COD
			SetValue.run_step = 0;
	    	SetValue.walk_step = 0;
			SetValue.all_day_step_address = 0;
			dev_extFlash_enable();
			dev_extFlash_erase(ALL_DAY_STEPS_START_ADDRESS+ALL_DAY_STEPS_SIZE*DayMainData.Index,ALL_DAY_STEPS_SIZE);
			dev_extFlash_disable();
			all_day_steps_clear();
			Store_all_day_time();
			#endif 
			
			SetValue.CurrentAddress.PowerOffHour = 0;
			SetValue.CurrentAddress.ChangedHour = 0xFF;
			
			//SetValue.Altitude_Offset = 0;
			SetValue.IsAllowBaseHDR = 1;//ÿ��0�㿪ʼ���������������

            //��ȡ���캣ƽ����ѹֵ
			ReadSeaLevelPressure(RTC_time.ui32Year,RTC_time.ui32Month,RTC_time.ui32DayOfMonth);
			
			//��ȡ��ʷ�ճ�����
			get_weekSteps();
			
#if defined WACTH_THEME_APP_DOWNLOAD
			//ÿ��0��ʱ����Ҫ���³�ʼ�����ձ���
			com_dial_festival_init();
#endif
		}
		else
		{
			COM_DATA_PRINTF("[com_data]:store Store_DayMainData!\n");
			//�洢�ճ�����
			Store_DayMainData(RTC_time.ui32Hour-1);
			SetValue.CurrentAddress.PowerOffHour = RTC_time.ui32Hour;					
		}
		
		//��������
		Store_SetData();
		
		//������ѹ�͸߶�����
		Store_AmbientData();
	}
}

//��ķ����ɭ���㹫ʽ�������ڼ������ڼ�
uint8_t CalculateWeekDay(uint32_t y, uint32_t m,uint32_t d){ 
		uint8_t tmp;
	
    if(m==1||m==2) 
			m+=12,y--;  
		
    int iWeek = (d+2*m+3*(m+1)/5+y+y/4-y/100+y/400)%7;  
		
    switch(iWeek){  
        case 0: tmp = 0; break;  
        case 1: tmp = 1; break;  
        case 2: tmp = 2; break;  
        case 3: tmp = 3; break;  
        case 4: tmp = 4; break;  
        case 5: tmp = 5; break;  
        case 6: tmp = 6; break;  
				default: tmp = 0; break;
    }
		
		return tmp;
}
typedef struct __attribute__((packed))
{
	uint8_t date;
	uint8_t month;
	uint8_t year;
}WeekDayStr;
/*��ȡһ�ܻ�������*/
void GetDayMainData_BasedHDRWeekData(uint8_t hdrValue[])
{
	uint32_t weekValue[7];
	WeekDayStr dayStr;
	int16_t m_days = 0;
	uint8_t  n,m;
	
	am_hal_rtc_time_get(&RTC_time);
	memset(weekValue,0,sizeof(weekValue));
	memset(hdrValue,0,7);
	dev_extFlash_enable();
	
	for(uint8_t i = 0;i < 7;i++)
	{
		dev_extFlash_read(DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*i, (uint8_t *)&dayStr,sizeof(dayStr));
		
		if(0xFF != dayStr.year || 0xFF != dayStr.month || 0xFF != dayStr.date)
		{
			n = CalculateWeekDay(2000 + dayStr.year, dayStr.month, dayStr.date);
			dev_extFlash_read(DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*i+DAY_BASE_HDR_OFFSET+DAY_BASE_HDR_LENGTH*READ_DAY_DATA,(uint8_t *)&weekValue[i],DAY_BASE_HDR_LENGTH);
			m_days = DaysDiff(RTC_time.ui32Year,RTC_time.ui32Month,RTC_time.ui32DayOfMonth,dayStr.year,dayStr.month,dayStr.date);
			
			COM_DATA_PRINTF("[com_data]:---hdr---dayStr.year=%d,month=%d,date=%d,n=%d,m_days=%d\n",dayStr.year,dayStr.month,dayStr.date,n,m_days);
			if(m_days > 0 || m_days <= -7)
			{//һ��ǰ�����ݲ���ʾ����״ͼ��
				weekValue[i] = 0;
			}
			if(0xFF == weekValue[i])
			{
				weekValue[i] = 0;
			}
			if(n == 6)
			{
				if(hdrValue[0] == 0)
				{
					hdrValue[0] += weekValue[i];
				}
				else
				{
					if(weekValue[i] != 0)
						hdrValue[0] = weekValue[i];
				}
				COM_DATA_PRINTF("[com_data]:---hdr---hdrValue[0]=%d,weekValue[%d]=%d!\n",hdrValue[0],i,weekValue[i]);
			}
			else
			{
				if(hdrValue[n+1] == 0)
				{
					hdrValue[n+1] += weekValue[i];
				}
				else
				{
					if(weekValue[i] != 0)
						hdrValue[n+1] = weekValue[i];
				}
				COM_DATA_PRINTF("[com_data]:---hdr---hdrValue[%d]=%d,weekValue[%d]=%d!\n",n+1,hdrValue[n+1],i,weekValue[i]);
			}	
		}
	}
	m = CalculateWeekDay(2000 + RTC_time.ui32Year, RTC_time.ui32Month, RTC_time.ui32DayOfMonth);

	if(m == 6)
	{
		hdrValue[0] = DayMainData.BasedHeartrate;
	}
	else
	{
		hdrValue[m+1] = DayMainData.BasedHeartrate;
	}
	
	for(uint8_t i=0;i<7;i++)
	{
		if(0xFF == hdrValue[i])
		{
			hdrValue[i] = 0;
		}
		COM_DATA_PRINTF("[com_data]:---RTCWeekday=%d,hdrValue[%d]=%d\n",RTC_time.ui32Weekday,i,hdrValue[i]);
	}

	
	last_basedheartrate = hdrValue[m];


	dev_extFlash_disable();
}

/*��ȡһ���ճ�����*/
void GetDayMainData_CommonStepWeekData(uint32_t dayOfWeekValue[])
{
	uint32_t weekValue[7];
	WeekDayStr dayStr;
	int16_t m_days = 0;
	uint8_t  n,m;
	
	am_hal_rtc_time_get(&RTC_time);
	memset(dayOfWeekValue,0,7);
	memset(weekValue,0,sizeof(weekValue));

	for(uint8_t i = 0;i < 7;i++)
	{
		dev_extFlash_enable();
		dev_extFlash_read(DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*i, (uint8_t *)&dayStr,sizeof(dayStr));
		dev_extFlash_disable();

		if(0xFF != dayStr.year || 0xFF != dayStr.month || 0xFF != dayStr.date)
		{
			n = CalculateWeekDay(2000 + dayStr.year, dayStr.month, dayStr.date);
			
			dev_extFlash_enable();
			dev_extFlash_read(DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*i+DAY_COMMON_STEPS_OFFSET+DAY_COMMON_STEPS_LENGTH*READ_DAY_DATA,(uint8_t *)&weekValue[i],DAY_COMMON_STEPS_LENGTH);
			dev_extFlash_disable();
			
			m_days = DaysDiff(RTC_time.ui32Year,RTC_time.ui32Month,RTC_time.ui32DayOfMonth,dayStr.year,dayStr.month,dayStr.date);
			COM_DATA_PRINTF("[com_data]:---CommonSteps---dayStr.year=%d,month=%d,date=%d,n=%d,weekValue[%d]=%d,m_days=%d\n",dayStr.year,dayStr.month,dayStr.date,n,i,weekValue[i],m_days);
			if(m_days > 0 || m_days <= -7)
			{//һ��ǰ�����ݲ���ʾ����״ͼ��
				weekValue[i] = 0;
			}
			if(0xFFFFFFFF == weekValue[i])
			{
				weekValue[i] = 0;
			}
			//�����ͬ��,�ϲ���ֵ(�ϲ�0��Ч��ֵ)
			if(n == 6)
			{
				if(dayOfWeekValue[0] == 0)
				{
					dayOfWeekValue[0] += weekValue[i];
				}
				else
				{
					if(weekValue[i] != 0)
						dayOfWeekValue[0] = weekValue[i];
				}
				COM_DATA_PRINTF("[com_data]:---CommonSteps---dayOfWeekValue[0]=%d,weekValue[%d]=%d!\n",dayOfWeekValue[0],i,weekValue[i]);
			}
			else
			{
				if(dayOfWeekValue[n+1] == 0)
				{
					dayOfWeekValue[n+1] += weekValue[i];
				}
				else
				{
					if(weekValue[i] != 0)
						dayOfWeekValue[n+1] = weekValue[i];
				}
				COM_DATA_PRINTF("[com_data]:---CommonSteps---dayOfWeekValue[%d]=%d,weekValue[%d]=%d!\n",n+1,dayOfWeekValue[n+1],i,weekValue[i]);
			}
		}
	}

	m = CalculateWeekDay(2000 + RTC_time.ui32Year, RTC_time.ui32Month, RTC_time.ui32DayOfMonth);

	if(m == 6)
	{
		dayOfWeekValue[0] = DayMainData.CommonSteps;
	}
	else
	{
		dayOfWeekValue[m+1] = DayMainData.CommonSteps;
	}
	
	for(uint8_t i=0;i<7;i++)
	{
		if(0xFFFFFFFF == dayOfWeekValue[i])
		{
			dayOfWeekValue[i] = 0;
		}
	}
}
/*��ȡһ�ܻ����*/
void GetDayMainData_SportsStepWeekData(uint32_t dayOfWeekValue[])
{
	uint32_t weekValue[7];
	WeekDayStr dayStr;
	int16_t m_days = 0;
	uint8_t  n,m;
	
	am_hal_rtc_time_get(&RTC_time);
	memset(dayOfWeekValue,0,7);
	memset(weekValue,0,sizeof(weekValue));

	for(uint8_t i = 0;i < 7;i++)
	{
		dev_extFlash_enable();
		dev_extFlash_read(DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*i, (uint8_t *)&dayStr,sizeof(dayStr));
		dev_extFlash_disable();

		if(0xFF != dayStr.year || 0xFF != dayStr.month || 0xFF != dayStr.date)
		{
			n = CalculateWeekDay(2000 + dayStr.year, dayStr.month, dayStr.date);
			
			dev_extFlash_enable();
			dev_extFlash_read(DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*i+DAY_SPORTS_STEPS_OFFSET+DAY_SPORTS_STEPS_LENGTH*READ_DAY_DATA,(uint8_t *)&weekValue[i],DAY_SPORTS_STEPS_LENGTH);
			dev_extFlash_disable();
			
			m_days = DaysDiff(RTC_time.ui32Year,RTC_time.ui32Month,RTC_time.ui32DayOfMonth,dayStr.year,dayStr.month,dayStr.date);
			COM_DATA_PRINTF("[com_data]:---SportsSteps---dayStr.year=%d,month=%d,date=%d,n=%d,m_days=%d\n",dayStr.year,dayStr.month,dayStr.date,n,m_days);
			if(m_days > 0 || m_days <= -7)
			{//һ��ǰ�����ݲ���ʾ����״ͼ��
				weekValue[i] = 0;
			}
			if(0xFFFFFFFF == weekValue[i])
			{
				weekValue[i] = 0;
			}
			//�����ͬ��,�ϲ���ֵ(�ϲ�0��Ч��ֵ)
			if(n == 6)
			{
				if(dayOfWeekValue[0] == 0)
				{
					dayOfWeekValue[0] += weekValue[i];
				}
				else
				{
					if(weekValue[i] != 0)
						dayOfWeekValue[0] = weekValue[i];
				}
				COM_DATA_PRINTF("[com_data]:---SportsSteps---dayOfWeekValue[0]=%d,weekValue[%d]=%d!\n",dayOfWeekValue[0],i,weekValue[i]);
			}
			else
			{
				if(dayOfWeekValue[n+1] == 0)
				{
					dayOfWeekValue[n+1] += weekValue[i];
				}
				else
				{
					if(weekValue[i] != 0)
						dayOfWeekValue[n+1] = weekValue[i];
				}
				COM_DATA_PRINTF("[com_data]:---SportsSteps---dayOfWeekValue[%d]=%d,weekValue[%d]=%d!\n",n+1,dayOfWeekValue[n+1],i,weekValue[i]);
			}
		}
	}
	
	m = CalculateWeekDay(2000 + RTC_time.ui32Year, RTC_time.ui32Month, RTC_time.ui32DayOfMonth);
	if(m == 6)
	{
		dayOfWeekValue[0] = DayMainData.SportSteps;
	}
	else
	{
		dayOfWeekValue[m+1] = DayMainData.SportSteps;
	}
	for(uint8_t i=0;i<7;i++)
	{
		if(0xFFFFFFFF == dayOfWeekValue[i])
		{
			dayOfWeekValue[i] = 0;
	}
	}
}
/*��ȡһ���ճ�����*/
void GetDayMainData_CommonDistanceWeekData(uint32_t dayOfWeekValue[])
{
	uint32_t weekValue[7];
	WeekDayStr dayStr;
	int16_t m_days = 0;
	uint8_t  n,m;
	
	am_hal_rtc_time_get(&RTC_time);
	memset(dayOfWeekValue,0,7);
	memset(weekValue,0,sizeof(weekValue));

	for(uint8_t i = 0;i < 7;i++)
	{
		dev_extFlash_enable();
		dev_extFlash_read(DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*i, (uint8_t *)&dayStr,sizeof(dayStr));
		dev_extFlash_disable();

		if(0xFF != dayStr.year || 0xFF != dayStr.month || 0xFF != dayStr.date)
		{
			n = CalculateWeekDay(2000 + dayStr.year, dayStr.month, dayStr.date);
			
			dev_extFlash_enable();
			dev_extFlash_read(DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*i+DAY_COMMON_DISTANCE_OFFSET+DAY_COMMON_DISTANCE_LENGTH*READ_DAY_DATA,(uint8_t *)&weekValue[i],DAY_COMMON_DISTANCE_LENGTH);
			dev_extFlash_disable();
			
			m_days = DaysDiff(RTC_time.ui32Year,RTC_time.ui32Month,RTC_time.ui32DayOfMonth,dayStr.year,dayStr.month,dayStr.date);
			COM_DATA_PRINTF("[com_data]:---CommonDistance---dayStr.year=%d,month=%d,date=%d,m_days=%d\n",dayStr.year,dayStr.month,dayStr.date,m_days);
			if(m_days > 0 || m_days <= -7)
			{//һ��ǰ�����ݲ���ʾ����״ͼ��
				weekValue[i] = 0;
			}
			if(0xFFFFFFFF == weekValue[i])
			{
				weekValue[i] = 0;
			}
			//�����ͬ��,�ϲ���ֵ(�ϲ�0��Ч��ֵ)
			if(n == 6)
			{
				if(dayOfWeekValue[0] == 0)
				{
					dayOfWeekValue[0] += weekValue[i];
				}
				else
				{
					if(weekValue[i] != 0)
						dayOfWeekValue[0] = weekValue[i];
				}
				COM_DATA_PRINTF("[com_data]:---CommonDistance---dayOfWeekValue[0]=%d,weekValue[%d]=%d!\n",dayOfWeekValue[0],i,weekValue[i]);
			}
			else
			{
				if(dayOfWeekValue[n+1] == 0)
				{
					dayOfWeekValue[n+1] += weekValue[i];
				}
				else
				{
					if(weekValue[i] != 0)
						dayOfWeekValue[n+1] = weekValue[i];
				}
				COM_DATA_PRINTF("[com_data]:---CommonDistance---dayOfWeekValue[%d]=%d,weekValue[%d]=%d!\n",n+1,dayOfWeekValue[n+1],i,weekValue[i]);
			}
		}
	}
	
	DayMainData.CommonDistance = DayMainData.CommonSteps * Get_StepStride();
	m = CalculateWeekDay(2000 + RTC_time.ui32Year, RTC_time.ui32Month, RTC_time.ui32DayOfMonth);
	if(m == 6)
	{
		dayOfWeekValue[0] = DayMainData.CommonDistance;
	}
	else
	{
		dayOfWeekValue[m+1] = DayMainData.CommonDistance;
	}
	for(uint8_t i=0;i<7;i++)
	{
		if(0xFFFFFFFF == dayOfWeekValue[i])
		{
			dayOfWeekValue[i] = 0;
		}
	}
}
/*��ȡһ�ܻ����*/
void GetDayMainData_SportsDistanceWeekData(uint32_t dayOfWeekValue[])
{
	uint32_t weekValue[7];
	WeekDayStr dayStr;
	int16_t m_days = 0;
	uint8_t  n,m;
	
	am_hal_rtc_time_get(&RTC_time);
	memset(dayOfWeekValue,0,7);
	memset(weekValue,0,sizeof(weekValue));

	for(uint8_t i = 0;i < 7;i++)
	{
		dev_extFlash_enable();
		dev_extFlash_read(DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*i, (uint8_t *)&dayStr,sizeof(dayStr));
		dev_extFlash_disable();
		
		if(0xFF != dayStr.year || 0xFF !=dayStr.month || 0xFF != dayStr.date)
		{
			n = CalculateWeekDay(2000 + dayStr.year, dayStr.month, dayStr.date);
			
			dev_extFlash_enable();
			dev_extFlash_read(DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*i+DAY_SPORTS_DISTANCE_OFFSET+DAY_SPORTS_DISTANCE_LENGTH*READ_DAY_DATA,(uint8_t *)&weekValue[i],DAY_SPORTS_DISTANCE_LENGTH);
			dev_extFlash_disable();
			
			m_days = DaysDiff(RTC_time.ui32Year,RTC_time.ui32Month,RTC_time.ui32DayOfMonth,dayStr.year,dayStr.month,dayStr.date);
			COM_DATA_PRINTF("[com_data]:---SportsDistance---dayStr.year=%d,month=%d,date=%d,n=%d,m_days=%d\n",dayStr.year,dayStr.month,dayStr.date,n,m_days);
			if(m_days > 0 || m_days <= -7)
			{//һ��ǰ�����ݲ���ʾ����״ͼ��
				weekValue[i] = 0;
			}
			if(0xFFFFFFFF == weekValue[i])
			{
				weekValue[i] = 0;
			}
			//�����ͬ��,�ϲ���ֵ(�ϲ�0��Ч��ֵ)
			if(n == 6)
			{
				if(dayOfWeekValue[0] == 0)
				{
					dayOfWeekValue[0] += weekValue[i];
				}
				else
				{
					if(weekValue[i] != 0)
						dayOfWeekValue[0] = weekValue[i];
				}
				COM_DATA_PRINTF("[com_data]:---SportsDistance---dayOfWeekValue[0]=%d,weekValue[%d]=%d!\n",dayOfWeekValue[0],i,weekValue[i]);
			}
			else
			{
				if(dayOfWeekValue[n+1] == 0)
				{
					dayOfWeekValue[n+1] += weekValue[i];
				}
				else
				{
					if(weekValue[i] != 0)
						dayOfWeekValue[n+1] = weekValue[i];
				}
				COM_DATA_PRINTF("[com_data]:---SportsDistance---dayOfWeekValue[%d]=%d,weekValue[%d]=%d!\n",n+1,dayOfWeekValue[n+1],i,weekValue[i]);
			}
		}
	}
	m = CalculateWeekDay(2000 + RTC_time.ui32Year, RTC_time.ui32Month, RTC_time.ui32DayOfMonth);
	if(m == 6)
	{
		dayOfWeekValue[0] = DayMainData.SportDistance;
	}
	else
	{
		dayOfWeekValue[m+1] = DayMainData.SportDistance;
	}
	for(uint8_t i=0;i<7;i++)
	{
		if(0xFFFFFFFF == dayOfWeekValue[i])
		{
			dayOfWeekValue[i] = 0;
	}
	}
}
/*��ȡһ�ܻ��·��*/
void GetDayMainData_SportsCaloriesWeekData(uint32_t dayOfWeekValue[])
{
	uint32_t weekValue[7];
	WeekDayStr dayStr;
	int16_t m_days = 0;
	uint8_t  n,m;
	am_hal_rtc_time_get(&RTC_time);
	memset(dayOfWeekValue,0,7);
	memset(weekValue,0,sizeof(weekValue));
	dev_extFlash_enable();

	for(uint8_t i = 0;i < 7;i++)
	{
		dev_extFlash_read(DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*i, (uint8_t *)&dayStr,sizeof(dayStr));
      
		if(0xFF != dayStr.year || 0xFF !=dayStr.month || 0xFF != dayStr.date)
		{
			n = CalculateWeekDay(2000 + dayStr.year, dayStr.month, dayStr.date);
			dev_extFlash_read(DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*i+DAY_SPORTS_ENERGY_OFFSET+DAY_SPORTS_ENERGY_LENGTH*READ_DAY_DATA,(uint8_t *)&weekValue_sport[i],DAY_SPORTS_ENERGY_LENGTH);
			
			dev_extFlash_read(DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*i+DAY_COMMON_ENERGY_OFFSET+DAY_COMMON_ENERGY_LENGTH*READ_DAY_DATA,(uint8_t *)&weekValue_common[i],DAY_COMMON_ENERGY_LENGTH);
			m_days = DaysDiff(RTC_time.ui32Year,RTC_time.ui32Month,RTC_time.ui32DayOfMonth,dayStr.year,dayStr.month,dayStr.date);
            
             if(0xFFFFFFFF == weekValue_sport[i])
			{
				weekValue_sport[i] = 0;
			}

			 if(0xFFFFFFFF == weekValue_common[i])
			{
				weekValue_common[i] = 0;
			}
           weekValue[i] = weekValue_sport[i] + weekValue_common[i];


		   
			if(m_days > 0 || m_days <= -7)
			{//һ��ǰ�����ݲ���ʾ����״ͼ��
				weekValue[i] = 0;
			}
			if(0xFFFFFFFF == weekValue[i])
			{
				weekValue[i] = 0;
			}
			//�����ͬ��,�ϲ���ֵ(�ϲ�0��Ч��ֵ)
			if(n == 6)
			{
				if(dayOfWeekValue[0] == 0)
				{
					dayOfWeekValue[0] += weekValue[i];
				}
				else
				{
					if(weekValue[i] != 0)
						dayOfWeekValue[0] = weekValue[i];
				}
				//COM_DATA_PRINTF("[com_data]:---SportsCalories---dayOfWeekValue[0]=%d,weekValue[%d]=%d!\r\n",dayOfWeekValue[0],i,weekValue[i]);
			}
			else
			{
				if(dayOfWeekValue[n+1] == 0)
				{
					dayOfWeekValue[n+1] += weekValue[i];
				}
				else
				{
					if(weekValue[i] != 0)
						dayOfWeekValue[n+1] = weekValue[i];
				}
				//COM_DATA_PRINTF("[com_data]:---SportsCalories---dayOfWeekValue[%d]=%d,weekValue[%d]=%d!\r\n",n+1,dayOfWeekValue[n+1],i,weekValue[i]);
			}
		}
	}
	m = CalculateWeekDay(2000 + RTC_time.ui32Year, RTC_time.ui32Month, RTC_time.ui32DayOfMonth);
/*	if(m == 6)
	{
		dayOfWeekValue[0] = DayMainData.SportEnergy;
	}
	else
	{
		dayOfWeekValue[m+1] = DayMainData.SportEnergy;
	}*/
	for(uint8_t i=0;i<7;i++)
	{
		if(0xFFFFFFFF == dayOfWeekValue[i])
		{
			dayOfWeekValue[i] = 0;
		}
	}
	dev_extFlash_disable();
}
/*��������:����ѵ���ƻ�һ���ж�����
����ֵ:����
*/
uint16_t Get_TrainPlanTotalDays(void)
{
	int16_t days = 0;
	
	am_hal_rtc_time_get(&RTC_time);

	days = DaysDiff(TrainPlanDownloadSettingStr.StartTime.Year,TrainPlanDownloadSettingStr.StartTime.Month,TrainPlanDownloadSettingStr.StartTime.Day
	                ,TrainPlanDownloadSettingStr.StopTime.Year,TrainPlanDownloadSettingStr.StopTime.Month,TrainPlanDownloadSettingStr.StopTime.Day);
	if(days <= 0)
		days = 0;
	return days;
}
/*��������:������ѵ���ƻ����ж�����
����ֵ:���� 0Ϊ���ھ��ǿ�ʼѵ���ƻ�ʱ��
*/
int16_t Get_TrainPlanBeforeDays(void)
{
	int16_t days = 0;
	
	am_hal_rtc_time_get(&RTC_time);

	days = DaysDiff(RTC_time.ui32Year,RTC_time.ui32Month,RTC_time.ui32DayOfMonth
								  ,TrainPlanDownloadSettingStr.StartTime.Year,TrainPlanDownloadSettingStr.StartTime.Month,TrainPlanDownloadSettingStr.StartTime.Day);
	if(days <= 0)
		days = 0;
	return days;
}
/*��������:����ʱ���Ƿ���Ч
����ֵ false ��Ч true ��Ч
*/
bool Is_ValidTimeTrainPlan(void)
{
	if((TrainPlanDownloadSettingStr.StartTime.Year < 0xFF) &&
				(TrainPlanDownloadSettingStr.StartTime.Month <= 12) &&
			  (TrainPlanDownloadSettingStr.StartTime.Day   <= 31))
	{
		return true;
	}
	else
	{
		return false;
	}
}

/*��������:��������Ƿ���ѵ���ƻ���
����ֵ false û�� true ��
*/
bool Is_TodayTimeTrainPlan(void)
{
	if(Get_SomeDayofTrainPlan() <= Get_TrainPlanTotalDays() && (Get_SomeDayofTrainPlan() > 0) && Is_ValidTimeTrainPlan())
	{
		return true;
	}
	else
	{
		return false;
	}
}
/*��������:�жϽ����Ƿ���ѵ���ƻ��еڼ���
����ֵ:1-40,��ʾ��1��~��40��,��Ϊ0����첻����ѵ���ƻ���
*/
uint8_t Get_SomeDayofTrainPlan(void)
{
	uint8_t theDays = 0;

	am_hal_rtc_time_get(&RTC_time);
	theDays = DaysDiff(TrainPlanDownloadSettingStr.StartTime.Year,TrainPlanDownloadSettingStr.StartTime.Month,TrainPlanDownloadSettingStr.StartTime.Day
	                   ,RTC_time.ui32Year,RTC_time.ui32Month,RTC_time.ui32DayOfMonth)+1;
	if(theDays <= 0)
		theDays = 0;
	return theDays;
}
/*��������:��ȡFlash ��ѵ���ƻ��·��ļ�¼�е�ĳ����ƻ����� (�����������)
           ÿ������ȡFlash��ѵ���ƻ����������ݻ�����ѵ���ƻ������ȡ�������˶������ȡ
���:day ѵ���ƻ��ĵڼ���(1~40)
ע:���뵽TrainPlanDetailStr��
*/
void Get_TrainPlanDayRecord(void)
{
	_com_config_train_plan_MD5_UUID  MD5_UUIDStr,MD5_UUIDResultStr;
	uint8_t day = 0;
	//���RAM�����е�ѵ���ƻ�����
	for(uint8_t i=0;i<2;i++)
		memset(&Upload[i],0,sizeof(UploadStr));
	for(uint8_t i=0;i<3;i++)
		memset(&Download[i],0,sizeof(DownloadStr));
	dev_extFlash_enable();
	dev_extFlash_read(TRAINING_DATA_START_ADDRESS,(uint8_t*)(&TrainPlanDownloadSettingStr),sizeof(TrainPlanDownStr));
	//���ϴ�ѵ���ƻ�������ȡMD5��UUID
	dev_extFlash_read(TRAINING_DATA_START_ADDRESS,(uint8_t*)(&MD5_UUIDStr),sizeof(_com_config_train_plan_MD5_UUID));
//	//д�뵽ѵ���ƻ����ͷ��
//	dev_extFlash_write(TRAINING_DATA_UPLOAD_ADDRESS,(uint8_t*)(&MD5_UUIDStr),sizeof(_com_config_train_plan_MD5_UUID));
	dev_extFlash_read(TRAINING_DATA_UPLOAD_ADDRESS,(uint8_t*)(&MD5_UUIDResultStr),sizeof(_com_config_train_plan_MD5_UUID));
	dev_extFlash_disable();
#if 0
	COM_DATA_PRINTF("MD5_UUIDDownloadStr:MD5:");
	for(uint16_t i=0;i<sizeof(MD5_UUIDStr.Md5Str);i++)
	{
		COM_DATA_PRINTF("0x%02X,",MD5_UUIDStr.Md5Str[i]);
	}
	COM_DATA_PRINTF("\n");
	
	COM_DATA_PRINTF("MD5_UUIDDownloadStr:UUID:");
	for(uint16_t i=0;i<sizeof(MD5_UUIDStr.UUIDStr);i++)
	{
		COM_DATA_PRINTF("0x%02X,",MD5_UUIDStr.UUIDStr[i]);
	}
	COM_DATA_PRINTF("\n");
	
	COM_DATA_PRINTF("MD5_UUIDResultStr:MD5:");
	for(uint16_t i=0;i<sizeof(MD5_UUIDResultStr.Md5Str);i++)
	{
		COM_DATA_PRINTF("0x%02X,",MD5_UUIDResultStr.Md5Str[i]);
	}
	COM_DATA_PRINTF("\n");
	
	COM_DATA_PRINTF("MD5_UUIDResultStr:UUID:");
	for(uint16_t i=0;i<sizeof(MD5_UUIDResultStr.UUIDStr);i++)
	{
		COM_DATA_PRINTF("0x%02X,",MD5_UUIDResultStr.UUIDStr[i]);
	}
	COM_DATA_PRINTF("\n");
#endif
	COM_DATA_PRINTF("Get_TrainPlanDayRecord(): MD5:%X,UUID:%s\n",MD5_UUIDStr.Md5Str,MD5_UUIDStr.UUIDStr);
	COM_DATA_PRINTF("MD5_UUIDResultStr: MD5:%s,UUID:%s\n",MD5_UUIDResultStr.Md5Str,MD5_UUIDResultStr.UUIDStr);

	if(get_trainplan_valid_flag() && Is_TodayTimeTrainPlan())
	{//ѵ���ƻ������ҽ��촦��ѵ���ƻ���
		day = Get_SomeDayofTrainPlan();
		dev_extFlash_enable();

		if(day == 1)//��һ��ʱ��ǰ�������Ϊ�ջ��M��ʱ����������Ϊ��
		{
			dev_extFlash_read(TRAINING_DATA_START_ADDRESS+sizeof(TrainPlanDownStr),
											 (uint8_t*)(&Download[1]),sizeof(DownloadStr));
			dev_extFlash_read(TRAINING_DATA_START_ADDRESS+sizeof(TrainPlanDownStr)+sizeof(DownloadStr)*day,
											 (uint8_t*)(&Download[2]),sizeof(DownloadStr));
			memset(&Download[0],0,sizeof(DownloadStr));
		}
		else if(day == Get_TrainPlanTotalDays())
		{
			dev_extFlash_read(TRAINING_DATA_START_ADDRESS+sizeof(TrainPlanDownStr)+sizeof(DownloadStr)*(day-2),
											 (uint8_t*)(&Download[0]),sizeof(DownloadStr));
			dev_extFlash_read(TRAINING_DATA_START_ADDRESS+sizeof(TrainPlanDownStr)+sizeof(DownloadStr)*(day-1),
											 (uint8_t*)(&Download[1]),sizeof(DownloadStr));
			memset(&Download[2],0,sizeof(DownloadStr));
		}
		else
		{
			for(uint8_t i=0;i<3;i++)
			{
				dev_extFlash_read(TRAINING_DATA_START_ADDRESS+sizeof(TrainPlanDownStr)+sizeof(DownloadStr)*(day-2+i),
											 (uint8_t*)(&Download[i]),sizeof(DownloadStr));
			}
		}
		
		dev_extFlash_read(TRAINING_DATA_UPLOAD_ADDRESS+sizeof(TrainPlanUploadStr)+sizeof(UploadStr)*(day-1),(uint8_t*)(&Upload[1]),sizeof(UploadStr));
		if(Upload[1].IsComplete != 1)
		{//��֤FlashΪ������ʱ�޷������˶����
			Upload[1].IsComplete = 0;
		}
		COM_DATA_PRINTF("[com_data]:------Is_TodayTimeTrainPlan=%d,Get_TrainPlanTotalDays=%d,Get_SomeDayofTrainPlan=%d,Get_TrainPlanBeforeDays=%d,Upload[1].IsComplete=%d\n"
	                ,Is_TodayTimeTrainPlan(),Get_TrainPlanTotalDays(),Get_SomeDayofTrainPlan(),Get_TrainPlanBeforeDays(),Upload[1].IsComplete);
		dev_extFlash_disable();

		if(Download[1].Type == Rest && Get_IsCompleteTodayTrainPlan() != 1)
		{//�����������Ϣ,Ĭ�ϱ���
			Set_IsCompleteTodayTrainPlan(1);
			Upload[1].Calories = 0;
			Upload[1].Distance = 0;
			Upload[1].Hour = 0;
			Upload[1].RecordIndex = 0;
			dev_extFlash_enable();
			dev_extFlash_write(TRAINING_DATA_UPLOAD_ADDRESS+sizeof(TrainPlanUploadStr)+sizeof(UploadStr)*(Get_SomeDayofTrainPlan()-1),(uint8_t*)(&Upload[1]),sizeof(UploadStr));
			dev_extFlash_disable();
		}
	}

}
/*��������:�ϴ���ѵ���ƻ���������ݴ洢
���:���ڼ��ɣ���
*/
void Store_TranPlanUploadData(void)
{
	if(Get_IsCompleteTodayTrainPlan() == 1)
	{//ѵ���ƻ���ɲű�������
		uint8_t get_days = Get_SomeDayofTrainPlan();//�������

		if(Get_TrainPlanType(Download[1].Type) == 1)
		{
			//����Ϣ���߽���ѵ����ѵ���ƻ����ϴ����ʱ���Ƿ�����ɰ�������
			Upload[1].Calories = 0;
			Upload[1].Distance = 0;
			Upload[1].Hour = 0;
			Upload[1].RecordIndex = 0;
		}
		COM_DATA_PRINTF("---GetUploadData:Upload[1].IsComplete=%d,Upload[1].Hour=%d,get_days=%d\n",Upload[1].IsComplete,Upload[1].Hour,get_days);
		//ȷ����ѵ���ƻ�����һ��
		dev_extFlash_enable();
		dev_extFlash_write(TRAINING_DATA_UPLOAD_ADDRESS+sizeof(TrainPlanUploadStr)+sizeof(UploadStr)*(get_days-1),(uint8_t*)(&Upload[1]),sizeof(UploadStr));
		dev_extFlash_disable();
	}
	else
	{
		COM_DATA_PRINTF("---**Get_IsCompleteTodayTrainPlan()=%d\n",Get_IsCompleteTodayTrainPlan());
	}
#if 0
	//���� �鿴ѵ���ƻ�������
	dev_extFlash_enable();
	dev_extFlash_read(TRAINING_DATA_UPLOAD_ADDRESS+sizeof(TrainPlanUploadStr)+sizeof(UploadStr)*(Get_SomeDayofTrainPlan()-1),(uint8_t*)(&Upload[0]),sizeof(UploadStr));
	dev_extFlash_disable();
	COM_DATA_PRINTF("---GetUploadData:Upload[0].IsComplete=%d,Upload[0].Hour=%d,Upload[0].Distance=%d,Upload[0].Calories=%d,Upload[0].Reserved=%d,address=0x%x\n",
	              Upload[0].IsComplete,Upload[0].Hour,Upload[0].Distance,Upload[0].Calories,Upload[0].Reserved,
	              TRAINING_DATA_UPLOAD_ADDRESS+sizeof(TrainPlanUploadStr)+sizeof(UploadStr)*(Get_SomeDayofTrainPlan()-1));
#endif
}
/*��������:��FLASH�л�ȡ���ϴ���ѵ���ƻ�����  ���������
���:���ϴ����ݵ�ĳ��,�ڼ��� 1-40,���ǽ��죬����Get_SomeDayofTrainPlan()
*/
void Get_TranPlanUploadData(uint8_t days)
{
	if(days <= 0) return;
	
	dev_extFlash_enable();
	dev_extFlash_read(TRAINING_DATA_UPLOAD_ADDRESS+sizeof(TrainPlanUploadStr)+sizeof(UploadStr)*(days-1),(uint8_t*)(&Upload[0]),sizeof(UploadStr));
	dev_extFlash_disable();
}

/*��������:ѵ���ƻ���������ϴ� ��BLE��*/
TrainPlanUploadBLEData Get_TrainPlanResult(uint8_t year,uint8_t month,uint8_t day)
{
	TrainPlanUploadBLEData UploadBleDataStr;
	int16_t theDays = 0,toDays = 0;
	
	am_hal_rtc_time_get(&RTC_time);
	dev_extFlash_enable();
	dev_extFlash_read(TRAINING_DATA_START_ADDRESS,(uint8_t*)(&TrainPlanDownloadSettingStr),sizeof(TrainPlanDownStr));
	dev_extFlash_disable();
	theDays = DaysDiff(year,month,day,RTC_time.ui32Year,RTC_time.ui32Month,RTC_time.ui32DayOfMonth);//app�·����ڵ���������
	toDays = DaysDiff(TrainPlanDownloadSettingStr.StartTime.Year,TrainPlanDownloadSettingStr.StartTime.Month,TrainPlanDownloadSettingStr.StartTime.Day,year,month,day);//app�·�������ѵ���ƻ���ʼʱ���

	if(theDays >= 0 && toDays >= 0)
	{
		UploadBleDataStr.Is_HasTodayResult = 1;
		UploadBleDataStr.Address = (toDays) * sizeof(UploadStr) + sizeof(TrainPlanUploadStr)+ TRAINING_DATA_UPLOAD_ADDRESS;
		UploadBleDataStr.Size = (theDays+1) * sizeof(UploadStr);
		UploadBleDataStr.Days = (theDays+1);
		COM_DATA_PRINTF("[com_data]:Get_TrainPlanResult()-->UploadBleDataStr.Is_HasTodayResult=%d,UploadBleDataStr.Address=0x%x,UploadBleDataStr.Size=%d,UploadBleDataStr.Days=%d\n"
		               ,UploadBleDataStr.Is_HasTodayResult,UploadBleDataStr.Address,UploadBleDataStr.Size,UploadBleDataStr.Days);
	}
	else
	{
		UploadBleDataStr.Is_HasTodayResult = 0;
		UploadBleDataStr.Address = 0;
		UploadBleDataStr.Size = 0;
		UploadBleDataStr.Days = 0;
	}
	
	return UploadBleDataStr;
}
/*����˵��:��ȡAPOLLOѵ���ƻ��洢�������еĵ�ַ�������ݴ�С*/
uint32_t Get_TrainPlanDownloadDataSize(void)
{
	return (sizeof(TrainPlanDownStr) + sizeof(DownloadStr) * Get_TrainPlanTotalDays());
}
/*��������:��ѵ���ƻ�
 ��״̬ ��ȡFlash��ѵ���ƻ�����ʱ���
*/
void IsOpen_TrainPlan_Settings(bool flag)
{
	if(flag)
	{
		SetValue.TrainPlanFlag = 1;
		dev_extFlash_enable();
		dev_extFlash_read(TRAINING_DATA_START_ADDRESS,(uint8_t*)(&TrainPlanDownloadSettingStr),sizeof(TrainPlanDownStr));
		dev_extFlash_disable();
	}
	else
	{
		SetValue.TrainPlanFlag = 0;
	}
	//���RAM�����е�ѵ���ƻ�����
//	for(uint8_t i=0;i<2;i++)
//		memset(&Upload[i],0,sizeof(UploadStr));
//	for(uint8_t i=0;i<3;i++)
//		memset(&Download[i],0,sizeof(DownloadStr));
	SetValue.TrainPlanHintFlag = 0;
//	Store_SetData();
}
/*����˵��:����ѵ���ƻ�֪ͨ���ѿ��� 1Ϊ�� 0Ϊ��*/
void Set_TrainPlan_Notify_IsOpen(bool flag)
{
	SetValue.TrainPlanNotifyFlag = flag;
}
/*����˵��:��ȡѵ���ƻ�֪ͨ���ѿ���״̬ 1Ϊ�� 0Ϊ��*/
bool Get_TrainPlan_Notify_IsOpen(void)
{
	return SetValue.TrainPlanNotifyFlag;
}
/*����˵��:��FLASH�л�ȡMD5��UUID  --��������*/
void Test_Get_Flash_MD5_UUID(void)
{
	TrainPlanUploadStr MD5_UUIDStr;
	
	//���ϴ�ѵ���ƻ�������ȡMD5��UUID
	dev_extFlash_enable();
	dev_extFlash_read(TRAINING_DATA_UPLOAD_ADDRESS,(uint8_t*)(&MD5_UUIDStr),sizeof(TrainPlanUploadStr));
	dev_extFlash_disable();
	
}

/*��һ���ϵ�*/
void First_Run_Config(void)
{
	bsp_rtc_init();
	//���ݽṹ������ʼ������
	SetValue.crc = FLASH_USED_FLAG;
	
	SetValue.AppSet.Age = 25;
	SetValue.AppSet.Sex = 1;
	SetValue.AppSet.Height = 170;
	SetValue.AppSet.Weight = 60;
	SetValue.AppSet.Stride = 77;
	SetValue.AppSet.Goal_Steps = 6000;
	#ifdef COD
	SetValue.AppSet.Goal_Energy = 300;
	#else 
	SetValue.AppSet.Goal_Energy = 1500;
	#endif
	SetValue.AppSet.Goal_Distance = 5;//�ݶ�
	SetValue.AppSet.VO2Max = 50;
	SetValue.AppSet.LactateThresholdHeartrate = 165;
	SetValue.AppSet.LactateThresholdSpeed = 255;//4:15����s
	SetValue.AppSet.htr_zone_one_min = 94;
	SetValue.AppSet.htr_zone_one_max = 112;
	SetValue.AppSet.htr_zone_two_min = 113;
	SetValue.AppSet.htr_zone_two_max = 131;
	SetValue.AppSet.htr_zone_three_min = 132;
	SetValue.AppSet.htr_zone_three_max = 150;
	SetValue.AppSet.htr_zone_four_min = 151;
	SetValue.AppSet.htr_zone_four_max = 169;
	SetValue.AppSet.htr_zone_five_min = 170;
	SetValue.AppSet.htr_zone_five_max = 188;
  SetValue.AppSet.Sea_Level_Pressure = 101325;//����
    
  //App��ȡ���ݸ�ʽ
	SetValue.AppGet.LengthSwimmingPool = 50;//��
	SetValue.AppGet.StepLengthRun = 102;//cm
	SetValue.AppGet.StepLengthCountryRace = 102;
	SetValue.AppGet.StepLengthMarathon = 102;
	SetValue.AppGet.StepLengthWalking = 77;
	SetValue.AppGet.StepLengthClimbing = 77;
	SetValue.AppGet.StepLengthIndoorRun = 102;
	SetValue.AppGet.StepLengthHike = 77;
	SetValue.AppGet.VO2MaxMeasuring = 50;//����
	SetValue.AppGet.LTBmpMeasuring = 0;
	SetValue.AppGet.LTSpeedMeasuring = 0;
	SetValue.AppGet.CalibrationStride = 0;//Ĭ�ϲ���
	memset(	SetValue.AppGet.Reserved,0,sizeof(SetValue.AppGet.Reserved));
    
	//�˶�������������
	SetValue.SportSet.SwRun = 0x08;
	SetValue.SportSet.SwMarathon = 0x08;
	SetValue.SportSet.SwCountryRace = 0x08;
	SetValue.SportSet.SwSwimmg = 0x08;
	SetValue.SportSet.SwCycling = 0x08;
	SetValue.SportSet.SwClimbing = 0x08;
	SetValue.SportSet.SwWalking = 0x08;
	SetValue.SportSet.SwHike = 0x08;
	SetValue.SportSet.SwIndoorRun = 0x08;
	SetValue.SportSet.SwOutdoorSwimmg = 0x08;
	SetValue.SportSet.DefHintSpeedRun = 360;//s/km
	SetValue.SportSet.DefHintSpeedCountryRace = 360;
	SetValue.SportSet.DefHintSpeedMarathon = 360;
	SetValue.SportSet.DefHintSpeedCycling = 150;//15km/h
	SetValue.SportSet.DefHintSpeedIndoorRun = 360;//�ݶ�
	SetValue.SportSet.DefHintDistanceRun = 1000;
	SetValue.SportSet.DefHintDistanceCountryRace = 1000;
	SetValue.SportSet.DefHintDistanceMarathon = 1000;
	SetValue.SportSet.DefHintDistanceCycling = 20000;
	SetValue.SportSet.DefHintDistanceClimbing = 1000;
	SetValue.SportSet.DefHintDistanceIndoorRun = 1000;//�ݶ�
	SetValue.SportSet.DefCircleDistanceRun = 1000;
	SetValue.SportSet.DefCircleDistanceWalk = 1000;
	SetValue.SportSet.DefCircleDistanceCountryRace = 1000;
	SetValue.SportSet.DefCircleDistanceMarathon = 1000;
	SetValue.SportSet.DefCircleDistanceCycling = 5000;
	SetValue.SportSet.DefCircleDistanceIndoorRun = 1000;//�ݶ�
	SetValue.SportSet.DefCircleDistanceClimbing = 1000;
	SetValue.SportSet.DefCircleDistanceCountryRace = 1000;
	SetValue.SportSet.DefCircleDistancOutdoorSwimmg = 1000;//�ݶ�
	SetValue.SportSet.DefCircleDistanceHike = 1000;
	SetValue.SportSet.DefHintTimeSwimming = 1800;
	SetValue.SportSet.DefHintTimeWalking = 1800;
	SetValue.SportSet.DefHintTimeOutdoorSwimmg = 0;
	SetValue.SportSet.DefWalkingFatBurnGoal = 100;
	SetValue.SportSet.DefHikeFatBurnGoal = 100;
	SetValue.SportSet.DefHintClimbingHeight = 2500;//��ɽ�߶�����Ĭ��ֵ
	SetValue.SportSet.WheelDiameter = 275;// 27.5��

   /*����*/
   
   SetValue.SportSet.DefHintTimeRun = 1800;
   SetValue.SportSet.DefHintTimeClimbing = 1800;
   SetValue.SportSet.DefHintTimeCycling = 1800;
   SetValue.SportSet.DefHintTimeHike = 1800;
   SetValue.SportSet.DefHintTimeCountryRace = 1800;

  SetValue.SportSet.DefHintSpeedHike= 360;


  SetValue.SportSet.DefCircleDistancSwimmg = 100;	//��ӾĬ�ϼ�Ȧ����
  SetValue.SportSet.DefHintDistanceHike = 1000;    //ͽ��ԽҰ��������Ĭ��ֵ
  SetValue.SportSet.DefHintDistanceSwimmg = 1000;	   //��Ӿ��������Ĭ��ֵֵ



  
	SetValue.SportSet.DefHintCircleSwimming = 10;
	SetValue.SportSet.Reserved = 0;
	
	SetValue.CurrentAddress.Index = 0;
	SetValue.CurrentAddress.IndexActivity = 0;
	SetValue.CurrentAddress.AddressStep = STEP_DATA_START_ADDRESS;
	SetValue.CurrentAddress.AddressHeartRate = HEARTRATE_DATA_START_ADDRESS;
	SetValue.CurrentAddress.AddressPressure = PRESSURE_DATA_START_ADDRESS;
	SetValue.CurrentAddress.AddressDistance = DISTANCE_DATA_START_ADDRESS;
	SetValue.CurrentAddress.AddressEnergy = ENERGY_DATA_START_ADDRESS;
	SetValue.CurrentAddress.AddressSpeed = SPEED_DATA_START_ADDRESS;
	SetValue.CurrentAddress.AddressGps = GPS_DATA_START_ADDRESS;
	SetValue.CurrentAddress.AddressDistancePoint = DISTANCEPOINT_START_ADDRESS;
	SetValue.CurrentAddress.AddressCircleTime = CIRCLETIME_START_ADDRESS;
	SetValue.CurrentAddress.AddressPauseTime = PAUSE_DATA_START_ADDRESS;
	SetValue.CurrentAddress.AddressGpsTrack = GPSTRACK_DATA_START_ADDRESS;
	SetValue.CurrentAddress.AddressStatelite = SATELLITE_DATA_START_ADDRESS;
	SetValue.CurrentAddress.AddressTrainplan = TRAINING_DATA_START_ADDRESS;
	SetValue.CurrentAddress.AddressTrainplanUpload = TRAINING_DATA_UPLOAD_ADDRESS;
	#ifdef COD 
	SetValue.CurrentAddress.Current_cod_id= 0;
	
	#else
	SetValue.CurrentAddress.AddressVerticalSize = VERTICALSIZE_START_ADDRESS;
	#endif
	SetValue.CurrentAddress.AddressTouchDown = TOUCHDOWN_START_ADDRESS;
	SetValue.CurrentAddress.AddressPause = PAUSE_DATA_START_ADDRESS;//ʵ�����ڴ洢�����������
	SetValue.CurrentAddress.Screen_Sta = DISPLAY_SCREEN_LOGO;
	SetValue.CurrentAddress.PowerOffHour = 0;
	SetValue.CurrentAddress.ChangedHour = 0xFF;
	SetValue.CurrentAddress.Reserved = 0;
	
	SetValue.ShortcutScreenUP = DISPLAY_SCREEN_SPORT;
	SetValue.ShortcutScreenDown = DISPLAY_SCREEN_TOOL;
	SetValue.ShortcutScreenLight = DISPLAY_SCREEN_LOGO;
	
	SetValue.Language = L_CHINESE;
	memset(SetValue.alarm_instance,0,sizeof(SetValue.alarm_instance));
	SetValue.alarm_instance[0].buzzer_enable = true;
	SetValue.alarm_instance[1].buzzer_enable = true;
	SetValue.alarm_instance[2].buzzer_enable = true;
	SetValue.alarm_instance[3].buzzer_enable = true;
	SetValue.alarm_instance[0].motor_enable = true;
	SetValue.alarm_instance[1].motor_enable = true;
	SetValue.alarm_instance[2].motor_enable = true;
	SetValue.alarm_instance[3].motor_enable = true;
    SetValue.alarm_instance[0].time.hour = 0;
	SetValue.alarm_instance[1].time.hour = 8;
	SetValue.alarm_instance[2].time.hour = 12;
	SetValue.alarm_instance[3].time.hour = 18;
	
	SetValue.backlight_timeout_mssec = 7000;

	SetValue.SwVibration = 0;
	SetValue.SwBuzzer = 1;
	SetValue.SwRealtimeHdr = 1;
	SetValue.SwAutoLockScreen = 1;
	SetValue.SwBle = BLE_DISCONNECT;//������
	SetValue.SwRaiseForLightScreen = 1;
	SetValue.SwNoDisturb = 0;
	SetValue.NoDisturbStartHour = 22;
	SetValue.NoDisturbStartMinute = 0;
	SetValue.NoDisturbEndHour = 7;
	SetValue.NoDisturbEndMinute = 0;
	SetValue.SwBasedMonitorHdr = 0;
	SetValue.DefBasedMonitorHdrHour = 7;
	SetValue.DefBasedMonitorHdrMinute = 0;
	SetValue.Theme = THEME_SIMPLENUMBER;//��Լ����
	SetValue.Compass_Offset_x = 0;
	SetValue.Compass_Offset_y = 0;
	SetValue.Compass_Offset_z = 0;
	SetValue.Altitude_Offset = 0;
	SetValue.U_CustomTool.val = 0xFF;
	SetValue.U_CustomSport.val = DEFAULT_SPORT_MODE;
	SetValue.U_CustomMonitor.val = 0x3F;
	SetValue.IsAllowBaseHDR = 1;//��������������ѱ��湦��

	memset(SetValue.TrainRecordIndex,0,sizeof(SetValue.TrainRecordIndex));
	memset(&SetValue.ToolRecordNumIndex,0,sizeof(SetValue.ToolRecordNumIndex));
	memset(&SetValue.SportRecordNumIndex,0,sizeof(SetValue.SportRecordNumIndex));
	SetValue.AutoLockScreenFlag = 0;
	SetValue.PowerOffMianData.CommonSteps = 0;
	SetValue.PowerOffMianData.CommonEnergy = 0;
	SetValue.PowerOffMianData.CommonDistance = 0;
	SetValue.PowerOffMianData.SportSteps = 0;
	SetValue.PowerOffMianData.SportEnergy = 0;
	SetValue.PowerOffMianData.SportDistance = 0;
	SetValue.PowerOffMianData.BasedHeartrate = 0;
	SetValue.PowerOffMianData.VO2MaxMeasuring = 0;
	SetValue.PowerOffMianData.LTBmpMeasuring = 0;
	SetValue.PowerOffMianData.LTSpeedMeasuring = 0;
	SetValue.TimeCalibrationValue = 0;
	SetValue.RecoverTime = 0;//�ָ�ʱ��
	SetValue.RecoverTimeTotal = 0;
	SetValue.TrainPlanWriteUploadMD5Flag = 0;
	SetValue.GPS_Gauss_Type = 3;	//3�ȴ�
	
	SetValue.IsFirstCalibrationStep = 1;//��δУ׼������ ��һ��
	SetValue.IsFirstSport = 1;//û���˶���¼ ��һ��
	//Ĭ�ϲ��������������˶���
	SetValue.TriathlonSport.IsTriathlonSport = 0;
	SetValue.TriathlonSport.IndexTriathlon = 0;
	SetValue.TriathlonSport.year = 0;
	SetValue.TriathlonSport.month = 0;
	SetValue.TriathlonSport.day = 0;
	SetValue.TriathlonSport.hour = 0;
	SetValue.TriathlonSport.minute = 0;
	SetValue.TriathlonSport.second = 0;
	//ѵ���ƻ�Ĭ�Ϲر�
	SetValue.TrainPlanFlag = 0;
	SetValue.TrainPlanHintFlag = 0;
	SetValue.IsNeedClearTrainPlanDayHint = 0;
	//ѵ���ƻ�����
	memset(&TrainPlanDownloadSettingStr,0,sizeof(TrainPlanDownStr));
	//�·���/�ϴ���ѵ���ƻ�������ʼ��
	for(uint8_t i=0;i<2;i++)
		memset(&Upload[i],0,sizeof(UploadStr));
	for(uint8_t i=0;i<3;i++)
		memset(&Download[i],0,sizeof(DownloadStr));
	
	SetValue.IsMeasuredVO2max = 0;
	SetValue.IsMeasuredLthreshold = 0;
	SetValue.custom_shortcut_up_switch_val = 0x3FF;
	SetValue.RunData1 = SPORT_DATA_DISPALY_REALTIME_PACE;
	SetValue.RunData2 = SPORT_DATA_DISPALY_TOTAL_TIME;
	SetValue.RunData3 = SPORT_DATA_DISPALY_REALTIME_FREQUENCY;
	SetValue.RunData4 = SPORT_DATA_DISPALY_TOTAL_DISTANCE;
	
	SetValue.WlakData1 = SPORT_DATA_DISPALY_TOTAL_DISTANCE;
	SetValue.WlakData2 = SPORT_DATA_DISPALY_REALTIME_PACE;
	SetValue.WlakData3 = SPORT_DATA_DISPALY_TOTAL_TIME;
	
	SetValue.MarathonData1 = SPORT_DATA_DISPALY_TOTAL_DISTANCE;
	SetValue.MarathonData2 = SPORT_DATA_DISPALY_REALTIME_PACE;
	SetValue.MarathonData3 = SPORT_DATA_DISPALY_TOTAL_TIME;
	
	SetValue.IndoorRunData1 = SPORT_DATA_DISPALY_TOTAL_DISTANCE;
	SetValue.IndoorRunData2 = SPORT_DATA_DISPALY_REALTIME_PACE;
	SetValue.IndoorRunData3 = SPORT_DATA_DISPALY_TOTAL_TIME;

	SetValue.SwimRunData1 = SPORT_DATA_DISPALY_PREVIOUS_SWOLF;
	SetValue.SwimRunData2 = SPORT_DATA_DISPALY_TOTAL_TIME;
	SetValue.SwimRunData3 = SPORT_DATA_DISPALY_SWIM_LAPS;
    SetValue.SwimRunData4 = SPORT_DATA_DISPALY_TOTAL_DISTANCE;


	
	SetValue.CyclingData1 = SPORT_DATA_DISPALY_REALTIME_SPEED;
	SetValue.CyclingData2 = SPORT_DATA_DISPALY_TOTAL_TIME;
	SetValue.CyclingData3 = SPORT_DATA_DISPALY_TOTAL_CLIMB;
    SetValue.CyclingData4 = SPORT_DATA_DISPALY_TOTAL_DISTANCE;

    SetValue.ClimbingData1 = SPORT_DATA_DISPALY_ALTITUDE;
	SetValue.ClimbingData2 = SPORT_DATA_DISPALY_TOTAL_TIME;
	SetValue.ClimbingData3 = SPORT_DATA_DISPALY_VER_AVE_SPEED;
    SetValue.ClimbingData4 = SPORT_DATA_DISPALY_TOTAL_DISTANCE;


    SetValue.HikeData1 = SPORT_DATA_DISPALY_REALTIME_PACE;
	SetValue.HikeData2 = SPORT_DATA_DISPALY_TOTAL_TIME;
	SetValue.HikeData3 = SPORT_DATA_DISPALY_TOTAL_CLIMB;
    SetValue.HikeData4 = SPORT_DATA_DISPALY_TOTAL_DISTANCE;

    SetValue.CrosscountryData1 = SPORT_DATA_DISPALY_REALTIME_PACE;
	SetValue.CrosscountryData2 = SPORT_DATA_DISPALY_TOTAL_TIME;
	SetValue.CrosscountryData3 = SPORT_DATA_DISPALY_TOTAL_CLIMB;
    SetValue.CrosscountryData4 = SPORT_DATA_DISPALY_TOTAL_DISTANCE;

	
	SetValue.IndoorLengSet = 0;
	SetValue.HeartRateSet = 0;
	

#if defined	(WATCH_COM_SPORT)
	SetValue.IndoorLengSet = 0;
#endif 

	SetValue.AutoLightStartHour = 17; 		//̧��������ʼСʱ
	SetValue.AutoLightStartMinute=0;		//̧��������ʼ����
	SetValue.AutoLightEndHour = 7;			//̧����������Сʱ
	SetValue.AutoLightEndMinute = 0; 		//̧��������������

	SetValue.HRRecoverySet = 1;  //���ʻָ������ÿ���

#if defined WATCH_TIMEZONE_SET
	SetValue.TimeZoneRecord = TIMEZONE_BJS;  
#endif
#ifdef COD 
	SetValue.agps_update_time.Year = 18;
	SetValue.agps_update_time.Month= 1;
	SetValue.agps_update_time.Day= 1;
	SetValue.agps_update_time.Hour= 0;
	SetValue.agps_update_time.Minute= 0;
	SetValue.agps_update_time.Second= 0;
	SetValue.phone_call_delay = 3;
	SetValue.rt_today_summary = 0;
	SetValue.rt_heart = 0;
	SetValue.run_step = 0;
	SetValue.walk_step = 0;
	SetValue.all_day_step_address = 0;
	SetValue.rest_heart = 70;

      #ifdef WATCH_SONY_GNSS
        SetValue.agps_effective_time.Year = 18;
	SetValue.agps_effective_time.Month= 1;
	SetValue.agps_effective_time.Day= 1;
	SetValue.agps_effective_time.Hour= 0;
	SetValue.agps_effective_time.Minute= 0;
	SetValue.agps_effective_time.Second= 0;
        SetValue.agps_update_flag = 0;
      #endif
	
#endif
	//SetValue.crc = am_bootloader_fast_crc32((uint8_t*)(&SetValue.crc)+sizeof(SetValue.crc), sizeof(SetValueStr)-sizeof(SetValue.crc));
	COM_DATA_PRINTF("[com_data]:SetValue.crc --> 0x%08X\n",SetValue.crc);
	dev_extFlash_enable();
	dev_extFlash_erase(SET_VALUE_ADDRESS, sizeof(SetValueStr));
	dev_extFlash_write(SET_VALUE_ADDRESS, (uint8_t*)(&SetValue), sizeof(SetValueStr));
	dev_extFlash_disable();
	
	am_hal_rtc_time_get(&RTC_time);
	
	memset(&DayMainData,0,sizeof(DayMainData));
	DayMainData.Year = RTC_time.ui32Year;
	DayMainData.Month = RTC_time.ui32Month;
	DayMainData.Date = RTC_time.ui32DayOfMonth;
	DayMainData.Index = RTC_time.ui32Weekday;
	
	dev_extFlash_enable();
	dev_extFlash_write(DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*DayMainData.Index,(uint8_t*)(&DayMainData),4);
	dev_extFlash_disable();
	
	//�����ѹ���߶�����
	AmbientData_Init();
}

//�䶯���ݲ�������
void modify_data_config(void)
{
	
	if (SetValue.IsMeasuredVO2max == 0xFF)
	{
		SetValue.IsMeasuredVO2max = 0;
		SetValue.IsMeasuredLthreshold = 0;
		SetValue.custom_shortcut_up_switch_val = 0x3FF;
	}
	
	if (SetValue.RunData1 == 0xFF)
	{
	SetValue.RunData1 = SPORT_DATA_DISPALY_REALTIME_PACE;
	SetValue.RunData2 = SPORT_DATA_DISPALY_TOTAL_TIME;
	SetValue.RunData3 = SPORT_DATA_DISPALY_REALTIME_FREQUENCY;
	SetValue.RunData4 = SPORT_DATA_DISPALY_TOTAL_DISTANCE;
	
	SetValue.WlakData1 = SPORT_DATA_DISPALY_TOTAL_DISTANCE;
	SetValue.WlakData2 = SPORT_DATA_DISPALY_REALTIME_PACE;
	SetValue.WlakData3 = SPORT_DATA_DISPALY_TOTAL_TIME;
	
	SetValue.MarathonData1 = SPORT_DATA_DISPALY_TOTAL_DISTANCE;
	SetValue.MarathonData2 = SPORT_DATA_DISPALY_REALTIME_PACE;
	SetValue.MarathonData3 = SPORT_DATA_DISPALY_TOTAL_TIME;
	
	SetValue.IndoorRunData1 = SPORT_DATA_DISPALY_TOTAL_DISTANCE;
	SetValue.IndoorRunData2 = SPORT_DATA_DISPALY_REALTIME_PACE;
	SetValue.IndoorRunData3 = SPORT_DATA_DISPALY_TOTAL_TIME;

	SetValue.SwimRunData1 = SPORT_DATA_DISPALY_PREVIOUS_SWOLF;
	SetValue.SwimRunData2 = SPORT_DATA_DISPALY_TOTAL_TIME;
	SetValue.SwimRunData3 = SPORT_DATA_DISPALY_SWIM_LAPS;
    SetValue.SwimRunData4 = SPORT_DATA_DISPALY_TOTAL_DISTANCE;


	
	SetValue.CyclingData1 = SPORT_DATA_DISPALY_REALTIME_SPEED;
	SetValue.CyclingData2 = SPORT_DATA_DISPALY_TOTAL_TIME;
	SetValue.CyclingData3 = SPORT_DATA_DISPALY_TOTAL_CLIMB;
    SetValue.CyclingData4 = SPORT_DATA_DISPALY_TOTAL_DISTANCE;

    SetValue.ClimbingData1 = SPORT_DATA_DISPALY_ALTITUDE;
	SetValue.ClimbingData2 = SPORT_DATA_DISPALY_TOTAL_TIME;
	SetValue.ClimbingData3 = SPORT_DATA_DISPALY_VER_AVE_SPEED;
    SetValue.ClimbingData4 = SPORT_DATA_DISPALY_TOTAL_DISTANCE;


    SetValue.HikeData1 = SPORT_DATA_DISPALY_REALTIME_PACE;
	SetValue.HikeData2 = SPORT_DATA_DISPALY_TOTAL_TIME;
	SetValue.HikeData3 = SPORT_DATA_DISPALY_TOTAL_CLIMB;
    SetValue.HikeData4 = SPORT_DATA_DISPALY_TOTAL_DISTANCE;

    SetValue.CrosscountryData1 = SPORT_DATA_DISPALY_REALTIME_PACE;
	SetValue.CrosscountryData2 = SPORT_DATA_DISPALY_TOTAL_TIME;
	SetValue.CrosscountryData3 = SPORT_DATA_DISPALY_TOTAL_CLIMB;
    SetValue.CrosscountryData4 = SPORT_DATA_DISPALY_TOTAL_DISTANCE;

	
	SetValue.IndoorLengSet = 0;
	SetValue.HeartRateSet = 0;
	}
#if defined	(WATCH_COM_SPORT)
	if (SetValue.IndoorLengSet == 0xFF)
	{
		SetValue.IndoorLengSet = 0;
	}
#endif 
    #ifdef COD 
	if(SetValue.AppSet.Goal_Energy == 1500)
   	{
		SetValue.AppSet.Goal_Energy = 300;
    }
	if((SetValue.CurrentAddress.AddressDistancePoint < DISTANCEPOINT_START_ADDRESS)|| (SetValue.CurrentAddress.AddressDistancePoint >  DISTANCEPOINT_STOP_ADDRESS))
	{
		SetValue.CurrentAddress.AddressDistancePoint = DISTANCEPOINT_START_ADDRESS;
	}
	#else 
   if(SetValue.AppSet.Goal_Energy == 300)
   	{
		SetValue.AppSet.Goal_Energy = 1500;
    }
 #endif
   #if defined WATCH_TIMEZONE_SET
   if (SetValue.TimeZoneRecord== 0xFFFF)
   {
		SetValue.TimeZoneRecord = TIMEZONE_BJS;
   }
   #endif

  #ifdef WATCH_SONY_GNSS
    if(SetValue.agps_effective_time.Second == 0xFF)
    {
        SetValue.agps_effective_time.Year = 18;
	SetValue.agps_effective_time.Month= 1;
	SetValue.agps_effective_time.Day= 1;
	SetValue.agps_effective_time.Hour= 0;
	SetValue.agps_effective_time.Minute= 0;
	SetValue.agps_effective_time.Second= 0;
    }
    if(SetValue.agps_update_flag == 0xFF)
     SetValue.agps_update_flag = 0;
  #endif
   
}
//����ݲ���
void com_sport_data_del(void)
{	
		//ɾ���˶����ݣ������ճ����ݺͻ����
		dev_extFlash_enable();
		dev_extFlash_erase(DAY_MAIN_DATA_START_ADDRESS,DAY_MAIN_DATA_SIZE*7);
		dev_extFlash_erase(STEP_DATA_START_ADDRESS,STEP_DATA_STOP_ADDRESS - STEP_DATA_START_ADDRESS + 1);	
		dev_extFlash_erase(HEARTRATE_DATA_START_ADDRESS,HEARTRATE_DATA_STOP_ADDRESS - HEARTRATE_DATA_START_ADDRESS + 1);	
		dev_extFlash_erase(PRESSURE_DATA_START_ADDRESS,PRESSURE_DATA_STOP_ADDRESS - PRESSURE_DATA_START_ADDRESS + 1);	
		dev_extFlash_erase(DISTANCE_DATA_START_ADDRESS,DISTANCE_DATA_STOP_ADDRESS - DISTANCE_DATA_START_ADDRESS + 1);	
		dev_extFlash_erase(ENERGY_DATA_START_ADDRESS,ENERGY_DATA_STOP_ADDRESS - ENERGY_DATA_START_ADDRESS + 1); 
		//dev_extFlash_erase(WEATHER_DATA_START_ADDRESS,SPEED_DATA_START_ADDRESS - WEATHER_DATA_START_ADDRESS);	
		dev_extFlash_erase(SPEED_DATA_START_ADDRESS,SPEED_DATA_STOP_ADDRESS - SPEED_DATA_START_ADDRESS + 1);	
		dev_extFlash_erase(DISTANCEPOINT_START_ADDRESS,DISTANCEPOINT_STOP_ADDRESS - DISTANCEPOINT_START_ADDRESS + 1);	
		dev_extFlash_erase(CIRCLETIME_START_ADDRESS,CIRCLETIME_STOP_ADDRESS - CIRCLETIME_START_ADDRESS + 1);	
		dev_extFlash_erase(PAUSE_DATA_START_ADDRESS,PAUSE_DATA_STOP_ADDRESS - PAUSE_DATA_START_ADDRESS + 1);	
		dev_extFlash_erase(GPS_DATA_START_ADDRESS,GPS_DATA_STOP_ADDRESS - GPS_DATA_START_ADDRESS + 1);	
		//dev_extFlash_erase(VERTICALSIZE_START_ADDRESS,VERTICALSIZE_STOP_ADDRESS - VERTICALSIZE_START_ADDRESS + 1);
		//dev_extFlash_erase(TOUCHDOWN_START_ADDRESS,TOUCHDOWN_STOP_ADDRESS - TOUCHDOWN_START_ADDRESS + 1);

		//ɾ��ѵ���ƻ�
		dev_extFlash_erase(TRAINING_DATA_START_ADDRESS,sizeof(DownloadStr)*TRAIN_PLAN_MAX_DAYS+sizeof(TrainPlanDownStr));
		dev_extFlash_erase(TRAINING_DATA_UPLOAD_ADDRESS,sizeof(UploadStr)*TRAIN_PLAN_MAX_DAYS+sizeof(TrainPlanUploadStr));
		#ifdef COD 
		dev_extFlash_erase(ALL_DAY_STEPS_START_ADDRESS, 7*ALL_DAY_STEPS_SIZE);
		dev_extFlash_erase(COD_BLE_DEV_START_ADDRESS, COD_BLE_DEV_SIZE);
		#endif
		dev_extFlash_disable();
		
		//ɾ������
		com_dial_delete(); //��ʱ����
}
void com_data_init(bool set_rtc)
{
	COM_DATA_PRINTF("[com_data]:com_data_init\n");
	dev_extFlash_enable();
	//dev_extFlash_erase(SET_VALUE_ADDRESS, sizeof(SetValueStr));
	dev_extFlash_read(SET_VALUE_ADDRESS, (uint8_t*)(&SetValue), sizeof(SetValueStr));
	dev_extFlash_disable();
	
	COM_DATA_PRINTF("[com_data]: SetValue.AppGet.LengthSwimmingPool --> %d\n",SetValue.AppGet.LengthSwimmingPool);	
#if 0	
	uint32_t calculate_crc = am_bootloader_fast_crc32((uint8_t*)(&SetValue.crc)+sizeof(SetValue.crc), sizeof(SetValueStr)-sizeof(SetValue.crc));

	//COM_DATA_PRINTF("[com_data]:calculate_crc --> 0x%08X\n",calculate_crc);
	//COM_DATA_PRINTF("[com_data]:SetValue crc --> 0x%08X\n",SetValue.crc);
	if( SetValue.crc == calculate_crc )  //�������ݱ���
#else	
	if( SetValue.crc == FLASH_USED_FLAG )  //�������ݱ���
#endif
	{
		COM_DATA_PRINTF("[com_data]:DayMainData_Init\n");
		//�䶯���ݲ�������
		modify_data_config();
		
	}
	else  //��һ���ϵ�����,crc�����䶯
	{
		com_sport_data_del();
		COM_DATA_PRINTF("[com_data]:First_Run_Config\n");
		First_Run_Config();
	}

	DayMainData_Init(set_rtc);
	Set_Select_Home_Index();
	Set_Curr_Home_Num(0);
	ActSemaphore = xSemaphoreCreateBinary();  //������ֵ�ź���

	//��ȡ��������
	AmbientData_Read();

	//��ȡѵ���ƻ�����
	Get_TrainPlanDayRecord();
	
	//��ȡ��ʷ�ճ�����
	get_weekSteps();
	
	//��ȡ��������
	gui_tool_calendar_weather_info_init();
	
#if defined(WATCH_PRECISION_TIME_CALIBRATION)
	
#if defined(WATCH_GPS_OR_NO_GPS_RESOURCE_SET_TIME)
	set_s_set_time_resource(SET_TIME_RESOURCE_BY_RESET);//����RTCʱ����Դ--����
#endif
	uint8_t m_timecalibration_status = 0;//����ʱ��У׼״̬
	//��ȡFlash�о���ʱ��洢״̬
	Set_GpsFirstTimingStatus(false);
	m_timecalibration_status = ReadCalibrationValueAndConfigAdjValue();
	Set_TimeCalibrationStatus(m_timecalibration_status);
	config_XT();
	s_is_open_nav_time_gps = TIME_NAV_GPS_NONE;//���� ��Ҫ���¼������
	set_s_is_mcu_restart_and_need_set_time(true);//���� ��Ҫ������ʱ
#endif
#if DEBUG_ENABLED == 1 && COM_DATA_LOG_ENABLED == 1
	/*For test*/
	Data_Log_Print();
#endif
#if defined(WATCH_AUTO_BACK_HOME)
//������ˢ��ҳ�泬��10�������κβ������ް������������ش������� ״̬��ʼ��
	AutoBackScreenStr m_auto_back_home;
	memset(&m_auto_back_home,0,sizeof(AutoBackScreenStr));
	set_m_AutoBackScreen(m_auto_back_home);
#endif
}

/*����˵��:RAM��FLASH LOG��ӡ �ճ����� ����� ѵ���ƻ�����*/
void Data_Log_Print(void)
{
	DayMainDataStr logDayMainDataStr,FlashDayMainDataStr[7];
	_com_config_train_plan_MD5_UUID  MD5_UUIDStr;
	UploadStr FlashUploadStr;
	TrainPlanDownStr  FlashDownTitleStr;
	DownloadStr      FlashDownloadStr;
	
	uint32_t FlashCommonSteps[24],FlashCommonEnergy[24],FlashCommonDistance[24],FlashSportSteps[24],FlashSportEnergy[24],FlashSportDistance[24]
	         ,FlashTemprature[24],FlashPressure[24];
	uint16_t FlashVO2MAXMeasuring[24],FlashLTSpeedMeasuring[24];
	uint8_t FlashBasedHdr[24],FlashLTBMPMeasuring[24];
	
	memset(&logDayMainDataStr,0,sizeof(DayMainDataStr));
	memset(FlashDayMainDataStr,0,sizeof(FlashDayMainDataStr));
	memset(FlashCommonSteps,0,sizeof(FlashCommonSteps));
	memset(FlashCommonEnergy,0,sizeof(FlashCommonEnergy));
	memset(FlashCommonDistance,0,sizeof(FlashCommonDistance));
	memset(FlashSportSteps,0,sizeof(FlashSportSteps));
	memset(FlashSportEnergy,0,sizeof(FlashSportEnergy));
	memset(FlashSportDistance,0,sizeof(FlashSportDistance));
	memset(FlashTemprature,0,sizeof(FlashTemprature));
	memset(FlashPressure,0,sizeof(FlashPressure));
	memset(FlashBasedHdr,0,sizeof(FlashBasedHdr));
	memset(FlashVO2MAXMeasuring,0,sizeof(FlashVO2MAXMeasuring));
	memset(FlashLTSpeedMeasuring,0,sizeof(FlashLTSpeedMeasuring));
	memset(FlashLTBMPMeasuring,0,sizeof(FlashLTBMPMeasuring));

	memset(&MD5_UUIDStr,0,sizeof(_com_config_train_plan_MD5_UUID));
	memset(&FlashUploadStr,0,sizeof(UploadStr));
	memset(&FlashDownTitleStr,0,sizeof(TrainPlanDownStr));
	memset(&FlashDownloadStr,0,sizeof(DownloadStr));
	
	dev_extFlash_enable();
	dev_extFlash_read(DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*DayMainData.Index,(uint8_t *)&logDayMainDataStr,sizeof(DayMainDataStr));
	COM_DATA_PRINTF("------------------------------------Data_Log_Print---------Start------------------------------------\n");
	
	COM_DATA_PRINTF("************************************Data_Log_Print---------RAM DayCommonSportData*******************\n");
	COM_DATA_PRINTF("RAM-DayMainDataStr:year=%d,month=%d,date=%d,index=%d\n",
	                  logDayMainDataStr.Year,logDayMainDataStr.Month,logDayMainDataStr.Date,logDayMainDataStr.Index);
	COM_DATA_PRINTF("RAM-DayMainDataStr:CommonSteps=%d,CommonEnergy=%d,CommonDistance=%d\n",
	               logDayMainDataStr.CommonSteps,logDayMainDataStr.CommonEnergy,logDayMainDataStr.CommonDistance);
	COM_DATA_PRINTF("RAM-DayMainDataStr:SportSteps=%d,SportEnergy=%d,SportDistance=%d,BasedHeartrate=%d,Temprature=%d,Pressure=%d,VO2MAX=%d,LTSpeed=%d,LTBMP=%d\n",
	               logDayMainDataStr.SportSteps,logDayMainDataStr.SportEnergy,logDayMainDataStr.SportDistance,
	               logDayMainDataStr.BasedHeartrate,logDayMainDataStr.Temprature,logDayMainDataStr.Pressure,
								 logDayMainDataStr.VO2MaxMeasuring,logDayMainDataStr.LTSpeedMeasuring,logDayMainDataStr.LTBmpMeasuring);
	COM_DATA_PRINTF("************************************Data_Log_Print---------FLASH DayCommonSportData*****************\n");

	for(uint8_t i=0;i<7;i++)
	{
		dev_extFlash_read(DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*i,(uint8_t*)(&FlashDayMainDataStr[i]),4);  //��ȡ�����
		COM_DATA_PRINTF("Flash--------Index=%d------------->>>>>>>\n year=%d,month=%d,date=%d\n"
		                 ,FlashDayMainDataStr[i].Index,FlashDayMainDataStr[i].Year,FlashDayMainDataStr[i].Month,FlashDayMainDataStr[i].Date);
		for(uint8_t j=0;j<24;j++)
		{
			dev_extFlash_read( DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*i+DAY_COMMON_STEPS_OFFSET+DAY_COMMON_STEPS_LENGTH*j,
										 (uint8_t*)(&FlashCommonSteps[j]),DAY_COMMON_STEPS_LENGTH);
			dev_extFlash_read( DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*i+DAY_COMMON_ENERGY_OFFSET+DAY_COMMON_ENERGY_LENGTH*j,
												 (uint8_t*)(&FlashCommonEnergy[j]),DAY_COMMON_ENERGY_LENGTH);
			dev_extFlash_read(DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*i+DAY_COMMON_DISTANCE_OFFSET+DAY_COMMON_DISTANCE_LENGTH*j,
												 (uint8_t*)(&FlashCommonDistance[j]),DAY_COMMON_DISTANCE_LENGTH);
			dev_extFlash_read( DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*i+DAY_SPORTS_STEPS_OFFSET+DAY_SPORTS_STEPS_LENGTH*j,
												 (uint8_t*)(&FlashSportSteps[j]),DAY_SPORTS_STEPS_LENGTH);
			dev_extFlash_read( DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*i+DAY_SPORTS_ENERGY_OFFSET+DAY_SPORTS_ENERGY_LENGTH*j,
												 (uint8_t*)(&FlashSportEnergy[j]),DAY_SPORTS_ENERGY_LENGTH);
			dev_extFlash_read(DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*i+DAY_SPORTS_DISTANCE_OFFSET+DAY_SPORTS_DISTANCE_LENGTH*j,
												 (uint8_t*)(&FlashSportDistance[j]),DAY_SPORTS_DISTANCE_LENGTH);			 
			dev_extFlash_read(DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*i+DAY_BASE_HDR_OFFSET+DAY_BASE_HDR_LENGTH*j,
												 (uint8_t*)(&FlashBasedHdr[j]),DAY_BASE_HDR_LENGTH);								 
			dev_extFlash_read(DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*i+DAY_TEMPRATURE_OFFSET+DAY_TEMPRATURE_LENGTH*j,
												 (uint8_t*)(&FlashTemprature[j]),DAY_TEMPRATURE_LENGTH);
			dev_extFlash_read(DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*i+DAY_PRESSURE_OFFSET+DAY_PRESSURE_LENGTH*j,
												 (uint8_t*)(&FlashPressure[j]),DAY_PRESSURE_LENGTH);	
			dev_extFlash_read(DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*i+DAY_VO2MAXMEASURING_OFFSET+DAY_VO2MAXMEASURING_LENGTH*j,
												 (uint8_t*)(&FlashVO2MAXMeasuring[j]),DAY_VO2MAXMEASURING_LENGTH);								 
			dev_extFlash_read(DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*i+DAY_LTSPEEDMEASURING_OFFSET+DAY_LTSPEEDMEASURING_LENGTH*j,
												 (uint8_t*)(&FlashLTSpeedMeasuring[j]),DAY_LTSPEEDMEASURING_LENGTH);
			dev_extFlash_read(DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*i+DAY_LTBMPMEASURING_OFFSET+DAY_LTBMPMEASURING_LENGTH*j,
												 (uint8_t*)(&FlashLTBMPMeasuring[j]),DAY_LTBMPMEASURING_LENGTH);
		}
		COM_DATA_PRINTF("Flash----CommonSteps=%d,%d,%d,%d,%d, %d,%d,%d,%d,%d, %d,%d,%d,%d,%d, %d,%d,%d,%d,%d, %d,%d,%d,%d\n",
		                FlashCommonSteps[0],FlashCommonSteps[1],FlashCommonSteps[2],FlashCommonSteps[3],FlashCommonSteps[4],
		                FlashCommonSteps[5],FlashCommonSteps[6],FlashCommonSteps[7],FlashCommonSteps[8],FlashCommonSteps[9],
		                FlashCommonSteps[10],FlashCommonSteps[11],FlashCommonSteps[12],FlashCommonSteps[13],FlashCommonSteps[14],
		                FlashCommonSteps[15],FlashCommonSteps[16],FlashCommonSteps[17],FlashCommonSteps[18],FlashCommonSteps[19],
		                FlashCommonSteps[20],FlashCommonSteps[21],FlashCommonSteps[22],FlashCommonSteps[23]);
		COM_DATA_PRINTF("Flash----CommonEnergy=%d,%d,%d,%d,%d, %d,%d,%d,%d,%d, %d,%d,%d,%d,%d, %d,%d,%d,%d,%d, %d,%d,%d,%d\n",
		                FlashCommonEnergy[0],FlashCommonEnergy[1],FlashCommonEnergy[2],FlashCommonEnergy[3],FlashCommonEnergy[4],
		                FlashCommonEnergy[5],FlashCommonEnergy[6],FlashCommonEnergy[7],FlashCommonEnergy[8],FlashCommonEnergy[9],
		                FlashCommonEnergy[10],FlashCommonEnergy[11],FlashCommonEnergy[12],FlashCommonEnergy[13],FlashCommonEnergy[14],
		                FlashCommonEnergy[15],FlashCommonEnergy[16],FlashCommonEnergy[17],FlashCommonEnergy[18],FlashCommonEnergy[19],
		                FlashCommonEnergy[20],FlashCommonEnergy[21],FlashCommonEnergy[22],FlashCommonEnergy[23]);
		COM_DATA_PRINTF("Flash----CommonDistance=%d,%d,%d,%d,%d, %d,%d,%d,%d,%d, %d,%d,%d,%d,%d, %d,%d,%d,%d,%d, %d,%d,%d,%d\n",
		                FlashCommonDistance[0],FlashCommonDistance[1],FlashCommonDistance[2],FlashCommonDistance[3],FlashCommonDistance[4],
		                FlashCommonDistance[5],FlashCommonDistance[6],FlashCommonDistance[7],FlashCommonDistance[8],FlashCommonDistance[9],
		                FlashCommonDistance[10],FlashCommonDistance[11],FlashCommonDistance[12],FlashCommonDistance[13],FlashCommonDistance[14],
		                FlashCommonDistance[15],FlashCommonDistance[16],FlashCommonDistance[17],FlashCommonDistance[18],FlashCommonDistance[19],
		                FlashCommonDistance[20],FlashCommonDistance[21],FlashCommonDistance[22],FlashCommonDistance[23]);
		COM_DATA_PRINTF("Flash----SportSteps=%d,%d,%d,%d,%d, %d,%d,%d,%d,%d, %d,%d,%d,%d,%d, %d,%d,%d,%d,%d, %d,%d,%d,%d\n",
		                FlashSportSteps[0],FlashSportSteps[1],FlashSportSteps[2],FlashSportSteps[3],FlashSportSteps[4],
		                FlashSportSteps[5],FlashSportSteps[6],FlashSportSteps[7],FlashSportSteps[8],FlashSportSteps[9],
		                FlashSportSteps[10],FlashSportSteps[11],FlashSportSteps[12],FlashSportSteps[13],FlashSportSteps[14],
		                FlashSportSteps[15],FlashSportSteps[16],FlashSportSteps[17],FlashSportSteps[18],FlashSportSteps[19],
		                FlashSportSteps[20],FlashSportSteps[21],FlashSportSteps[22],FlashSportSteps[23]);
		COM_DATA_PRINTF("Flash----SportEnergy=%d,%d,%d,%d,%d, %d,%d,%d,%d,%d, %d,%d,%d,%d,%d, %d,%d,%d,%d,%d, %d,%d,%d,%d\n",
		                FlashSportEnergy[0],FlashSportEnergy[1],FlashSportEnergy[2],FlashSportEnergy[3],FlashSportEnergy[4],
		                FlashSportEnergy[5],FlashSportEnergy[6],FlashSportEnergy[7],FlashSportEnergy[8],FlashSportEnergy[9],
		                FlashSportEnergy[10],FlashSportEnergy[11],FlashSportEnergy[12],FlashSportEnergy[13],FlashSportEnergy[14],
		                FlashSportEnergy[15],FlashSportEnergy[16],FlashSportEnergy[17],FlashSportEnergy[18],FlashSportEnergy[19],
		                FlashSportEnergy[20],FlashSportEnergy[21],FlashSportEnergy[22],FlashSportEnergy[23]);
		COM_DATA_PRINTF("Flash----SportDistance=%d,%d,%d,%d,%d, %d,%d,%d,%d,%d, %d,%d,%d,%d,%d, %d,%d,%d,%d,%d, %d,%d,%d,%d\n",
		                FlashSportDistance[0],FlashSportDistance[1],FlashSportDistance[2],FlashSportDistance[3],FlashSportDistance[4],
		                FlashSportDistance[5],FlashSportDistance[6],FlashSportDistance[7],FlashSportDistance[8],FlashSportDistance[9],
		                FlashSportDistance[10],FlashSportDistance[11],FlashSportDistance[12],FlashSportDistance[13],FlashSportDistance[14],
		                FlashSportDistance[15],FlashSportDistance[16],FlashSportDistance[17],FlashSportDistance[18],FlashSportDistance[19],
		                FlashSportDistance[20],FlashSportDistance[21],FlashSportDistance[22],FlashSportDistance[23]);								
		COM_DATA_PRINTF("Flash----BasedHdr=%d,%d,%d,%d,%d, %d,%d,%d,%d,%d, %d,%d,%d,%d,%d, %d,%d,%d,%d,%d, %d,%d,%d,%d\n",
		                FlashBasedHdr[0],FlashBasedHdr[1],FlashBasedHdr[2],FlashBasedHdr[3],FlashBasedHdr[4],
		                FlashBasedHdr[5],FlashBasedHdr[6],FlashBasedHdr[7],FlashBasedHdr[8],FlashBasedHdr[9],
		                FlashBasedHdr[10],FlashBasedHdr[11],FlashBasedHdr[12],FlashBasedHdr[13],FlashBasedHdr[14],
		                FlashBasedHdr[15],FlashBasedHdr[16],FlashBasedHdr[17],FlashBasedHdr[18],FlashBasedHdr[19],
		                FlashBasedHdr[20],FlashBasedHdr[21],FlashBasedHdr[22],FlashBasedHdr[23]);									
		COM_DATA_PRINTF("Flash----Temprature=%d,%d,%d,%d,%d, %d,%d,%d,%d,%d, %d,%d,%d,%d,%d, %d,%d,%d,%d,%d, %d,%d,%d,%d\n",
		                FlashTemprature[0],FlashTemprature[1],FlashTemprature[2],FlashTemprature[3],FlashTemprature[4],
		                FlashTemprature[5],FlashTemprature[6],FlashTemprature[7],FlashTemprature[8],FlashTemprature[9],
		                FlashTemprature[10],FlashTemprature[11],FlashTemprature[12],FlashTemprature[13],FlashTemprature[14],
		                FlashTemprature[15],FlashTemprature[16],FlashTemprature[17],FlashTemprature[18],FlashTemprature[19],
		                FlashTemprature[20],FlashTemprature[21],FlashTemprature[22],FlashTemprature[23]);										
		COM_DATA_PRINTF("Flash----Pressure=%d,%d,%d,%d,%d, %d,%d,%d,%d,%d, %d,%d,%d,%d,%d, %d,%d,%d,%d,%d, %d,%d,%d,%d\n",
		                FlashPressure[0],FlashPressure[1],FlashPressure[2],FlashPressure[3],FlashPressure[4],
		                FlashPressure[5],FlashPressure[6],FlashPressure[7],FlashPressure[8],FlashPressure[9],
		                FlashPressure[10],FlashPressure[11],FlashPressure[12],FlashPressure[13],FlashPressure[14],
		                FlashPressure[15],FlashPressure[16],FlashPressure[17],FlashPressure[18],FlashPressure[19],
		                FlashPressure[20],FlashPressure[21],FlashPressure[22],FlashPressure[23]);
		COM_DATA_PRINTF("Flash----FlashVO2MAXMeasuring=%d,%d,%d,%d,%d, %d,%d,%d,%d,%d, %d,%d,%d,%d,%d, %d,%d,%d,%d,%d, %d,%d,%d,%d\n",
		                FlashVO2MAXMeasuring[0],FlashVO2MAXMeasuring[1],FlashVO2MAXMeasuring[2],FlashVO2MAXMeasuring[3],FlashVO2MAXMeasuring[4],
		                FlashVO2MAXMeasuring[5],FlashVO2MAXMeasuring[6],FlashVO2MAXMeasuring[7],FlashVO2MAXMeasuring[8],FlashVO2MAXMeasuring[9],
		                FlashVO2MAXMeasuring[10],FlashVO2MAXMeasuring[11],FlashVO2MAXMeasuring[12],FlashVO2MAXMeasuring[13],FlashVO2MAXMeasuring[14],
		                FlashVO2MAXMeasuring[15],FlashVO2MAXMeasuring[16],FlashVO2MAXMeasuring[17],FlashVO2MAXMeasuring[18],FlashVO2MAXMeasuring[19],
		                FlashVO2MAXMeasuring[20],FlashVO2MAXMeasuring[21],FlashVO2MAXMeasuring[22],FlashVO2MAXMeasuring[23]);		
		COM_DATA_PRINTF("Flash----FlashLTSpeedMeasuring=%d,%d,%d,%d,%d, %d,%d,%d,%d,%d, %d,%d,%d,%d,%d, %d,%d,%d,%d,%d, %d,%d,%d,%d\n",
		                FlashLTSpeedMeasuring[0],FlashLTSpeedMeasuring[1],FlashLTSpeedMeasuring[2],FlashLTSpeedMeasuring[3],FlashLTSpeedMeasuring[4],
		                FlashLTSpeedMeasuring[5],FlashLTSpeedMeasuring[6],FlashLTSpeedMeasuring[7],FlashLTSpeedMeasuring[8],FlashLTSpeedMeasuring[9],
		                FlashLTSpeedMeasuring[10],FlashLTSpeedMeasuring[11],FlashLTSpeedMeasuring[12],FlashLTSpeedMeasuring[13],FlashLTSpeedMeasuring[14],
		                FlashLTSpeedMeasuring[15],FlashLTSpeedMeasuring[16],FlashLTSpeedMeasuring[17],FlashLTSpeedMeasuring[18],FlashLTSpeedMeasuring[19],
		                FlashLTSpeedMeasuring[20],FlashLTSpeedMeasuring[21],FlashLTSpeedMeasuring[22],FlashLTSpeedMeasuring[23]);		
		COM_DATA_PRINTF("Flash----FlashLTBMPMeasuring=%d,%d,%d,%d,%d, %d,%d,%d,%d,%d, %d,%d,%d,%d,%d, %d,%d,%d,%d,%d, %d,%d,%d,%d\n",
		                FlashLTBMPMeasuring[0],FlashLTBMPMeasuring[1],FlashLTBMPMeasuring[2],FlashLTBMPMeasuring[3],FlashLTBMPMeasuring[4],
		                FlashLTBMPMeasuring[5],FlashLTBMPMeasuring[6],FlashLTBMPMeasuring[7],FlashLTBMPMeasuring[8],FlashLTBMPMeasuring[9],
		                FlashLTBMPMeasuring[10],FlashLTBMPMeasuring[11],FlashLTBMPMeasuring[12],FlashLTBMPMeasuring[13],FlashLTBMPMeasuring[14],
		                FlashLTBMPMeasuring[15],FlashLTBMPMeasuring[16],FlashLTBMPMeasuring[17],FlashLTBMPMeasuring[18],FlashLTBMPMeasuring[19],
		                FlashLTBMPMeasuring[20],FlashLTBMPMeasuring[21],FlashLTBMPMeasuring[22],FlashLTBMPMeasuring[23]);												
	}
	COM_DATA_PRINTF("************************************Data_Log_Print---------FLASH TrainPlan**************************\n");

	dev_extFlash_read(TRAINING_DATA_START_ADDRESS,(uint8_t*)(&FlashDownTitleStr),sizeof(TrainPlanDownStr));
	COM_DATA_PRINTF("---------------DownloadTitle----------------->>>>>\n");
	COM_DATA_PRINTF("Flash:MD5Str=%s,UUID=%s,NameStr=%s,Hint.Hour=%d,Hint.Minute=%d\n StartTime.Year=%d,month=%d,day=%d,StopTime.Year=%d,month=%d,day=%d\n",
	         FlashDownTitleStr.Md5Str,FlashDownTitleStr.UUIDStr,FlashDownTitleStr.NameStr,FlashDownTitleStr.Hint.Hour,FlashDownTitleStr.Hint.Minute,
	         FlashDownTitleStr.StartTime.Year,FlashDownTitleStr.StartTime.Month,FlashDownTitleStr.StartTime.Day,
	         FlashDownTitleStr.StopTime.Year,FlashDownTitleStr.StopTime.Month,FlashDownTitleStr.StopTime.Day);
	COM_DATA_PRINTF("---------------DownloadStr-------Totalday=%d---------->>>>>\n",Get_TrainPlanTotalDays());
//	for(uint8_t i=0;i<Get_TrainPlanTotalDays();i++)
//	{
//		dev_extFlash_read(TRAINING_DATA_START_ADDRESS+sizeof(TrainPlanDownStr)*i,(uint8_t*)(&FlashDownloadStr),sizeof(DownloadStr));
//		COM_DATA_PRINTF("%d:DownloadStr:Type=%d,TargetHour=%d,TargetDistance=%d,TargetCalories=%d",
//		 (i+1),FlashDownloadStr.Type,FlashDownloadStr.TargetHour,FlashDownloadStr.TargetDistance,FlashDownloadStr.TargetCalories);
//	}
	dev_extFlash_read(TRAINING_DATA_UPLOAD_ADDRESS,(uint8_t*)(&MD5_UUIDStr),sizeof(_com_config_train_plan_MD5_UUID));
	COM_DATA_PRINTF("---------------UploadTitle----------------->>>>>\n");
	COM_DATA_PRINTF("Flash:MD5Str=%s,UUID=%s\n",MD5_UUIDStr.Md5Str,MD5_UUIDStr.UUIDStr);
	COM_DATA_PRINTF("---------------UploadStr---------Totalday=%d---------->>>>>\n",Get_TrainPlanTotalDays());

	dev_extFlash_disable();
	COM_DATA_PRINTF("------------------------------------Data_Log_Print---------End--------------------------------------\n");
}

//��ȡĳ��ĺ�ƽ����ѹֵ
int32_t ReadSeaLevelPressure(uint8_t year,uint8_t month,uint8_t day)
{
	uint8_t i;
	uint32_t addr = WEATHER_DATA_START_ADDRESS + (sizeof(AppWeatherSync) -7* sizeof(AppWeatherSyncDaily)) ;
	AppWeatherSyncDaily wea = {0};
	
	dev_extFlash_enable();
	for(i=0;i<7;i++)
	{
		dev_extFlash_read(addr,(uint8_t *)(&wea),sizeof(AppWeatherSyncDaily));
       // SEGGER_RTT_printf(0,"ReadSeaLevelPressure %d,%d\n",wea.pressure,wea.date[0]);
		if((wea.date[0] == year) && (wea.date[1]== month) && (wea.date[2] == day))
		{
			if( (wea.pressure >= 800) && (wea.pressure <= 1100 ) )
			{
				SetValue.AppSet.Sea_Level_Pressure = wea.pressure*100;
			}
			break;
		}
		
		addr += sizeof(AppWeatherSyncDaily);
	}
	dev_extFlash_disable();
	
	return wea.pressure;
}


/*�ָ���������*/
#include "com_dial.h"
void Factory_Reset(void)
{

	COM_DATA_PRINTF("----------Factory_Reset------------\n");
	//ɾ���˶����ݣ������ճ����ݺͻ����
	dev_extFlash_enable();
	dev_extFlash_erase(DAY_MAIN_DATA_START_ADDRESS,DAY_MAIN_DATA_SIZE*7);
	dev_extFlash_erase(STEP_DATA_START_ADDRESS,STEP_DATA_STOP_ADDRESS - STEP_DATA_START_ADDRESS + 1);	
	dev_extFlash_erase(HEARTRATE_DATA_START_ADDRESS,HEARTRATE_DATA_STOP_ADDRESS - HEARTRATE_DATA_START_ADDRESS + 1);	
	dev_extFlash_erase(PRESSURE_DATA_START_ADDRESS,PRESSURE_DATA_STOP_ADDRESS - PRESSURE_DATA_START_ADDRESS + 1);	
	dev_extFlash_erase(DISTANCE_DATA_START_ADDRESS,DISTANCE_DATA_STOP_ADDRESS - DISTANCE_DATA_START_ADDRESS + 1);	
	dev_extFlash_erase(ENERGY_DATA_START_ADDRESS,ENERGY_DATA_STOP_ADDRESS - ENERGY_DATA_START_ADDRESS + 1);	
	dev_extFlash_erase(WEATHER_DATA_START_ADDRESS,SPEED_DATA_START_ADDRESS - WEATHER_DATA_START_ADDRESS);	
	dev_extFlash_erase(SPEED_DATA_START_ADDRESS,SPEED_DATA_STOP_ADDRESS - SPEED_DATA_START_ADDRESS + 1);	
	dev_extFlash_erase(DISTANCEPOINT_START_ADDRESS,DISTANCEPOINT_STOP_ADDRESS - DISTANCEPOINT_START_ADDRESS + 1);	
	dev_extFlash_erase(CIRCLETIME_START_ADDRESS,CIRCLETIME_STOP_ADDRESS - CIRCLETIME_START_ADDRESS + 1);	
	dev_extFlash_erase(PAUSE_DATA_START_ADDRESS,PAUSE_DATA_STOP_ADDRESS - PAUSE_DATA_START_ADDRESS + 1);	
	dev_extFlash_erase(GPS_DATA_START_ADDRESS,GPS_DATA_STOP_ADDRESS - GPS_DATA_START_ADDRESS + 1);
	
#ifndef WATCH_GPS_HAEDWARE_ANALYSIS_INFOR
	dev_extFlash_erase(VERTICALSIZE_START_ADDRESS,VERTICALSIZE_STOP_ADDRESS - VERTICALSIZE_START_ADDRESS + 1);
#endif
	dev_extFlash_erase(SATELLITE_DATA_START_ADDRESS,REALTIME_STEP_START_ADDRESS - SATELLITE_DATA_START_ADDRESS);
	
	//ɾ��֪ͨ����
	dev_extFlash_erase(NOTIFY_DATA_START_ADDRESS,TRAINING_DATA_START_ADDRESS - NOTIFY_DATA_START_ADDRESS);

	//ɾ��ѵ���ƻ�
	dev_extFlash_erase(TOUCHDOWN_START_ADDRESS,TOUCHDOWN_STOP_ADDRESS - TOUCHDOWN_START_ADDRESS + 1);	
	dev_extFlash_erase(TRAINING_DATA_START_ADDRESS,sizeof(DownloadStr)*TRAIN_PLAN_MAX_DAYS+sizeof(TrainPlanDownStr));
	dev_extFlash_erase(TRAINING_DATA_UPLOAD_ADDRESS,sizeof(UploadStr)*TRAIN_PLAN_MAX_DAYS+sizeof(TrainPlanUploadStr));

	//ɾ������켣
	dev_extFlash_erase(SHARE_TRACK_START_ADDR, ONE_SHARED_TRACK_SIZE * SHARED_TRACK_NUM_MAX);

	#ifdef COD 
	dev_extFlash_erase(ALL_DAY_STEPS_START_ADDRESS, 7*ALL_DAY_STEPS_SIZE);
	dev_extFlash_erase(COD_BLE_DEV_START_ADDRESS, COD_BLE_DEV_SIZE);
	#endif 

	dev_extFlash_disable();	

	if(SetValue.SwBle >= BLE_DISCONNECT)
	{
		dev_ble_system_off();
		SetValue.SwBle = BLE_DISCONNECT;
	}
	
	//ɾ������
	#ifndef COD 
	com_dial_delete();
	#endif
	
	//�ָ�������������
	First_Run_Config();
	
	
	lib_app_data_erase();		
	
    //�Ʋ�����
	ClearStepCount();
	//����
	taskENTER_CRITICAL();
	am_hal_reset_por();
	while(1);
}















