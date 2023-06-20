#include "com_config.h"
#include "com_sport.h"
#include "drv_extflash.h"

#include "task_battery.h"
#include "task_sport.h"
#include "task_gps.h"
#include "task_sport.h"

#include "gui_sports_record.h"
#include "gui_sport.h"
#include "gui_tool_gps.h"

#include "algo_hdr.h"
#include "algo_sport.h"
#include "algo_time_calibration.h"
#include "time_notify.h"
#include "drv_ms5837.h"

#include "img_sport.h"

#include "algo_trackjudge.h"
#include <stdlib.h>


#if DEBUG_ENABLED == 1 && COM_SPORT_LOG_ENABLED == 1
	#define COM_SPORT_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define COM_SPORT_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define COM_SPORT_WRITESTRING(...)
	#define COM_SPORT_PRINTF(...)		        
#endif

ActivityDataStr  ActivityData,ActivityRecordData,ActivityRecordDataDown;
//ShareTrackStr ShareTrack[SHARED_TRACK_NUM_MAX]; 

ContinueLaterDataStr ContinueLaterDataSave;  //�Ժ�������ݼ�¼
TrainRecordStr TrainRecord[2];//���ڽ������˶���¼��
uint8_t IS_SPORT_CONTINUE_LATER = 0;  //�˶��Ժ�������

PauseTimeStr PauseTime;
extern SetValueStr SetValue;
extern _drv_bat_event   g_bat_evt;

LatLng dLatLng;//��ƫ��γ��

static double pi = 3.14159265358979324;
static double a = 6378245.0;
static double ee = 0.00669342162296594323;


bool accessory_heartrate_status = false;//�����������״̬
bool accessory_cycling_cadence_status = false;//̤Ƶ����״̬

static uint8_t g_TrainRecordSelect = 0;//�����˶���¼��������ѡ��
static uint8_t g_TrainRecordNum = 0;
static uint8_t g_TrainRecordPage = 0;

static uint8_t heartrate_threshold = 0; //����������ֵ����
static uint8_t heartrate_last = 0;      //�ϴ�����ֵ
static uint32_t daily_kcal_last = 0;  	//�ϴ�ÿ�տ�·������ֵ
static uint8_t kcal_tags = 0;			//ÿ�տ�·�����ѱ�ʶ
static uint32_t daily_step_last = 0;  	//�ϴ�ÿ�ղ���ֵ
static uint8_t step_tags = 0;  			//ÿ�ղ���ֵ���ѱ�ʶ
static uint16_t kcal_last = 0;          //�ϴο�·��ֵ
static uint16_t circle_last = 0;        //�ϴ�Ȧ��
static uint16_t time_last = 0;          //�ϴ�ʱ��
static uint16_t goal_remind_distance_last = 0;	//�ϴ�Ŀ�����Ѿ���
static uint16_t circle_remind_distance_last = 0;//�ϴμ�Ȧ���Ѿ���
static uint16_t distance_nums = 1;      //�������Ѵ���
static uint16_t altitude_last = 0;      //�ϴθ߶�
static uint32_t speed_nums = 0;        //�ٶ����Ѽ���
static uint16_t speed_nums_limit = 60;      //�ٶ����Ѽ���ʱ�����ƣ���ʼ1���ӣ�����3��5����
static uint32_t speed_nums_cycleindex = 0;      //�ٶ����Ѽ���ѭ������
static uint32_t speed_trainplan_nums = 0;        //�ٶ����Ѽ���
static uint32_t distance_circle_nums = 0;//ѵ���ƻ�4x400����
static uint16_t circledistance_nums = 1;    //��ȦȦ��
static uint16_t circledistance_last = 0;    //�ϴμ�Ȧ����
static uint16_t kilometer_nums = 1;    //��ȦȦ��
static uint16_t kilometer_last = 0;    //�ϴμ�Ȧ����

static bool isTrainPlanOnGoing = false;//ѵ���ƻ������˶�������
static uint8_t m_Read_Num = 2;                    //һ�ζ����������������ʾ

int32_t trackgpsstore[6000];//��γ��˳��洢

int16_t trackxystore[6000] = {0};//x,y˳�򴢴棬

int32_t loadgpsstore[3000];//���ؾ�γ��˳��洢

int16_t loadxystore[3000] = {0};//���ع켣x,y˳�򴢴棬
float start_temp_altitude[7] = {0};
float start_temp_drv_pres[7] = {1013.25};

float start_temp_drv_alt[7] = {0};


#if defined WATCH_SPORT_NEW_CLOUD_NAVIGATION
TrackMarkStr TrackMark[TRACK_MARK_NUM_MAX],m_compare_track_mark[TRACK_MARK_NUM_MAX];//������� ����ʱ������õĺ������
extern float GpsDistance5Bit(int32_t  LonA, int32_t  LatA, int32_t  LonB, int32_t  LatB);
#else
TrackMarkStr TrackMark[TRACK_MARK_NUM_MAX];//�������
extern float GpsDistance5Bit(int32_t  LonA, int32_t  LatA, int32_t  LonB, int32_t  LatB);
#endif

uint8_t TrackMarkNums = 0; //����������
uint8_t TrackMarkIndex =0xFF;//��ǰ�����������

uint16_t gpslen,xylen,zoom,zoommax,gpsbacklen,xybacklen;         //gps�洢���ȣ�����ӳ�䳤��,���ŵȼ�

uint8_t xybackflag,startpointflag,drawbackflag;//������־��

int32_t center_gps_lon;             //���ĵ㾭γ��
int32_t center_gps_lat;
int32_t last_center_gps_lon;             //�ϴ����ĵ㾭γ��
int32_t last_center_gps_lat;

int32_t center_gps_load_lon;             //�������ĵ㾭γ��
int32_t center_gps_load_lat;
extern int32_t Last_GPS_Lon;               //�ϴξ�γ��
extern int32_t Last_GPS_Lat;
uint16_t loadlen = 0; //���ع켣ӳ��洢����
uint8_t loadflag = 0; //������־
uint16_t loadgpslen = 0;//����GPS����
uint16_t zoomload =1;
uint8_t loadstartfalg,loadendfalg;


uint32_t store_gps_nums = 0; //gps�洢��˳��
uint32_t dot_track_nums = 0;//���������������20��
uint8_t dottrack_draw_time = 0; //���������ʱ��
uint8_t dottrack_out_time = 0;	//������㵽������ʾ����
uint8_t dottrack_nogps_time = 0;//�������gpsû�ж�λ��ʾ����

uint8_t g_ActivityData_Total_Num = 0;//�˶���¼�����
ActitivyDataResult_s ActivityDataResult;//�˶��������Ϣ
ActivityDataStr Sport_Record[2] = {0};//����˶���¼������Ϣ
extern uint8_t LOADINGFLAG;
uint8_t m_InvertFlagHead = 0;//����鵽ͷ��,��������β���鵽daynum-1Ϊֹ �˶���¼�м�¼����λ��
uint8_t m_InvertFlagTail = 0;//�鵽ĩβ���ͷ�鵽daynum-1
static uint8_t m_LastIndexNum = 0;//�����˶���¼ʱ�����˶��Ŀռ�

GpspointStr gps_point = {0};


uint8_t cod_gps_curpoint_num = 0;

extern gps_data_out gps_data_output;

#if defined WATCH_SPORT_NEW_CLOUD_NAVIGATION
SportTrackMarkDataStr g_track_mark_data;//����ʱ��
bool gps_store_flag =false;
#endif



void Set_TrainRecordNum(uint8_t Num)
{
	g_TrainRecordNum = Num;
}
uint8_t Get_TrainRecordNum(void)
{
	return g_TrainRecordNum;
}
void Set_TrainRecordSelect(uint8_t Select)
{
	g_TrainRecordSelect = Select;
}
uint8_t Get_TrainRecordSelect(void)
{
	return g_TrainRecordSelect;
}
void Set_TrainRecordPage(uint8_t Page)
{
	g_TrainRecordPage = Page;
}
uint8_t Get_TrainRecordPage(void)
{
	return g_TrainRecordPage;
}
/*����ѵ���ƻ��˶����ڽ�����״̬*/
void Set_IsTrainPlanOnGoing(bool flag)
{
	isTrainPlanOnGoing = flag;
}
/*��ȡѵ���ƻ��˶����ڽ�����״̬*/
bool Get_IsTrainPlanOnGoing(void)
{
	return isTrainPlanOnGoing;
}

//�����˶�ģʽ,�˶�ģʽ�¼Ӵ����ʹ���
uint8_t IsHeartSportMode(void)
{
	uint8_t ret = 0;
	#ifdef COD
	if (IsSportReady(ScreenState) == true ||ScreenState == DISPLAY_SCREEN_SPORT || IsSportMode(ScreenState)==true || SetValue.rt_heart == 1)
	#else
	if (IsSportReady(ScreenState) == true ||ScreenState == DISPLAY_SCREEN_SPORT || IsSportMode(ScreenState)==true)
	#endif
	{
		ret = 1;
	}
	return ret;
}


//�ж��Ƿ������˶�ģʽ��  --������
bool IsSportMode(ScreenState_t index)
{
	//�ܲ�����ɽ�����ܽ��� ��������� ������ �Ʋ�У׼����
#if defined WATCH_SIM_SPORT
	if((index ==DISPLAY_SCREEN_RUN)
#elif defined WATCH_COM_SPORT
	if ((index ==DISPLAY_SCREEN_RUN_PAUSE)	//��ͣû��׼��״̬
#else	
	if(  ((index >= DISPLAY_SCREEN_RUN_DATA) && (index <= DISPLAY_SCREEN_RUN_TRACKBACK_PAUSE))
#endif
	  || (index == DISPLAY_SCREEN_SPORT_FEEL)
	  || (index == DISPLAY_SCREEN_SPORT_HRRECROVY)
	  || (index == DISPLAY_SCREEN_VO2MAX_MEASURING) ||(index == DISPLAY_SCREEN_STEP_CALIBRATION_SHOW)
#if defined WATCH_SIM_SPORT || defined WATCH_COM_SPORT
#else
	  || ((index >= DISPLAY_SCREEN_SWIM_DATA) && (index <= DISPLAY_SCREEN_SWIM_STATS_PAUSE))
#endif
#if defined WATCH_COM_SPORT
#else
	  || ((index >= DISPLAY_SCREEN_CLIMBING_TRACK) && (index <= DISPLAY_SCREEN_CLIMBING_TRACKBACK_PAUSE))
	  || ((index >= DISPLAY_SCREEN_CROSSCOUNTRY_TRACK) && (index <= DISPLAY_SCREEN_CROSSCOUNTRY_DATA_PAUSE))
#endif
	  || (index == DISPLAY_SCREEN_STEP_CALIBRATION_HINT)
	  || (index == DISPLAY_SCREEN_VO2MAX_HINT)
	  || (index == DISPLAY_SCREEN_LACTATE_THRESHOLD_HINT))
	{
		return true;
	}
	else
	{
		switch(index)
		{
#if defined WATCH_SIM_SPORT 
			case DISPLAY_SCREEN_RUN_PAUSE:
			case DISPLAY_SCREEN_RUN_CANCEL:
				
			case DISPLAY_SCREEN_SWIMMING:
			case DISPLAY_SCREEN_SWIM_PAUSE:
			case DISPLAY_SCREEN_SWIM_CANCEL:
			case DISPLAY_SCREEN_REMIND_PAUSE:
						
#endif
#if defined WATCH_COM_SPORT
			case DISPLAY_SCREEN_RUN:					//�ܲ�
			//case DISPLAY_SCREEN_RUN_PAUSE:
			case DISPLAY_SCREEN_RUN_CANCEL:
			case DISPLAY_SCREEN_CROSSCOUNTRY:
			case DISPLAY_SCREEN_CROSSCOUNTRY_PAUSE:				//ԽҰ��
			case DISPLAY_SCREEN_CROSSCOUNTRY_CANCEL:

			case DISPLAY_SCREEN_CLIMBING:
			case DISPLAY_SCREEN_CLIMBING_PAUSE:				//��ɽ
			case DISPLAY_SCREEN_CLIMBING_CANCEL:
			case DISPLAY_SCREEN_SWIMMING:
			case DISPLAY_SCREEN_SWIM_PAUSE:
			case DISPLAY_SCREEN_SWIM_CANCEL:
			case DISPLAY_SCREEN_REMIND_PAUSE:
			
#endif
			case DISPLAY_SCREEN_CYCLING:				//����
			case DISPLAY_SCREEN_CYCLING_PAUSE:
			case DISPLAY_SCREEN_CYCLING_CANCEL:
				
			case DISPLAY_SCREEN_CROSSCOUNTRY_HIKE:		//ͽ��ԽҰ
			case DISPLAY_SCREEN_CROSSCOUNTRY_HIKE_PAUSE:
			case DISPLAY_SCREEN_CROSSCOUNTRY_HIKE_CANCEL:
			
			case DISPLAY_SCREEN_INDOORRUN:				//������
			case DISPLAY_SCREEN_INDOORRUN_PAUSE:
			case DISPLAY_SCREEN_INDOORRUN_CANCEL:
				
			case DISPLAY_SCREEN_WALK:					//����
			case DISPLAY_SCREEN_WALK_PAUSE:
			case DISPLAY_SCREEN_WALK_CANCEL:
				
			case DISPLAY_SCREEN_MARATHON:				//������
			case DISPLAY_SCREEN_MARATHON_PAUSE:
			case DISPLAY_SCREEN_MARATHON_CANCEL:
				
			case DISPLAY_SCREEN_TRIATHLON_SWIMMING:		//��������
			case DISPLAY_SCREEN_TRIATHLON_CYCLING:
			case DISPLAY_SCREEN_TRIATHLON_RUNNING:
			case DISPLAY_SCREEN_TRIATHLON_PAUSE:
			case DISPLAY_SCREEN_TRIATHLON_CANCEL:
			case DISPLAY_SCREEN_TRIATHLON_FIRST_CHANGE:
			case DISPLAY_SCREEN_TRIATHLON_SECOND_CHANGE:
				if(g_sport_status == SPORT_STATUS_READY)
				{
					//׼��״̬����δ��ʼ�˶�
					return false;
				}
				else
				{
					//�����������˶�״̬
					return true;
				}
			default:
				return false;
		}
	}
}


//�ж��Ƿ������˶�׼��ģʽ��  --������
bool IsSportReady(ScreenState_t index)
{
		switch(index)
		{
#if defined WATCH_SIM_SPORT 
#elif defined WATCH_COM_SPORT				
#else			
			case DISPLAY_SCREEN_RUN_READY:

			case DISPLAY_SCREEN_CROSSCOUNTRY_READY:
			case DISPLAY_SCREEN_CLIMBING_READY:
#endif
			case DISPLAY_SCREEN_TRIATHLON:
			case DISPLAY_SCREEN_TRIATHLON_CYCLING_READY:
			case DISPLAY_SCREEN_TRIATHLON_RUNNING_READY:
			
				return true;

#if  defined WATCH_COM_SPORT
			case DISPLAY_SCREEN_RUN:
			case DISPLAY_SCREEN_CROSSCOUNTRY:
			case DISPLAY_SCREEN_CLIMBING:
			case DISPLAY_SCREEN_SWIMMING:
			//case DISPLAY_SCREEN_SWIM_POOL_SET:				//��Ӿ���ȸ���	

          /*��������*/
			case DISPLAY_SCREEN_SPORT_READY://�����˶�׼������
			
			case DISPLAY_SCREEN_SPORT_DATA_DISPLAY_PREVIEW:		//�����˶�������ʾԤ������
			case DISPLAY_SCREEN_SPORT_DATA_DISPLAY:		//�����˶�������ʾ����
			case DISPLAY_SCREEN_SPORT_DATA_DISPLAY_SET:		//�����˶�������ʾ���ý���
			case DISPLAY_SCREEN_SPORT_REMIND_SET:			//�����˶��������ý���
	
			case DISPLAY_SCREEN_SPORT_REMIND_SET_HAERT:					//���ʸ澯���ý���
			case DISPLAY_SCREEN_SPORT_REMIND_SET_PACE:				//�����������ý���
			case DISPLAY_SCREEN_SPORT_REMIND_SET_DISTANCE:			//�����������ý���
			case DISPLAY_SCREEN_SPORT_REMIND_SET_AUTOCIRCLE:				//�Զ���Ȧ���ý���
			case DISPLAY_SCREEN_SPORT_REMIND_SET_COALKCAL:			//ȼ֬Ŀ�����ý���
			case DISPLAY_SCREEN_SPORT_REMIND_SET_TIME:				//ʱ���������ý���
			case DISPLAY_SCREEN_SPORT_REMIND_SET_SPEED:				//�ٶ��������ý���
			case DISPLAY_SCREEN_SPORT_REMIND_SET_SWIMPOOL:				//Ӿ�س������ý���
			case DISPLAY_SCREEN_SPORT_REMIND_SET_GOALCIRCLE:
			case DISPLAY_SCREEN_SPORT_REMIND_SET_INDOORLENG:

			
#endif
			case DISPLAY_SCREEN_CYCLING:			//����
			case DISPLAY_SCREEN_CROSSCOUNTRY_HIKE:	//ͽ��ԽҰ
			case DISPLAY_SCREEN_INDOORRUN:			//������
			case DISPLAY_SCREEN_WALK:				//����
			case DISPLAY_SCREEN_MARATHON:			//������
			case DISPLAY_SCREEN_TRIATHLON_SWIMMING:	//��������
			case DISPLAY_SCREEN_TRIATHLON_CYCLING:
			case DISPLAY_SCREEN_TRIATHLON_RUNNING:
				if(g_sport_status == SPORT_STATUS_READY)
				{
					return true;
				}
				else
				{
					return false;
				}
			default:
				return false;
		}
}

//�Ƿ������˶�ģʽ   --������

//�Ƿ������������˶�ģʽ 
bool IsTriathlonMode(ScreenState_t index)
{
	bool status = false;
	switch(index)
	{
		case DISPLAY_SCREEN_TRIATHLON_SWIMMING:		//��������
		case DISPLAY_SCREEN_TRIATHLON_CYCLING:
		case DISPLAY_SCREEN_TRIATHLON_RUNNING:
		case DISPLAY_SCREEN_TRIATHLON_PAUSE:
		case DISPLAY_SCREEN_TRIATHLON_CANCEL:
		case DISPLAY_SCREEN_TRIATHLON_FIRST_CHANGE:
		case DISPLAY_SCREEN_TRIATHLON_SECOND_CHANGE:
			status = true;
			break;
		default:
			break;
	}
	return status;
}
/*����˵��:�Ƿ����˶������ܽ���� */
bool is_sport_record_detail_screen(uint32_t m_screen)
{
	bool m_status = false;
	switch( m_screen )
	{
		/*���������ܽ����ǰ����İ�������  ����ڸ��˶��������������ʱ���*/

		/*����Ϊ�����ܽ���水������*/
		//��Ӿ
#if (defined WATCH_SIM_SPORT || defined WATCH_COM_SPORT)
		//�ܲ�
		case DISPLAY_SCREEN_RUN_DETAIL:

		case DISPLAY_SCREEN_SWIM_DETAIL:
#else
		case DISPLAY_SCREEN_SWIM_SAVE_DETIAL_1:
		case DISPLAY_SCREEN_SWIM_SAVE_DETIAL_2:
		case DISPLAY_SCREEN_SWIM_SAVE_HEARTRATEZONE:
		case DISPLAY_SCREEN_RUN_SAVE_DETIAL_1:
		case DISPLAY_SCREEN_RUN_SAVE_DETIAL_2:
		case DISPLAY_SCREEN_RUN_SAVE_TRACK:
		case DISPLAY_SCREEN_RUN_SAVE_HEARTRATEZONE:
#endif
		//ԽҰ��
#if defined WATCH_COM_SPORT
		case DISPLAY_SCREEN_CROSSCOUNTRY_DETAIL:
#else
		case DISPLAY_SCREEN_CROSSCOUNTRY_SAVE_DETIAL_1:
		case DISPLAY_SCREEN_CROSSCOUNTRY_SAVE_DETIAL_2:
		case DISPLAY_SCREEN_CROSSCOUNTRY_SAVE_DETIAL_3:
		case DISPLAY_SCREEN_CROSSCOUNTRY_SAVE_TRACK:
		case DISPLAY_SCREEN_CROSSCOUNTRY_SAVE_HEARTRATEZONE:
#endif
		//����
		case DISPLAY_SCREEN_CYCLING_DETAIL:
		//������
		case DISPLAY_SCREEN_INDOORRUN_DETAIL:
		//����
		case DISPLAY_SCREEN_WALK_DETAIL:
		//������
		case DISPLAY_SCREEN_MARATHON_DETAIL:
		//ͽ��ԽҰ
		case DISPLAY_SCREEN_CROSSCOUNTRY_HIKE_DETAIL:
		//��ɽ
#if defined WATCH_COM_SPORT
		case DISPLAY_SCREEN_CLIMBING_DETAIL:
#else
		case DISPLAY_SCREEN_CLIMBING_SAVE_DETIAL_1:
		case DISPLAY_SCREEN_CLIMBING_SAVE_DETIAL_2:
		case DISPLAY_SCREEN_CLIMBING_SAVE_TRACK:
		case DISPLAY_SCREEN_CLIMBING_SAVE_HEARTRATEZONE:
#endif
		//��������
		case DISPLAY_SCREEN_TRIATHLON_DETAIL:
		{
			m_status = true;
		}
			break;
		default:
			break;
	}
	
	return m_status;
}
//������������
void SetSubjectiveEvaluate(uint8_t feel)
{
    ActivityData.ActivityDetails.CommonDetail.SubjectiveEvaluate = feel;
}
/**************************************
����Ϊ�����˶���¼���ݱ���
***********************************/
/*�Ʋ�
*ÿ������4�ֽڣ�Ϊ��ǰ�ܲ�������λΪ����ÿ���Ӵ洢һ������
*/
void	Store_StepData( uint32_t * p_step )
{
#ifndef COD

	dev_extFlash_enable();
	dev_extFlash_write( SetValue.CurrentAddress.AddressStep,(uint8_t*)(p_step),STORE_STEP_BYTE_SIZE);
	dev_extFlash_disable();
	SetValue.CurrentAddress.AddressStep += STORE_STEP_BYTE_SIZE;
	if( ( SetValue.CurrentAddress.AddressStep & 0x00000FFF ) == 0) //��ַ��4K�߽�
	{
		if( SetValue.CurrentAddress.AddressStep >= HEARTRATE_DATA_START_ADDRESS )
		{
			SetValue.CurrentAddress.AddressStep = STEP_DATA_START_ADDRESS;
		}
		dev_extFlash_enable();
		dev_extFlash_erase(SetValue.CurrentAddress.AddressStep,STORE_STEP_BYTE_SIZE);//����4k��4k
		dev_extFlash_disable();
	}
#endif
}
#ifdef COD
/*�Ʋ�
*ÿ������4�ֽڣ�Ϊ��ǰ�ܲ�������λΪ����ÿ���Ӵ洢һ������
 ʱ��
 ÿ������4�ֽڣ�Ϊ��ǰ�˶���ʱ�䣬��λΪs��ÿ���Ӵ洢һ������
*/
void  Store_cod_StepData( uint32_t * p_step,uint32_t *time)
{
	dev_extFlash_enable();
	dev_extFlash_write( SetValue.CurrentAddress.AddressStep,(uint8_t*)(p_step),STORE_STEP_BYTE_SIZE);
	dev_extFlash_write( SetValue.CurrentAddress.AddressStep+4,(uint8_t*)(time),STORE_STEP_BYTE_SIZE);
	dev_extFlash_disable();
	SetValue.CurrentAddress.AddressStep += (STORE_STEP_BYTE_SIZE *2);
	if( ( SetValue.CurrentAddress.AddressStep & 0x00000FFF ) == 0) //��ַ��4K�߽�
	{
		if( SetValue.CurrentAddress.AddressStep >= (STEP_DATA_STOP_ADDRESS +1) )
		{
			SetValue.CurrentAddress.AddressStep = STEP_DATA_START_ADDRESS;
		}
		dev_extFlash_enable();
		dev_extFlash_erase(SetValue.CurrentAddress.AddressStep,STORE_STEP_BYTE_SIZE);//����4k��4k
		dev_extFlash_disable();
	}
}
/*����
* ÿ������һ�ֽڣ���λΪ��/�֣�ÿ20s�洢һ������
ʱ��
ÿ������4�ֽڣ�Ϊ��ǰ�˶���ʱ�䣬��λΪs��ÿ20s�洢һ������

*/
void Store_cod_HeartRateData( uint8_t *hdr_value ,uint32_t *time )
{
	dev_extFlash_enable();
	dev_extFlash_write( SetValue.CurrentAddress.AddressHeartRate,(uint8_t*)(hdr_value),DAY_BASE_HDR_LENGTH);
	dev_extFlash_write( SetValue.CurrentAddress.AddressHeartRate +1,(uint8_t*)(time),STORE_STEP_BYTE_SIZE);
	dev_extFlash_disable();
	SetValue.CurrentAddress.AddressHeartRate += (STORE_STEP_BYTE_SIZE *2);
	if( ( SetValue.CurrentAddress.AddressHeartRate & 0x00000FFF ) == 0) //��ַ��4K�߽�
	{
		if( SetValue.CurrentAddress.AddressHeartRate >= (HEARTRATE_DATA_STOP_ADDRESS +1) )
		{			
			SetValue.CurrentAddress.AddressHeartRate = HEARTRATE_DATA_START_ADDRESS;
		}
		dev_extFlash_enable();
		dev_extFlash_erase(SetValue.CurrentAddress.AddressHeartRate,DAY_BASE_HDR_LENGTH);		
		dev_extFlash_disable();		
	}
}

#endif 
/*����
* ÿ������һ�ֽڣ���λΪ��/�֣�ÿ���Ӵ洢һ������
*/
void Store_HeartRateData( uint8_t *hdr_value )
{
#ifndef COD

	dev_extFlash_enable();
	dev_extFlash_write( SetValue.CurrentAddress.AddressHeartRate,(uint8_t*)(hdr_value),DAY_BASE_HDR_LENGTH);
	dev_extFlash_disable();
	SetValue.CurrentAddress.AddressHeartRate += DAY_BASE_HDR_LENGTH;
	if( ( SetValue.CurrentAddress.AddressHeartRate & 0x00000FFF ) == 0) //��ַ��4K�߽�
	{
		if( SetValue.CurrentAddress.AddressHeartRate >= PRESSURE_DATA_START_ADDRESS )
		{			
			SetValue.CurrentAddress.AddressHeartRate = HEARTRATE_DATA_START_ADDRESS;
		}
		dev_extFlash_enable();
		dev_extFlash_erase(SetValue.CurrentAddress.AddressHeartRate,DAY_BASE_HDR_LENGTH);		
		dev_extFlash_disable();		
	}
#endif
}
/*��ѹ 
ÿ�����ݰ�����ѹ�͸߶�����ֵ��ÿ��ֵ�ĸ��ֽڣ���ѹΪ4�ֽ���������λΪ����
�߶�ֵΪ���ֽڸ���������λΪ�ס�
5���Ӵ洢����
*/
#ifdef COD //������Ŀʵ�����ڹ����ơ�Ӿ�˵�
void  Store_KiloMeter(uint32_t now_distence,uint32_t sec)
{
	uint16_t i,nums;
	if((now_distence >= (kilometer_nums*1000) )&&(kilometer_last < (kilometer_nums*1000)))
	{
		nums =( now_distence - ( kilometer_nums - 1 ) * 1000 ) / 1000;
		kilometer_nums += nums;
		int32_t Lon = GetGpsLon()/10;
		int32_t Lat = GetGpsLat()/10;
		for (i = 0;i < nums ;i++)
		{
			dev_extFlash_enable();
			dev_extFlash_write(SetValue.CurrentAddress.AddressPressure,(uint8_t*)(&sec),STORE_DISTANCEPOINT_BYTE_SIZE);
			dev_extFlash_write(SetValue.CurrentAddress.AddressPressure+STORE_DISTANCEPOINT_BYTE_SIZE,(uint8_t*)(&Lon),STORE_DISTANCEPOINT_BYTE_SIZE);
			dev_extFlash_write(SetValue.CurrentAddress.AddressPressure+STORE_DISTANCEPOINT_BYTE_SIZE*2,(uint8_t*)(&Lat),STORE_DISTANCEPOINT_BYTE_SIZE);
			dev_extFlash_disable();
			SetValue.CurrentAddress.AddressPressure += STORE_DISTANCEPOINT_BYTE_SIZE *4;
			if( ( SetValue.CurrentAddress.AddressPressure & 0x00000FFF ) == 0) //��ַ��4K�߽�
			{
				if( SetValue.CurrentAddress.AddressPressure >= DISTANCE_DATA_START_ADDRESS )
				{			
					SetValue.CurrentAddress.AddressPressure = PRESSURE_DATA_START_ADDRESS;
				}
				dev_extFlash_enable();
				dev_extFlash_erase(SetValue.CurrentAddress.AddressPressure,(STORE_PRESSURE_BYTE_SIZE*2));		
				dev_extFlash_disable();		
			}
		}
	}
	kilometer_last = now_distence;
}
////������Ŀʵ�����ڹ����ơ�Ӿ�˵�
void  Stores_Stroke(uint8_t type,uint8_t laps)
{
	uint32_t timestamp;
	rtc_time_t time;
	
	time.Year= RTC_time.ui32Year;  //����ֹͣʱ��
	time.Month= RTC_time.ui32Month;
	time.Day= RTC_time.ui32DayOfMonth;
	time.Hour = RTC_time.ui32Hour;
	time.Minute = RTC_time.ui32Minute;
	time.Second = RTC_time.ui32Second;
	
	timestamp = GetTimeSecond(time);
		
	dev_extFlash_enable();
	dev_extFlash_write(SetValue.CurrentAddress.AddressPressure,(uint8_t*)(&type),1);
	dev_extFlash_write(SetValue.CurrentAddress.AddressPressure+1,(uint8_t*)(&laps),1);
	dev_extFlash_write(SetValue.CurrentAddress.AddressPressure +2,(uint8_t*)(&timestamp),STORE_DISTANCEPOINT_BYTE_SIZE);
	dev_extFlash_disable();
	SetValue.CurrentAddress.AddressPressure += STORE_DISTANCEPOINT_BYTE_SIZE *4;
	
	if( ( SetValue.CurrentAddress.AddressPressure & 0x00000FFF ) == 0) //��ַ��4K�߽�
	{
		if( SetValue.CurrentAddress.AddressPressure >= DISTANCE_DATA_START_ADDRESS )
		{			
			SetValue.CurrentAddress.AddressPressure = PRESSURE_DATA_START_ADDRESS;
		}
		dev_extFlash_enable();
		dev_extFlash_erase(SetValue.CurrentAddress.AddressPressure,(STORE_PRESSURE_BYTE_SIZE*2));		
		dev_extFlash_disable();		
	}
	
}

#else
void  Store_PressureData(int32_t *pres,float *alt)
{
	dev_extFlash_enable();
	dev_extFlash_write(SetValue.CurrentAddress.AddressPressure,(uint8_t*)(pres),STORE_PRESSURE_BYTE_SIZE);
	dev_extFlash_write(SetValue.CurrentAddress.AddressPressure+STORE_PRESSURE_BYTE_SIZE,(uint8_t*)(alt),STORE_PRESSURE_BYTE_SIZE);
	dev_extFlash_disable();
	SetValue.CurrentAddress.AddressPressure += (STORE_PRESSURE_BYTE_SIZE*2);
	if( ( SetValue.CurrentAddress.AddressPressure & 0x00000FFF ) == 0) //��ַ��4K�߽�
	{
		if( SetValue.CurrentAddress.AddressPressure >= DISTANCE_DATA_START_ADDRESS )
		{			
			SetValue.CurrentAddress.AddressPressure = PRESSURE_DATA_START_ADDRESS;
		}
		dev_extFlash_enable();
		dev_extFlash_erase(SetValue.CurrentAddress.AddressPressure,(STORE_PRESSURE_BYTE_SIZE*2));		
		dev_extFlash_disable();		
	}
}
#endif
/*����
ÿ������Ϊ�ĸ��ֽڣ���λΪcm��
1���Ӵ洢����
*/
void Store_DistanceData( uint32_t *p_dis  )
{
	dev_extFlash_enable();
	dev_extFlash_write( SetValue.CurrentAddress.AddressDistance,(uint8_t*)(p_dis),STORE_DISTANCE_BYTE_SIZE);
	dev_extFlash_disable();
	SetValue.CurrentAddress.AddressDistance += STORE_DISTANCE_BYTE_SIZE;
	if( ( SetValue.CurrentAddress.AddressDistance & 0x00000FFF ) == 0) //��ַ��4K�߽�
	{
		if( SetValue.CurrentAddress.AddressDistance >= ENERGY_DATA_START_ADDRESS )
		{
			SetValue.CurrentAddress.AddressDistance = DISTANCE_DATA_START_ADDRESS;
		}
		dev_extFlash_enable();
		dev_extFlash_erase(SetValue.CurrentAddress.AddressDistance,STORE_DISTANCE_BYTE_SIZE);
		dev_extFlash_disable();		
	}
}
/*���� ��·��
ÿ������Ϊ�ĸ��ֽڣ���λΪ����
1���Ӵ洢����
*/
void Store_EnergyData( uint32_t *caloriesValue)
{
	dev_extFlash_enable();
	dev_extFlash_write( SetValue.CurrentAddress.AddressEnergy,(uint8_t*)(caloriesValue),STORE_CALORIES_BYTE_SIZE);
	dev_extFlash_disable();
	SetValue.CurrentAddress.AddressEnergy += STORE_CALORIES_BYTE_SIZE;
	if( ( SetValue.CurrentAddress.AddressEnergy & 0x00000FFF ) == 0) //��ַ��4K�߽�
	{
		if( SetValue.CurrentAddress.AddressEnergy >= WEATHER_DATA_START_ADDRESS )
		{			
			SetValue.CurrentAddress.AddressEnergy = ENERGY_DATA_START_ADDRESS;
		}
		dev_extFlash_enable();
		dev_extFlash_erase(SetValue.CurrentAddress.AddressEnergy,STORE_CALORIES_BYTE_SIZE);
		dev_extFlash_disable();	
	}
}
/*����
������Э��洢��ʵ�ʳ���Ϊ��31*7=217�ֽڣ�
*/

/*�ٶ�
ÿ������Ϊ�����ֽڣ���λm/s(�����˶���λ)��
1���Ӵ洢����
*/
void Store_SpeedData( uint16_t * p_speed )
{
	dev_extFlash_enable();
	dev_extFlash_write( SetValue.CurrentAddress.AddressSpeed,(uint8_t*)p_speed,STORE_SPEED_BYTE_SIZE);
	dev_extFlash_disable();
	SetValue.CurrentAddress.AddressSpeed += STORE_SPEED_BYTE_SIZE;
	if( ( SetValue.CurrentAddress.AddressSpeed & 0x00000FFF ) == 0) //��ַ��4K�߽�
	{
		if( SetValue.CurrentAddress.AddressSpeed >= DISTANCEPOINT_START_ADDRESS )
		{			
			SetValue.CurrentAddress.AddressSpeed = SPEED_DATA_START_ADDRESS;
		}
		dev_extFlash_enable();
		dev_extFlash_erase(SetValue.CurrentAddress.AddressSpeed,STORE_SPEED_BYTE_SIZE);		
		dev_extFlash_disable();
	}

}

/*
*����:�Զ���Ȧʱ�䣬�켣
*����:now_circledistance ��ǰ����,��λ:��,�洢���뵥λ:cm,ʵ�ʳ���100
      goal_circledistance ��Ȧ����
      sec ��ǰʱ��
     swsport ���˶����أ����ܲ�����:SetValue.SportSet.SwSwimmg 
*����ֵ :��

*��ע:
*/
void Store_AuotCirclle(uint32_t now_circledistance,uint16_t goal_circledistance, uint32_t sec,uint8_t swsport)
{
	uint16_t i,nums;
	if((now_circledistance >= (circledistance_nums*goal_circledistance) )&&(circledistance_last < (circledistance_nums*goal_circledistance)))
	{
		nums =( now_circledistance - ( circledistance_nums - 1 ) * goal_circledistance ) / goal_circledistance;
		circledistance_nums += nums;
		for (i = 0;i < nums ;i++)
		{
			#ifndef COD 
			//������
			Store_DistancePointData(GetGpsLon()/10,GetGpsLat()/10,swsport);
			#endif
		
			//��Ȧʱ��
			
        #ifndef WATCH_GPS_HAEDWARE_ANALYSIS_INFOR
			Store_CircleTimeData(sec,swsport);
        #endif
		}
	}
	circledistance_last = now_circledistance;
	#ifdef COD 
	Store_KiloMeter(now_circledistance,sec);
	#endif
}


/*������
ÿ��������Ϊ8�ֽڣ����Ⱥ�γ�ȸ�ռ4�ֽڣ�Ϊ������������ʵ��ֵ��100000����
������ǰ������Ϊ��������Ϊ������γΪ������γΪ����
*/
void Store_DistancePointData(int32_t Lon,int32_t Lat,uint8_t swsport)
{
	if(((swsport & SW_CIRCLEDISTANCE_MASK) == SW_CIRCLEDISTANCE_MASK)&&(Lon != 0)&&(Lat != 0))
	{
	    dev_extFlash_enable();
	    dev_extFlash_write(SetValue.CurrentAddress.AddressDistancePoint,(uint8_t*)(&Lon),STORE_DISTANCEPOINT_BYTE_SIZE);
	    dev_extFlash_write(SetValue.CurrentAddress.AddressDistancePoint+sizeof(Lon),(uint8_t*)(&Lat),STORE_DISTANCEPOINT_BYTE_SIZE);
	    dev_extFlash_disable();
	    SetValue.CurrentAddress.AddressDistancePoint += (STORE_DISTANCEPOINT_BYTE_SIZE*2);
	    if( ( SetValue.CurrentAddress.AddressDistancePoint & 0x00000FFF ) == 0) //��ַ��4K�߽�
	    {
		    if( SetValue.CurrentAddress.AddressDistancePoint >= CIRCLETIME_START_ADDRESS )
		    {			
			    SetValue.CurrentAddress.AddressDistancePoint = DISTANCEPOINT_START_ADDRESS;
		    }
		    dev_extFlash_enable();
		    dev_extFlash_erase(SetValue.CurrentAddress.AddressDistancePoint,(STORE_DISTANCEPOINT_BYTE_SIZE*2));		
		    dev_extFlash_disable();		
	    }
    }
}
/*��Ȧʱ��  ��λ:��
4���ֽڵ�ʱ��
*/
#ifndef WATCH_GPS_HAEDWARE_ANALYSIS_INFOR

void Store_CircleTimeData(uint32_t sec,uint8_t swsport)
{
	if((swsport & SW_CIRCLEDISTANCE_MASK) == SW_CIRCLEDISTANCE_MASK)
	{
		dev_extFlash_enable();
		dev_extFlash_write( SetValue.CurrentAddress.AddressCircleTime,(uint8_t*)(&sec),sizeof(sec));
		dev_extFlash_disable();
		SetValue.CurrentAddress.AddressCircleTime += sizeof(sec);
		if( ( SetValue.CurrentAddress.AddressCircleTime & 0x00000FFF ) == 0) //��ַ��4K�߽�
		{
			if( SetValue.CurrentAddress.AddressCircleTime >= PAUSE_DATA_START_ADDRESS )
			{
				SetValue.CurrentAddress.AddressCircleTime = CIRCLETIME_START_ADDRESS;
			}
			dev_extFlash_enable();
			dev_extFlash_erase(SetValue.CurrentAddress.AddressCircleTime,sizeof(sec));		
			dev_extFlash_disable();		
		}
	}
}
#endif
/*��ͣʱ��
ÿ������Ϊ16���ֽڣ�ǰ8���ֽ�Ϊ��ͣ��ʼʱ��(ʵ��ռ6�ֽ�)����8���ֽ�Ϊ��ͣ����ʱ��(ʵ��ռ6�ֽ�)
*/
void Store_PauseTimeData(PauseTimeStr *pause)
{
#if 0 //��ͣ�洢�ռ䣬�������ں������
	dev_extFlash_enable();
	dev_extFlash_write( SetValue.CurrentAddress.AddressPauseTime,(uint8_t*)&pause->Pause_Start_Time,6);
	dev_extFlash_write( SetValue.CurrentAddress.AddressPauseTime+8,(uint8_t*)&pause->Pause_Stop_Time,6);
	dev_extFlash_disable();
	SetValue.CurrentAddress.AddressPauseTime += 16;
	if( ( SetValue.CurrentAddress.AddressPauseTime & 0x00000FFF ) == 0) //��ַ��4K�߽�
	{
		if( SetValue.CurrentAddress.AddressPauseTime >= AMBIENT_DATA_START_ADDRESS )
		{			
			SetValue.CurrentAddress.AddressPauseTime = PAUSE_DATA_START_ADDRESS;
		}
		dev_extFlash_enable();
		dev_extFlash_erase(SetValue.CurrentAddress.AddressPauseTime,16);		
		dev_extFlash_disable();		
	}
#endif
}

/*�����������,ʵ��ʹ����ͣʱ��洢�ռ�
ÿ��������Ϊ4�ֽڣ��洢gps��洢���
*/
void Store_DotTrack(uint32_t nums)
{	

	dev_extFlash_enable();
	dev_extFlash_write( SetValue.CurrentAddress.AddressPause,(uint8_t*)(&nums),STORE_GPS_BYTE_SIZE);
	//dev_extFlash_write( SetValue.CurrentAddress.AddressPause+STORE_GPS_BYTE_SIZE,(uint8_t*)(&Lat),STORE_GPS_BYTE_SIZE);
	dev_extFlash_disable();
	SetValue.CurrentAddress.AddressPause += (STORE_GPS_BYTE_SIZE);
	if( ( SetValue.CurrentAddress.AddressPause & 0x00000FFF ) == 0) //��ַ��4K�߽�
	{
		if( SetValue.CurrentAddress.AddressPause >= AMBIENT_DATA_START_ADDRESS )
		{			
			SetValue.CurrentAddress.AddressPause = PAUSE_DATA_START_ADDRESS;
		}
		dev_extFlash_enable();
		dev_extFlash_erase(SetValue.CurrentAddress.AddressPause,(STORE_GPS_BYTE_SIZE*2));		
		dev_extFlash_disable();		
	}

}

#ifdef COD 
uint8_t last_pausesta = 0xFF;
uint32_t gps_store_num = 0;
int32_t last_store_Lon = 0;
int32_t last_store_Lat = 0;
float last_sport_dis = 0;

float last_altitude = 0;
float temp_dis = 0;
float last_temp_dis = 0;

float temp_increase_dis = 0;
GpspointStr pre_gps_point[OUTPUT_DATA_SUM_PRE*2];

float algo_last_press_and_altitude[2] = {101.325,0};

uint32_t pre_distance[OUTPUT_DATA_SUM_PRE*2];

int16_t start_alt = 0;




void Store_GPSData(GpspointStr gps_point,uint32_t distance)
{
		dev_extFlash_enable();
		dev_extFlash_write( SetValue.CurrentAddress.AddressGps,(uint8_t*)(&gps_point),STORE_GPS_BYTE_SIZE*4);
		dev_extFlash_disable();
		SetValue.CurrentAddress.AddressGps += (STORE_GPS_BYTE_SIZE*4);
		if(( SetValue.CurrentAddress.AddressGps & 0x00000FFF ) == 0) //��ַ��4K�߽�
		{
			if( SetValue.CurrentAddress.AddressGps >= GPSTRACK_DATA_START_ADDRESS )
			{			
			SetValue.CurrentAddress.AddressGps = GPS_DATA_START_ADDRESS;
			}
			dev_extFlash_enable();
			dev_extFlash_erase(SetValue.CurrentAddress.AddressGps,(STORE_GPS_BYTE_SIZE*2));		
			dev_extFlash_disable();	
   		}
		last_store_Lon = gps_point.Lon;
		last_store_Lat = gps_point.Lat;
		
		//����
		dev_extFlash_enable();
		dev_extFlash_write( SetValue.CurrentAddress.AddressDistancePoint,(uint8_t*)(&distance),STORE_DISTANCE_BYTE_SIZE);
		dev_extFlash_disable();
		SetValue.CurrentAddress.AddressDistancePoint += STORE_DISTANCE_BYTE_SIZE;
		if( ( SetValue.CurrentAddress.AddressDistancePoint & 0x00000FFF ) == 0) //��ַ��4K�߽�
		{
			if( SetValue.CurrentAddress.AddressDistancePoint >= (DISTANCEPOINT_STOP_ADDRESS+ 1) )
			{			
				SetValue.CurrentAddress.AddressDistancePoint = DISTANCEPOINT_START_ADDRESS;
			}
			dev_extFlash_enable();
			dev_extFlash_erase(SetValue.CurrentAddress.AddressDistancePoint,STORE_DISTANCE_BYTE_SIZE);		
			dev_extFlash_disable();		
		}
		
}
void cod_bubble_sort(float *data, int n) 
{
    int i, j;
    float temp;
    for (j = 0; j < n - 1; j++)
    {
       for (i = 0; i < n - 1 - j; i++)
       {
          if(data[i] > data[i + 1])
          {
             temp=data[i];
             data[i]=data[i+1]; 
             data[i+1]=temp;
          }
       }
    }
}



//pausesta��ͣ״̬(����),altitudeʵʱ���� ��RiseHeight�ۼ�����(�����߶�)��Risestate����״̬ ,RiseAlt��������
void cod_store_gpsdata(uint8_t pausesta,int16_t *altitude,float *RiseHeight,float *RiseDrop,uint8_t * Risestate,int16_t *RiseAlt,ActivityDataStr*sport_type_data)
{
	rtc_time_t start;
	float pres,alt,temp,temp_altitude,temp_RiseHeight,temp_RiseDrop,start_zero_altitude,start_zero_drv_alt,start_zero_drv_pres;
     uint8_t i,j;
	 float dis_limit = 2.0,update_alt_rise=2.0;
	*Risestate = 0;
	uint8_t del_gps_point_num = 0;
	float two_point_dis = 0;
	temp_increase_dis = 0;
	//if (GetGpsStatus() == true)

if (GetGpsStatus() == true)



	{
		//��γ��
		gps_point.Lon = GetGpsLon()/10; 
		gps_point.Lat = GetGpsLat()/10;
		if((sport_type_data->Activity_Type==ACT_RUN)||(sport_type_data->Activity_Type==ACT_TRIATHLON_RUN)||(sport_type_data->Activity_Type==ACT_TRAIN_PLAN_RUN))
			{
			temp_dis = sport_type_data->ActivityDetails.RunningDetail.Distance/100.f;
          	dis_limit = 2.0;
			update_alt_rise = UPDATE_ALTITUDE_RISE_ELSE;
			
		   }
		else if(sport_type_data->Activity_Type==ACT_CYCLING)
			{
             
			 temp_dis = sport_type_data->ActivityDetails.CyclingDetail.Distance/100.f;
			 dis_limit = 2.5;
			 update_alt_rise = UPDATE_ALTITUDE_RISE_CYCLING;
		
			  }
		else if(sport_type_data->Activity_Type==ACT_CLIMB)
			{
             
			 temp_dis = sport_type_data->ActivityDetails.ClimbingDetail.Distance/100.f;
			 dis_limit = 1.5;
			 update_alt_rise = UPDATE_ALTITUDE_RISE_CLIMB;	 
		    }
		else if((sport_type_data->Activity_Type==ACT_HIKING)||(sport_type_data->Activity_Type==ACT_CROSSCOUNTRY))
			{
             
			 temp_dis = sport_type_data->ActivityDetails.CrosscountryDetail.Distance/100.f;
			 dis_limit = 2.0;
             update_alt_rise = UPDATE_ALTITUDE_RISE_ELSE;
	 
		   }
		
		else;

		
		//��λ����
		gps_point.accuracy = GetGpspdop()/100;
		//������
		if ((pausesta == 0)&&(last_pausesta != 1))//pausesta 0:�˶�;1:��ͣ
		{
			gps_point.point_type = COD_GPS_POINT_NORMAL;
			last_pausesta = pausesta;
		}
		else if((pausesta == 0)&&(last_pausesta == 1))
		{
			gps_point.point_type = COD_GPS_POINT_RESUME;
			last_pausesta = pausesta;
		}
		else if ((pausesta == 1)&&(last_pausesta == 0))
		{
			gps_point.point_type = COD_GPS_POINT_PAUSE;
			last_pausesta = pausesta;
		}
		else
		{
			last_pausesta = pausesta;
			return;
		}
			//ʱ��� s
		start.Year= RTC_time.ui32Year;  
		start.Month= RTC_time.ui32Month;
		start.Day= RTC_time.ui32DayOfMonth;
		start.Hour = RTC_time.ui32Hour;
		start.Minute = RTC_time.ui32Minute;
		start.Second = RTC_time.ui32Second;
	    gps_point.timestamp = GetTimeSecond(start);

		//2������
		two_point_dis = temp_dis- last_sport_dis;
		if(gps_store_num < 6)
		{
           
		   drv_ms5837_data_get(&pres,&alt,&temp);
		   temp_altitude = GetGpsAlt()*0.1;
		   gps_point.altitude = GetGpsAlt()/10;
           start_temp_altitude[gps_store_num] = temp_altitude;
		   start_temp_drv_pres[gps_store_num] = pres;
		   start_temp_drv_alt[gps_store_num] = alt;
		   pre_gps_point[gps_store_num] = gps_point;
		   pre_distance[gps_store_num] = (uint32_t)(two_point_dis*100);
			if(gps_store_num ==0)
			{
				start_alt = gps_point.altitude;
				last_altitude = temp_altitude;
			}
			
	
		}
		else if (gps_store_num == 6)
		{

		start_temp_altitude[gps_store_num] = GetGpsAlt()*0.1;
        cod_bubble_sort(start_temp_altitude,7);
		start_zero_altitude = 0;
		 for(i=2;i<5;i++)
		 	{
			   start_zero_altitude += start_temp_altitude[i];
		 	}
		    start_zero_altitude = start_zero_altitude/3.f;
            drv_ms5837_data_get(&pres,&alt,&temp);
			start_temp_drv_pres[gps_store_num] = pres;
			start_temp_drv_alt[gps_store_num] = alt;
			 cod_bubble_sort(start_temp_drv_pres,7);
			start_zero_drv_pres = 0;
             for(i=2;i<5;i++)
		 	 {
			   start_zero_drv_pres += start_temp_drv_pres[i];
		 	 }
		      pres = start_zero_drv_pres/3.f;
			  
            cod_bubble_sort(start_temp_drv_alt,7);
			start_zero_drv_alt = 0;
            for(i=2;i<5;i++)
		 	 {
			   start_zero_drv_alt += start_temp_drv_alt[i];
		 	 }
		      alt = start_zero_drv_alt/3.f;
			
			
			if ((pres > 300)&&(pres < 1300))
			{
			   		   	
				SetValue.Altitude_Offset = start_zero_altitude - alt;
				temp_altitude = alt + SetValue.Altitude_Offset;
				gps_point.altitude = (int16_t)(alt + SetValue.Altitude_Offset);
				start_alt = gps_point.altitude;
				last_altitude = temp_altitude;

				pre_gps_point[gps_store_num]= gps_point;
				pre_distance[gps_store_num] = (uint32_t)(two_point_dis *100);
			}
			else
			{
				SetValue.Altitude_Offset = 0;//�쳣ʱ��У׼
				temp_altitude = start_zero_altitude;//�쳣ʱʹ��ƽ��ֵ
				gps_point.altitude = (int16_t)(start_zero_altitude);
				/*�쳣ʱʹ��GPS�߶ȷ�����ѹ��ֵ*/
				alt = temp_altitude;
				pres = algo_altitude_to_press(alt,SetValue.AppSet.Sea_Level_Pressure);
				start_alt = gps_point.altitude;
				last_altitude = temp_altitude;

				pre_gps_point[gps_store_num]= gps_point;
				pre_distance[gps_store_num] = (uint32_t)(two_point_dis *100);
			}
		
			
		}
		else
		{
			drv_ms5837_data_get(&pres,&alt,&temp);
			if ((pres > 300)&&(pres < 1300))
			{
			     if(((alt-algo_last_press_and_altitude[1])>MAX_RISE_EXCEPTION)||((alt-algo_last_press_and_altitude[1])<(-MAX_DECLINE_EXCEPTION)))//�������½�ֵ�쳣
			        {
                         alt = 0.5*(GetGpsAlt()/10) + 0.5*algo_last_press_and_altitude[1];//��ѹ���쳣ʱ�߶���GPS�߶�ֵ����
                         pres = algo_altitude_to_press(alt,SetValue.AppSet.Sea_Level_Pressure);
                         temp_altitude = alt;
				         gps_point.altitude = (int16_t)alt;
					 }
				else
					 {
			    
						temp_altitude = alt + SetValue.Altitude_Offset;
						gps_point.altitude = (int16_t)(alt + SetValue.Altitude_Offset);
					 }
			}
			else
			{
				temp_altitude = last_altitude;//�쳣ʱʹ����һ��ֵ
				gps_point.altitude = (int16_t)(last_altitude);

				/*�쳣ʱʹ��GPS�߶ȷ�����ѹ��ֵ*/
				alt = temp_altitude;
				pres = algo_altitude_to_press(alt,SetValue.AppSet.Sea_Level_Pressure);
			}

		}

	        algo_last_press_and_altitude[0] = pres;
			
			algo_last_press_and_altitude[1] = alt;
	
      

			if (((temp_altitude -last_altitude) >(update_alt_rise + AIR_PRESSURE_DEVIATION))&&(gps_store_num>=6))
			{
				temp_RiseHeight = *RiseHeight;
			    *RiseHeight = temp_RiseHeight + (temp_altitude -last_altitude );	
				*Risestate = 1;
				temp_increase_dis = temp_dis - last_temp_dis;
			}

			if(((temp_altitude -last_altitude) < (-UPDATE_ALTITUDE_DECLINE-AIR_PRESSURE_DEVIATION))&&(gps_store_num>=6))
			{
				temp_RiseDrop = *RiseDrop;
			    *RiseDrop = temp_RiseDrop + (last_altitude - temp_altitude);	
			}
         
		   if((((fabs(temp_altitude -last_altitude))<(UPDATA_POINT_THRESHOLD+AIR_PRESSURE_DEVIATION))||
				((temp_altitude -last_altitude)<(-UPDATA_POINT_THRESHOLD-AIR_PRESSURE_DEVIATION)))&&(gps_store_num>=6))
				{
                 
				 last_temp_dis = temp_dis;

			    }
			else;
			
			if ((((temp_altitude -last_altitude) > (update_alt_rise+AIR_PRESSURE_DEVIATION))||
				((temp_altitude -last_altitude) < (-UPDATE_ALTITUDE_DECLINE-AIR_PRESSURE_DEVIATION)))
                &&(gps_store_num>=6))
			{
				last_altitude = temp_altitude;
			}
		
			*altitude = gps_point.altitude;
			
			if(gps_store_num>=6)
				{
			      *RiseAlt = gps_point.altitude - start_alt;
				}
			else;
			if(gps_store_num==6)
				{ 
					for(i=0;i<7;i++)
					{
					 
					  pre_gps_point[i].altitude = gps_point.altitude;
					  
					  cod_gps_curpoint_num++; 
					 
					}
					                       					
							if(gps_data_output.output_data_sum > SMOOTH_DATA_SUM)
							{
								
								gps_data_output.output_data_sum = SMOOTH_DATA_SUM;
							}
						
							for(j = 0; j < gps_data_output.output_data_sum; j++)
							{

							   pre_gps_point[j].Lon = gps_data_output.data[j][0];
							   pre_gps_point[j].Lat = gps_data_output.data[j][1];
							   
							   Store_GPSData(pre_gps_point[j],pre_distance[j]);

							  // COM_SPORT_PRINTF("%d,%d\r\n", pre_gps_point[j].Lon, pre_gps_point[j].Lat);
							}
						
						del_gps_point_num = j;
						if(del_gps_point_num>cod_gps_curpoint_num)
							{
                             del_gps_point_num = cod_gps_curpoint_num;
						   }
                      cod_gps_curpoint_num = cod_gps_curpoint_num - del_gps_point_num;
					
					  for(j=0;j<cod_gps_curpoint_num;j++)
					  	{
                          
                           pre_gps_point[j] = pre_gps_point[j+del_gps_point_num];
						   pre_distance[j]  = pre_distance[j+del_gps_point_num];

					    }
							
				}
			else if(gps_store_num>6)
				{
                   pre_gps_point[cod_gps_curpoint_num] = gps_point;
                  pre_distance[cod_gps_curpoint_num] = (uint32_t)(two_point_dis*100);
				  cod_gps_curpoint_num++; 
				   
				         if(gps_data_output.output_data_sum > OUTPUT_DATA_SUM_PRE)
							{
								
								gps_data_output.output_data_sum = OUTPUT_DATA_SUM_PRE;
							}
						for(j = 0; j < gps_data_output.output_data_sum; j++)
							{

							   pre_gps_point[j].Lon = gps_data_output.data[j][0];
							   pre_gps_point[j].Lat = gps_data_output.data[j][1];
							   
							   Store_GPSData(pre_gps_point[j],pre_distance[j]);
							   
							 //  COM_SPORT_PRINTF("%d,%d\r\n",pre_gps_point[j].Lon, pre_gps_point[j].Lat);
							}
							del_gps_point_num = j;
						if(del_gps_point_num>cod_gps_curpoint_num)
							{
                             del_gps_point_num = cod_gps_curpoint_num;
						   }
                      cod_gps_curpoint_num = cod_gps_curpoint_num - del_gps_point_num;
					
					  for(j=0;j<cod_gps_curpoint_num;j++)
					  	{
                          
                           pre_gps_point[j] = pre_gps_point[j+del_gps_point_num];
						   
						   pre_distance[j]  = pre_distance[j+del_gps_point_num];

					    }

			     }
			else;
		
           last_sport_dis = temp_dis;
		   gps_store_num++;
			
		
	}
	
}


#else
/*�GPS����
ÿ��������Ϊ8�ֽڣ����Ⱥ�γ�ȸ�ռ4�ֽڣ�Ϊ������������ʵ��ֵ��100000����
������ǰ������Ϊ��������Ϊ������γΪ������γΪ����

*/
void Store_GPSData(int32_t Lon,int32_t Lat)
{
   if ((Lon != 0)&&(Lat != 0))
   {
#if defined WATCH_SPORT_NEW_CLOUD_NAVIGATION
		if (is_crosscountry == 1) //�ж��Ƿ�ԽҰ
		{
			gps_store_flag = ~gps_store_flag;//����洢
		}
		else
		{
			gps_store_flag = true;
		}
		if (gps_store_flag == true)
		{
			Lon = Lon/10;
			Lat = Lat/10;
			dev_extFlash_enable();
			dev_extFlash_write( SetValue.CurrentAddress.AddressGps,(uint8_t*)(&Lon),STORE_GPS_BYTE_SIZE);
			dev_extFlash_write( SetValue.CurrentAddress.AddressGps+STORE_GPS_BYTE_SIZE,(uint8_t*)(&Lat),STORE_GPS_BYTE_SIZE);
			dev_extFlash_disable();
			SetValue.CurrentAddress.AddressGps += (STORE_GPS_BYTE_SIZE*2);
			if( ( SetValue.CurrentAddress.AddressGps & 0x00000FFF ) == 0) //��ַ��4K�߽�
			{
			if( SetValue.CurrentAddress.AddressGps >= GPSTRACK_DATA_START_ADDRESS )
			{			
				SetValue.CurrentAddress.AddressGps = GPS_DATA_START_ADDRESS;
			}
			dev_extFlash_enable();
			dev_extFlash_erase(SetValue.CurrentAddress.AddressGps,(STORE_GPS_BYTE_SIZE*2));		
			dev_extFlash_disable();		
	}
		}
#else
	Lon = Lon/10;
	Lat = Lat/10;
	dev_extFlash_enable();
	dev_extFlash_write( SetValue.CurrentAddress.AddressGps,(uint8_t*)(&Lon),STORE_GPS_BYTE_SIZE);
	dev_extFlash_write( SetValue.CurrentAddress.AddressGps+STORE_GPS_BYTE_SIZE,(uint8_t*)(&Lat),STORE_GPS_BYTE_SIZE);
	dev_extFlash_disable();
	SetValue.CurrentAddress.AddressGps += (STORE_GPS_BYTE_SIZE*2);
	if( ( SetValue.CurrentAddress.AddressGps & 0x00000FFF ) == 0) //��ַ��4K�߽�
	{
	if( SetValue.CurrentAddress.AddressGps >= GPSTRACK_DATA_START_ADDRESS )
	{			
		SetValue.CurrentAddress.AddressGps = GPS_DATA_START_ADDRESS;
	}
	dev_extFlash_enable();
	dev_extFlash_erase(SetValue.CurrentAddress.AddressGps,(STORE_GPS_BYTE_SIZE*2));		
	dev_extFlash_disable();		
	}
#endif

   }
	
}
#endif

#ifdef STORE_GPS_LOG_DATA_TEST

void store_log_data(uint8_t *log ,uint32_t len)
{

	//��ֹ�ڴ�Խ��
	if(SetValue.CurrentAddress.AddressTouchDown +len > TOUCHDOWN_STOP_ADDRESS)
	{
		return;
	}
	
	dev_extFlash_enable();
	dev_extFlash_write(SetValue.CurrentAddress.AddressTouchDown,(uint8_t*)(log),len);
	dev_extFlash_disable();
	
	SetValue.CurrentAddress.AddressTouchDown += (len);
	
	if( ( SetValue.CurrentAddress.AddressTouchDown & 0x00000FFF ) == 0) //��ַ��4K�߽�
	{
		
	//if( SetValue.CurrentAddress.AddressTouchDown >= TOUCHDOWN_START_ADDRESS )
	//{			
	//	SetValue.CurrentAddress.AddressTouchDown = TOUCHDOWN_START_ADDRESS;
	//}
		
	//dev_extFlash_enable();
	//dev_extFlash_erase(SetValue.CurrentAddress.AddressTouchDown,(STORE_GPS_BYTE_SIZE*2));		
	//dev_extFlash_disable();		
	}	
}

void read_log_data(void)
{
    uint8_t log[256];
    uint32_t len ;
	
    len = SetValue.CurrentAddress.AddressTouchDown - TOUCHDOWN_START_ADDRESS;
    COM_SPORT_PRINTF("read_log_data\n");
    for(uint16_t i= 0;i <100;i++ )
    {
	dev_extFlash_enable();
	dev_extFlash_read(TOUCHDOWN_START_ADDRESS+256*i,(uint8_t*)(log),256);
	dev_extFlash_disable();
	for(uint16_t j= 0;j <256;j++ )
	{
	  if (log[j] == 0xFF)  
	    return;
	  COM_SPORT_PRINTF("%c",log[j]);
	}
	vTaskDelay(200);
     }

}

#endif

#if defined WATCH_GPS_HAEDWARE_ANALYSIS_INFOR

void Store_OrgGPSData(int32_t Lon, int32_t Lat, uint8_t status, uint8_t g_sn, uint8_t bd_sn,uint8_t possl_gpsnum,uint8_t possl_bdnum)
{
	//��ֹ�ڴ�Խ��
	if(SetValue.CurrentAddress.AddressCircleTime > TOUCHDOWN_START_ADDRESS - 30)
	{
		return;
	}
	dev_extFlash_enable();
	dev_extFlash_write( SetValue.CurrentAddress.AddressCircleTime,(uint8_t*)(&Lon),STORE_GPS_BYTE_SIZE);
	dev_extFlash_write( SetValue.CurrentAddress.AddressCircleTime+STORE_GPS_BYTE_SIZE,(uint8_t*)(&Lat),STORE_GPS_BYTE_SIZE);
	dev_extFlash_write( SetValue.CurrentAddress.AddressCircleTime+2*STORE_GPS_BYTE_SIZE,(uint8_t*)(&status),STORE_HEARTRATE_BYTE_SIZE);
	dev_extFlash_write( SetValue.CurrentAddress.AddressCircleTime+2*STORE_GPS_BYTE_SIZE+STORE_HEARTRATE_BYTE_SIZE,(uint8_t*)(&g_sn),STORE_HEARTRATE_BYTE_SIZE);
	dev_extFlash_write( SetValue.CurrentAddress.AddressCircleTime+2*STORE_GPS_BYTE_SIZE+2*STORE_HEARTRATE_BYTE_SIZE,(uint8_t*)(&bd_sn),STORE_HEARTRATE_BYTE_SIZE);
	dev_extFlash_write( SetValue.CurrentAddress.AddressCircleTime+2*STORE_GPS_BYTE_SIZE+3*STORE_HEARTRATE_BYTE_SIZE,(uint8_t*)(&possl_gpsnum),STORE_HEARTRATE_BYTE_SIZE);
	dev_extFlash_write( SetValue.CurrentAddress.AddressCircleTime+2*STORE_GPS_BYTE_SIZE+4*STORE_HEARTRATE_BYTE_SIZE,(uint8_t*)(&possl_bdnum),STORE_HEARTRATE_BYTE_SIZE);
	
	dev_extFlash_disable();
	SetValue.CurrentAddress.AddressCircleTime += (2*STORE_GPS_BYTE_SIZE+5*STORE_HEARTRATE_BYTE_SIZE+3*STORE_HEARTRATE_BYTE_SIZE);
	if( ( SetValue.CurrentAddress.AddressCircleTime & 0x00000FFF ) == 0) //��ַ��4K�߽�
	{
		
	//if( SetValue.CurrentAddress.AddressTouchDown >= TOUCHDOWN_START_ADDRESS )
	//{			
	//	SetValue.CurrentAddress.AddressTouchDown = TOUCHDOWN_START_ADDRESS;
	//}
		
	dev_extFlash_enable();
	dev_extFlash_erase(SetValue.CurrentAddress.AddressCircleTime,(STORE_GPS_BYTE_SIZE*2));		
	dev_extFlash_disable();		
	}	

}
#endif




#if defined STORE_ORG_DATA_TEST_VERSION
/*
���ݴ洢��־��liv_storedataflag��Ĭ��Ϊ1;
Store_OrgGPSData()ԭʼ�GPS���ݴ洢��g_storedataflag = 1;
Store_OrgAxisData()ԭʼ�������ݴ洢��g_storedataflag = 2;
Store_OrgPresData()ԭʼѹǿ���ݴ洢��g_storedataflag = 3;
Store_OrgHeartRateData()ԭʼ�������ݴ洢��g_storedataflag = 4.
*/
static uint8_t g_storedataflag = 1;

void Set_StoreData_Flag(uint8_t liv_storedataflag)
{
	g_storedataflag = liv_storedataflag;
}


///*
//VerticalSize�洢�飬0x00400000-0x004fffff
void Store_OrgData_Start()
{
	//��֤�˶��׵�ַ4���ֽڶ���
	uint8_t liv_complbyte;
	liv_complbyte = SetValue.CurrentAddress.AddressCircleTime%4;
	if(liv_complbyte != 0)
	{
		//SetValue.CurrentAddress.AddressTouchDown += (4-(SetValue.CurrentAddress.AddressTouchDown%4));
		liv_complbyte = 4 - liv_complbyte;
		for(uint8_t i = 1; i <= liv_complbyte; i++)
		{
			Store_OrgHeartRateData(0xaa);
		}		
	}
	Store_OrgHeartRateData(0xaa);
	Store_OrgHeartRateData(0xaa);
	Store_OrgHeartRateData(0xaa);
	switch(g_storedataflag)
	{
		case 1:
			Store_OrgHeartRateData(1);
		break;
		case 2:
			Store_OrgHeartRateData(2);
		break;
		case 3:
			Store_OrgHeartRateData(3);
		break;
		case 4:
			Store_OrgHeartRateData(4);
		break;
		default:
		break;
	}
}




void store_orggps_data(int32_t Lon, int32_t Lat, int32_t step, int32_t time,int32_t dis,int32_t speed)
{
		//��ֹ�ڴ�Խ��
	if(SetValue.CurrentAddress.AddressCircleTime > TOUCHDOWN_START_ADDRESS - 30)
	{
		return;
	}
	dev_extFlash_enable();
	dev_extFlash_write( SetValue.CurrentAddress.AddressCircleTime,(uint8_t*)(&Lon),STORE_GPS_BYTE_SIZE);
	dev_extFlash_write( SetValue.CurrentAddress.AddressCircleTime+STORE_GPS_BYTE_SIZE,(uint8_t*)(&Lat),STORE_GPS_BYTE_SIZE);
	dev_extFlash_write( SetValue.CurrentAddress.AddressCircleTime+2*STORE_GPS_BYTE_SIZE,(uint8_t*)(&step),STORE_GPS_BYTE_SIZE);
	dev_extFlash_write( SetValue.CurrentAddress.AddressCircleTime+3*STORE_GPS_BYTE_SIZE,(uint8_t*)(&time),STORE_GPS_BYTE_SIZE);
	dev_extFlash_write( SetValue.CurrentAddress.AddressCircleTime+4*STORE_GPS_BYTE_SIZE,(uint8_t*)(&dis),STORE_GPS_BYTE_SIZE);
	dev_extFlash_write( SetValue.CurrentAddress.AddressCircleTime+5*STORE_GPS_BYTE_SIZE,(uint8_t*)(&speed),STORE_GPS_BYTE_SIZE);
	dev_extFlash_disable();
	SetValue.CurrentAddress.AddressCircleTime += (STORE_GPS_BYTE_SIZE*8);
	if( ( SetValue.CurrentAddress.AddressCircleTime & 0x00000FFF ) == 0) //��ַ��4K�߽�
	{
		
	//if( SetValue.CurrentAddress.AddressTouchDown >= TOUCHDOWN_START_ADDRESS )
	//{			
	//	SetValue.CurrentAddress.AddressTouchDown = TOUCHDOWN_START_ADDRESS;
	//}
		
	dev_extFlash_enable();
	dev_extFlash_erase(SetValue.CurrentAddress.AddressCircleTime,(STORE_GPS_BYTE_SIZE*2));		
	dev_extFlash_disable();		
	}	
}



void Store_OrgAxisData(int16_t liv_x, int16_t liv_y, int16_t liv_z)
{
	//��ֹ�ڴ�Խ��
	if(SetValue.CurrentAddress.AddressCircleTime > TOUCHDOWN_START_ADDRESS - 20)
	{
		return;
	}
	dev_extFlash_enable();
	dev_extFlash_write( SetValue.CurrentAddress.AddressCircleTime,(uint8_t*)(&liv_x),STORE_AXIS_BYTE_SIZE);
	dev_extFlash_write( SetValue.CurrentAddress.AddressCircleTime+STORE_AXIS_BYTE_SIZE,(uint8_t*)(&liv_y),STORE_AXIS_BYTE_SIZE);
	dev_extFlash_write( SetValue.CurrentAddress.AddressCircleTime+STORE_AXIS_BYTE_SIZE*2,(uint8_t*)(&liv_z),STORE_AXIS_BYTE_SIZE);
	dev_extFlash_disable();
	SetValue.CurrentAddress.AddressCircleTime += (STORE_AXIS_BYTE_SIZE*3);
	if( ( SetValue.CurrentAddress.AddressCircleTime & 0x00000FFF ) == 0) //��ַ��4K�߽�
	{
		
	//if( SetValue.CurrentAddress.AddressTouchDown >= (TOUCHDOWN_START_ADDRESS +1))
	//{				
	//	SetValue.CurrentAddress.AddressTouchDown = TOUCHDOWN_START_ADDRESS;
	//}
		
	dev_extFlash_enable();
	dev_extFlash_erase(SetValue.CurrentAddress.AddressCircleTime,(STORE_AXIS_BYTE_SIZE*3));		
	dev_extFlash_disable();		
	}
}

void Store_OrgPresData(int32_t liv_pres, int32_t liv_height)
{
	//��ֹ�ڴ�Խ��
	if(SetValue.CurrentAddress.AddressCircleTime > TOUCHDOWN_START_ADDRESS - 16)
	{
		return;
	}
	dev_extFlash_enable();
	dev_extFlash_write( SetValue.CurrentAddress.AddressCircleTime,(uint8_t*)(&liv_pres),STORE_PRES_BYTE_SIZE);
	dev_extFlash_write( SetValue.CurrentAddress.AddressCircleTime+STORE_PRES_BYTE_SIZE,(uint8_t*)(&liv_height),STORE_PRES_BYTE_SIZE);
	dev_extFlash_disable();
	SetValue.CurrentAddress.AddressCircleTime += (STORE_PRES_BYTE_SIZE*2);
	if( ( SetValue.CurrentAddress.AddressCircleTime & 0x00000FFF ) == 0) //��ַ��4K�߽�
	{
		
	//if( SetValue.CurrentAddress.AddressTouchDown >= (NOTIFY_DATA_START_ADDRESS +1))
	//{				
	//	SetValue.CurrentAddress.AddressTouchDown = TOUCHDOWN_START_ADDRESS;
	//}
		
	dev_extFlash_enable();
	dev_extFlash_erase(SetValue.CurrentAddress.AddressCircleTime,(STORE_PRES_BYTE_SIZE*3));		
	dev_extFlash_disable();		
	}
}

void Store_OrgHeartRateData(uint8_t liv_heartrate)
{
	//��ֹ�ڴ�Խ��
	if(SetValue.CurrentAddress.AddressCircleTime > TOUCHDOWN_START_ADDRESS - 8)
	{
		return;
	}
	dev_extFlash_enable();
	dev_extFlash_write( SetValue.CurrentAddress.AddressCircleTime,(uint8_t*)(&liv_heartrate),STORE_HEARTRATE_BYTE_SIZE);
	dev_extFlash_disable();
	SetValue.CurrentAddress.AddressCircleTime += STORE_HEARTRATE_BYTE_SIZE;
	if( ( SetValue.CurrentAddress.AddressCircleTime & 0x00000FFF ) == 0) //��ַ��4K�߽�
	{
		
	//if( SetValue.CurrentAddress.AddressTouchDown >= (NOTIFY_DATA_START_ADDRESS +1))
	//{				
		//SetValue.CurrentAddress.AddressTouchDown = TOUCHDOWN_START_ADDRESS;
	//}
		
	dev_extFlash_enable();
	dev_extFlash_erase(SetValue.CurrentAddress.AddressCircleTime, STORE_HEARTRATE_BYTE_SIZE);		
	dev_extFlash_disable();		
	}
}
//*/
#endif

/*
�ڵ�ɽ��б���ÿһ�����ѹ�߶�ֵ��GPS�ź�ǿ�ȣ�6����ǿ���źţ���GPS�߶�ֵ
ÿ����Ϊ16���ֽ�,��������ֽ�Ԥ��
��ѹ�߶�ֵ,GPS�߶�ֵ��GPS�ź�ǿ��(6���ֽ�)
*/
#if defined STORE_ALT_TEST_VERTION && !(defined WATCH_SPORT_EVENT_SCHEDULE)//TOUCHDOWN_START_ADDRESSʹ�þ���
void Store_AltRelateData(float *Press_alt,int32_t *Gps_alt,uint8_t *Gps_sn,uint16_t *Gps_hdop)
{
	COM_SPORT_PRINTF("Store_AltRelateData\r\n");
	COM_SPORT_PRINTF("Press_alt-->%d,Gps_alt-->%d,Gps_hdop-->%d\r\n",(uint32_t)(*Press_alt),(uint32_t)(*Gps_alt),(uint16_t)(*Gps_hdop));
	
	uint32_t Sea_Level_Pressure_Data = 0;
	static bool Sea_Level_Pressure_store_flag = false;
	//��һ�α��溣ƽ��߶�
	if (Sea_Level_Pressure_store_flag == false){
		Sea_Level_Pressure_store_flag = true;
		dev_extFlash_enable();		
		dev_extFlash_read(TOUCHDOWN_START_ADDRESS+STORE_ALT_BYTE_SIZE,(uint8_t *)(&Sea_Level_Pressure_Data),sizeof(uint32_t));	
		if(Sea_Level_Pressure_Data == 0xffffffff){
			COM_SPORT_PRINTF("Sea_Level_Pressure-->%d\r\n",SetValue.AppSet.Sea_Level_Pressure);
			dev_extFlash_write( SetValue.CurrentAddress.AddressTouchDown+STORE_ALT_BYTE_SIZE,(uint8_t*)(&SetValue.AppSet.Sea_Level_Pressure),STORE_ALT_BYTE_SIZE);
		}
		dev_extFlash_disable();
		SetValue.CurrentAddress.AddressTouchDown += (STORE_ALT_BYTE_SIZE*4);		
	}
	
    //if((*Press_alt != 0) && (*Gps_alt != 0))
    {		
		dev_extFlash_enable();
		dev_extFlash_write( SetValue.CurrentAddress.AddressTouchDown,(uint8_t*)(Press_alt),STORE_ALT_BYTE_SIZE);
		dev_extFlash_write( SetValue.CurrentAddress.AddressTouchDown+STORE_ALT_BYTE_SIZE,(uint8_t*)(Gps_alt),STORE_ALT_BYTE_SIZE);
		dev_extFlash_write( SetValue.CurrentAddress.AddressTouchDown+2*STORE_GPS_BYTE_SIZE,Gps_sn,6);
		dev_extFlash_write( SetValue.CurrentAddress.AddressTouchDown+2*STORE_GPS_BYTE_SIZE+6,(uint8_t*)(Gps_hdop),2);
		dev_extFlash_disable();

		for(uint8_t i=0; i<6; i++)
		{
			COM_SPORT_PRINTF("Gps_sn-->%d\r\n",*Gps_sn);
			Gps_sn++;
		}
		
		SetValue.CurrentAddress.AddressTouchDown += (STORE_ALT_BYTE_SIZE*4);
		if( ( SetValue.CurrentAddress.AddressTouchDown & 0x00000FFF ) == 0) //��ַ��4K�߽�
		{
			if( SetValue.CurrentAddress.AddressTouchDown >= NOTIFY_DATA_START_ADDRESS )
			{			
				SetValue.CurrentAddress.AddressTouchDown = TOUCHDOWN_START_ADDRESS;
			}
			dev_extFlash_enable();
			dev_extFlash_erase(SetValue.CurrentAddress.AddressTouchDown,(STORE_ALT_BYTE_SIZE*4));		
			dev_extFlash_disable();		
	    }			
    }
		
}
#endif


/*��ȡ����� 
����ֵ1��ʾ����������������Ӿ*/
uint8_t Read_ActivityData(uint32_t addr1,uint32_t addr2)
{
	int i = 0;
	uint8_t addr1Flag = 0,addr2Flag = 0,m_TRIATHLON_flag = 0;//
	memset(&ActivityRecordData,0,sizeof(ActivityRecordData));
	memset(&ActivityRecordDataDown,0,sizeof(ActivityRecordDataDown));
	 //��ʼ��ַ���� ����������������
	for(i = 0;i < MAX_RECORD_DAY;i++)
	{
		if((addr1 >= (DAY_MAIN_DATA_START_ADDRESS + DAY_MAIN_DATA_SIZE * i + DAY_COMMON_DATA_SIZE + 0*ACTIVITY_DAY_DATA_SIZE))
			 && addr1 <= (DAY_MAIN_DATA_START_ADDRESS + DAY_MAIN_DATA_SIZE * i + DAY_COMMON_DATA_SIZE + DAY_ACTIVITY_MAX_NUM*ACTIVITY_DAY_DATA_SIZE))
		{//�ҵ�ʱ
			addr1Flag = 1;
			break;
		}
		else
		{//��δ�ҵ�ʱ
			addr1Flag = 0;
		}
	}

	for(i = 0;i < MAX_RECORD_DAY;i++)
	{
		if((addr2 >= (DAY_MAIN_DATA_START_ADDRESS + DAY_MAIN_DATA_SIZE * i + DAY_COMMON_DATA_SIZE + 0*ACTIVITY_DAY_DATA_SIZE))
			 && addr2 <= (DAY_MAIN_DATA_START_ADDRESS + DAY_MAIN_DATA_SIZE * i + DAY_COMMON_DATA_SIZE + DAY_ACTIVITY_MAX_NUM*ACTIVITY_DAY_DATA_SIZE))
		{//�ҵ�ʱ
			addr2Flag = 1;
			break;
		}
		else
		{//��δ�ҵ�ʱ
			addr2Flag = 0;
		}
	}

	dev_extFlash_enable();
	if(addr1Flag == 1)
		dev_extFlash_read( addr1,(uint8_t*)(&ActivityRecordData),ACTIVITY_DAY_DATA_SIZE);
	if(addr2Flag == 1)
		dev_extFlash_read( addr2,(uint8_t*)(&ActivityRecordDataDown),ACTIVITY_DAY_DATA_SIZE);
	dev_extFlash_disable();
	if(ActivityRecordData.Activity_Type == ACT_TRIATHLON_SWIM)
	{
		m_TRIATHLON_flag = 1;
	}
	else
	{
		m_TRIATHLON_flag = 0;
	}
	COM_SPORT_PRINTF("[com_sport]:addr1=0x%X,Activity_Type=%d,Activity_Index=%d;addr2=0x%X,Activity_Type=%d,Activity_Index=%d;g_ActivityData_Total_Num=%d,m_TRIATHLON_flag=%d,year=%d,month=%d,day=%d\n",
	                    addr1,ActivityRecordData.Activity_Type,ActivityRecordData.Activity_Index,
											addr2,ActivityRecordDataDown.Activity_Type,ActivityRecordDataDown.Activity_Index,g_ActivityData_Total_Num,m_TRIATHLON_flag,
	                    ActivityRecordData.Act_Start_Time.Year,ActivityRecordData.Act_Start_Time.Month,ActivityRecordData.Act_Start_Time.Day);
	return m_TRIATHLON_flag;
}
/*����˵��:�ճ������ LOG��ӡ*/
void Com_Flash_Common_Sport(void)
{
#if DEBUG_ENABLED == 1 && COM_SPORT_LOG_ENABLED == 1
	TrainRecordStr temp ={0};
	ActivityDataStr activityType;
//	int32_t pressure;
//	float high;
	
	COM_SPORT_PRINTF("-----------------------Com_Flash_Common_Sport---Start-----------------------------\n");
	dev_extFlash_enable();
	for(uint8_t i=0;i<7;i++)
	{
		dev_extFlash_read(DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*i,(uint8_t*)(&temp),4);  //��ȡ�����
		COM_SPORT_PRINTF("DAY_MAIN_DATA:year=%d,month=%d,date=%d,DayIndex=%d,i=%d\n",
		temp.Year,temp.Month,temp.Date,temp.DayIndex,i);
		COM_SPORT_PRINTF("SetValue.TrainRecordIndex[%d]=%d\n",i,SetValue.TrainRecordIndex[i].ActivityNum);
	}
	for(uint8_t i=0;i<7;i++)
	{
		for(uint8_t j=0;j<DAY_ACTIVITY_MAX_NUM;j++)
		{
			dev_extFlash_read( DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*i +
																			ACTIVITY_DATA_START_ADDRESS+ACTIVITY_DAY_DATA_SIZE*(j),(uint8_t*)(&activityType),sizeof(ActivityDataStr));
			if(activityType.Activity_Index > 0 && activityType.Activity_Index < DAY_ACTIVITY_MAX_NUM
				&& activityType.Activity_Type != 0xFF && activityType.Activity_Type != 0)
			{
					COM_SPORT_PRINTF("DAY_SPORT_DATA:the %d ->ActivityType=%d,ActivityIndex=%d,time=%d\n",
			                i,activityType.Activity_Type,activityType.Activity_Index,activityType.ActTime);
					COM_SPORT_PRINTF("-->dayIndex=%d,Activity_Index=%d,Activity_Type=%d,Adress_step_start=0x%X,stop=0x%X;Adress_heart_start=0x%X,stop=0x%X\n",
						i,activityType.Activity_Index,activityType.Activity_Type,activityType.Steps_Start_Address,
						activityType.Steps_Stop_Address,activityType.HeartRate_Start_Address,activityType.HeartRate_Stop_Address);
					COM_SPORT_PRINTF("Adress_Pressure_Start=0x%X,Stop=0x%X;Adress_GPS_Start=0x%x,Stop=0x%x;Adress_Distance_Start=0x%X,stop=0x%X;Adress_Energy_Start=0x%X,Stop=0x%X;\n",
						activityType.Pressure_Start_Address,activityType.Pressure_Stop_Address,activityType.GPS_Start_Address,activityType.GPS_Stop_Address,
						activityType.Distance_Start_Address,activityType.Distance_Stop_Address,activityType.Energy_Start_Address,activityType.Energy_Stop_Address);
//					COM_SPORT_PRINTF("Adress_Speed_Start=0x%X,Stop=0x%X;Adress_DistancePoint_Start=0x%x,Stop=0x%x;Adress_CircleTime_Start=0x%X,stop=0x%X;Adress_VerticalSize_Start=0x%X,Stop=0x%X;\n",
//						activityType.Speed_Start_Address,activityType.Speed_Stop_Address,activityType.DistancePoint_Start_Address,activityType.DistancePoint_Stop_Address
//						,activityType.CircleTime_Start_Address,activityType.CircleTime_Stop_Address,activityType.VerticalSize_Start_Address,activityType.VerticalSize_Stop_Address);
//					COM_SPORT_PRINTF("Adress_TouchDown_Start=0x%X,Stop=0x%X;Adress_Pause_Start=0x%x,Stop=0x%X;\n"
//						,activityType.TouchDown_Start_Address,activityType.TouchDown_Stop_Address,activityType.Pause_Start_Address,activityType.Pause_Stop_Address);
//				
//					COM_SPORT_PRINTF("First Address=0x%X\n",DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*i +
//																			ACTIVITY_DATA_START_ADDRESS+ACTIVITY_DAY_DATA_SIZE*(j));
//				for(uint16_t k=0;k<(activityType.Pressure_Stop_Address-activityType.Pressure_Start_Address)/8;k++)
//				{
//					dev_extFlash_read(activityType.Pressure_Start_Address+k*4,(uint8_t *)&pressure, 4); 
//					dev_extFlash_read(activityType.Pressure_Start_Address+k*4+4,(uint8_t *)&high, 4); 
//					COM_SPORT_PRINTF("pressure[%d]=0x%x,pressure[%d]=%d",k,pressure,k,pressure);
//					COM_SPORT_PRINTF("high[%d]=0x%x,high[%d]=%d\n",k,high,k,high);
//				}
			}
		}
	}
	
	
	dev_extFlash_disable();
	COM_SPORT_PRINTF("-----------------COM_SPORT_PRINTF--------END------------------------------------\n");
#endif
}

/*��������:�ж��˶������Ƿ�Ϊ��Ч����, �ж����ݻ����
����:valid 1, else 0. 
*/
bool IsActivityDataValid(ActivityDataStr *actdata)
{
    if(NULL == actdata)
        return 0;

    if((actdata->Activity_Type >= ACT_RUN) && (actdata->Activity_Type <= ACT_TRIATHLON_SWIM))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/*��������:��������˶����ݣ��ж���Ч������Ŀ,��������һ��������ʼ��ַ
  ����
*/
uint32_t ActivityAddressForwardLookup(uint32_t address_s)
{
    int i = 0, j = 0;
		
		uint32_t activitynum = 0;
		uint32_t tmp = 0;
		uint32_t tmp_address = 0;
		ActivityDataStr sport_data_rec;
	  rtc_time_t tmp_rtc_time = {0};
		uint32_t address_re;

		//���Ҽ�¼��ʼʱ,��¼λ�ñ������
		m_InvertFlagHead = 0;
		m_InvertFlagTail = 0;
		
		uint32_t daynum = (address_s - DAY_MAIN_DATA_START_ADDRESS) / DAY_MAIN_DATA_SIZE;   //�жϴ�����һ��

		tmp = address_s - DAY_MAIN_DATA_START_ADDRESS - daynum * DAY_MAIN_DATA_SIZE;
		if(tmp >= ACTIVITY_DATA_START_ADDRESS)      //�ж�Ϊĳһ��ĵڼ����˶�����
			activitynum = (tmp - ACTIVITY_DATA_START_ADDRESS) / ACTIVITY_DAY_DATA_SIZE;
		else
			activitynum = 0;
		
		g_ActivityData_Total_Num = 0;//ÿ�������¹���ͳ��
    for(i = 0; i < MAX_RECORD_DAY; i++)
    {
        for(j = activitynum; j < MAX_NUM_ACTIVITY_DATA; j++)
        {			
						tmp_address = i * DAY_MAIN_DATA_SIZE + DAY_MAIN_DATA_START_ADDRESS + ACTIVITY_DATA_START_ADDRESS + j * ACTIVITY_DAY_DATA_SIZE;
						
            ActivityDataGetFlash(tmp_address, &sport_data_rec);
            if(IsActivityDataValid(&sport_data_rec))
            {
							g_ActivityData_Total_Num++; //�˶���������+1

							if(Get_DiffTime(sport_data_rec.Act_Start_Time,tmp_rtc_time) >= 0)
							{
								tmp_rtc_time = sport_data_rec.Act_Start_Time;    //��¼�����ʱ��
								address_re = tmp_address;                        //��¼�����ʱ���Ӧ�ĵ�ַ
								m_LastIndexNum = i;                              //��¼�����ʱ���Ӧ�Ŀռ�index
							}
            }
        }
				
				activitynum = 0;
    }
		
		return address_re;
}
/*����˵��:��������˶�����
����ֵ:ActitivyDataResult_s�ṹ��
*/
ActitivyDataResult_s ActivityDataForwardLookup(uint32_t address_s, ActivityDataStr *sport_data_rec)
{
    int8_t i = 0, j = 0, n = 0,k = 0;
		ActitivyDataResult_s tmp_struct;
		tmp_struct.number = 0;
		uint32_t tmp_address = 0;
		uint32_t activitynum = 0;
		uint32_t tmp = 0;
		
	
		uint32_t daynum = (address_s - DAY_MAIN_DATA_START_ADDRESS) / DAY_MAIN_DATA_SIZE;   //�жϴ�����һ��
	
		//flag = (address_s - DAY_MAIN_DATA_START_ADDRESS) % DAY_MAIN_DATA_SIZE;   //�жϵ�ַ�Ƿ�Ϊÿ�����ݴ洢����ʼ��ַ 

		tmp = address_s - DAY_MAIN_DATA_START_ADDRESS - daynum * DAY_MAIN_DATA_SIZE;
		if(tmp >= ACTIVITY_DATA_START_ADDRESS)      ////�ж�Ϊĳһ��ĵڼ����˶�����
			activitynum = (tmp - ACTIVITY_DATA_START_ADDRESS) / ACTIVITY_DAY_DATA_SIZE;
		else
			activitynum = 0;
		//����
	  for(uint8_t m = 0;m<m_Read_Num;m++)
			memset(&sport_data_rec[m],0,sizeof(sport_data_rec[m]));		
    memset(&tmp_struct,0,sizeof(ActitivyDataResult_s));
		
    for(k = 0,i = daynum; (i < MAX_RECORD_DAY && k < MAX_RECORD_DAY); i++,k++)
    {
				if( m_InvertFlagTail == 1 && m_LastIndexNum == i)
				{//��β���������鲻�ܴ�������m_LastIndexNum
					goto RESULT;
				}
        for(j = activitynum; j < MAX_NUM_ACTIVITY_DATA; j++)
        {			
						tmp_address = i * DAY_MAIN_DATA_SIZE + DAY_MAIN_DATA_START_ADDRESS + ACTIVITY_DATA_START_ADDRESS + j * ACTIVITY_DAY_DATA_SIZE;
						
            ActivityDataGetFlash(tmp_address, &sport_data_rec[m_Read_Num - n - 1]);
            if(IsActivityDataValid(&sport_data_rec[m_Read_Num - n - 1]))
            {			
							n++;
							tmp_struct.number = n;
							if(1 == n) 
							{
								tmp_struct.address0 = tmp_address;
							}
							else
							{
								tmp_struct.address1 = tmp_address;
							}

            }

            if(n == m_Read_Num)   //��������������
						{
                goto RESULT;
						}
        }

				if(i == MAX_RECORD_DAY-1)
				{
					i = -1;
					m_InvertFlagTail = 1;
				}
				if(j == MAX_NUM_ACTIVITY_DATA)        //����
				{
					activitynum = 0;
				}
    }
		
RESULT:

    return tmp_struct;
}
/*����˵��:��������˶�����
����:ActitivyDataResult_s�ṹ��
*/
ActitivyDataResult_s ActivityDataReverseLookup(uint32_t address_s, ActivityDataStr *sport_data_rec)
{
    int8_t i = 0, j = 0, n = 0,k = 0;
		ActitivyDataResult_s tmp_struct;
		tmp_struct.number = 0;
		uint32_t tmp_address = 0;
		uint32_t activitynum = 0;
		uint32_t tmp = 0, flag = 0;

		uint32_t daynum = (address_s - DAY_MAIN_DATA_START_ADDRESS) / DAY_MAIN_DATA_SIZE;    //��һ��

		if(daynum > 6)
			goto RESULT;

		flag = (address_s - DAY_MAIN_DATA_START_ADDRESS) % DAY_MAIN_DATA_SIZE;   //�жϵ�ַ�Ƿ�Ϊÿ�����ݴ洢����ʼ��ַ 

		if(0 == daynum && 0 == flag)   //��һ�����ʼ��ַ
			goto RESULT;

		tmp = address_s - DAY_MAIN_DATA_START_ADDRESS - daynum * DAY_MAIN_DATA_SIZE;
		
		if(tmp >= ACTIVITY_DATA_START_ADDRESS)       //�ж�Ϊĳһ��ĵڼ����˶�����
		{
			activitynum = (tmp - ACTIVITY_DATA_START_ADDRESS) / ACTIVITY_DAY_DATA_SIZE;
		}
		else
			activitynum = 0;
		//����
	  for(uint8_t m = 0;m<m_Read_Num;m++)
			memset(&sport_data_rec[m],0,sizeof(sport_data_rec[m]));		
    memset(&tmp_struct,0,sizeof(ActitivyDataResult_s));
		
    for(k = 0,i = daynum; (i >= 0 && k < MAX_RECORD_DAY); i--,k++)
    {
				if( m_InvertFlagHead == 1 && m_LastIndexNum == i)
				{//��ͷ������β���鲻�ܵ���m_LastIndexNum
					goto RESULT;
				}
        for(j = activitynum ; j >= 0; j--)
        {			
						tmp_address = i * DAY_MAIN_DATA_SIZE + DAY_MAIN_DATA_START_ADDRESS + ACTIVITY_DATA_START_ADDRESS + j * ACTIVITY_DAY_DATA_SIZE;
						
            ActivityDataGetFlash(tmp_address, &sport_data_rec[n]);
            if(IsActivityDataValid(&sport_data_rec[n]))
            {
							n++;
							tmp_struct.number = n;
							
							if(1 == n)
							{
								tmp_struct.address1 = tmp_address;
							}
							else
							{
								tmp_struct.address0 = tmp_address;
							}
							
            }

            if(n == m_Read_Num)   //��������������
						{
                goto RESULT;
						}
        }

				if(i == 0)
				{
					i = MAX_RECORD_DAY;
					m_InvertFlagHead = 1;
				}

				if(j == -1)        //����
				{
					activitynum = DAY_ACTIVITY_MAX_NUM-1;
				}
    }
		
RESULT:
    return tmp_struct;
}
/*����˵��:ɾ��ѡ�еĻ �û����Ϊ0*/
void DeleteActivityRecord(uint32_t addr)
{
	uint8_t sportRecordType = 0;
	uint8_t data[1] = {0};
	uint8_t temp = 0;
	dev_extFlash_enable();
	dev_extFlash_read( addr,&sportRecordType,1);

	if(sportRecordType == ACT_TRIATHLON_SWIM
	 || sportRecordType == ACT_TRIATHLON_CYCLING
	 || sportRecordType == ACT_TRIATHLON_RUN)
	{
		for(uint8_t i = 0;i < 5;i++)
		{
			dev_extFlash_read( addr+sizeof(ActivityDataStr)*i,&temp,1);

			if((ACT_TRIATHLON_SWIM + i) == temp)
			{
				dev_extFlash_write( addr+sizeof(ActivityDataStr)*i,data,1);
			}
			else
			{
				//�������������˶�
				break;
			}
		}
	}
	else
	{
			dev_extFlash_write( addr,data,1);
	}
	dev_extFlash_disable();
}
#if defined(WATCH_SPORT_RECORD_TEST)
SportRecordYyMmSsStr s_sport_record_select_str[MAX_NUM_ACTIVITY_DATA*MAX_RECORD_DAY];

/*�˶���¼:����ȫ����Ч�˶���¼ �������е�����s_sport_record_select_str��
����ֵ:���µ�һ���˶���¼*/
uint32_t search_sport_record(void)
{
	SportRecordYyMmSsStr temp_sport_record;
	ActivityTitleStr m_ActivityTitleStr;
	uint32_t tmp_address = 0;
	uint32_t m_index = 0;//��������
	
	//�������
	memset(&s_sport_record_select_str[0],0,sizeof(s_sport_record_select_str));
	g_ActivityData_Total_Num = 0;

	//������Ч�˶���¼
	dev_extFlash_enable();
	for(uint32_t i = 0;i < (MAX_RECORD_DAY);i++)
	{
		for(uint32_t j = 0;j < (MAX_NUM_ACTIVITY_DATA);j++)
		{
			tmp_address = i * DAY_MAIN_DATA_SIZE + DAY_MAIN_DATA_START_ADDRESS + ACTIVITY_DATA_START_ADDRESS + j * ACTIVITY_DAY_DATA_SIZE;
			dev_extFlash_read( tmp_address,(uint8_t *)&m_ActivityTitleStr,sizeof(ActivityTitleStr));
			//��buf
			if((m_ActivityTitleStr.Activity_Type >= ACT_RUN) && (m_ActivityTitleStr.Activity_Type <= ACT_TRIATHLON_SWIM))
			{
				s_sport_record_select_str[m_index].address = tmp_address;
				memcpy(&s_sport_record_select_str[m_index].start_time,&m_ActivityTitleStr.Act_Start_Time,sizeof(rtc_time_t));
				m_index ++;
			}
		}
	}
	dev_extFlash_disable();
	g_ActivityData_Total_Num = m_index;//��Ч�˶���¼������
	COM_SPORT_PRINTF("[com_sport]:search_sport_record -->g_ActivityData_Total_Num=%d\n",g_ActivityData_Total_Num);
	if(m_index != 0)
	{//���˶�����
		//��������
		for(uint32_t i = 0;i < (g_ActivityData_Total_Num) - 1;i++)
		{
			for(uint32_t j = 0;j < (g_ActivityData_Total_Num) - i - 1;j++)
			{
				if(GetTimeSecond(s_sport_record_select_str[j].start_time) < GetTimeSecond(s_sport_record_select_str[j+1].start_time))
				{
					memcpy(&temp_sport_record,&s_sport_record_select_str[j],sizeof(SportRecordYyMmSsStr));
					memcpy(&s_sport_record_select_str[j],&s_sport_record_select_str[j+1],sizeof(SportRecordYyMmSsStr));
					memcpy(&s_sport_record_select_str[j+1],&temp_sport_record,sizeof(SportRecordYyMmSsStr));
				}
			}
		}
	}
	else
	{//���˶�����
		memset(&s_sport_record_select_str[0],0,sizeof(SportRecordYyMmSsStr));
	}
	COM_SPORT_PRINTF("[com_sport]:address1=0x%X,time:%d-%02d-%02d %d:%02d:%02d\n"
	   ,s_sport_record_select_str[0].address,s_sport_record_select_str[0].start_time.Year,s_sport_record_select_str[0].start_time.Month
	   ,s_sport_record_select_str[0].start_time.Day,s_sport_record_select_str[0].start_time.Hour,s_sport_record_select_str[0].start_time.Minute
		 ,s_sport_record_select_str[0].start_time.Second);
	return s_sport_record_select_str[0].address;
}
/*�����˶���¼��ַ����������ϸ������
���:�������� �������¼��ļ�¼ 
 key_index_first �м���ʾ��
 key_index_second ��һ����ʾ�� ���Ϊ0 �򲻲�
����ֵ:true���˶����� false ���˶�����*/
bool get_sport_detail_record(uint32_t key_index_first,uint32_t key_index_second)
{
	if(s_sport_record_select_str[key_index_first].address != 0)
	{
		dev_extFlash_enable();
		dev_extFlash_read( s_sport_record_select_str[key_index_first].address,(uint8_t *)&ActivityRecordData,sizeof(ActivityDataStr));
		if(key_index_second != 0 && s_sport_record_select_str[key_index_second].address != 0)
		{//��һ��
			dev_extFlash_read( s_sport_record_select_str[key_index_second].address,(uint8_t *)&ActivityRecordDataDown,sizeof(ActivityDataStr));
		}
		dev_extFlash_disable();
		COM_SPORT_PRINTF("[com_sport]:get_sport_detail_record-->key_index_first=%d,add1=0x%x,key_index_second=%d,add2=0x%x\n"
		 ,key_index_first,s_sport_record_select_str[key_index_first].address,key_index_second,s_sport_record_select_str[key_index_second].address);
		return true;
	}
	else
	{
		return false;
	}
}
#endif
/**************************************
����Ϊ�˶�������
***********************************/

//�˶����ѳ�ʼ��
void Remind_Sport_Init(void)
{
     heartrate_threshold = 0; //����������ֵ����
     heartrate_last = 0;      //�ϴ�����ֵ
     kcal_last = 0;          //�ϴο�·��ֵ
     circle_last = 0;        //�ϴ�Ȧ��
     time_last = 0;          //�ϴ�ʱ��
     goal_remind_distance_last = 0;	//�ϴ�Ŀ�����Ѿ���
	 circle_remind_distance_last = 0;//�ϴμ�Ȧ���Ѿ���
     distance_nums = 1;      //�������Ѵ���
     altitude_last = 0;      //�ϴθ߶�
     speed_nums = 0;        //�ٶ����Ѽ������˶���ʼ(60-55)s������
     speed_nums_limit = 60;      //�ٶ����Ѽ���ʱ�����ƣ���ʼ1���ӣ�����3��5����
     speed_nums_cycleindex = 0;      //�ٶ����Ѽ���ѭ������

     circledistance_nums =1;//��ȦȦ��
     circledistance_last =0;//�ϴμ�Ȧ����

	 kilometer_nums =1;
     kilometer_last =0;
		 if(Get_IsTrainPlanOnGoing() && (Get_TrainPlanType(Download[1].Type) == 3))
		 {//ѵ���ƻ��˶�������������������
			 speed_trainplan_nums = 0;
			 distance_circle_nums = 0;
		 }
}

/*
*����:���ʸ澯���ﵽ��������5��������
*����:now_heart��ǰ����ֵ
     swsport ���˶����أ����ܲ�����:SetValue.SportSet.SwRun 
*����ֵ :��

*��ע:�˺������������������֮��
*/
void Remind_Heartrate(uint8_t now_heart,uint8_t swsport)
{
 if ((swsport & SW_HEARTRATE_MASK) == SW_HEARTRATE_MASK)//���ʿ���
 {
//	 COM_SPORT_PRINTF("(swsport & SW_HEARTRATE_MASK) == SW_HEARTRATE_MASK\n%d, %d\n",swsport, SW_HEARTRATE_MASK);
	if ((now_heart >= g_HeartRemind) && (heartrate_last < g_HeartRemind ))
	{
COM_SPORT_PRINTF("(now_heart < g_HeartRemind)&&(heartrate_last < g_HeartRemind)\n%d, %d, %d\n",now_heart, heartrate_last,g_HeartRemind);
		heartrate_threshold = 1;
	}
	else if ((now_heart < g_HeartRemind) &&(heartrate_last>= g_HeartRemind )) //����ֵ������ֵ
	{
COM_SPORT_PRINTF("(now_heart < g_HeartRemind)&&(heartrate_last >= g_HeartRemind)\n%d, %d, %d\n",now_heart, heartrate_last,g_HeartRemind);
		if( ScreenState == DISPLAY_SCREEN_NOTIFY_HEARTRATE )//�����������ѽ���
		{
			COM_SPORT_PRINTF("ScreenState == DISPLAY_SCREEN_NOTIFY_HEARTRATE,HDR TH= %d, \n",heartrate_threshold);
			//�˳��������ѽ���
			timer_notify_display_stop(false);
			timer_notify_motor_stop(false);
			timer_notify_buzzer_stop(false);
			DISPLAY_MSG  msg = {0,0};
			ScreenState = ScreenStateSave;
			msg.cmd = MSG_DISPLAY_SCREEN;
//			msg.value= 150;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);	
			
		}
		heartrate_threshold = 0;
	}
	
	if(heartrate_threshold > 0)
	{
		if(now_heart >= g_HeartRemind)
		{
			heartrate_threshold++;
COM_SPORT_PRINTF("HEARTRATE Threshold: %d, \n",heartrate_threshold);
			if(heartrate_threshold >= 5) //����ֵ������ֵ��ά��5S��
			{
COM_SPORT_PRINTF("HEARTRATE Threshold Trigger: %d, \n",heartrate_threshold);
				//������������
			    if((ScreenState != DISPLAY_SCREEN_LOGO)
					&& (ScreenState != DISPLAY_SCREEN_NOTIFY_BAT_CHG)
					&& time_notify_is_ok())
	            {
		            if(ScreenState < DISPLAY_SCREEN_NOTIFY)
		            {
			            ScreenStateSave = ScreenState;
		            }
		            timer_notify_display_start(0xFFFFFFFF,1,false);
		            timer_notify_motor_start(500,100,0xFFFFFFFF,false,NOTIFY_MODE_SPORTS);
		            timer_notify_buzzer_start(500,100,0xFFFFFFFF,false,NOTIFY_MODE_SPORTS);
		            DISPLAY_MSG  msg = {0,0};
		            ScreenState = DISPLAY_SCREEN_NOTIFY_HEARTRATE;
		            msg.cmd = MSG_DISPLAY_SCREEN;
                msg.value= now_heart | 0xF1000000;
		            xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
								heartrate_threshold = 0;								
	            }	
			
			}
		}
		else
		{
			heartrate_threshold = 0;
		}
	}

    heartrate_last = now_heart;
 }
}

/*
*����:ÿ�ղ�������
*����:daily_step_now ���յ�ǰ����
      daily_step_goal ����Ŀ�경��
*����ֵ :��

*��ע:�޿��أ�ÿ��ﵽ������һ�Ρ�
*/
void Remind_DailyStep(uint32_t daily_step_now,uint32_t daily_step_goal)
{
	if(step_tags == 0)
	{
		//��ֹ����ʱ����
		daily_step_last = daily_step_now;
		
		step_tags = 1;
	}
	
        if ((daily_step_now >= daily_step_goal )&&(daily_step_last < daily_step_goal))
        {		
            //Ŀ�경������
        	if((ScreenState != DISPLAY_SCREEN_LOGO)
//				&& (step_tags>1)
				&& (ScreenState != DISPLAY_SCREEN_NOTIFY_BAT_CHG)
				&& time_notify_is_ok())
	        {
		        if(ScreenState < DISPLAY_SCREEN_NOTIFY)
		        {
			       ScreenStateSave = ScreenState;
		        }
		        timer_notify_display_start(5000,1,false);
		        timer_notify_motor_start(1000,500,3,false,NOTIFY_MODE_SYSTEM_TIME_REACH);
		        timer_notify_buzzer_start(1000,500,3,false,NOTIFY_MODE_SYSTEM_TIME_REACH);
		        DISPLAY_MSG  msg = {0,0};
		        ScreenState = DISPLAY_SCREEN_NOTIFY_STEP;
		        msg.cmd = MSG_DISPLAY_SCREEN;
            //    msg.value= now_step | (goal_step <<16);//��ǰֵ��16λ��Ŀ��ֵ��16λ
		        xQueueSend(DisplayQueue, &msg, portMAX_DELAY);	
//						daily_step_last = daily_step_now;	
	        }
//			step_tags++; 
        }
        daily_step_last = daily_step_now;
}

/*
*����:ÿ�տ�·������
*����:daily_kcal_now ���յ�ǰ��·��, ��λ���󿨣����˶��洢��λΪ���������1000
      daily_kcal_goal ����Ŀ�꿨·��
*����ֵ :��

*��ע:�޿��أ�ÿ��ﵽ������һ�Ρ�
*/
void Remind_DailyKcal(uint32_t daily_kcal_now,uint32_t daily_kcal_goal)
{
	if(kcal_tags == 0)
	{
		//��ֹ����ʱ����
		daily_kcal_last = daily_kcal_now;
		kcal_tags = 1;
	}
	
    if ((daily_kcal_now >= daily_kcal_goal )&&(daily_kcal_last < daily_kcal_goal))
        {
					//Ŀ�꿨·������
        	if((ScreenState != DISPLAY_SCREEN_LOGO)
//				&&(kcal_tags>1)
				&& (ScreenState != DISPLAY_SCREEN_NOTIFY_BAT_CHG)
				&& time_notify_is_ok())
	        {
		        if(ScreenState < DISPLAY_SCREEN_NOTIFY)
		        {
							ScreenStateSave = ScreenState;
		        }
		        timer_notify_display_start(5000,1,false);
		        timer_notify_motor_start(1000,500,3,false,NOTIFY_MODE_SYSTEM_TIME_REACH);
		        timer_notify_buzzer_start(1000,500,3,false,NOTIFY_MODE_SYSTEM_TIME_REACH);
		        DISPLAY_MSG  msg = {0,0};
		        ScreenState = DISPLAY_SCREEN_NOTIFY_CALORY;
		        msg.cmd = MSG_DISPLAY_SCREEN;
            //    msg.value= now_kcal | (goal_kcal <<16);//��ǰֵ��16λ��Ŀ��ֵ��16λ
		        xQueueSend(DisplayQueue, &msg, portMAX_DELAY);		
//						daily_kcal_last = daily_kcal_now;
	        }
//			kcal_tags++;	
        }
		
		daily_kcal_last = daily_kcal_now;
}


/*
*����:Ŀ��ȼ֬����
*����:now_kcal ��ǰ��·��, ��λ���󿨣����˶��洢��λΪ���������1000
      goal_kcal Ŀ�꿨·��
     swsport ���˶����أ����ܲ�����:SetValue.SportSet.SwRun 
*����ֵ :��

*��ע:���ߣ�ͽ��ԽҰ
*/
void Remind_GoalKcal(uint16_t now_kcal,uint16_t goal_kcal,uint8_t swsport)
{

    if((swsport & SW_GOALKCAL_MASK) == SW_GOALKCAL_MASK)
    {
        if ((now_kcal >= goal_kcal )&&(kcal_last < goal_kcal))
        {
            //Ŀ��ȼ֬����
        	if((ScreenState != DISPLAY_SCREEN_LOGO)
				&& (ScreenState != DISPLAY_SCREEN_NOTIFY_BAT_CHG)
				&& time_notify_is_ok())
	        {
		        if(ScreenState < DISPLAY_SCREEN_NOTIFY)
		        {
			       ScreenStateSave = ScreenState;
		        }
		        timer_notify_display_start(5000,1,false);
		        timer_notify_motor_start(1000,500,3,false,NOTIFY_MODE_SPORTS);
		        timer_notify_buzzer_start(1000,500,3,false,NOTIFY_MODE_SPORTS);
		        DISPLAY_MSG  msg = {0,0};
		        ScreenState = DISPLAY_SCREEN_NOTIFY_GOALKCAL;
		        msg.cmd = MSG_DISPLAY_SCREEN;
						msg.value = goal_kcal; //���ڰ汾ֻ��ʾĿ��ֵ VB103
                //msg.value= now_kcal | (goal_kcal <<16);//��ǰֵ��16λ��Ŀ��ֵ��16λ
		        xQueueSend(DisplayQueue, &msg, portMAX_DELAY);							
	        }	
        }
        kcal_last = now_kcal;
    }

}

/*
*����:Ŀ��Ȧ������
*����:now_circle ��ǰȦ��
      goal_circle Ŀ��Ȧ��
     swsport ���˶����أ����ܲ�����:SetValue.SportSet.SwSwimmg 
*����ֵ :��

*��ע:��Ӿ
*/
void Remind_GoalCircle(uint16_t now_circle,uint16_t goal_circle,uint8_t swsport)
{

    if((swsport & SW_GOALCIRCLE_MASK) == SW_GOALCIRCLE_MASK)
    {
COM_SPORT_PRINTF("(swsport & SW_GOALCIRCLE_MASK) == SW_GOALCIRCLE_MASK\n%d, %d\n",swsport, SW_GOALCIRCLE_MASK);
        if ((now_circle >= goal_circle )&&(circle_last < goal_circle))
        {
            //Ŀ��Ȧ������
        	if((ScreenState != DISPLAY_SCREEN_LOGO)
				&& (ScreenState != DISPLAY_SCREEN_NOTIFY_BAT_CHG)
				&& time_notify_is_ok())
	        {
		        if(ScreenState < DISPLAY_SCREEN_NOTIFY)
		        {
			       ScreenStateSave = ScreenState;
		        }
		        timer_notify_display_start(5000,1,false);
		        timer_notify_motor_start(1000,500,3,false,NOTIFY_MODE_SPORTS);
		        timer_notify_buzzer_start(1000,500,3,false,NOTIFY_MODE_SPORTS);
		        DISPLAY_MSG  msg = {0,0};
		        ScreenState = DISPLAY_SCREEN_NOTIFY_GOALCIRCLE;
		        msg.cmd = MSG_DISPLAY_SCREEN;
						msg.value = goal_circle; //���ڰ汾ֻ��ʾĿ��ֵ VB103
            //    msg.value= now_circle | (goal_circle <<16);//��ǰֵ��16λ��Ŀ��ֵ��16λ
		        xQueueSend(DisplayQueue, &msg, portMAX_DELAY);							
	        }	
        }
        circle_last = now_circle;
    }

}

/*
*����:Ŀ��ʱ������
*����:now_time ��ǰȦ��
      goal_time Ŀ��Ȧ��
     swsport ���˶����أ����ܲ�����:SetValue.SportSet.SwSwimmg 
*����ֵ :��

*��ע:��Ӿ,���ߣ�����������Ӿ
*/
void Remind_GoalTime(uint16_t now_time,uint16_t goal_time,uint8_t swsport)
{

    if((swsport & SW_GOALTIME_MASK) == SW_GOALTIME_MASK)
    {
COM_SPORT_PRINTF("(swsport & SW_GOALTIME_MASK) == SW_GOALTIME_MASK\n%d, %d\n",swsport, SW_GOALTIME_MASK);
        if ((now_time >= goal_time )&&(time_last < goal_time))
        {
COM_SPORT_PRINTF("(now_time >= goal_time )&&(time_last < goal_time)\n%d, %d, %d\n",now_time, goal_time, time_last);        
					//Ŀ��ʱ������
        	if((ScreenState != DISPLAY_SCREEN_LOGO)
				&& (ScreenState != DISPLAY_SCREEN_NOTIFY_BAT_CHG)
				&& time_notify_is_ok())
	        {
		        if(ScreenState < DISPLAY_SCREEN_NOTIFY)
		        {
			       ScreenStateSave = ScreenState;
		        }
		        timer_notify_display_start(5000,1,false);
		        timer_notify_motor_start(1000,500,3,false,NOTIFY_MODE_SPORTS);
		        timer_notify_buzzer_start(1000,500,3,false,NOTIFY_MODE_SPORTS);
		        DISPLAY_MSG  msg = {0,0};
		        ScreenState = DISPLAY_SCREEN_NOTIFY_GOALTIME;
		        msg.cmd = MSG_DISPLAY_SCREEN;
						msg.value = goal_time; //���ڰ汾ֻ��ʾĿ��ֵ VB103
            //    msg.value= now_time | (goal_time <<16);//��ǰֵ��16λ��Ŀ��ֵ��16λ
		        xQueueSend(DisplayQueue, &msg, portMAX_DELAY);							
	        }	
        }
        time_last = now_time;
    }

}

/*
*����:Ŀ���������
*����:now_distance ��ǰ����,��λ:��,�洢���뵥λ:cm,ʵ�ʳ���100
      goal_distance ���Ѿ���
     swsport ���˶����أ����ܲ�����:SetValue.SportSet.SwSwimmg 
*����ֵ :��

*��ע:
*/
void Remind_GoalDistance(uint16_t now_distance,uint16_t goal_distance,uint8_t swsport)
{
    if((swsport & SW_DISTANCE_MASK) == SW_DISTANCE_MASK)
    {
        if ((now_distance >= goal_distance)&&(goal_remind_distance_last < goal_distance))
        {
            //��������
        	if((ScreenState != DISPLAY_SCREEN_LOGO)
				&& (ScreenState != DISPLAY_SCREEN_NOTIFY_BAT_CHG)
				&& time_notify_is_ok())
	        {
		        if(ScreenState < DISPLAY_SCREEN_NOTIFY)
		        {
			       ScreenStateSave = ScreenState;
		        }
		        timer_notify_display_start(5000,1,false);
		        timer_notify_motor_start(1000,500,3,false,NOTIFY_MODE_SPORTS);
		        timer_notify_buzzer_start(1000,500,3,false,NOTIFY_MODE_SPORTS);
		        DISPLAY_MSG  msg = {0,0};
		        ScreenState = DISPLAY_SCREEN_NOTIFY_GOALDISTANCE;
		        msg.cmd = MSG_DISPLAY_SCREEN;
				msg.value = goal_distance; //���ڰ汾ֻ��ʾĿ��ֵ VB103
//				msg.value= now_distance | (goal_distance <<16);//��ǰֵ��16λ��Ŀ��ֵ��16λ
		        xQueueSend(DisplayQueue, &msg, portMAX_DELAY);							
	        }	
        }
        goal_remind_distance_last = now_distance;
    }
}

/*
*����:��Ȧ��������
*����:now_distance ��ǰ����,��λ:��,�洢���뵥λ:cm,ʵ�ʳ���100
      goal_distance ���Ѿ���
     swsport ���˶����أ����ܲ�����:SetValue.SportSet.SwSwimmg 
*����ֵ :��

*��ע:
*/
uint8_t Remind_CircleDistance(uint16_t now_distance,uint16_t goal_distance,uint8_t swsport)
{
	uint8_t status = 0;
	uint16_t nums;
    if((swsport & SW_CIRCLEDISTANCE_MASK) == SW_CIRCLEDISTANCE_MASK)
    {
        if ((now_distance >= (distance_nums*goal_distance) )&&(circle_remind_distance_last < (distance_nums*goal_distance)))
        {
            //�Զ���Ȧ����
            nums =( now_distance - ( distance_nums - 1 ) * goal_distance ) / goal_distance;
            distance_nums+= nums; 			

			status = 1;
        }
        circle_remind_distance_last = now_distance;
    }
	
	return status;
}

//�Զ���Ȧ������ʾ,��Ҫ�ȼ������һȦ�˶�����
void remind_autocircle_display(void)
{
	if((ScreenState != DISPLAY_SCREEN_LOGO)
					&& (ScreenState != DISPLAY_SCREEN_NOTIFY_BAT_CHG)
					&& time_notify_is_ok())
		{
			if(ScreenState < DISPLAY_SCREEN_NOTIFY)
			{
	
				 ScreenStateSave = ScreenState;
			}
				//�Զ���Ȧ����
				timer_notify_display_start(10000,1,false);
				timer_notify_motor_start(1000,500,1,false,NOTIFY_MODE_SPORTS);
				timer_notify_buzzer_start(1000,500,1,false,NOTIFY_MODE_SPORTS);
						
				DISPLAY_MSG  msg = {0,0};
				ScreenState = DISPLAY_SCREEN_NOTIFY_CIRCLEDISTANCE;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);	
						
		}			

}

/*
*����:�߶�����
*����:now_altitude ��ǰ�߶�
      goal_altitude  ���Ѹ߶�
     swsport ���˶����أ����ܲ�����:SetValue.SportSet.SwSwimmg 
*����ֵ :��

*��ע:��ɽ
*/
void Remind_Altitude (uint16_t now_altitude,uint16_t goal_altitude,uint8_t swsport)
{

    if((swsport & SW_ALTITUDE_MASK) == SW_ALTITUDE_MASK)
    {
//		if ((now_altitude >= goal_altitude )&&(altitude_last < goal_altitude))//�������Ѹ߶�
		if ((now_altitude >= goal_altitude )&&(altitude_last == 0))//�������Ѹ߶�
        {
			//ֻ����һ��
			altitude_last = 1;
			
            //�߶�����
        	if((ScreenState != DISPLAY_SCREEN_LOGO)
				&& (ScreenState != DISPLAY_SCREEN_NOTIFY_BAT_CHG)
				&& time_notify_is_ok())
	        {
		        if(ScreenState < DISPLAY_SCREEN_NOTIFY)
		        {
			       ScreenStateSave = ScreenState;
		        }
		        timer_notify_display_start(5000,1,false);
		        timer_notify_motor_start(1000,500,3,false,NOTIFY_MODE_SPORTS);
		        timer_notify_buzzer_start(1000,500,3,false,NOTIFY_MODE_SPORTS);
		        DISPLAY_MSG  msg = {0,0};
		        ScreenState = DISPLAY_SCREEN_NOTIFY_ALTITUDE;
		        msg.cmd = MSG_DISPLAY_SCREEN;
				msg.value = now_altitude; //���ڰ汾ֻ��ʾĿ��ֵ VB103
            //    msg.value= now_altitude | (goal_altitude <<16);//��ǰֵ��16λ��Ŀ��ֵ��16λ
		        xQueueSend(DisplayQueue, &msg, portMAX_DELAY);							
	        }	
        }

//		else if ((now_altitude <goal_altitude )&&(altitude_last>= goal_altitude))//�������Ѹ߶�
//		{
//			if( ScreenState == DISPLAY_SCREEN_NOTIFY_ALTITUDE )//���ڸ߶����ѽ���
//			{   
//				//�˳��߶����ѽ���
//				timer_notify_display_stop(false);
//				timer_notify_motor_stop(false);
//				timer_notify_buzzer_stop(false);
//				DISPLAY_MSG  msg = {0,0};
//				ScreenState = ScreenStateSave;
//				msg.cmd = MSG_DISPLAY_SCREEN;
//				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);	
//			}

//		}
//		altitude_last = now_altitude;
    }

}

/*
*����:�ٶ�����
*����:now_speed ��ǰ�ٶ�
      goal_speed �����ٶ�
     swsport ���˶�����
*����ֵ :��

*��ע:����km/h����ɽm/h
*/
void Remind_Speed (uint16_t now_speed,uint16_t goal_speed,uint8_t swsport)
{

    if((swsport & SW_PACE_MASK) == SW_PACE_MASK)
    {
        speed_nums++;
       if ((now_speed < goal_speed )&&(now_speed !=0))
        {    
            //�ٶ�����
            if (speed_nums > speed_nums_limit)//��ʼ���1��������
            {
                speed_nums = 0;
				speed_nums_cycleindex++;
				if (speed_nums_cycleindex >= 3 )//��������3��
				{
					speed_nums_limit = 300; //���5����
				}
        	    if((ScreenState != DISPLAY_SCREEN_LOGO)
					&& (ScreenState != DISPLAY_SCREEN_NOTIFY_BAT_CHG)
					&& time_notify_is_ok())
	            {
		            if(ScreenState < DISPLAY_SCREEN_NOTIFY)
		            {
			            ScreenStateSave = ScreenState;
		            }
		            timer_notify_display_start(5000,1,false);
								timer_notify_motor_start(1000,500,3,false,NOTIFY_MODE_SPORTS);
								timer_notify_buzzer_start(1000,500,3,false,NOTIFY_MODE_SPORTS);
		            DISPLAY_MSG  msg = {0,0};
		            ScreenState = DISPLAY_SCREEN_NOTIFY_SPEED;
		            msg.cmd = MSG_DISPLAY_SCREEN;
                    msg.value= now_speed | (goal_speed <<16);//��ǰֵ��16λ��Ŀ��ֵ��16λ
		            xQueueSend(DisplayQueue, &msg, portMAX_DELAY);							
	           }	
            }

        }

    }

}

/*
*����:��������
*����:now_pace ��ǰ����
      goal_pace ��������
     swsport ���˶�����
*����ֵ :��

*��ע:�ܲ���+ͽ��ԽҰ
*/
void Remind_Pace (uint16_t now_pace,uint16_t goal_pace,uint8_t swsport)
{

    if((swsport & SW_PACE_MASK) == SW_PACE_MASK)
    {
        speed_nums++;
        if (now_pace > goal_pace)
        {
            //��������
            if (speed_nums > speed_nums_limit)//��ʼ���1��������
            {
                speed_nums = 0;
				speed_nums_cycleindex++;
				if (speed_nums_cycleindex >= 3 )//��������3��
				{
					speed_nums_limit = 300; //���5����
				}
        	    if((ScreenState != DISPLAY_SCREEN_LOGO)
					&& (ScreenState != DISPLAY_SCREEN_NOTIFY_BAT_CHG)
					&& time_notify_is_ok())
	            {
		            if(ScreenState < DISPLAY_SCREEN_NOTIFY)
		            {
			            ScreenStateSave = ScreenState;
		            }
		            timer_notify_display_start(5000,1,false);
								timer_notify_motor_start(1000,500,3,false,NOTIFY_MODE_SPORTS);
								timer_notify_buzzer_start(1000,500,3,false,NOTIFY_MODE_SPORTS);
		            DISPLAY_MSG  msg = {0,0};
		            ScreenState = DISPLAY_SCREEN_NOTIFY_PACE;
		            msg.cmd = MSG_DISPLAY_SCREEN;
                    msg.value= now_pace | (goal_pace <<16); //��ǰֵ��16λ��Ŀ��ֵ��16λ
		            xQueueSend(DisplayQueue, &msg, portMAX_DELAY);							
	           }	
            }

        }

    }

}
/***************
����Ϊѵ���ƻ�����
*/
/*����˵��:ѵ���ƻ��Ƿ���Ч 
�ж�������SetValue.TrainPlanFlagֵ:��ֵ�����·�ѵ���ƻ������
������ѵ���ƻ��·����ݵ�MD5ͷ������MD5ͷ��ͬ����Ϊѵ���ƻ��������Ч
������������������,����Ϊ��ǰѵ���ƻ���Ч*/
bool get_trainplan_valid_flag(void)
{
	_com_config_train_plan_MD5_UUID  MD5_UUIDStr,MD5_UUIDResultStr;
	bool m_md5str_status = false,m_uuidstr_status = false,status = false;
	dev_extFlash_enable();
	//���ϴ�ѵ���ƻ�������ȡMD5��UUID
	dev_extFlash_read(TRAINING_DATA_START_ADDRESS,(uint8_t*)(&MD5_UUIDStr),sizeof(_com_config_train_plan_MD5_UUID));
	//д�뵽ѵ���ƻ����ͷ��
	dev_extFlash_read(TRAINING_DATA_UPLOAD_ADDRESS,(uint8_t*)(&MD5_UUIDResultStr),sizeof(_com_config_train_plan_MD5_UUID));
	dev_extFlash_disable();
	m_md5str_status = strcmp((char *)MD5_UUIDStr.Md5Str,(char *)MD5_UUIDResultStr.Md5Str);
	m_uuidstr_status = strcmp((char *)MD5_UUIDStr.UUIDStr,(char *)MD5_UUIDResultStr.UUIDStr);
	status = m_md5str_status & m_uuidstr_status & SetValue.TrainPlanFlag;
//	COM_SPORT_PRINTF("[com_sport]: m_md5str_status=%d,m_uuidstr_status=%d,status=%d\n"
//	  ,m_md5str_status,m_uuidstr_status,status);
	
	return status;
}
/*����˵��:��ȡѵ���ƻ����������ʱ�俪��״̬
1���� �����ѵ���ƻ�������ѵ���ƻ�δ���,0���� δ�������߿�������� ֪ͨ�Ƿ�򿪵�
*/
uint8_t Get_TrainPlanFlag(void)
{
	return (get_trainplan_valid_flag() && (Upload[1].IsComplete != 1) && Is_TodayTimeTrainPlan()&&Get_TrainPlan_Notify_IsOpen());
}
/*��������:����ѵ���ƻ�Ŀ������Ƿ���*/
static uint8_t IsComplete_TargetDistance(void)
{
	uint8_t isComplete = 0;
	
	if(Upload[1].Distance >= Download[1].TargetDistance && Upload[1].Distance != 0
		 && Upload[1].Distance != 0xFFFF)
	{
		isComplete = 1;
	}
	else
	{
		isComplete = 0;
	}
	return isComplete;
}
/*��������:����ѵ���ƻ�Ŀ��ʱ���Ƿ���*/
static uint8_t IsComplete_TargetHour(void)
{
	uint8_t isComplete = 0;
	
	if(Upload[1].Hour >= ((Download[1].TargetHour)*60) && Upload[1].Hour != 0
		 && Upload[1].Hour != 0xFFFF)
	{
		isComplete = 1;
	}
	else
	{
		isComplete = 0;
	}
	return isComplete;
}
/*��������:����ѵ���ƻ�Ŀ�꿨·���Ƿ���*/
//static uint8_t IsComplete_TargetCalories(void)
//{
//	uint8_t isComplete = 0;
//	
//	if(Upload[1].Calories*1000 >= Download[1].TargetCalories && Upload[1].Calories != 0
//    && Upload[1].Calories != 0xFFFF)
//	{
//		isComplete = 1;
//	}
//	else
//	{
//		isComplete = 0;
//	}
//	return isComplete;
//}
/*��������:��������ѵ����Ƿ��� 
ע:��ͬ�����˶����۱�׼��ͬ
���:  TrainPlanType����ֵ
����ֵ:true��꣬false�����
*/
uint8_t Get_IsCompleteBothTodayTrainPlan(TrainPlanType type)
{
	uint8_t isComplete = 0;
	
	//��������ѡ�����۱�׼ ��·���׼����
	switch(type)
	{
		case EasyToRun_Distance: //������-����
		case NormalRun_Distance://�ܲ�-����
		case TempoRuns_Distance://������-����
		case PacetoRun_Distance://������-����
		case Intervals_Distance://��Ъѵ��-����
		case Marathon_Distance://������-����
		case Rest_Run_Distance://��Ϣ����-����
			isComplete = IsComplete_TargetDistance();//����
			break;
			
		case EasyToRun_Time://������-ʱ��
		case NormalRun_Time://�ܲ�-ʱ��
		case TempoRuns_Time://������-ʱ��
		case PacetoRun_Time://������-ʱ��
		case Intervals_Time://��Ъѵ��-ʱ��
			isComplete = IsComplete_TargetHour();//ʱ��
			break;
		case Rest://��Ϣ
		case StrengthTraining://����ѵ��
		case CrossTraining://����ѵ��
		case Rest_StrengthTraining://��Ϣ������ѵ��
		case Rest_CrossTraining://��Ϣ�򽻲�ѵ��
			isComplete = 1;//Ĭ�����
			break;
	
		default://Ĭ�Ͼ���
			isComplete = IsComplete_TargetDistance();//����
			break;
	}
	
	return isComplete;
}

/*��������:���ý����ѵ����Ƿ���
���:true��� false�����
*/
void Set_IsCompleteTodayTrainPlan(uint8_t isComplete)
{
	Upload[1].IsComplete = isComplete;
}
/*��������:��ȡ�����ѵ����Ƿ���
����ֵ:true��� false�����
*/
uint8_t Get_IsCompleteTodayTrainPlan(void)
{
	return Upload[1].IsComplete;
}
/*��������:�ж�ѵ���ƻ�����
���: ��Download[1].Type
����ֵ:0 ��Ϣ   ֻ��ʾ��
       1 ����ѵ������Ҫ����������ɵ�
       2 ��Ϣ������ѵ���� Ĭ����ɵ������˶�
       3 �����ܺͽ����ܵ� �����������
       4 �м�Ъѵ�� 4x400�����������
			 5 �����ܲ�
       
*/
uint8_t Get_TrainPlanType(TrainPlanType type)
{
	uint8_t backType = 0;
	
	switch(type)
	{
		case Rest://��Ϣ
			backType = 0;
			break;
		case StrengthTraining://����ѵ��
		case CrossTraining://����ѵ��
		case Rest_StrengthTraining://��Ϣ������ѵ��
		case Rest_CrossTraining://��Ϣ�򽻲�ѵ��
			backType = 1;
			break;
		
		case Rest_Run_Distance://��Ϣ����-����
			backType = 2;
			break;
		case PacetoRun_Time://������-ʱ��
		case PacetoRun_Distance://������-����
		case TempoRuns_Distance://������-����
		case TempoRuns_Time://������-ʱ��
			backType = 3;
			break;
		case EasyToRun_Distance: //������-����
		case NormalRun_Distance://�ܲ�-����
		case Intervals_Distance://��Ъѵ��-����
		case Marathon_Distance://������-����
		case EasyToRun_Time://������-ʱ��
		case NormalRun_Time://�ܲ�-ʱ��
			backType = 5;
			break;
		case Intervals_Time://��Ъѵ��-ʱ��
			backType = 4;
			break;
		default:
			backType = 0;
			break;
	}
	return backType;
}
/*��������:��ȡ��Ъ�� 4x400���е����ܵ�400����
��Ŀ�����=(�����ܾ���+�����߾���)x����
*/
uint16_t Get_Intervals_TrainPlan_Single_Distance(void)
{
	uint16_t distance = 0;
	if(Download[1].IntermittentRun != 0)
	{
		distance = (Download[1].TargetDistance/Download[1].IntermittentRun)- Download[1].RestTime;
	}
	else
	{
		distance = 0;
	}
	return distance;
}
/*��������:ѵ��������
��Ϊ���ڷ�ѵ���ƻ��ʹ��ڷ�ѵ���ƻ��˶�ģʽ��ʱ
ǰ��ֱ�����ѣ������˶�ģʽ��������Ҫ����,ֻҪ���ڵ��죬ѵ���ƻ���Ҫ����
����ǰ��Ҫ��FLASH�л�ȡ��Ӧ��ѵ���ƻ�(3��)
*/
void Remind_TodayTrainPlan (void)
{
#ifndef COD 
	if(SetValue.IsNeedClearTrainPlanDayHint == 1 && ScreenState == DISPLAY_SCREEN_NOTIFY_TRAINING_DAY
		&& RTC_time.ui32Minute == 0 && RTC_time.ui32Hour == 0)
	{//��������ѵ���ƻ�����һֱ��,���˵ڶ����0����Ҫ����������,�˳���������
		SetValue.IsNeedClearTrainPlanDayHint = 0;
		DISPLAY_MSG  msg = {0,0};
		ScreenState = DISPLAY_SCREEN_HOME;
		msg.cmd = MSG_DISPLAY_SCREEN;
		xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
	}
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
	if(get_trainplan_valid_flag() && Is_TodayTimeTrainPlan() && (Get_IsCompleteTodayTrainPlan() != 1)
		 && Get_TrainPlan_Notify_IsOpen() == true)
	{//ѵ���ƻ��Ƿ���� ���촦��ѵ���ƻ��е�ĳ�� ��Ŀ��δ��� ֪ͨ���ش�
		if(Get_IsTrainPlanOnGoing() == false && IsSportMode(ScreenState) == false && IsSportReady(ScreenState) == false 
			&& Get_IsInSportMode() == false)
		{// ����ʱ�� ���ڷ��˶�ģʽ
			if((TrainPlanDownloadSettingStr.Hint.Hour == RTC_time.ui32Hour 
			   && TrainPlanDownloadSettingStr.Hint.Minute == RTC_time.ui32Minute)
			   || (SetValue.TrainPlanHintFlag == 1 
			   && (RTC_time.ui32Hour*60 + RTC_time.ui32Minute ) > (TrainPlanDownloadSettingStr.Hint.Hour*60 + TrainPlanDownloadSettingStr.Hint.Minute)))
			{//����ʱ��
				SetValue.TrainPlanHintFlag = 0;
				//��ȡѵ���ƻ�
				if(get_trainplan_valid_flag())
				{//��ѵ���ƻ�ͬ��
					Get_TrainPlanDayRecord();
				}
				if((ScreenState != DISPLAY_SCREEN_LOGO)
					&& (ScreenState != DISPLAY_SCREEN_NOTIFY_BAT_CHG)
					&& time_notify_is_ok())
				{
					if(ScreenState < DISPLAY_SCREEN_NOTIFY)
					{
						ScreenStateSave = ScreenState;
					}
					SetValue.IsNeedClearTrainPlanDayHint = 1;
					timer_notify_motor_start(1000,500,3,false,NOTIFY_MODE_SYSTEM_TIME_REACH);
					timer_notify_buzzer_start(1000,500,3,false,NOTIFY_MODE_SYSTEM_TIME_REACH);
					DISPLAY_MSG  msg = {0,0};
					ScreenState = DISPLAY_SCREEN_NOTIFY_TRAINING_DAY;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				}	
			}
		}
		else 	if(Get_IsTrainPlanOnGoing() == false && (IsSportMode(ScreenState) == true || IsSportReady(ScreenState) == true 
			   || Get_IsInSportMode() == true))
		{//���ڷ�ѵ���ƻ��˶�ģʽ��
			static uint16_t hint_minute = 0,realtime_minute = 0;
			hint_minute = TrainPlanDownloadSettingStr.Hint.Hour * 60 + TrainPlanDownloadSettingStr.Hint.Minute;
			realtime_minute = RTC_time.ui32Hour * 60 + RTC_time.ui32Minute;
			if(hint_minute == realtime_minute)
			{
				SetValue.TrainPlanHintFlag = 1;//�Ժ�����
			}
		}
	}
#endif
}
/*��������:ѵ��Ŀ��������*/
void Remind_TrainPlan_Complete(void)
{
	//ѵ���ƻ��Ƿ���� ���촦��ѵ���ƻ��е�ĳ��  ��� ����ѵ����
	if((ScreenState != DISPLAY_SCREEN_LOGO)
		&& (ScreenState != DISPLAY_SCREEN_NOTIFY_BAT_CHG)
		&& time_notify_is_ok())
	{
		if(ScreenState < DISPLAY_SCREEN_NOTIFY)
		{
			ScreenStateSave = ScreenState;
		}
//		timer_notify_display_start(8000,1,false);
		timer_notify_motor_start(1000,500,3,false,NOTIFY_MODE_SPORTS);
		timer_notify_buzzer_start(1000,500,3,false,NOTIFY_MODE_SPORTS);
		DISPLAY_MSG  msg = {0,0};
		ScreenState = DISPLAY_SCREEN_NOTIFY_TRAINING_COMPLETE;
		msg.cmd = MSG_DISPLAY_SCREEN;
		xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
	 }
}
/*
*����:ѵ���ƻ���������
*����:now_pace ��ǰ����
			now_distance ��ǰ����
      up_pace ��������
     down_pace ��������

�������޷�Χ�ڵ����ٲ����ѣ��������ѣ���ÿ31s����һ�Σ���ѵ���ƻ���ɣ����ٲ�������
*����ֵ :��

*��ע:
*/
void Remind_Pace_TrainPlan (uint16_t now_pace,uint16_t now_distance,uint16_t up_pace,uint16_t down_pace)
{
	if(get_trainplan_valid_flag() && Is_TodayTimeTrainPlan() && (Get_IsCompleteTodayTrainPlan() != 1)
		&& Get_IsTrainPlanOnGoing())
	{
		if(Get_TrainPlanType(Download[1].Type) == 3)
		{//�����������
			speed_trainplan_nums++;
			if (now_pace > up_pace || now_pace < down_pace)
			{
					//��������
					if (speed_trainplan_nums > 31)//���31s���� ��ֹ����ѵ���ƻ�������ѽ���
					{
						speed_trainplan_nums = 0;
						if((ScreenState != DISPLAY_SCREEN_LOGO)
							&& (ScreenState != DISPLAY_SCREEN_NOTIFY_BAT_CHG)
							&& time_notify_is_ok())
						{
							if(ScreenState < DISPLAY_SCREEN_NOTIFY)
							{
								ScreenStateSave = ScreenState;
							}
							timer_notify_display_start(3000,1,false);
							timer_notify_motor_start(1000,500,3,false,NOTIFY_MODE_SPORTS);
							timer_notify_buzzer_start(1000,500,3,false,NOTIFY_MODE_SPORTS);
							DISPLAY_MSG  msg = {0,0};
							ScreenState = DISPLAY_SCREEN_NOTIFY_PACE_TRAINPLAN;
							msg.cmd = MSG_UPDATE_PACE_TRAINPLAN_SPORT;
							msg.value = now_pace;
							xQueueSend(DisplayQueue, &msg, portMAX_DELAY);							
						}	
					}
			}
		}
		else if(Get_TrainPlanType(Download[1].Type) == 4)
		{//�м�Ъѵ�� 4x400�����������
			for(uint8_t i=1;i<(Download[1].IntermittentRun*2);i++)
			{
				if((i+1)%2 == 0)
				{//������
					if(now_distance >= (Get_Intervals_TrainPlan_Single_Distance() * (i+1)/2+Download[1].RestTime*((i+1)/2-1))
						  && distance_circle_nums == (i-1))
					{
						distance_circle_nums++;
						if((ScreenState != DISPLAY_SCREEN_LOGO)
							&& (ScreenState != DISPLAY_SCREEN_NOTIFY_BAT_CHG)
							&& time_notify_is_ok())
						{
							if(ScreenState < DISPLAY_SCREEN_NOTIFY)
							{
								ScreenStateSave = ScreenState;
							}
							timer_notify_display_start(3000,1,false);
							timer_notify_motor_start(1000,500,3,false,NOTIFY_MODE_SPORTS);
							timer_notify_buzzer_start(1000,500,3,false,NOTIFY_MODE_SPORTS);
							DISPLAY_MSG  msg = {0,0};
							ScreenState = DISPLAY_SCREEN_NOTIFY_PACE_INTER_TRAINPLAN;
							msg.cmd = MSG_UPDATE_INTERVAL_TRAINPLAN_SPORT;
							msg.value = distance_circle_nums;
							xQueueSend(DisplayQueue, &msg, portMAX_DELAY);							
						}	
					}
				}
				else
				{//������
					if(now_distance >= ((Get_Intervals_TrainPlan_Single_Distance()+Download[1].RestTime)*(i/2))
						&& distance_circle_nums == (i-1))
					{
						distance_circle_nums++;
						if((ScreenState != DISPLAY_SCREEN_LOGO)
							&& (ScreenState != DISPLAY_SCREEN_NOTIFY_BAT_CHG)
							&& time_notify_is_ok())
						{
							if(ScreenState < DISPLAY_SCREEN_NOTIFY)
							{
								ScreenStateSave = ScreenState;
							}
							timer_notify_display_start(3000,1,false);
						  timer_notify_motor_start(1000,500,3,false,NOTIFY_MODE_SPORTS);
							timer_notify_buzzer_start(1000,500,3,false,NOTIFY_MODE_SPORTS);
							DISPLAY_MSG  msg = {0,0};
							ScreenState = DISPLAY_SCREEN_NOTIFY_PACE_INTER_TRAINPLAN;
							msg.cmd = MSG_UPDATE_INTERVAL_TRAINPLAN_SPORT;
							msg.value = distance_circle_nums;
							xQueueSend(DisplayQueue, &msg, portMAX_DELAY);							
						}
					}
				}
			}
		}
	}

}
/*
���¹켣��ƫ�㷨
*/
bool outOfChina(double lat, double lon)
{
	if (lon < 72.004 || lon > 137.8347)
		return true;
	if (lat < 0.8293 || lat > 55.8271)
		return true;
	return false;

}

double transformLat(double x, double y) 
{
		double ret = -100.0 + 2.0 * x + 3.0 * y + 0.2 * y * y + 0.1 * x * y+ 0.2 * sqrt(abs(x));
		ret += (20.0 * sin(6.0 * x * pi) + 20.0 *sin(2.0 * x * pi)) * 2.0 / 3.0;
		ret += (20.0 * sin(y * pi) + 40.0 * sin(y / 3.0 * pi)) * 2.0 / 3.0;
		ret += (160.0 * sin(y / 12.0 * pi) + 320 *sin(y * pi / 30.0)) * 2.0 / 3.0;
		return ret;
}

double transformLon(double x, double y) 
{
		double ret = 300.0 + x + 2.0 * y + 0.1 * x * x + 0.1 * x * y + 0.1* sqrt(abs(x));
		ret += (20.0 * sin(6.0 * x * pi) + 20.0 * sin(2.0 * x * pi)) * 2.0 / 3.0;
		ret += (20.0 * sin(x * pi) + 40.0 * sin(x / 3.0 * pi)) * 2.0 / 3.0;
		ret += (150.0 * sin(x / 12.0 * pi) + 300.0 * sin(x / 30.0* pi)) * 2.0 / 3.0;
		return ret;
}
//��ƫ
void transform(double wgLon,double wgLat) {
	if (outOfChina(wgLat, wgLon))
	{
		dLatLng.dlat = wgLat;
		dLatLng.dlon = wgLon;
		return;
	}
	double dLat = transformLat(wgLon - 105.0, wgLat - 35.0);
	double dLon = transformLon(wgLon - 105.0, wgLat - 35.0);
	double radLat = wgLat / 180.0 * pi;
	double magic = sin(radLat);
	magic = 1 - ee * magic * magic;
	double sqrtMagic = sqrt(magic);
	dLat = (dLat * 180.0) / ((a * (1 - ee)) / (magic * sqrtMagic) * pi);
	dLon = (dLon * 180.0) / (a / sqrtMagic * cos(radLat) * pi);
	
	dLatLng.dlat = wgLat + dLat;
	dLatLng.dlon = wgLon + dLon;
	
}

//��ʼ���켣����
void InitTrack(void)
{
    gpslen = 0;
    gpsbacklen = 0;
    xylen = 0;
    zoom =1;
    zoommax =1;
    xybacklen = 0;
    xybackflag = 0;
    startpointflag =0;
    center_gps_lon =0;
    center_gps_lat = 0;
    drawbackflag =0;

    last_center_gps_lon = 0;
    last_center_gps_lat = 0;
    SportAngle = 0 ;
    memset(trackgpsstore,0,sizeof(trackgpsstore));
    memset(trackxystore,0,sizeof(trackxystore));

}




void StoreTrackPoint(void)
{
    //int16_t x,y,i;
    if (GetGpsStatus())
    {
#if !defined (WATCH_SPORT_NEW_CLOUD_NAVIGATION)
         transform((double)(GetGpsLon()/GPS_SCALE_DECIMAL),(double)(GetGpsLat()/GPS_SCALE_DECIMAL));
         Last_GPS_Lon =(int32_t )(dLatLng.dlon*100000);//�켣��gps�㱣��5λС���㣬�򻯼���������λ����Ϊ�׼�
         Last_GPS_Lat = (int32_t )(dLatLng.dlat*100000);
#endif
         center_gps_lon = Last_GPS_Lon;
         center_gps_lat = Last_GPS_Lat;
         trackgpsstore[gpslen] = Last_GPS_Lon;
         trackgpsstore[gpslen +1] = Last_GPS_Lat;
         gpslen +=2;
         if (gpslen> 5998)
         {
             gpslen =5998;

         }
    }

}

void DrawViewTrack(uint16_t len)
{
    uint16_t i;
    int32_t x,y;
    uint8_t drawflag =0;
    uint8_t lenaddflag = 0;
    
    int32_t center_tmp_lon = 0;
    int32_t center_tmp_lat = 0;

    center_tmp_lon = center_gps_lon;
    center_tmp_lat = center_gps_lat;
    
     xylen =0;
     xybacklen =0;
     startpointflag = 0;
     
    for (i =0; i< len;i+=2 )
     {
       x = (trackgpsstore[i] -center_tmp_lon) *240/(ZoomBase)+120;
       y = 120-(trackgpsstore[i+1] -center_tmp_lat) *240/(ZoomBase);
       
       if ((pow((x-120),2)+pow((y-120),2))< 12100)
       {
           lenaddflag =0;
           if (xylen == 0)
           {
               trackxystore[xylen] = x;
               trackxystore[xylen+1] = y;
               xylen +=2;
               lenaddflag =1;
    
            }
            else if ((trackxystore[xylen-2] != x)||(trackxystore[xylen-1] != y))
            {
               trackxystore[xylen] = x;
               trackxystore[xylen+1] = y;
               xylen +=2;
               lenaddflag =1;
            }
            if ((i >= (gpsbacklen+2))&&(xybacklen ==0)&&(xybackflag ==1))
            {
                 xybacklen = xylen;
            }

            if ((drawflag ==1) && (xylen>=4)&&(lenaddflag ==1))
            {
               if(xybackflag == 0)//�жϷ���״̬
               {
               		#ifdef COD 
					LCD_SetLine(trackxystore[xylen-1],trackxystore[xylen-2],trackxystore[xylen-3],trackxystore[xylen-4],LCD_BLACK,4);
					#else
                    LCD_SetLine(trackxystore[xylen-1],trackxystore[xylen-2],trackxystore[xylen-3],trackxystore[xylen-4],LCD_LIGHTGRAY,4);
					#endif
               }
               else
               {
                  if (xybacklen ==0)
                  {
                  	  #ifdef COD 
					  LCD_SetLine(trackxystore[xylen-1],trackxystore[xylen-2],trackxystore[xylen-3],trackxystore[xylen-4],LCD_BLACK,4);
					  #else
                      LCD_SetLine(trackxystore[xylen-1],trackxystore[xylen-2],trackxystore[xylen-3],trackxystore[xylen-4],LCD_LIGHTGRAY,4);
					  #endif
                  }
                  else
                  {
                      LCD_SetLine(trackxystore[xylen-1],trackxystore[xylen-2],trackxystore[xylen-3],trackxystore[xylen-4],LCD_AQUA,4);
                  }
               }
            
            }

            
            if (i==0)
            {
                startpointflag =1;
    
            }
    
           drawflag = 1;     
        }
       else
       {
           drawflag = 0;  

       }
    
     }
    

}

//���켣
void DrawTrack(void)
{
        //uint16_t i;
       // int32_t x,y;
        
       if(gpslen != 0)
       {
           DrawViewTrack(gpslen);

            if (startpointflag == 1)
            {
                LCD_SetPoint(trackxystore[1],trackxystore[0],LCD_CYAN,6);
            }
      }
      
}

//�켣ָ��
void Drawsport_arrows(void)
{
#if !defined WATCH_SPORT_NO_COMPASS 
	#ifdef COD 
	gui_sport_arrows(SportAngle,120,120,LCD_NAVY);
	#else
    gui_sport_arrows(SportAngle,120,120,LCD_AQUA);
	#endif
#endif
   
#if defined WATCH_SIM_SPORT
	LCD_SetPicture(LCD_CENTER_LINE -(Img_Pentastar_24X24.height/2), LCD_CENTER_ROW -(Img_Pentastar_24X24.width/2), LCD_NONE, LCD_NONE, &Img_Pentastar_24X24);
#endif 
}
//��������켣����ʾ����
void SaveGpsToXY(void)
{
     int32_t center_tmp_lon =0;
     int32_t center_tmp_lat = 0;
	 int32_t center_tmp_lon_min = 0;
     int32_t center_tmp_lat_min = 0;
	 int32_t center_tmp_lon_max = 0;
     int32_t center_tmp_lat_max = 0;
     uint16_t i;
     uint64_t lentmp;
     uint16_t zoomtmp =1;
     uint16_t zoomsave =1;
     int32_t x,y;
      for (i =0; i< gpslen;i+=2 ) 
      {

		if (i == 0)
		{
		  	center_tmp_lon_min = trackgpsstore[i];
            center_tmp_lat_min = trackgpsstore[i+1];
			center_tmp_lon_max = trackgpsstore[i];
            center_tmp_lat_max = trackgpsstore[i+1];	
		}
		else
		{
		
			  if (center_tmp_lon_min > trackgpsstore[i])
			  {
				  center_tmp_lon_min = trackgpsstore[i];
			  }
			  if (center_tmp_lat_min > trackgpsstore[i+1])
			  {
				  center_tmp_lat_min = trackgpsstore[i+1];
			  }
			  if (center_tmp_lon_max < trackgpsstore[i])
			  {
				  center_tmp_lon_max = trackgpsstore[i];
			  }
			  if (center_tmp_lat_max <trackgpsstore[i+1])
			  {
				  center_tmp_lat_max = trackgpsstore[i+1];
			  }
		}
       
      }
       center_tmp_lon =(center_tmp_lon_min+center_tmp_lon_max)/2;
       center_tmp_lat = (center_tmp_lat_min+center_tmp_lat_max)/2;
       
     for (i =0; i< gpslen;i+=2)
      {
         lentmp = pow((trackgpsstore[i]- center_tmp_lon),2)+pow((trackgpsstore[i+1]- center_tmp_lat),2);
         zoomtmp = sqrt(lentmp)/(ZoomBase_Track/2)+1;
          
           if (zoomtmp > zoomsave)
           {
               zoomsave = zoomtmp;
           }

      
      }
     
      xylen=0;
      xybacklen =0;
      for (i =0; i< gpslen;i+=2 )
       {
           x = (trackgpsstore[i] -center_tmp_lon) *240/(ZoomBase_Track*zoomsave)+120;
           y = 120-(trackgpsstore[i+1] -center_tmp_lat) *240/(ZoomBase_Track*zoomsave);
    
            if (abs(x-120)<120 && abs(y-120)< 120)
             {
                if (xylen == 0)
                 {
                      trackxystore[xylen] = x;
                      trackxystore[xylen+1] = y;
                      xylen +=2;
     
                 }
                 else if ((trackxystore[xylen-2] != x)||(trackxystore[xylen-1] != y))
               {
                    trackxystore[xylen] = x;
                    trackxystore[xylen+1] = y;
                    xylen +=2;  
               }
               if ((i >= gpsbacklen)&&(xybacklen ==0)&&(xybackflag ==1))
               {
                  xybacklen = xylen;
               }   
                        
           }               
        }
      if ((xybackflag ==1)&&(xybacklen ==0))
      {
          xybacklen =2;//��ֹֻ�е�һ��Ҳ����
    
      }

}

//�˶�����켣
void DrawSaveTrack(void)
{
      uint16_t i;
      
      if(xybackflag == 0)//�жϷ���״̬
     {
          for(i = 2; i< xylen;i+=2)
           {
           	   #ifdef COD 
				LCD_SetLine(trackxystore[i+1],trackxystore[i],trackxystore[i-1],trackxystore[i-2],LCD_BLACK,4);
				#else
                LCD_SetLine(trackxystore[i+1],trackxystore[i],trackxystore[i-1],trackxystore[i-2],LCD_LIGHTGRAY,4);
				#endif
     
           }
          
     }     
     else 
     {
    
            for(i = 2; i< xybacklen;i+=2)
            {
              #ifdef COD 
			   LCD_SetLine(trackxystore[i+1],trackxystore[i],trackxystore[i-1],trackxystore[i-2],LCD_BLACK,4);
			   #else
               LCD_SetLine(trackxystore[i+1],trackxystore[i],trackxystore[i-1],trackxystore[i-2],LCD_LIGHTGRAY,4);
			   #endif
           
            }
    
            for(i = xybacklen; i< xylen;i+=2)
            {
                   
               LCD_SetLine(trackxystore[i+1],trackxystore[i],trackxystore[i-1],trackxystore[i-2],LCD_AQUA,4);

            }
           
     }
       
    if (xylen != 0)
    {
       LCD_SetPoint(trackxystore[1],trackxystore[0],LCD_CYAN,6);
       LCD_SetPoint(trackxystore[xylen-1],trackxystore[xylen-2],LCD_RED,6);
    }

}

//ѭ�������켣��� 
void TrackBackFlag(void)
{
    //xybacklen = xylen;
    gpsbacklen = gpslen;
    xybackflag = 1;
    

}

//��ʼ�����ع켣����,���ع켣���˳��˶�ʱ����
void InitLoadTrack(void)
{
    loadlen = 0;
    loadflag =0;
    loadgpslen =0;
    zoomload =1;
    loadstartfalg =0;
    loadendfalg =0;
    TrackMarkNums = 0;
    TrackMarkIndex =0xFF;
    memset(loadxystore,0,sizeof(loadxystore));
    memset(loadgpsstore,0,sizeof(loadgpsstore));
    memset(TrackMark,0,sizeof(TrackMarkStr)*TRACK_MARK_NUM_MAX);
}
void LoadGpsToXY(void)
{
    uint16_t zoomtmp = 1;
    uint64_t lentmp;
    int32_t i =0;
    int32_t x,y;

    for(i = 0;i < loadgpslen;i+=2)
     {
         if (LOADINGFLAG == 0)
         {
             return;
         }
         lentmp = pow((loadgpsstore[i]- center_gps_load_lon),2)+pow((loadgpsstore[i+1]- center_gps_load_lat),2);
         zoomtmp = sqrt(lentmp)/(ZoomBase_Track/2)+1;
         if (zoomtmp > zoomload)
         {
             zoomload = zoomtmp;
         }
    
     
     }
      
    
     for(i = 0;i < loadgpslen;i+=2)
     {
       if (LOADINGFLAG == 0)
       {
           return;
       }
     
       x=(loadgpsstore[i] - center_gps_load_lon)*240/(ZoomBase_Track*zoomload)+120;
       y = 120 - (loadgpsstore[i+1] - center_gps_load_lat)*240/(ZoomBase_Track*zoomload);
       
    
       if (loadlen == 0)
       {
          loadxystore[loadlen] = x;
          loadxystore[loadlen+1] = y;
          loadlen +=2;
     
       }
       else if ((loadxystore[loadlen-2] != x)||(loadxystore[loadlen-1] != y))
       {
          loadxystore[loadlen] = x;
          loadxystore[loadlen+1] = y;
          loadlen +=2;
       } 
    
     }

}

//�����˶��켣
void LoadMotiontTrail(uint32_t startaddress,uint32_t endaddress)
{
    int32_t center_tmp_lon_min = 0;
    int32_t center_tmp_lat_min = 0;
	int32_t center_tmp_lon_max = 0;
    int32_t center_tmp_lat_max = 0;

    int32_t n,i;
    
    //uint16_t zoomload =1;
    int32_t lonlat[2];
   
    int16_t addtmp;
   // uint16_t loadlen = 0;
    
   // memset(loadxystore,0,sizeof(loadxystore));
    
    //ͨ���ȷֵ�5����������ĵ�λ�ü����ŵȼ�
    if (endaddress >= startaddress)//�Ƿ񳬳�������ַ
    {
    	#ifdef COD 
		n = (endaddress - startaddress)/16;
		#else
        n = (endaddress - startaddress)/8;
		#endif
       
    }
    else
    {
    	#ifdef COD 
		n = (GPS_DATA_STOP_ADDRESS +1 - startaddress + endaddress - GPS_DATA_START_ADDRESS)/16;
		#else
        n = (GPS_DATA_STOP_ADDRESS +1 - startaddress + endaddress - GPS_DATA_START_ADDRESS)/8;
		#endif
    }
          

    addtmp = n/1500 +1;

    for (i = 0;i < n;i+=addtmp)
    {
       if (LOADINGFLAG == 0)
       {
            return;
       }
	   #ifdef COD 
	    if(startaddress + 16*i < GPS_DATA_STOP_ADDRESS)
       {
           dev_extFlash_enable();
           dev_extFlash_read(startaddress +8+ 16*i,(uint8_t*)lonlat,8 );         
           dev_extFlash_disable();
       }
       else
       {
       
           dev_extFlash_enable();
           dev_extFlash_read(GPS_DATA_START_ADDRESS +8+ 16*i -(GPS_DATA_STOP_ADDRESS +1 - startaddress) ,(uint8_t*)lonlat,8 );
           dev_extFlash_disable();
        
       }
	   #else
        if(startaddress + 8*i < GPS_DATA_STOP_ADDRESS)
       {
           dev_extFlash_enable();
           dev_extFlash_read(startaddress + 8*i,(uint8_t*)lonlat,8 );         
           dev_extFlash_disable();
       }
       else
       {
       
           dev_extFlash_enable();
           dev_extFlash_read(GPS_DATA_START_ADDRESS + 8*i -(GPS_DATA_STOP_ADDRESS +1 - startaddress) ,(uint8_t*)lonlat,8 );
           dev_extFlash_disable();
        
       }
	   #endif
        transform((double)(lonlat[0]/100000.0),(double)(lonlat[1]/100000.0));//�洢��gps����5λ
        loadgpsstore[loadgpslen] =(int32_t)(dLatLng.dlon*100000);//�켣��gps�㱣��5λС����
        loadgpsstore[loadgpslen+1] =(int32_t)(dLatLng.dlat*100000);   

		if (i==0)
		{
		  	center_tmp_lon_min = loadgpsstore[loadgpslen];
            center_tmp_lat_min = loadgpsstore[loadgpslen+1];
			center_tmp_lon_max = loadgpsstore[loadgpslen];
            center_tmp_lat_max = loadgpsstore[loadgpslen+1];	
		}
		else
		{
		
			  if (center_tmp_lon_min > loadgpsstore[loadgpslen])
			  {
				  center_tmp_lon_min = loadgpsstore[loadgpslen];
			  }
			  if (center_tmp_lat_min > loadgpsstore[loadgpslen+1])
			  {
				  center_tmp_lat_min = loadgpsstore[loadgpslen+1];
			  }
			  if (center_tmp_lon_max < loadgpsstore[loadgpslen])
			  {
				  center_tmp_lon_max = loadgpsstore[loadgpslen];
			  }
			  if (center_tmp_lat_max < loadgpsstore[loadgpslen+1])
			  {
				  center_tmp_lat_max = loadgpsstore[loadgpslen+1];
			  }
		}
		
		loadgpslen +=2;
        
     }
    
    center_gps_load_lon = (center_tmp_lon_min+center_tmp_lon_max)/2;
    center_gps_load_lat =  (center_tmp_lat_min+center_tmp_lat_max)/2;
  
    LoadGpsToXY();
 
    loadflag =1;    

}



//�����Ƽ������켣
void LoadNavigation(uint32_t startaddress,uint32_t endaddress)
{
    int32_t center_tmp_lon_min = 0;
    int32_t center_tmp_lat_min = 0;
	int32_t center_tmp_lon_max = 0;
    int32_t center_tmp_lat_max = 0;
    int32_t tmp_lon = 0;
    int32_t tmp_lat = 0;
    //int32_t last_tmp_lon = 0;
    //int32_t last_tmp_lat = 0;
    //int64_t pow_len,tmp_len,tmp_nums;
    uint32_t n,i;//j;
   
   //int16_t zoomload =1;
    float lonlat[2];
    
    int16_t addtmp;
    //uint16_t loadlen = 0;
    
   // memset(loadxystore,0,sizeof(loadxystore));

    n = (endaddress - startaddress)/8;
      
    addtmp = n/1500 +1;
//    COM_SPORT_PRINTF("[com_sport]:start-->loadgpslen=%d,n=%d,addtmp=%d,start=0x%x,end=0x%x\r\n"
//				,loadgpslen,n,addtmp,startaddress,endaddress);
     for (i = 0;i < n;i+=addtmp)
    {
        if (LOADINGFLAG == 0)
         {
           return;
         }
         dev_extFlash_enable();
         dev_extFlash_read(startaddress + 8*i,(uint8_t*)lonlat,8 );         
         dev_extFlash_disable();
         tmp_lon = (int32_t)(lonlat[0]*100000);//�켣��gps�㱣��5λС����
         tmp_lat = (int32_t)( lonlat[1]*100000);
         if (i==0)
        {
            loadgpsstore[loadgpslen] = tmp_lon;
            loadgpsstore[loadgpslen+1] =tmp_lat;
            center_tmp_lon_min = loadgpsstore[loadgpslen];
            center_tmp_lat_min = loadgpsstore[loadgpslen+1];
			center_tmp_lon_max = loadgpsstore[loadgpslen];
            center_tmp_lat_max = loadgpsstore[loadgpslen+1];
			
            loadgpslen +=2;
        }
        else if (loadgpslen < 3000)
        {
				loadgpsstore[loadgpslen] = tmp_lon;//loadgpsstore[loadgpslen-2] + (tmp_lon - last_tmp_lon)/tmp_nums;
				loadgpsstore[loadgpslen+1] =tmp_lat;//loadgpsstore[loadgpslen-1] + (tmp_lat - last_tmp_lat )/tmp_nums;

			  if (center_tmp_lon_min > loadgpsstore[loadgpslen])
			  {
				  center_tmp_lon_min = loadgpsstore[loadgpslen];
			  }
			  if (center_tmp_lat_min > loadgpsstore[loadgpslen+1])
			  {
				  center_tmp_lat_min = loadgpsstore[loadgpslen+1];
			  }
			  if (center_tmp_lon_max < loadgpsstore[loadgpslen])
			  {
				  center_tmp_lon_max = loadgpsstore[loadgpslen];
			  }
			  if (center_tmp_lat_max < loadgpsstore[loadgpslen+1])
			  {
				  center_tmp_lat_max = loadgpsstore[loadgpslen+1];
			  }

			if(loadgpslen < 3000)
			{
				loadgpslen +=2;
			}
			else
			{
				loadgpslen =3000;
				//break;
			}
          }

//        }
//        last_tmp_lon = tmp_lon;
//        last_tmp_lat = tmp_lat;

        
     }
    center_gps_load_lon = (center_tmp_lon_min+center_tmp_lon_max)/2;
    center_gps_load_lat =  (center_tmp_lat_min+center_tmp_lat_max)/2;
    
   
    LoadGpsToXY();
    
    loadflag =1;    


}
void DrawLoadViewTrack(uint16_t len)
{
    uint8_t drawflag =0;
    int32_t x,y;
    uint16_t i;
    int32_t center_tmp_lon = 0;
    int32_t center_tmp_lat = 0;
    uint8_t lenaddflag =0;

    center_tmp_lon = center_gps_lon;
    center_tmp_lat = center_gps_lat;
    
    for (i =0; i< len;i+=2 )
    {

    x = (loadgpsstore[i] -center_tmp_lon) *240/(ZoomBase)+120;
    y = 120-(loadgpsstore[i+1] -center_tmp_lat) *240/(ZoomBase);
            
    if ((pow((x-120),2)+pow((y-120),2))< 12100)
    {
        lenaddflag =0;
       if (loadlen == 0)
       {
           loadxystore[loadlen] = x;
           loadxystore[loadlen+1] = y;
           loadlen +=2;
           lenaddflag =1;
      
       }
       else if ((loadxystore[loadlen-2] != x)||(loadxystore[loadlen-1] != y))
       {
           loadxystore[loadlen] = x;
           loadxystore[loadlen+1] = y;
           loadlen +=2;
           lenaddflag =1;
       } 
    
       if ((drawflag ==1) && (loadlen>=4)&&(lenaddflag ==1))
       {
          LCD_SetLine(loadxystore[loadlen-1],loadxystore[loadlen-2],loadxystore[loadlen-3],loadxystore[loadlen-4],LCD_OLIVERDRAB,4);
    
       }
              
       //���
       if (i==0)
       {
           loadstartfalg =1;
                  
       }
       //�յ�
       if (i == len -2)
       {
           loadendfalg =1;
                
       }
              
        drawflag = 1;
        
     }
     else
     {
        drawflag = 0;
     }
  }

}

void DrawLoadTrack(void)
{
    //uint16_t i,len;
   // int32_t x,y;
    
   if (gpslen ==0)
    {
       DrawLoadALLTrack();

    }
   else if (loadgpslen !=0)
   {  
        loadlen =0;
        loadstartfalg = 0;
        loadendfalg = 0;
        
      //������ع켣����ʾ��Χ�ڵĵ�
			
      DrawLoadViewTrack(loadgpslen);
      
     
       if (loadstartfalg == 1)
       {
           LCD_SetPoint(loadxystore[1],loadxystore[0],LCD_CYAN,6);
       }
       if (loadendfalg ==1)
       {
           LCD_SetPoint(loadxystore[loadlen-1],loadxystore[loadlen -2],LCD_RED,6);
       }
      
    }

}

void DrawLoadALLTrack(void)
{
    uint16_t i;
    
    for(i = 2; i< loadlen;i+=2)
    {
        LCD_SetLine(loadxystore[i+1],loadxystore[i],loadxystore[i-1],loadxystore[i-2],LCD_OLIVERDRAB,4);
    
    }
    
    if (loadlen !=0)
    {
         LCD_SetPoint(loadxystore[1],loadxystore[0],LCD_CYAN,6);
         LCD_SetPoint(loadxystore[loadlen-1],loadxystore[loadlen -2],LCD_RED,6);
         
    }


}


//���غ������
void load_track_mark(uint32_t startaddress,uint32_t endaddress)
{

    uint8_t i;
    
    TrackMarkNums = (endaddress - startaddress)/sizeof(TrackMarkStr);
	
    if (TrackMarkNums > TRACK_MARK_NUM_MAX)
    {
        TrackMarkNums = TRACK_MARK_NUM_MAX;
    }

    for (i =0;i < TrackMarkNums; i++)
    {
        //��ȡ�������
        dev_extFlash_enable();
        dev_extFlash_read(startaddress + sizeof(TrackMarkStr)*i,(uint8_t*)&(TrackMark[i]),sizeof(TrackMarkStr));   
        dev_extFlash_disable();
    }
#if defined WATCH_SPORT_NEW_CLOUD_NAVIGATION
		//ˢ����ʱ��ע��
		memset(&m_compare_track_mark,0,sizeof(TrackMarkStr)*TRACK_MARK_NUM_MAX);
		memcpy(&m_compare_track_mark,&TrackMark,sizeof(TrackMarkStr)*TrackMarkNums);
#endif

}

//���������
void draw_track_mark(void)
{
    int32_t x,y;
    if(TrackMarkNums != 0)
    {
  		
        x = (TrackMark[TrackMarkIndex].lon *100000-center_gps_lon) *240/(ZoomBase)+120;
        y = 120-(TrackMark[TrackMarkIndex].lat*100000 -center_gps_lat) *240/(ZoomBase);
			

        if ((pow((x-120),2)+pow((y-120),2))< 12100)
        { 
            SetWord_t word = {0};
            word.x_axis = y -(Img_Pentastar_24X24.height/2);
			word.y_axis = x - (Img_Pentastar_24X24.width/2);
			LCD_SetPicture(word.x_axis, word.y_axis, LCD_NONE, LCD_NONE, &Img_Pentastar_24X24);
               
            word.x_axis = 70;
            word.y_axis = LCD_CENTER_JUSTIFIED;
            word.size = LCD_FONT_16_SIZE;
            word.bckgrndcolor = LCD_NONE;
            word.forecolor = LCD_WHITE;
            word.kerning = 0;
            LCD_SetString((char *)TrackMark[TrackMarkIndex].Name,&word); 
			  
        }   
         
    }
}

/*
���뵱ǰ������ı�㣬
���������ǰ��γ��
����ֵ�����������ֵ���
*/
uint8_t min_track_mark(int32_t lon,int32_t lat)
{
    
    uint8_t i;
    uint64_t min_sum ;
    uint8_t trackpointindex;
    uint64_t tmp;
	min_sum =  pow((TrackMark[0].lon*100000 - lon),2) + pow((TrackMark[0].lat*100000 - lat),2);
	trackpointindex =0;

    for (i = 1; i < TrackMarkNums; i++)
    {
          tmp = pow((TrackMark[i].lon*100000 - lon),2) + pow((TrackMark[i].lat*100000 - lat),2);
    
          if (tmp < min_sum)
          {
              min_sum = tmp;
              trackpointindex = i;
          }
    
    }
    //�������ľ��������ʾ������Լ300�ײ���ʾ��
    if(min_sum > 90000)
    {
        trackpointindex = 0xFF;
    }

    return trackpointindex;


}

/*��ȡ��������GPS���һ����ַ*/
uint32_t GetTriathionGPSEndAddress(uint32_t startAddr)
{
	uint32_t mEndAddr = 0;
	ActivityDataStr TriathionGPSData;
	dev_extFlash_enable();
	for(uint8_t m=0;m<5;m++)
	{
		memset(&TriathionGPSData,0,sizeof(ActivityDataStr));
		dev_extFlash_read( startAddr+m*sizeof(ActivityDataStr),(uint8_t*)(&TriathionGPSData),sizeof(ActivityDataStr));
		if( TriathionGPSData.Activity_Type == ACT_TRIATHLON_SWIM
			|| TriathionGPSData.Activity_Type == ACT_TRIATHLON_FIRST
			|| TriathionGPSData.Activity_Type == ACT_TRIATHLON_CYCLING
			|| TriathionGPSData.Activity_Type == ACT_TRIATHLON_SECOND
			|| TriathionGPSData.Activity_Type == ACT_TRIATHLON_RUN)
		{
			mEndAddr = TriathionGPSData.GPS_Stop_Address;
		}
		else
		{
			//mEndAddr = 0;
			break;
		}	
	}
	dev_extFlash_disable();
	
	return mEndAddr;
}
#if defined(WATCH_AUTO_BACK_HOME)
/*
������ˢ��ҳ�泬��10�������κβ������ް������������ش�������
�˶���:�˶�������(���˶�׼��,���˶�����)
������:GPS��λ��ָ���롢������⡢����ʱ��Ѱ���ֻ�������ʱ�ȹ����Բ���(����������)
��ݹ�����:���ʲ��ԡ��Ʋ�����������������С������в���
�˵���:��������С�������ʱ����
����֪ͨ��:��硢��פ֪ͨ(ѵ���ƻ���������������)������
���⹦����:SOS���ܡ�CO��⹦�ܵ�
����:�������桢�Լ��
����:true ����ˢ�½�������Զ����ش�������,false��ˢ�½��治�����Զ�����
*/
bool is_auto_back_home(ScreenState_t index)
{
	bool m_status = false;
	
	if(IsSportMode(index) == true || IsSportReady(index) == true || index == DISPLAY_SCREEN_SPORT)
	{//�˶���:�˶�������(���˶�׼��)
		m_status = false;
	}
	else if((index >= DISPLAY_SCREEN_TOOL && index < DISPLAY_SCREEN_WEATHER_INFO)
		     || (index == DISPLAY_SCREEN_NOTIFY_FIND_WATCH))
	{//������:GPS��λ��ָ����ȹ����Բ���
		m_status = false;
	}
	else if(index == DISPLAY_SCREEN_HEARTRATE || index == DISPLAY_SCREEN_HEARTRATE_STABLE
					|| index == DISPLAY_SCREEN_STEP)
	{//��ݹ�����:���ʲ��ԡ��Ʋ�����������������С������в���(�˶����Ѱ���)
		m_status = false;
	}
	
	else if(index == DISPLAY_SCREEN_ACCESSORY)
	{//�˵���:��������С�������ʱ����
		m_status = false;
	}

	else if(index == DISPLAY_SCREEN_UTC_GET)
	{
		m_status = false;
	}
	else if(index == DISPLAY_SCREEN_NOTIFY_BAT_CHG || index == DISPLAY_SCREEN_NOTIFY_TRAINING_DAY
		|| index == DISPLAY_SCREEN_NOTIFY_FIND_WATCH || index == DISPLAY_SCREEN_NOTIFY_TIME_IS_MONITOR_HDR
	  || ((index >= DISPLAY_SCREEN_ACCESSORY_HEARTRATE) && (index <= DISPLAY_SCREEN_ACCESSORY_CYCLING_CADENCE))
	  || (index == DISPLAY_SCREEN_NOTIFY_ALARM))
	{//����֪ͨ��:��硢��פ֪ͨ(ѵ���ƻ���������������)������
		m_status = false;
	}
	else if((index >= DISPLAY_SCREEN_POST_SETTINGS && index <= DISPLAY_SCREEN_PC_HWT) 
		|| (index >= DISPLAY_SCREEN_TIME_CALIBRATION && index <= DISPLAY_SCREEN_TIME_CALIBRATION_REALTIME)
		|| (index == DISPLAY_SCREEN_HOME) || (index == DISPLAY_SCREEN_LOGO)
	  || (index == DISPLAY_SCREEN_TEST_CALIBRATION_VIEW_STATUS) || (index == DISPLAY_SCREEN_TEST_REAL_DIFF_TIME)
	  || (index == DISPLAY_SCREEN_STEP_WEEK_STEPS) || (index == DISPLAY_SCREEN_STEP_WEEK_MILEAGE)
	  || (index == DISPLAY_SCREEN_CALORIES_WEEK_SPORTS))
	{//����:�������桢�Լ��
		m_status = false;
	}
	else
	{
		m_status = true;
	}

	return m_status;
}
AutoBackScreenStr m_AutoBackScreenStr;

void set_m_AutoBackScreen(AutoBackScreenStr str)
{
	m_AutoBackScreenStr = str;
};
AutoBackScreenStr get_m_AutoBackScreen(void)
{
	return m_AutoBackScreenStr;
}
#endif
#if defined WATCH_SPORT_EVENT_SCHEDULE
/*����ID�Ƿ���Ч*/
bool is_sport_event_id_valid(SportEventStr m_str)
{
	//��һ��Ҫ����0 ����������Ч��ַ������Ч����
	if((m_str.U_EventSport.bits.Year > 0 && m_str.U_EventSport.bits.Year < 70) 
		  && (m_str.U_EventSport.bits.Month >= 1  && m_str.U_EventSport.bits.Month <= 12)
		  && (m_str.U_EventSport.bits.Day >= 1 && m_str.U_EventSport.bits.Day <= 31) 
	    && ( m_str.U_EventSport.bits.Hour < 24)
	    && ( m_str.U_EventSport.bits.Minute < 60)
	    && ( m_str.U_EventSport.bits.Second_Start < 60)
	    && ( m_str.U_EventSport.bits.Second_End < 60))
	{
		return true;
	}
	else
	{
		return false;
	}
}
bool is_sport_type_valid(uint32_t type)
{
	if(type == ACT_CROSSCOUNTRY || type == ACT_HIKING)
	{
		return true;
	}	
	else
	{
		return false;
	}
}
bool is_sport_status_str(SportEventStatusStr m_str)
{
	if(m_str.event_id == 0 || m_str.event_id == 0xFFFFFFFFFFFFFFFF
		 || is_sport_event_id_valid(m_str.event_str) == false
		 || is_sport_type_valid(m_str.sport_type) == false)
	{
		return false;
	}
	else
	{
		return true;
	}
}
/*�����Ƿ���Ч*/
bool is_sport_event_schedule_valid(SportEventStatusStr m_sport_event_str)
{
	if(is_sport_status_str(m_sport_event_str) && m_sport_event_str.nums >= 1)
	{//��Ч����
		return true;
	}
	else
	{
		return false;
	}
}
/*��ȡ����ID ������ʱ����miao*/
int64_t get_sport_event_id(SportEventStr str)
{
	int64_t m_back = 0;
	
	m_back = (((int64_t)str.U_EventSport.bits.Year << (64-6)) 
					| ((int64_t)str.U_EventSport.bits.Month << (64-10))
	        | ((int64_t)str.U_EventSport.bits.Day << (64-15))
					| ((int64_t)str.U_EventSport.bits.Hour << (64-20))
					| ((int64_t)str.U_EventSport.bits.Minute << (64-26))
					| ((int64_t)str.U_EventSport.bits.Second_Start << (64-32))
					| ((int64_t)str.U_EventSport.bits.Second_End << (64-38))) >> SPORT_EVENT_ID_TO_LAST_BYTE;

	
	return m_back;
}
/*
����˵��:�Ƿ��и��˶����͵���������
���:�����
���ز���:SportEventStatusStr
*/
SportEventStatusStr get_sport_event_schedule_number(uint32_t m_type)
{
	SportEventStr m_sport_event,m_compare_str,m_lase_sport_event;
	SportEventStatusStr m_event;
	int64_t m_64_sport_event = 0,m_64_compare_str = 0,m_64_new_event_id_str = 0;
	
	uint8_t m_sport_type = 0,m_sport_schedule_compare = 0;//���αȽ�
	
	memset(&m_event,0,sizeof(SportEventStatusStr));
	m_event.sport_type = m_type;//���»����
	if(is_sport_type_valid(m_type) == false)
	{//��μ��
		return m_event;
	}
	
	memset(&m_sport_event,0,sizeof(SportEventStr));
	memset(&m_compare_str,0,sizeof(SportEventStr));
	memset(&m_lase_sport_event,0,sizeof(SportEventStr));
	
	dev_extFlash_enable();
	for(uint16_t i = 0; i < 7; i++)
	{
		for(uint16_t j = 0; j < DAY_ACTIVITY_MAX_NUM; j++)
		{
			dev_extFlash_read(DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*i+ACTIVITY_DATA_START_ADDRESS+j*ACTIVITY_DAY_DATA_SIZE
			                   ,(uint8_t *)&m_sport_type,sizeof(m_sport_type));
			if(is_sport_type_valid(m_sport_type) && m_sport_type == m_type)
			{//�˶�������Ч
				dev_extFlash_read(DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*i+ACTIVITY_DATA_START_ADDRESS+j*ACTIVITY_DAY_DATA_SIZE+SPORT_EVENT_BEFORE_BYTE
												 ,(uint8_t *)&m_64_sport_event,sizeof(m_sport_event));
				memcpy(&m_sport_event,&m_64_sport_event,sizeof(m_64_sport_event));
				m_64_new_event_id_str = get_sport_event_id(m_sport_event);
				if(is_sport_event_id_valid(m_sport_event))
				{//����ID��Ч 
					//��ȡ���µ��˶�����ID
					if(m_64_compare_str <= m_64_new_event_id_str)
					{
						m_64_compare_str = m_64_new_event_id_str;
						m_lase_sport_event = m_sport_event;
					}
				}
			}
		}
	}
	if(m_64_compare_str != 0 && m_64_compare_str != 0xFFFFFFFFFFFFFFFF)
	{
		memcpy(&m_compare_str,&m_lase_sport_event,sizeof(m_sport_event));
		if(m_compare_str.U_EventSport.bits.Event_Status == 0)
		{
			//���½��� 
			dev_extFlash_disable();
			
			memset(&m_event,0,sizeof(SportEventStatusStr));
			
			return m_event;
		}
		else
		{
			//����δ����
			m_event.event_id = m_64_compare_str;//����ID
			m_event.status = m_compare_str.U_EventSport.bits.Event_Status;
			m_event.nums = m_compare_str.U_EventSport.bits.Shedule_Nums;
			m_event.total_nums = m_compare_str.U_EventSport.bits.Shedule_Nums;
			
			memcpy(&m_event.event_str,&m_compare_str,sizeof(SportEventStr));
			//����ID����������
			for(uint16_t i = 0; i < 7; i++)
			{
				for(uint16_t j = 0; j < DAY_ACTIVITY_MAX_NUM; j++)
				{
					dev_extFlash_read(DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*i+ACTIVITY_DATA_START_ADDRESS+j*ACTIVITY_DAY_DATA_SIZE
														 ,(uint8_t *)&m_sport_type,sizeof(m_sport_type));
					if(is_sport_type_valid(m_sport_type))
					{
						dev_extFlash_read(DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*i+ACTIVITY_DATA_START_ADDRESS+j*ACTIVITY_DAY_DATA_SIZE+SPORT_EVENT_BEFORE_BYTE
								 ,(uint8_t *)&m_64_sport_event,sizeof(m_sport_event));
						memcpy(&m_sport_event,&m_64_sport_event,sizeof(int64_t));
						m_sport_schedule_compare = m_sport_event.U_EventSport.bits.Shedule_Nums;
						
						m_64_sport_event = get_sport_event_id(m_sport_event);//����ID
						if(m_64_sport_event == m_event.event_id)
						{//����ID��ͬ
							if(m_sport_event.U_EventSport.bits.Event_Status == 0)
							{//���½�����־ ֹͣ����
								dev_extFlash_disable();
								memset(&m_event,0,sizeof(SportEventStatusStr));
								return m_event;
							}
							else
							{
								if(m_event.nums < m_sport_schedule_compare)
								{//��ȡ���µ�����
									m_event.status = m_sport_event.U_EventSport.bits.Event_Status;
									m_event.nums = m_sport_schedule_compare;
									m_event.total_nums = m_sport_event.U_EventSport.bits.Shedule_Nums;
									memcpy(&m_event.event_str,&m_sport_event,sizeof(SportEventStr));
								}
							}
						}
					}
				}
			}
		}
	}
	else
	{
		memset(&m_event,0,sizeof(SportEventStatusStr));
	}
	dev_extFlash_disable();
	COM_SPORT_PRINTF("[com_sport]:0000----nums=%d,sport_type=%d,status=%d,total_nums=%d,event_id=%d\r\n"
		,m_event.nums,m_event.sport_type,m_event.status,m_event.total_nums,m_event.event_id);
	return m_event;
}
/*�ܽ������˶� �˶��������˶�������ʾ
����̱�����ڵ���1��
���:�˶�����,����ID,���� SportEventStatusStr
���ز���:SportScheduleCalStr
*/
SportScheduleCalStr get_sport_event_detail_total(SportEventStatusStr m_sport_event)
{
	ActivityDataStr m_activity_data;
	SportScheduleCalStr m_sport_schedule_data;
	SportEventStr m_sport_event_str;
	int64_t m_64_sport_event = 0,m_64_new_event_id_str = 0;
	uint8_t m_sport_type = 0;
	COM_SPORT_PRINTF("[com_sport]:--1111->>event_status=%d,ActivityData.sport_event.U_EventSport.bits=%d-%02d-%02d %d:%02d:%02d-endsec:%02d,status=%d,num=%d\r\n"
								,get_sport_pause_event_status()
								,m_sport_event.event_str.U_EventSport.bits.Year
								,m_sport_event.event_str.U_EventSport.bits.Month
								,m_sport_event.event_str.U_EventSport.bits.Day
								,m_sport_event.event_str.U_EventSport.bits.Hour
								,m_sport_event.event_str.U_EventSport.bits.Minute
								,m_sport_event.event_str.U_EventSport.bits.Second_Start
								,m_sport_event.event_str.U_EventSport.bits.Second_End
								,m_sport_event.event_str.U_EventSport.bits.Event_Status
								,m_sport_event.event_str.U_EventSport.bits.Shedule_Nums
								);
	COM_SPORT_PRINTF("[com_sport]:--1111->>g_sport_event_status nums=%d,sport_type=%d,status=%d,total_nums=%d,event_id=%d\r\n"
								,m_sport_event.nums,m_sport_event.sport_type,m_sport_event.status
								,m_sport_event.total_nums,m_sport_event.event_id);
	memset(&m_activity_data,0,sizeof(ActivityDataStr));
	memset(&m_sport_schedule_data,0,sizeof(SportScheduleCalStr));

	memcpy(&m_sport_event_str,&m_sport_event.event_str,sizeof(SportEventStr));
	//��μ��
	if(is_sport_event_schedule_valid(m_sport_event) == false)
	{//����ID��Ч����������Ч
		return m_sport_schedule_data;
	}
	//���ݳ�ʼ��
	memset(&m_sport_event_str,0,sizeof(SportEventStr));
	memset(&m_sport_schedule_data,0,sizeof(SportScheduleCalStr));
	memcpy(&m_sport_schedule_data.schedule,&m_sport_event,sizeof(SportEventStatusStr));//����ֵ����ID��Ϣ

	//��Ч�˶��в��Ҹ�����ID������
	dev_extFlash_enable();
	for(uint16_t i = 0; i < 7; i++)
	{
		for(uint16_t j = 0; j < DAY_ACTIVITY_MAX_NUM; j++)
		{
			dev_extFlash_read(DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*i+ACTIVITY_DATA_START_ADDRESS+j*ACTIVITY_DAY_DATA_SIZE
			                   ,(uint8_t *)&m_sport_type,sizeof(m_sport_type));
			if(is_sport_type_valid(m_sport_type) && m_sport_type == m_sport_event.sport_type)
			{//�˶�������Ч
				dev_extFlash_read(DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*i+ACTIVITY_DATA_START_ADDRESS+j*ACTIVITY_DAY_DATA_SIZE+SPORT_EVENT_BEFORE_BYTE
												 ,(uint8_t *)&m_64_sport_event,sizeof(SportEventStr));
				memcpy(&m_sport_event_str,&m_64_sport_event,sizeof(SportEventStr));
				m_64_new_event_id_str = get_sport_event_id(m_sport_event_str);//����ID
				if(m_sport_event.event_id == m_64_new_event_id_str)
				{//����ID��ͬ
					if(m_sport_event.nums >= m_sport_event_str.U_EventSport.bits.Shedule_Nums)
					{//��ȡ��Ч���εĻ����
						dev_extFlash_read(DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*i+ACTIVITY_DATA_START_ADDRESS+j*ACTIVITY_DAY_DATA_SIZE
									 ,(uint8_t *)&m_activity_data,sizeof(ActivityDataStr));
						if(is_sport_type_valid(m_activity_data.Activity_Type) && m_activity_data.Activity_Type == m_sport_event.sport_type)
						{//���¼����Ļ��Ч
							if(m_activity_data.ActTime != 0xFFFFFFFF 
								&& m_activity_data.CircleDistance != 0xFFFF)
							{//������Ч
								m_sport_schedule_data.total_time += m_activity_data.ActTime;//����ֵ������ʱ��
								m_sport_schedule_data.total_distance += m_activity_data.ActivityDetails.CrosscountryDetail.Distance;//����ֵ�����ܾ���
								
								//����ֵ������ƽ������
								m_sport_schedule_data.total_avg_pace = CalculateSpeed(m_sport_schedule_data.total_time
																																				,0,m_sport_schedule_data.total_distance
																																				,m_activity_data.Activity_Type);
							}
						}
					}
				}
			}
		}
	}
	dev_extFlash_disable();
	memcpy(&m_sport_schedule_data.schedule,&m_sport_event,sizeof(SportEventStatusStr));//����ֵ����ID��Ϣ
	COM_SPORT_PRINTF("[com_sport]:--66->>event_status=%d,ActivityData.sport_event.U_EventSport.bits=%d-%02d-%02d %d:%02d:%02d-endsec:%02d,status=%d,num=%d\r\n"
								,get_sport_pause_event_status()
								,m_sport_schedule_data.schedule.event_str.U_EventSport.bits.Year
								,m_sport_schedule_data.schedule.event_str.U_EventSport.bits.Month
								,m_sport_schedule_data.schedule.event_str.U_EventSport.bits.Day
								,m_sport_schedule_data.schedule.event_str.U_EventSport.bits.Hour
								,m_sport_schedule_data.schedule.event_str.U_EventSport.bits.Minute
								,m_sport_schedule_data.schedule.event_str.U_EventSport.bits.Second_Start
								,m_sport_schedule_data.schedule.event_str.U_EventSport.bits.Second_End
								,m_sport_schedule_data.schedule.event_str.U_EventSport.bits.Event_Status
								,m_sport_schedule_data.schedule.event_str.U_EventSport.bits.Shedule_Nums
								);
	COM_SPORT_PRINTF("[com_sport]:--66->>g_sport_event_status nums=%d,sport_type=%d,status=%d,total_nums=%d,event_id=%d\r\n"
								,m_sport_schedule_data.schedule.nums,m_sport_schedule_data.schedule.sport_type,m_sport_schedule_data.schedule.status
								,m_sport_schedule_data.schedule.total_nums,m_sport_schedule_data.schedule.event_id);
	return m_sport_schedule_data;
}
#endif
#if defined WATCH_SPORT_NEW_CLOUD_NAVIGATION
//���Ƽ�����

/*�Ƽ����� ��Ч�˶�����*/
bool is_sport_type_cloud_navigation_valid(uint8_t sport_type)
{
	if(sport_type < ACT_RUN || sport_type > ACT_TRAIN_PLAN_SWIMMING)
	{
		return false;
	}	
	else
	{
		return true;
	}
}
/*�˶��м���Ƽ����������Ƿ���Ч*/
bool is_has_sport_cloud_navigation_valid(ShareTrackStr *share_track_data)
{
#if defined WATCH_SPORT_NEW_CLOUD_NAVIGATION
	if(is_sport_type_cloud_navigation_valid(share_track_data->Activity_Type) == false)
	{
		return false;
	}
	if(NULL == share_track_data  ||
		share_track_data->gps_end_address == share_track_data->gps_start_address ||
		0xFF == share_track_data->Number )
	{
		return 0;
	}
	else
	{
		return 1;
	}
#else
	//��Ч���������ж�
	if(NULL == share_track_data  ||
		share_track_data->gps_end_address == share_track_data->gps_start_address ||
		0xFF == share_track_data->Number)
	{
		return 0;
	}
	else
		return 1;
#endif
}
/*
�������Ƿ��������µ�ͽ��ԽҰ�ȹ켣
*/
bool is_has_sport_new_cloud_navigation(uint8_t sport_type)
{
	ShareTrackStr m_share_track_str;
	
	if(is_sport_type_valid(sport_type) == false)
	{
		return false;
	}
	memset(&m_share_track_str,0,sizeof(ShareTrackStr));
	dev_extFlash_enable();
	for(uint16_t i = 0; i < SHARED_TRACK_NUM_MAX; i++)
	{
		dev_extFlash_read(SHARE_TRACK_START_ADDR + i * ONE_SHARED_TRACK_SIZE
											,(uint8_t *)&m_share_track_str,sizeof(ShareTrackStr));
		if(m_share_track_str.Activity_Type == sport_type)
		{//�������Ч
			if(m_share_track_str.new_sign_value == SPORT_NEW_CLOUD_DISTINGUISH_SIGN_VALUE 
				 && is_has_sport_cloud_navigation_valid(&m_share_track_str))
			{//�������Ƽ����� �ҵ�ַ��Ч
				dev_extFlash_disable();
				return true;
			}
		}
	}
	dev_extFlash_disable();
	return false;
}
/*
���뵱ǰ������ı��
���������ǰ��γ�� 
����ֵ�����������ֵ���
*/
uint8_t is_track_mark_valid(int32_t lon,int32_t lat)
{
    uint8_t i = 0;
    uint64_t min_sum = 0;
    uint8_t trackpointindex = 0;
    uint64_t tmp = 0;
		
		min_sum =  pow((m_compare_track_mark[0].lon*100000 - lon),2) + pow((m_compare_track_mark[0].lat*100000 - lat),2);
	
    for (i = 1; i < TrackMarkNums; i++)
    {
				tmp = pow((m_compare_track_mark[i].lon*100000 - lon),2) + pow((m_compare_track_mark[i].lat*100000 - lat),2);
	
				if (tmp < min_sum)
				{
						min_sum = tmp;
						trackpointindex = i;
				}
    
    }

    return trackpointindex;
}
/*�Ƿ�Խ��CPx(�й���ʱ���)��   �㷨�����Ƿ�Խ��CP��
num:��ע�����lon,lat��ת������GPS��
true:Խ��,ShareTrack���cpx
*/
bool is_cross_cpx_valid(int32_t lon,int32_t lat,uint8_t num)
{
	OfftrackDetailStr m_offtrack;

	if(num >TRACK_MARK_NUM_MAX) return false;
	
	memset(&m_offtrack,0,sizeof(OfftrackDetailStr));
	
	if(lon != 0 && lat != 0)
	{
		m_offtrack = get_distCP_OfftrackDetail(lon,lat
																						,(int32_t )(TrackMark[num].lon*100000)
																						,(int32_t )(TrackMark[num].lat*100000)
																						,TRACE_BACK_DATA.sport_distance);
		if(m_offtrack.WhetherinTrack == 1)
		{//�ڹ켣��
			if(m_offtrack.RemainDistance == 0)
			{//Խ��
				return true;
			}
			else
			{//δԽ��
				return false;
			}
		}
		else
		{//δ�ڹ켣��
			if(m_offtrack.NearestDistance == 0)
			{//Խ��
				return true;
			}
			else
			{//δԽ��
				return false;
			}
		}
	}
	else
	{
		return false;
	}
	
//	float m_distance = 50;//50m
//	uint8_t i = 0;
//	uint32_t min_sum = 0;
//	uint8_t trackpointindex = 0;
//	uint64_t tmp = 0;
//	
//	if(num >TRACK_MARK_NUM_MAX) return false;
//	
////	min_sum =  pow((TrackMark[num].lon*100000 - lon),2) + pow((TrackMark[num].lat*100000 - lat),2);
//	min_sum = GpsDistance5Bit(lon
//														,lat
//														,(int32_t )(TrackMark[num].lon*100000)
//														,(int32_t )(TrackMark[num].lat*100000));

//	if(min_sum <= m_distance)
//	{
//		return true;
//	}
//	else
//	{
//		return false;
//	}
}
/*�Ƿ������¹���ʱ��
����ȡ����ʱ���g_track_mark_data
num:��������ShareTrack[SHARED_TRACK_NUM_MAX]�ڼ����켣
*/
bool is_sport_has_close_time(uint8_t num,uint8_t sport_type)
{
	uint8_t m_point_num_max = 0,m_point_position = 0;//��ע�����  ��ע��λ��

	if(num >= SHARED_TRACK_NUM_MAX)
	{
		return false;
	}
	if(IsShareTrackDataValid(&ShareTrack[num]) == false)
	{//�켣ͷ�����Ƿ���Ч
		memset(&g_track_mark_data,0,sizeof(SportTrackMarkDataStr));
		return false;
	}
	if(ShareTrack[num].point_end_address > ShareTrack[num].point_start_address
		&& ShareTrack[num].point_start_address > SHARE_TRACK_START_ADDR
		&& sizeof(TrackMarkStr) != 0)
	{//������ע��
		m_point_num_max = (ShareTrack[num].point_end_address - ShareTrack[num].point_start_address)/sizeof(TrackMarkStr);
	}
	else
	{
		memset(&g_track_mark_data,0,sizeof(SportTrackMarkDataStr));
		return false;
	}
	
	m_point_position = is_track_mark_valid(Last_GPS_Lon,Last_GPS_Lat);
	//�ҵ�����ı�ע��
	if( m_point_position < TRACK_MARK_NUM_MAX && m_point_position < m_point_num_max)
	{
		if(TrackMark[m_point_position].close_time_index > 0)
		{//�й���ʱ��
			if(is_cross_cpx_valid(Last_GPS_Lon,Last_GPS_Lat,m_point_position) == true)
			{//Խ��CP�� ����һ��CP��
				for(uint16_t i = m_point_position + 1; i < m_point_num_max;i++)
				{
					dev_extFlash_enable();
					dev_extFlash_read(ShareTrack[num].point_start_address + i * sizeof(TrackMarkStr)
															,(uint8_t *)&g_track_mark_data.track,sizeof(TrackMarkStr));
					dev_extFlash_disable();
					
					if(g_track_mark_data.track.close_time_index > 0)
					{
						g_track_mark_data.is_close_time_valid = 1;
						//��ȡ������ʱ���CP��, m_compare_track_mark �Ӽ���cp��buf���Ƴ�
						memset(&m_compare_track_mark[m_point_position],0,sizeof(TrackMarkStr));
						return true;
					}
				}
			}
			else
			{//û��Խ��CP�� ��ȡ��ǰCP�����ʱ��
				memcpy(&g_track_mark_data.track,&TrackMark[m_point_position],sizeof(TrackMarkStr));
				g_track_mark_data.is_close_time_valid = 1;
				return true;
			}
		}
		else
		{//�ñ�ע���޹���ʱ�� Ѱ����һ����ע��  ����CP��ע�����100m�ݲ�����
			for(uint16_t i = m_point_position + 1; i < m_point_num_max;i++)
			{
				dev_extFlash_enable();
				dev_extFlash_read(ShareTrack[num].point_start_address + i * sizeof(TrackMarkStr)
														,(uint8_t *)&g_track_mark_data.track,sizeof(TrackMarkStr));
				dev_extFlash_disable();
				
				if(g_track_mark_data.track.close_time_index > 0)
				{
					g_track_mark_data.is_close_time_valid = 1;
					return true;
				}
			}
		}
	}
	else
	{
		memset(&g_track_mark_data,0,sizeof(SportTrackMarkDataStr));
		return false;
	}
	memset(&g_track_mark_data,0,sizeof(SportTrackMarkDataStr));
	
	return false;
}
/*�Ƽ����� flash���ݶ�ȡ ������*/
void test_share_cloud_detail_flash_print(void)
{
	ShareTrackStr m_test_share_title;
	TrackMarkStr m_track_str[20],m_test_track[3];
	GPS_Str m_gps_str[5];
	uint32_t m_data[1000],m_track_max = 0,m_gps_max = 0,m_size = 300;//��GPS����1000���㣬ֻ��ǰ300��
	uint8_t num = 0;
	
	memset(&m_test_share_title,0,sizeof(ShareTrackStr));
	memset(&m_track_str,0,sizeof(TrackMarkStr)*20);
	memset(&m_test_track,0,sizeof(TrackMarkStr)*3);
	memset(&m_gps_str,0,sizeof(GPS_Str)*5);
	memset(&m_data,0,sizeof(uint32_t)*1000);
	
	dev_extFlash_enable();
	dev_extFlash_read(SHARE_TRACK_START_ADDR + num * ONE_SHARED_TRACK_SIZE
														, (uint8_t*)&m_test_share_title, sizeof(ShareTrackStr));
	
	m_track_max = (m_test_share_title.point_end_address - m_test_share_title.point_start_address);
	
	m_gps_max = (m_test_share_title.gps_end_address - m_test_share_title.gps_start_address);
	
	dev_extFlash_read(m_test_share_title.point_start_address
													, (uint8_t*)&m_test_track[0], sizeof(TrackMarkStr));
	dev_extFlash_read(m_test_share_title.point_start_address + sizeof(TrackMarkStr)
													, (uint8_t*)&m_test_track[1], sizeof(TrackMarkStr));
	if(m_track_max/sizeof(TrackMarkStr) > 0 && m_track_max/sizeof(TrackMarkStr) <= TRACK_MARK_NUM_MAX)
	{
		for(uint16_t i=0;i<(m_track_max/sizeof(TrackMarkStr));i++)
		{
			dev_extFlash_read(m_test_share_title.point_start_address + sizeof(TrackMarkStr)*i
														,(uint8_t*)&m_track_str[i],sizeof(TrackMarkStr));
		}
	}
	if(m_gps_max/sizeof(float) > 0)
	{
		if(m_gps_max/sizeof(float) <= 1000)
		{
			m_size = m_gps_max/sizeof(float);
		}
		for(uint32_t i=0;i<(m_size);i++)
		{
			dev_extFlash_read(m_test_share_title.gps_start_address + sizeof(uint32_t)*i
														,(uint8_t*)&m_data[i],sizeof(uint32_t));
		}
	}
	dev_extFlash_disable();

	COM_SPORT_PRINTF("[com_sport]:---->>>>TEST Title-1-->>>>number=%d,Type=0x%x,distance=%d,new_sign=0x%x,old_sign=0x%x\r\n"
			,m_test_share_title.Number,m_test_share_title.Activity_Type,m_test_share_title.diatance
			,m_test_share_title.new_sign_value,m_test_share_title.old_special_value);
	COM_SPORT_PRINTF("[com_sport]:---->>>>TEST Title-2--->>>> gps:0x%x,0x%x,point:0x%x,0x%x\r\n"
			,m_test_share_title.gps_start_address,m_test_share_title.gps_end_address
			,m_test_share_title.point_start_address,m_test_share_title.point_end_address);
	COM_SPORT_PRINTF("[com_sport]:---->>>>TEST Track-3--->>>> gpsindex:%d,timeindex:%d,hour:%d,minute:%d,lon:0x%x,lat:0x%x\r\n"
			,m_test_track[0].gps_index,m_test_track[0].close_time_index,m_test_track[0].close_time_hour
			,m_test_track[0].close_time_minute,m_test_track[0].lat,m_test_track[0].lon,m_track_str[0].lat);
	COM_SPORT_PRINTF("[com_sport]:---->>>>TEST Track-4--->>>> gpsindex:%d,timeindex:%d,hour:%d,minute:%d,lon:0x%x,lat:0x%x\r\n"
			,m_test_track[1].gps_index,m_test_track[1].close_time_index,m_test_track[1].close_time_hour
			,m_test_track[1].close_time_minute,m_test_track[1].lat,m_track_str[1].lon,m_test_track[1].lat);
	COM_SPORT_PRINTF("[com_sport]:---->>>>TEST Track Point:size=%d,num=%d\r\n",m_track_max,m_track_max/sizeof(TrackMarkStr));
	if(m_track_max/sizeof(TrackMarkStr) > 0 && m_track_max/sizeof(TrackMarkStr) <= TRACK_MARK_NUM_MAX)
	{
		for(uint16_t i=0;i<(m_track_max/sizeof(TrackMarkStr));i++)
		{
			if(i % 10 == 0)
			{
				COM_SPORT_PRINTF("\r\n");
			}
			else
			{
				COM_SPORT_PRINTF(" %d,%d,",m_track_str[i].lon,m_track_str[i].lat);
			}
		}
	}
	COM_SPORT_PRINTF(" --end!!!\r\n");
	
	COM_SPORT_PRINTF("[com_sport]:---->>>>TEST GPS Point:size=%d,num=%d\r\n",m_gps_max,m_gps_max/sizeof(float));
	if(m_gps_max/sizeof(float) > 0)
	{
		for(uint32_t i=0;i<(m_size);i++)
		{
			if(i % 10 == 0)
			{
				COM_SPORT_PRINTF("\r\n");
			}
			else
			{
				COM_SPORT_PRINTF(" %d,",m_data[i]);
			}
		}
	}
	COM_SPORT_PRINTF(" --end***\r\n");
	
}
#endif



