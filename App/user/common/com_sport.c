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

ContinueLaterDataStr ContinueLaterDataSave;  //稍后继续数据记录
TrainRecordStr TrainRecord[2];//用于界面存放运动记录数
uint8_t IS_SPORT_CONTINUE_LATER = 0;  //运动稍后继续标记

PauseTimeStr PauseTime;
extern SetValueStr SetValue;
extern _drv_bat_event   g_bat_evt;

LatLng dLatLng;//纠偏后经纬度

static double pi = 3.14159265358979324;
static double a = 6378245.0;
static double ee = 0.00669342162296594323;


bool accessory_heartrate_status = false;//心率配件连接状态
bool accessory_cycling_cadence_status = false;//踏频连接状态

static uint8_t g_TrainRecordSelect = 0;//用于运动记录界面索引选择
static uint8_t g_TrainRecordNum = 0;
static uint8_t g_TrainRecordPage = 0;

static uint8_t heartrate_threshold = 0; //心率提醒阈值计数
static uint8_t heartrate_last = 0;      //上次心率值
static uint32_t daily_kcal_last = 0;  	//上次每日卡路里总数值
static uint8_t kcal_tags = 0;			//每日卡路里提醒标识
static uint32_t daily_step_last = 0;  	//上次每日步数值
static uint8_t step_tags = 0;  			//每日步数值提醒标识
static uint16_t kcal_last = 0;          //上次卡路里值
static uint16_t circle_last = 0;        //上次圈数
static uint16_t time_last = 0;          //上次时间
static uint16_t goal_remind_distance_last = 0;	//上次目标提醒距离
static uint16_t circle_remind_distance_last = 0;//上次计圈提醒距离
static uint16_t distance_nums = 1;      //距离提醒次数
static uint16_t altitude_last = 0;      //上次高度
static uint32_t speed_nums = 0;        //速度提醒计数
static uint16_t speed_nums_limit = 60;      //速度提醒计数时间限制，初始1分钟，超过3次5分钟
static uint32_t speed_nums_cycleindex = 0;      //速度提醒计数循环次数
static uint32_t speed_trainplan_nums = 0;        //速度提醒计数
static uint32_t distance_circle_nums = 0;//训练计划4x400计数
static uint16_t circledistance_nums = 1;    //计圈圈数
static uint16_t circledistance_last = 0;    //上次计圈距离
static uint16_t kilometer_nums = 1;    //计圈圈数
static uint16_t kilometer_last = 0;    //上次计圈距离

static bool isTrainPlanOnGoing = false;//训练计划正在运动进行中
static uint8_t m_Read_Num = 2;                    //一次读三个活动数据用于显示

int32_t trackgpsstore[6000];//经纬度顺序存储

int16_t trackxystore[6000] = {0};//x,y顺序储存，

int32_t loadgpsstore[3000];//加载经纬度顺序存储

int16_t loadxystore[3000] = {0};//加载轨迹x,y顺序储存，
float start_temp_altitude[7] = {0};
float start_temp_drv_pres[7] = {1013.25};

float start_temp_drv_alt[7] = {0};


#if defined WATCH_SPORT_NEW_CLOUD_NAVIGATION
TrackMarkStr TrackMark[TRACK_MARK_NUM_MAX],m_compare_track_mark[TRACK_MARK_NUM_MAX];//航迹标点 关门时间检索用的航迹标点
extern float GpsDistance5Bit(int32_t  LonA, int32_t  LatA, int32_t  LonB, int32_t  LatB);
#else
TrackMarkStr TrackMark[TRACK_MARK_NUM_MAX];//航迹标点
extern float GpsDistance5Bit(int32_t  LonA, int32_t  LatA, int32_t  LonB, int32_t  LatB);
#endif

uint8_t TrackMarkNums = 0; //航迹标点个数
uint8_t TrackMarkIndex =0xFF;//当前航迹标点索引

uint16_t gpslen,xylen,zoom,zoommax,gpsbacklen,xybacklen;         //gps存储长度，坐标映射长度,缩放等级

uint8_t xybackflag,startpointflag,drawbackflag;//返航标志，

int32_t center_gps_lon;             //中心点经纬度
int32_t center_gps_lat;
int32_t last_center_gps_lon;             //上次中心点经纬度
int32_t last_center_gps_lat;

int32_t center_gps_load_lon;             //加载中心点经纬度
int32_t center_gps_load_lat;
extern int32_t Last_GPS_Lon;               //上次经纬度
extern int32_t Last_GPS_Lat;
uint16_t loadlen = 0; //加载轨迹映射存储长度
uint8_t loadflag = 0; //导航标志
uint16_t loadgpslen = 0;//加载GPS长度
uint16_t zoomload =1;
uint8_t loadstartfalg,loadendfalg;


uint32_t store_gps_nums = 0; //gps存储点顺序
uint32_t dot_track_nums = 0;//航迹打点个数；最多20个
uint8_t dottrack_draw_time = 0; //画航迹打点时间
uint8_t dottrack_out_time = 0;	//航迹打点到上限显示次数
uint8_t dottrack_nogps_time = 0;//航迹打点gps没有定位显示次数

uint8_t g_ActivityData_Total_Num = 0;//运动记录活动数量
ActitivyDataResult_s ActivityDataResult;//运动活动数据信息
ActivityDataStr Sport_Record[2] = {0};//存放运动记录三条信息
extern uint8_t LOADINGFLAG;
uint8_t m_InvertFlagHead = 0;//如果查到头了,翻过来从尾部查到daynum-1为止 运动记录中记录查找位置
uint8_t m_InvertFlagTail = 0;//查到末尾后从头查到daynum-1
static uint8_t m_LastIndexNum = 0;//查找运动记录时最新运动的空间

GpspointStr gps_point = {0};


uint8_t cod_gps_curpoint_num = 0;

extern gps_data_out gps_data_output;

#if defined WATCH_SPORT_NEW_CLOUD_NAVIGATION
SportTrackMarkDataStr g_track_mark_data;//关门时间
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
/*设置训练计划运动正在进行中状态*/
void Set_IsTrainPlanOnGoing(bool flag)
{
	isTrainPlanOnGoing = flag;
}
/*获取训练计划运动正在进行中状态*/
bool Get_IsTrainPlanOnGoing(void)
{
	return isTrainPlanOnGoing;
}

//心率运动模式,运动模式下加大心率功率
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


//判断是否是在运动模式下  --待完善
bool IsSportMode(ScreenState_t index)
{
	//跑步、登山，感受界面 最大摄氧量 乳酸阈 计步校准界面
#if defined WATCH_SIM_SPORT
	if((index ==DISPLAY_SCREEN_RUN)
#elif defined WATCH_COM_SPORT
	if ((index ==DISPLAY_SCREEN_RUN_PAUSE)	//暂停没有准备状态
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
			case DISPLAY_SCREEN_RUN:					//跑步
			//case DISPLAY_SCREEN_RUN_PAUSE:
			case DISPLAY_SCREEN_RUN_CANCEL:
			case DISPLAY_SCREEN_CROSSCOUNTRY:
			case DISPLAY_SCREEN_CROSSCOUNTRY_PAUSE:				//越野跑
			case DISPLAY_SCREEN_CROSSCOUNTRY_CANCEL:

			case DISPLAY_SCREEN_CLIMBING:
			case DISPLAY_SCREEN_CLIMBING_PAUSE:				//登山
			case DISPLAY_SCREEN_CLIMBING_CANCEL:
			case DISPLAY_SCREEN_SWIMMING:
			case DISPLAY_SCREEN_SWIM_PAUSE:
			case DISPLAY_SCREEN_SWIM_CANCEL:
			case DISPLAY_SCREEN_REMIND_PAUSE:
			
#endif
			case DISPLAY_SCREEN_CYCLING:				//骑行
			case DISPLAY_SCREEN_CYCLING_PAUSE:
			case DISPLAY_SCREEN_CYCLING_CANCEL:
				
			case DISPLAY_SCREEN_CROSSCOUNTRY_HIKE:		//徒步越野
			case DISPLAY_SCREEN_CROSSCOUNTRY_HIKE_PAUSE:
			case DISPLAY_SCREEN_CROSSCOUNTRY_HIKE_CANCEL:
			
			case DISPLAY_SCREEN_INDOORRUN:				//室内跑
			case DISPLAY_SCREEN_INDOORRUN_PAUSE:
			case DISPLAY_SCREEN_INDOORRUN_CANCEL:
				
			case DISPLAY_SCREEN_WALK:					//健走
			case DISPLAY_SCREEN_WALK_PAUSE:
			case DISPLAY_SCREEN_WALK_CANCEL:
				
			case DISPLAY_SCREEN_MARATHON:				//马拉松
			case DISPLAY_SCREEN_MARATHON_PAUSE:
			case DISPLAY_SCREEN_MARATHON_CANCEL:
				
			case DISPLAY_SCREEN_TRIATHLON_SWIMMING:		//铁人三项
			case DISPLAY_SCREEN_TRIATHLON_CYCLING:
			case DISPLAY_SCREEN_TRIATHLON_RUNNING:
			case DISPLAY_SCREEN_TRIATHLON_PAUSE:
			case DISPLAY_SCREEN_TRIATHLON_CANCEL:
			case DISPLAY_SCREEN_TRIATHLON_FIRST_CHANGE:
			case DISPLAY_SCREEN_TRIATHLON_SECOND_CHANGE:
				if(g_sport_status == SPORT_STATUS_READY)
				{
					//准备状态，则还未开始运动
					return false;
				}
				else
				{
					//其它都属于运动状态
					return true;
				}
			default:
				return false;
		}
	}
}


//判断是否是在运动准备模式下  --待完善
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
			//case DISPLAY_SCREEN_SWIM_POOL_SET:				//游泳长度更改	

          /*飞碗新增*/
			case DISPLAY_SCREEN_SPORT_READY://飞腕运动准备界面
			
			case DISPLAY_SCREEN_SPORT_DATA_DISPLAY_PREVIEW:		//飞腕运动数据显示预览界面
			case DISPLAY_SCREEN_SPORT_DATA_DISPLAY:		//飞腕运动数据显示界面
			case DISPLAY_SCREEN_SPORT_DATA_DISPLAY_SET:		//飞腕运动数据显示设置界面
			case DISPLAY_SCREEN_SPORT_REMIND_SET:			//飞腕运动提醒设置界面
	
			case DISPLAY_SCREEN_SPORT_REMIND_SET_HAERT:					//心率告警设置界面
			case DISPLAY_SCREEN_SPORT_REMIND_SET_PACE:				//配速提醒设置界面
			case DISPLAY_SCREEN_SPORT_REMIND_SET_DISTANCE:			//距离提醒设置界面
			case DISPLAY_SCREEN_SPORT_REMIND_SET_AUTOCIRCLE:				//自动计圈设置界面
			case DISPLAY_SCREEN_SPORT_REMIND_SET_COALKCAL:			//燃脂目标设置界面
			case DISPLAY_SCREEN_SPORT_REMIND_SET_TIME:				//时间提醒设置界面
			case DISPLAY_SCREEN_SPORT_REMIND_SET_SPEED:				//速度提醒设置界面
			case DISPLAY_SCREEN_SPORT_REMIND_SET_SWIMPOOL:				//泳池长度设置界面
			case DISPLAY_SCREEN_SPORT_REMIND_SET_GOALCIRCLE:
			case DISPLAY_SCREEN_SPORT_REMIND_SET_INDOORLENG:

			
#endif
			case DISPLAY_SCREEN_CYCLING:			//骑行
			case DISPLAY_SCREEN_CROSSCOUNTRY_HIKE:	//徒步越野
			case DISPLAY_SCREEN_INDOORRUN:			//室内跑
			case DISPLAY_SCREEN_WALK:				//健走
			case DISPLAY_SCREEN_MARATHON:			//马拉松
			case DISPLAY_SCREEN_TRIATHLON_SWIMMING:	//铁人三项
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

//是否开启过运动模式   --待完善

//是否是铁人三项运动模式 
bool IsTriathlonMode(ScreenState_t index)
{
	bool status = false;
	switch(index)
	{
		case DISPLAY_SCREEN_TRIATHLON_SWIMMING:		//铁人三项
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
/*函数说明:是否处于运动详情总结界面 */
bool is_sport_record_detail_screen(uint32_t m_screen)
{
	bool m_status = false;
	switch( m_screen )
	{
		/*进入详情总结界面前界面的按键处理  详见在各运动结束保存后开启超时检测*/

		/*以下为详情总结界面按键处理*/
		//游泳
#if (defined WATCH_SIM_SPORT || defined WATCH_COM_SPORT)
		//跑步
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
		//越野跑
#if defined WATCH_COM_SPORT
		case DISPLAY_SCREEN_CROSSCOUNTRY_DETAIL:
#else
		case DISPLAY_SCREEN_CROSSCOUNTRY_SAVE_DETIAL_1:
		case DISPLAY_SCREEN_CROSSCOUNTRY_SAVE_DETIAL_2:
		case DISPLAY_SCREEN_CROSSCOUNTRY_SAVE_DETIAL_3:
		case DISPLAY_SCREEN_CROSSCOUNTRY_SAVE_TRACK:
		case DISPLAY_SCREEN_CROSSCOUNTRY_SAVE_HEARTRATEZONE:
#endif
		//骑行
		case DISPLAY_SCREEN_CYCLING_DETAIL:
		//室内跑
		case DISPLAY_SCREEN_INDOORRUN_DETAIL:
		//健走
		case DISPLAY_SCREEN_WALK_DETAIL:
		//马拉松
		case DISPLAY_SCREEN_MARATHON_DETAIL:
		//徒步越野
		case DISPLAY_SCREEN_CROSSCOUNTRY_HIKE_DETAIL:
		//登山
#if defined WATCH_COM_SPORT
		case DISPLAY_SCREEN_CLIMBING_DETAIL:
#else
		case DISPLAY_SCREEN_CLIMBING_SAVE_DETIAL_1:
		case DISPLAY_SCREEN_CLIMBING_SAVE_DETIAL_2:
		case DISPLAY_SCREEN_CLIMBING_SAVE_TRACK:
		case DISPLAY_SCREEN_CLIMBING_SAVE_HEARTRATEZONE:
#endif
		//铁人三项
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
//设置主观评价
void SetSubjectiveEvaluate(uint8_t feel)
{
    ActivityData.ActivityDetails.CommonDetail.SubjectiveEvaluate = feel;
}
/**************************************
以下为各项运动记录数据保存
***********************************/
/*计步
*每个数据4字节，为当前总步数，单位为步，每分钟存储一个数据
*/
void	Store_StepData( uint32_t * p_step )
{
#ifndef COD

	dev_extFlash_enable();
	dev_extFlash_write( SetValue.CurrentAddress.AddressStep,(uint8_t*)(p_step),STORE_STEP_BYTE_SIZE);
	dev_extFlash_disable();
	SetValue.CurrentAddress.AddressStep += STORE_STEP_BYTE_SIZE;
	if( ( SetValue.CurrentAddress.AddressStep & 0x00000FFF ) == 0) //地址到4K边界
	{
		if( SetValue.CurrentAddress.AddressStep >= HEARTRATE_DATA_START_ADDRESS )
		{
			SetValue.CurrentAddress.AddressStep = STEP_DATA_START_ADDRESS;
		}
		dev_extFlash_enable();
		dev_extFlash_erase(SetValue.CurrentAddress.AddressStep,STORE_STEP_BYTE_SIZE);//不足4k擦4k
		dev_extFlash_disable();
	}
#endif
}
#ifdef COD
/*计步
*每个数据4字节，为当前总步数，单位为步，每分钟存储一个数据
 时间
 每个数据4字节，为当前运动总时间，单位为s，每分钟存储一个数据
*/
void  Store_cod_StepData( uint32_t * p_step,uint32_t *time)
{
	dev_extFlash_enable();
	dev_extFlash_write( SetValue.CurrentAddress.AddressStep,(uint8_t*)(p_step),STORE_STEP_BYTE_SIZE);
	dev_extFlash_write( SetValue.CurrentAddress.AddressStep+4,(uint8_t*)(time),STORE_STEP_BYTE_SIZE);
	dev_extFlash_disable();
	SetValue.CurrentAddress.AddressStep += (STORE_STEP_BYTE_SIZE *2);
	if( ( SetValue.CurrentAddress.AddressStep & 0x00000FFF ) == 0) //地址到4K边界
	{
		if( SetValue.CurrentAddress.AddressStep >= (STEP_DATA_STOP_ADDRESS +1) )
		{
			SetValue.CurrentAddress.AddressStep = STEP_DATA_START_ADDRESS;
		}
		dev_extFlash_enable();
		dev_extFlash_erase(SetValue.CurrentAddress.AddressStep,STORE_STEP_BYTE_SIZE);//不足4k擦4k
		dev_extFlash_disable();
	}
}
/*心率
* 每个数据一字节，单位为次/分，每20s存储一个数据
时间
每个数据4字节，为当前运动总时间，单位为s，每20s存储一个数据

*/
void Store_cod_HeartRateData( uint8_t *hdr_value ,uint32_t *time )
{
	dev_extFlash_enable();
	dev_extFlash_write( SetValue.CurrentAddress.AddressHeartRate,(uint8_t*)(hdr_value),DAY_BASE_HDR_LENGTH);
	dev_extFlash_write( SetValue.CurrentAddress.AddressHeartRate +1,(uint8_t*)(time),STORE_STEP_BYTE_SIZE);
	dev_extFlash_disable();
	SetValue.CurrentAddress.AddressHeartRate += (STORE_STEP_BYTE_SIZE *2);
	if( ( SetValue.CurrentAddress.AddressHeartRate & 0x00000FFF ) == 0) //地址到4K边界
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
/*心率
* 每个数据一字节，单位为次/分，每分钟存储一个数据
*/
void Store_HeartRateData( uint8_t *hdr_value )
{
#ifndef COD

	dev_extFlash_enable();
	dev_extFlash_write( SetValue.CurrentAddress.AddressHeartRate,(uint8_t*)(hdr_value),DAY_BASE_HDR_LENGTH);
	dev_extFlash_disable();
	SetValue.CurrentAddress.AddressHeartRate += DAY_BASE_HDR_LENGTH;
	if( ( SetValue.CurrentAddress.AddressHeartRate & 0x00000FFF ) == 0) //地址到4K边界
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
/*气压 
每个数据包括气压和高度两个值，每个值四个字节，气压为4字节整数，单位为帕，
高度值为四字节浮点数，单位为米。
5分钟存储数据
*/
#ifdef COD //咕咚项目实际用于公里牌、泳姿点
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
			if( ( SetValue.CurrentAddress.AddressPressure & 0x00000FFF ) == 0) //地址到4K边界
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
////咕咚项目实际用于公里牌、泳姿点
void  Stores_Stroke(uint8_t type,uint8_t laps)
{
	uint32_t timestamp;
	rtc_time_t time;
	
	time.Year= RTC_time.ui32Year;  //保存活动停止时间
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
	
	if( ( SetValue.CurrentAddress.AddressPressure & 0x00000FFF ) == 0) //地址到4K边界
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
	if( ( SetValue.CurrentAddress.AddressPressure & 0x00000FFF ) == 0) //地址到4K边界
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
/*距离
每个数据为四个字节，单位为cm。
1分钟存储数据
*/
void Store_DistanceData( uint32_t *p_dis  )
{
	dev_extFlash_enable();
	dev_extFlash_write( SetValue.CurrentAddress.AddressDistance,(uint8_t*)(p_dis),STORE_DISTANCE_BYTE_SIZE);
	dev_extFlash_disable();
	SetValue.CurrentAddress.AddressDistance += STORE_DISTANCE_BYTE_SIZE;
	if( ( SetValue.CurrentAddress.AddressDistance & 0x00000FFF ) == 0) //地址到4K边界
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
/*热量 卡路里
每个数据为四个字节，单位为卡。
1分钟存储数据
*/
void Store_EnergyData( uint32_t *caloriesValue)
{
	dev_extFlash_enable();
	dev_extFlash_write( SetValue.CurrentAddress.AddressEnergy,(uint8_t*)(caloriesValue),STORE_CALORIES_BYTE_SIZE);
	dev_extFlash_disable();
	SetValue.CurrentAddress.AddressEnergy += STORE_CALORIES_BYTE_SIZE;
	if( ( SetValue.CurrentAddress.AddressEnergy & 0x00000FFF ) == 0) //地址到4K边界
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
/*天气
按天气协议存储，实际长度为（31*7=217字节）
*/

/*速度
每个数据为两个字节，单位m/s(按各运动单位)。
1分钟存储数据
*/
void Store_SpeedData( uint16_t * p_speed )
{
	dev_extFlash_enable();
	dev_extFlash_write( SetValue.CurrentAddress.AddressSpeed,(uint8_t*)p_speed,STORE_SPEED_BYTE_SIZE);
	dev_extFlash_disable();
	SetValue.CurrentAddress.AddressSpeed += STORE_SPEED_BYTE_SIZE;
	if( ( SetValue.CurrentAddress.AddressSpeed & 0x00000FFF ) == 0) //地址到4K边界
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
*函数:自动计圈时间，轨迹
*输入:now_circledistance 当前距离,单位:米,存储距离单位:cm,实际除以100
      goal_circledistance 计圈距离
      sec 当前时间
     swsport 各运动开关，如跑步开关:SetValue.SportSet.SwSwimmg 
*返回值 :无

*备注:
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
			//距离标点
			Store_DistancePointData(GetGpsLon()/10,GetGpsLat()/10,swsport);
			#endif
		
			//计圈时间
			
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


/*距离标点
每个点数据为8字节，经度和纬度各占4字节，为带符号整数，实际值的100000倍。
经度在前，东经为正，西经为负，南纬为正，北纬为负。
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
	    if( ( SetValue.CurrentAddress.AddressDistancePoint & 0x00000FFF ) == 0) //地址到4K边界
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
/*计圈时间  单位:秒
4个字节的时长
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
		if( ( SetValue.CurrentAddress.AddressCircleTime & 0x00000FFF ) == 0) //地址到4K边界
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
/*暂停时间
每个数据为16个字节，前8个字节为暂停开始时间(实际占6字节)，后8个字节为暂停结束时间(实际占6字节)
*/
void Store_PauseTimeData(PauseTimeStr *pause)
{
#if 0 //暂停存储空间，现在用于航迹打点
	dev_extFlash_enable();
	dev_extFlash_write( SetValue.CurrentAddress.AddressPauseTime,(uint8_t*)&pause->Pause_Start_Time,6);
	dev_extFlash_write( SetValue.CurrentAddress.AddressPauseTime+8,(uint8_t*)&pause->Pause_Stop_Time,6);
	dev_extFlash_disable();
	SetValue.CurrentAddress.AddressPauseTime += 16;
	if( ( SetValue.CurrentAddress.AddressPauseTime & 0x00000FFF ) == 0) //地址到4K边界
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

/*航迹打点数据,实际使用暂停时间存储空间
每个点数据为4字节，存储gps点存储序号
*/
void Store_DotTrack(uint32_t nums)
{	

	dev_extFlash_enable();
	dev_extFlash_write( SetValue.CurrentAddress.AddressPause,(uint8_t*)(&nums),STORE_GPS_BYTE_SIZE);
	//dev_extFlash_write( SetValue.CurrentAddress.AddressPause+STORE_GPS_BYTE_SIZE,(uint8_t*)(&Lat),STORE_GPS_BYTE_SIZE);
	dev_extFlash_disable();
	SetValue.CurrentAddress.AddressPause += (STORE_GPS_BYTE_SIZE);
	if( ( SetValue.CurrentAddress.AddressPause & 0x00000FFF ) == 0) //地址到4K边界
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
		if(( SetValue.CurrentAddress.AddressGps & 0x00000FFF ) == 0) //地址到4K边界
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
		
		//距离
		dev_extFlash_enable();
		dev_extFlash_write( SetValue.CurrentAddress.AddressDistancePoint,(uint8_t*)(&distance),STORE_DISTANCE_BYTE_SIZE);
		dev_extFlash_disable();
		SetValue.CurrentAddress.AddressDistancePoint += STORE_DISTANCE_BYTE_SIZE;
		if( ( SetValue.CurrentAddress.AddressDistancePoint & 0x00000FFF ) == 0) //地址到4K边界
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



//pausesta暂停状态(传参),altitude实时海拔 ，RiseHeight累计爬升(上升高度)，Risestate爬升状态 ,RiseAlt海拔上升
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
		//经纬度
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

		
		//定位精度
		gps_point.accuracy = GetGpspdop()/100;
		//点类型
		if ((pausesta == 0)&&(last_pausesta != 1))//pausesta 0:运动;1:暂停
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
			//时间戳 s
		start.Year= RTC_time.ui32Year;  
		start.Month= RTC_time.ui32Month;
		start.Day= RTC_time.ui32DayOfMonth;
		start.Hour = RTC_time.ui32Hour;
		start.Minute = RTC_time.ui32Minute;
		start.Second = RTC_time.ui32Second;
	    gps_point.timestamp = GetTimeSecond(start);

		//2点间距离
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
				SetValue.Altitude_Offset = 0;//异常时不校准
				temp_altitude = start_zero_altitude;//异常时使用平均值
				gps_point.altitude = (int16_t)(start_zero_altitude);
				/*异常时使用GPS高度反算气压计值*/
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
			     if(((alt-algo_last_press_and_altitude[1])>MAX_RISE_EXCEPTION)||((alt-algo_last_press_and_altitude[1])<(-MAX_DECLINE_EXCEPTION)))//上升或下降值异常
			        {
                         alt = 0.5*(GetGpsAlt()/10) + 0.5*algo_last_press_and_altitude[1];//气压计异常时高度用GPS高度值代替
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
				temp_altitude = last_altitude;//异常时使用上一个值
				gps_point.altitude = (int16_t)(last_altitude);

				/*异常时使用GPS高度反算气压计值*/
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
/*活动GPS数据
每个点数据为8字节，经度和纬度各占4字节，为带符号整数，实际值的100000倍。
经度在前，东经为正，西经为负，南纬为正，北纬为负。

*/
void Store_GPSData(int32_t Lon,int32_t Lat)
{
   if ((Lon != 0)&&(Lat != 0))
   {
#if defined WATCH_SPORT_NEW_CLOUD_NAVIGATION
		if (is_crosscountry == 1) //判断是否越野
		{
			gps_store_flag = ~gps_store_flag;//间隔存储
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
			if( ( SetValue.CurrentAddress.AddressGps & 0x00000FFF ) == 0) //地址到4K边界
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
	if( ( SetValue.CurrentAddress.AddressGps & 0x00000FFF ) == 0) //地址到4K边界
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

	//防止内存越界
	if(SetValue.CurrentAddress.AddressTouchDown +len > TOUCHDOWN_STOP_ADDRESS)
	{
		return;
	}
	
	dev_extFlash_enable();
	dev_extFlash_write(SetValue.CurrentAddress.AddressTouchDown,(uint8_t*)(log),len);
	dev_extFlash_disable();
	
	SetValue.CurrentAddress.AddressTouchDown += (len);
	
	if( ( SetValue.CurrentAddress.AddressTouchDown & 0x00000FFF ) == 0) //地址到4K边界
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
	//防止内存越界
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
	if( ( SetValue.CurrentAddress.AddressCircleTime & 0x00000FFF ) == 0) //地址到4K边界
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
数据存储标志符liv_storedataflag，默认为1;
Store_OrgGPSData()原始活动GPS数据存储，g_storedataflag = 1;
Store_OrgAxisData()原始三轴数据存储，g_storedataflag = 2;
Store_OrgPresData()原始压强数据存储，g_storedataflag = 3;
Store_OrgHeartRateData()原始心率数据存储，g_storedataflag = 4.
*/
static uint8_t g_storedataflag = 1;

void Set_StoreData_Flag(uint8_t liv_storedataflag)
{
	g_storedataflag = liv_storedataflag;
}


///*
//VerticalSize存储块，0x00400000-0x004fffff
void Store_OrgData_Start()
{
	//保证运动首地址4个字节对齐
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
		//防止内存越界
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
	if( ( SetValue.CurrentAddress.AddressCircleTime & 0x00000FFF ) == 0) //地址到4K边界
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
	//防止内存越界
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
	if( ( SetValue.CurrentAddress.AddressCircleTime & 0x00000FFF ) == 0) //地址到4K边界
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
	//防止内存越界
	if(SetValue.CurrentAddress.AddressCircleTime > TOUCHDOWN_START_ADDRESS - 16)
	{
		return;
	}
	dev_extFlash_enable();
	dev_extFlash_write( SetValue.CurrentAddress.AddressCircleTime,(uint8_t*)(&liv_pres),STORE_PRES_BYTE_SIZE);
	dev_extFlash_write( SetValue.CurrentAddress.AddressCircleTime+STORE_PRES_BYTE_SIZE,(uint8_t*)(&liv_height),STORE_PRES_BYTE_SIZE);
	dev_extFlash_disable();
	SetValue.CurrentAddress.AddressCircleTime += (STORE_PRES_BYTE_SIZE*2);
	if( ( SetValue.CurrentAddress.AddressCircleTime & 0x00000FFF ) == 0) //地址到4K边界
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
	//防止内存越界
	if(SetValue.CurrentAddress.AddressCircleTime > TOUCHDOWN_START_ADDRESS - 8)
	{
		return;
	}
	dev_extFlash_enable();
	dev_extFlash_write( SetValue.CurrentAddress.AddressCircleTime,(uint8_t*)(&liv_heartrate),STORE_HEARTRATE_BYTE_SIZE);
	dev_extFlash_disable();
	SetValue.CurrentAddress.AddressCircleTime += STORE_HEARTRATE_BYTE_SIZE;
	if( ( SetValue.CurrentAddress.AddressCircleTime & 0x00000FFF ) == 0) //地址到4K边界
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
在登山活动中保存每一秒的气压高度值、GPS信号强度（6个最强的信号）、GPS高度值
每个点为16个字节,最后两个字节预留
气压高度值,GPS高度值，GPS信号强度(6个字节)
*/
#if defined STORE_ALT_TEST_VERTION && !(defined WATCH_SPORT_EVENT_SCHEDULE)//TOUCHDOWN_START_ADDRESS使用局限
void Store_AltRelateData(float *Press_alt,int32_t *Gps_alt,uint8_t *Gps_sn,uint16_t *Gps_hdop)
{
	COM_SPORT_PRINTF("Store_AltRelateData\r\n");
	COM_SPORT_PRINTF("Press_alt-->%d,Gps_alt-->%d,Gps_hdop-->%d\r\n",(uint32_t)(*Press_alt),(uint32_t)(*Gps_alt),(uint16_t)(*Gps_hdop));
	
	uint32_t Sea_Level_Pressure_Data = 0;
	static bool Sea_Level_Pressure_store_flag = false;
	//第一次保存海平面高度
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
		if( ( SetValue.CurrentAddress.AddressTouchDown & 0x00000FFF ) == 0) //地址到4K边界
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


/*读取活动数据 
返回值1表示搜索到铁人三项游泳*/
uint8_t Read_ActivityData(uint32_t addr1,uint32_t addr2)
{
	int i = 0;
	uint8_t addr1Flag = 0,addr2Flag = 0,m_TRIATHLON_flag = 0;//
	memset(&ActivityRecordData,0,sizeof(ActivityRecordData));
	memset(&ActivityRecordDataDown,0,sizeof(ActivityRecordDataDown));
	 //起始地址错误 遍历七天活动数据区间
	for(i = 0;i < MAX_RECORD_DAY;i++)
	{
		if((addr1 >= (DAY_MAIN_DATA_START_ADDRESS + DAY_MAIN_DATA_SIZE * i + DAY_COMMON_DATA_SIZE + 0*ACTIVITY_DAY_DATA_SIZE))
			 && addr1 <= (DAY_MAIN_DATA_START_ADDRESS + DAY_MAIN_DATA_SIZE * i + DAY_COMMON_DATA_SIZE + DAY_ACTIVITY_MAX_NUM*ACTIVITY_DAY_DATA_SIZE))
		{//找到时
			addr1Flag = 1;
			break;
		}
		else
		{//还未找到时
			addr1Flag = 0;
		}
	}

	for(i = 0;i < MAX_RECORD_DAY;i++)
	{
		if((addr2 >= (DAY_MAIN_DATA_START_ADDRESS + DAY_MAIN_DATA_SIZE * i + DAY_COMMON_DATA_SIZE + 0*ACTIVITY_DAY_DATA_SIZE))
			 && addr2 <= (DAY_MAIN_DATA_START_ADDRESS + DAY_MAIN_DATA_SIZE * i + DAY_COMMON_DATA_SIZE + DAY_ACTIVITY_MAX_NUM*ACTIVITY_DAY_DATA_SIZE))
		{//找到时
			addr2Flag = 1;
			break;
		}
		else
		{//还未找到时
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
/*函数说明:日常活动数据 LOG打印*/
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
		dev_extFlash_read(DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*i,(uint8_t*)(&temp),4);  //读取活动日期
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

/*函数描述:判断运动数据是否为有效数据, 判断依据活动类型
返回:valid 1, else 0. 
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

/*函数描述:正向遍历运动数据，判断有效数据数目,返回最新一项数据起始地址
  返回
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

		//查找记录开始时,记录位置标记清零
		m_InvertFlagHead = 0;
		m_InvertFlagTail = 0;
		
		uint32_t daynum = (address_s - DAY_MAIN_DATA_START_ADDRESS) / DAY_MAIN_DATA_SIZE;   //判断处于哪一天

		tmp = address_s - DAY_MAIN_DATA_START_ADDRESS - daynum * DAY_MAIN_DATA_SIZE;
		if(tmp >= ACTIVITY_DATA_START_ADDRESS)      //判断为某一天的第几项运动数据
			activitynum = (tmp - ACTIVITY_DATA_START_ADDRESS) / ACTIVITY_DAY_DATA_SIZE;
		else
			activitynum = 0;
		
		g_ActivityData_Total_Num = 0;//每次需重新归零统计
    for(i = 0; i < MAX_RECORD_DAY; i++)
    {
        for(j = activitynum; j < MAX_NUM_ACTIVITY_DATA; j++)
        {			
						tmp_address = i * DAY_MAIN_DATA_SIZE + DAY_MAIN_DATA_START_ADDRESS + ACTIVITY_DATA_START_ADDRESS + j * ACTIVITY_DAY_DATA_SIZE;
						
            ActivityDataGetFlash(tmp_address, &sport_data_rec);
            if(IsActivityDataValid(&sport_data_rec))
            {
							g_ActivityData_Total_Num++; //运动数据总数+1

							if(Get_DiffTime(sport_data_rec.Act_Start_Time,tmp_rtc_time) >= 0)
							{
								tmp_rtc_time = sport_data_rec.Act_Start_Time;    //记录最近的时间
								address_re = tmp_address;                        //记录最近的时间对应的地址
								m_LastIndexNum = i;                              //记录最近的时间对应的空间index
							}
            }
        }
				
				activitynum = 0;
    }
		
		return address_re;
}
/*函数说明:正向遍历运动数据
返回值:ActitivyDataResult_s结构体
*/
ActitivyDataResult_s ActivityDataForwardLookup(uint32_t address_s, ActivityDataStr *sport_data_rec)
{
    int8_t i = 0, j = 0, n = 0,k = 0;
		ActitivyDataResult_s tmp_struct;
		tmp_struct.number = 0;
		uint32_t tmp_address = 0;
		uint32_t activitynum = 0;
		uint32_t tmp = 0;
		
	
		uint32_t daynum = (address_s - DAY_MAIN_DATA_START_ADDRESS) / DAY_MAIN_DATA_SIZE;   //判断处于哪一天
	
		//flag = (address_s - DAY_MAIN_DATA_START_ADDRESS) % DAY_MAIN_DATA_SIZE;   //判断地址是否为每天数据存储的起始地址 

		tmp = address_s - DAY_MAIN_DATA_START_ADDRESS - daynum * DAY_MAIN_DATA_SIZE;
		if(tmp >= ACTIVITY_DATA_START_ADDRESS)      ////判断为某一天的第几项运动数据
			activitynum = (tmp - ACTIVITY_DATA_START_ADDRESS) / ACTIVITY_DAY_DATA_SIZE;
		else
			activitynum = 0;
		//清零
	  for(uint8_t m = 0;m<m_Read_Num;m++)
			memset(&sport_data_rec[m],0,sizeof(sport_data_rec[m]));		
    memset(&tmp_struct,0,sizeof(ActitivyDataResult_s));
		
    for(k = 0,i = daynum; (i < MAX_RECORD_DAY && k < MAX_RECORD_DAY); i++,k++)
    {
				if( m_InvertFlagTail == 1 && m_LastIndexNum == i)
				{//从尾部翻过来查不能大于最新m_LastIndexNum
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

            if(n == m_Read_Num)   //到达数组最大存量
						{
                goto RESULT;
						}
        }

				if(i == MAX_RECORD_DAY-1)
				{
					i = -1;
					m_InvertFlagTail = 1;
				}
				if(j == MAX_NUM_ACTIVITY_DATA)        //清零
				{
					activitynum = 0;
				}
    }
		
RESULT:

    return tmp_struct;
}
/*函数说明:反向遍历运动数据
返回:ActitivyDataResult_s结构体
*/
ActitivyDataResult_s ActivityDataReverseLookup(uint32_t address_s, ActivityDataStr *sport_data_rec)
{
    int8_t i = 0, j = 0, n = 0,k = 0;
		ActitivyDataResult_s tmp_struct;
		tmp_struct.number = 0;
		uint32_t tmp_address = 0;
		uint32_t activitynum = 0;
		uint32_t tmp = 0, flag = 0;

		uint32_t daynum = (address_s - DAY_MAIN_DATA_START_ADDRESS) / DAY_MAIN_DATA_SIZE;    //哪一天

		if(daynum > 6)
			goto RESULT;

		flag = (address_s - DAY_MAIN_DATA_START_ADDRESS) % DAY_MAIN_DATA_SIZE;   //判断地址是否为每天数据存储的起始地址 

		if(0 == daynum && 0 == flag)   //第一天的起始地址
			goto RESULT;

		tmp = address_s - DAY_MAIN_DATA_START_ADDRESS - daynum * DAY_MAIN_DATA_SIZE;
		
		if(tmp >= ACTIVITY_DATA_START_ADDRESS)       //判断为某一天的第几项运动数据
		{
			activitynum = (tmp - ACTIVITY_DATA_START_ADDRESS) / ACTIVITY_DAY_DATA_SIZE;
		}
		else
			activitynum = 0;
		//清零
	  for(uint8_t m = 0;m<m_Read_Num;m++)
			memset(&sport_data_rec[m],0,sizeof(sport_data_rec[m]));		
    memset(&tmp_struct,0,sizeof(ActitivyDataResult_s));
		
    for(k = 0,i = daynum; (i >= 0 && k < MAX_RECORD_DAY); i--,k++)
    {
				if( m_InvertFlagHead == 1 && m_LastIndexNum == i)
				{//从头部翻到尾部查不能低于m_LastIndexNum
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

            if(n == m_Read_Num)   //到达数组最大存量
						{
                goto RESULT;
						}
        }

				if(i == 0)
				{
					i = MAX_RECORD_DAY;
					m_InvertFlagHead = 1;
				}

				if(j == -1)        //清零
				{
					activitynum = DAY_ACTIVITY_MAX_NUM-1;
				}
    }
		
RESULT:
    return tmp_struct;
}
/*函数说明:删除选中的活动 置活动类型为0*/
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
				//不是铁人三项运动
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

/*运动记录:搜索全部有效运动记录 降序排列到数组s_sport_record_select_str中
返回值:最新的一条运动记录*/
uint32_t search_sport_record(void)
{
	SportRecordYyMmSsStr temp_sport_record;
	ActivityTitleStr m_ActivityTitleStr;
	uint32_t tmp_address = 0;
	uint32_t m_index = 0;//数组索引
	
	//清空数组
	memset(&s_sport_record_select_str[0],0,sizeof(s_sport_record_select_str));
	g_ActivityData_Total_Num = 0;

	//查找有效运动记录
	dev_extFlash_enable();
	for(uint32_t i = 0;i < (MAX_RECORD_DAY);i++)
	{
		for(uint32_t j = 0;j < (MAX_NUM_ACTIVITY_DATA);j++)
		{
			tmp_address = i * DAY_MAIN_DATA_SIZE + DAY_MAIN_DATA_START_ADDRESS + ACTIVITY_DATA_START_ADDRESS + j * ACTIVITY_DAY_DATA_SIZE;
			dev_extFlash_read( tmp_address,(uint8_t *)&m_ActivityTitleStr,sizeof(ActivityTitleStr));
			//存buf
			if((m_ActivityTitleStr.Activity_Type >= ACT_RUN) && (m_ActivityTitleStr.Activity_Type <= ACT_TRIATHLON_SWIM))
			{
				s_sport_record_select_str[m_index].address = tmp_address;
				memcpy(&s_sport_record_select_str[m_index].start_time,&m_ActivityTitleStr.Act_Start_Time,sizeof(rtc_time_t));
				m_index ++;
			}
		}
	}
	dev_extFlash_disable();
	g_ActivityData_Total_Num = m_index;//有效运动记录总条数
	COM_SPORT_PRINTF("[com_sport]:search_sport_record -->g_ActivityData_Total_Num=%d\n",g_ActivityData_Total_Num);
	if(m_index != 0)
	{//有运动数据
		//降序排列
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
	{//无运动数据
		memset(&s_sport_record_select_str[0],0,sizeof(SportRecordYyMmSsStr));
	}
	COM_SPORT_PRINTF("[com_sport]:address1=0x%X,time:%d-%02d-%02d %d:%02d:%02d\n"
	   ,s_sport_record_select_str[0].address,s_sport_record_select_str[0].start_time.Year,s_sport_record_select_str[0].start_time.Month
	   ,s_sport_record_select_str[0].start_time.Day,s_sport_record_select_str[0].start_time.Hour,s_sport_record_select_str[0].start_time.Minute
		 ,s_sport_record_select_str[0].start_time.Second);
	return s_sport_record_select_str[0].address;
}
/*根据运动记录地址查找两条详细的数据
入参:按键索引 用于上下键的记录 
 key_index_first 中间显示的
 key_index_second 下一条显示的 如果为0 则不查
返回值:true有运动详情 false 无运动详情*/
bool get_sport_detail_record(uint32_t key_index_first,uint32_t key_index_second)
{
	if(s_sport_record_select_str[key_index_first].address != 0)
	{
		dev_extFlash_enable();
		dev_extFlash_read( s_sport_record_select_str[key_index_first].address,(uint8_t *)&ActivityRecordData,sizeof(ActivityDataStr));
		if(key_index_second != 0 && s_sport_record_select_str[key_index_second].address != 0)
		{//下一条
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
以下为运动中提醒
***********************************/

//运动提醒初始化
void Remind_Sport_Init(void)
{
     heartrate_threshold = 0; //心率提醒阈值计数
     heartrate_last = 0;      //上次心率值
     kcal_last = 0;          //上次卡路里值
     circle_last = 0;        //上次圈数
     time_last = 0;          //上次时间
     goal_remind_distance_last = 0;	//上次目标提醒距离
	 circle_remind_distance_last = 0;//上次计圈提醒距离
     distance_nums = 1;      //距离提醒次数
     altitude_last = 0;      //上次高度
     speed_nums = 0;        //速度提醒计数，运动开始(60-55)s可提醒
     speed_nums_limit = 60;      //速度提醒计数时间限制，初始1分钟，超过3次5分钟
     speed_nums_cycleindex = 0;      //速度提醒计数循环次数

     circledistance_nums =1;//计圈圈数
     circledistance_last =0;//上次计圈距离

	 kilometer_nums =1;
     kilometer_last =0;
		 if(Get_IsTrainPlanOnGoing() && (Get_TrainPlanType(Download[1].Type) == 3))
		 {//训练计划运动中且有配速提醒需求
			 speed_trainplan_nums = 0;
			 distance_circle_nums = 0;
		 }
}

/*
*函数:心率告警，达到心率区间5进行提醒
*输入:now_heart当前心率值
     swsport 各运动开关，如跑步开关:SetValue.SportSet.SwRun 
*返回值 :无

*备注:此函数放在心率区间计算之后
*/
void Remind_Heartrate(uint8_t now_heart,uint8_t swsport)
{
 if ((swsport & SW_HEARTRATE_MASK) == SW_HEARTRATE_MASK)//心率开关
 {
//	 COM_SPORT_PRINTF("(swsport & SW_HEARTRATE_MASK) == SW_HEARTRATE_MASK\n%d, %d\n",swsport, SW_HEARTRATE_MASK);
	if ((now_heart >= g_HeartRemind) && (heartrate_last < g_HeartRemind ))
	{
COM_SPORT_PRINTF("(now_heart < g_HeartRemind)&&(heartrate_last < g_HeartRemind)\n%d, %d, %d\n",now_heart, heartrate_last,g_HeartRemind);
		heartrate_threshold = 1;
	}
	else if ((now_heart < g_HeartRemind) &&(heartrate_last>= g_HeartRemind )) //心率值低于阈值
	{
COM_SPORT_PRINTF("(now_heart < g_HeartRemind)&&(heartrate_last >= g_HeartRemind)\n%d, %d, %d\n",now_heart, heartrate_last,g_HeartRemind);
		if( ScreenState == DISPLAY_SCREEN_NOTIFY_HEARTRATE )//正在心率提醒界面
		{
			COM_SPORT_PRINTF("ScreenState == DISPLAY_SCREEN_NOTIFY_HEARTRATE,HDR TH= %d, \n",heartrate_threshold);
			//退出心率提醒界面
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
			if(heartrate_threshold >= 5) //心率值高于阈值，维持5S，
			{
COM_SPORT_PRINTF("HEARTRATE Threshold Trigger: %d, \n",heartrate_threshold);
				//弹出心率提醒
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
*函数:每日步数提醒
*输入:daily_step_now 今日当前步数
      daily_step_goal 今日目标步数
*返回值 :无

*备注:无开关，每天达到后提醒一次。
*/
void Remind_DailyStep(uint32_t daily_step_now,uint32_t daily_step_goal)
{
	if(step_tags == 0)
	{
		//防止开机时提醒
		daily_step_last = daily_step_now;
		
		step_tags = 1;
	}
	
        if ((daily_step_now >= daily_step_goal )&&(daily_step_last < daily_step_goal))
        {		
            //目标步数提醒
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
            //    msg.value= now_step | (goal_step <<16);//当前值低16位，目标值高16位
		        xQueueSend(DisplayQueue, &msg, portMAX_DELAY);	
//						daily_step_last = daily_step_now;	
	        }
//			step_tags++; 
        }
        daily_step_last = daily_step_now;
}

/*
*函数:每日卡路里提醒
*输入:daily_kcal_now 今日当前卡路里, 单位：大卡，而运动存储单位为卡，需除以1000
      daily_kcal_goal 今日目标卡路里
*返回值 :无

*备注:无开关，每天达到后提醒一次。
*/
void Remind_DailyKcal(uint32_t daily_kcal_now,uint32_t daily_kcal_goal)
{
	if(kcal_tags == 0)
	{
		//防止开机时提醒
		daily_kcal_last = daily_kcal_now;
		kcal_tags = 1;
	}
	
    if ((daily_kcal_now >= daily_kcal_goal )&&(daily_kcal_last < daily_kcal_goal))
        {
					//目标卡路里提醒
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
            //    msg.value= now_kcal | (goal_kcal <<16);//当前值低16位，目标值高16位
		        xQueueSend(DisplayQueue, &msg, portMAX_DELAY);		
//						daily_kcal_last = daily_kcal_now;
	        }
//			kcal_tags++;	
        }
		
		daily_kcal_last = daily_kcal_now;
}


/*
*函数:目标燃脂提醒
*输入:now_kcal 当前卡路里, 单位：大卡，而运动存储单位为卡，需除以1000
      goal_kcal 目标卡路里
     swsport 各运动开关，如跑步开关:SetValue.SportSet.SwRun 
*返回值 :无

*备注:健走，徒步越野
*/
void Remind_GoalKcal(uint16_t now_kcal,uint16_t goal_kcal,uint8_t swsport)
{

    if((swsport & SW_GOALKCAL_MASK) == SW_GOALKCAL_MASK)
    {
        if ((now_kcal >= goal_kcal )&&(kcal_last < goal_kcal))
        {
            //目标燃脂提醒
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
						msg.value = goal_kcal; //现在版本只显示目标值 VB103
                //msg.value= now_kcal | (goal_kcal <<16);//当前值低16位，目标值高16位
		        xQueueSend(DisplayQueue, &msg, portMAX_DELAY);							
	        }	
        }
        kcal_last = now_kcal;
    }

}

/*
*函数:目标圈数提醒
*输入:now_circle 当前圈数
      goal_circle 目标圈数
     swsport 各运动开关，如跑步开关:SetValue.SportSet.SwSwimmg 
*返回值 :无

*备注:游泳
*/
void Remind_GoalCircle(uint16_t now_circle,uint16_t goal_circle,uint8_t swsport)
{

    if((swsport & SW_GOALCIRCLE_MASK) == SW_GOALCIRCLE_MASK)
    {
COM_SPORT_PRINTF("(swsport & SW_GOALCIRCLE_MASK) == SW_GOALCIRCLE_MASK\n%d, %d\n",swsport, SW_GOALCIRCLE_MASK);
        if ((now_circle >= goal_circle )&&(circle_last < goal_circle))
        {
            //目标圈数提醒
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
						msg.value = goal_circle; //现在版本只显示目标值 VB103
            //    msg.value= now_circle | (goal_circle <<16);//当前值低16位，目标值高16位
		        xQueueSend(DisplayQueue, &msg, portMAX_DELAY);							
	        }	
        }
        circle_last = now_circle;
    }

}

/*
*函数:目标时间提醒
*输入:now_time 当前圈数
      goal_time 目标圈数
     swsport 各运动开关，如跑步开关:SetValue.SportSet.SwSwimmg 
*返回值 :无

*备注:游泳,健走，铁人三项游泳
*/
void Remind_GoalTime(uint16_t now_time,uint16_t goal_time,uint8_t swsport)
{

    if((swsport & SW_GOALTIME_MASK) == SW_GOALTIME_MASK)
    {
COM_SPORT_PRINTF("(swsport & SW_GOALTIME_MASK) == SW_GOALTIME_MASK\n%d, %d\n",swsport, SW_GOALTIME_MASK);
        if ((now_time >= goal_time )&&(time_last < goal_time))
        {
COM_SPORT_PRINTF("(now_time >= goal_time )&&(time_last < goal_time)\n%d, %d, %d\n",now_time, goal_time, time_last);        
					//目标时间提醒
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
						msg.value = goal_time; //现在版本只显示目标值 VB103
            //    msg.value= now_time | (goal_time <<16);//当前值低16位，目标值高16位
		        xQueueSend(DisplayQueue, &msg, portMAX_DELAY);							
	        }	
        }
        time_last = now_time;
    }

}

/*
*函数:目标距离提醒
*输入:now_distance 当前距离,单位:米,存储距离单位:cm,实际除以100
      goal_distance 提醒距离
     swsport 各运动开关，如跑步开关:SetValue.SportSet.SwSwimmg 
*返回值 :无

*备注:
*/
void Remind_GoalDistance(uint16_t now_distance,uint16_t goal_distance,uint8_t swsport)
{
    if((swsport & SW_DISTANCE_MASK) == SW_DISTANCE_MASK)
    {
        if ((now_distance >= goal_distance)&&(goal_remind_distance_last < goal_distance))
        {
            //距离提醒
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
				msg.value = goal_distance; //现在版本只显示目标值 VB103
//				msg.value= now_distance | (goal_distance <<16);//当前值低16位，目标值高16位
		        xQueueSend(DisplayQueue, &msg, portMAX_DELAY);							
	        }	
        }
        goal_remind_distance_last = now_distance;
    }
}

/*
*函数:计圈距离提醒
*输入:now_distance 当前距离,单位:米,存储距离单位:cm,实际除以100
      goal_distance 提醒距离
     swsport 各运动开关，如跑步开关:SetValue.SportSet.SwSwimmg 
*返回值 :无

*备注:
*/
uint8_t Remind_CircleDistance(uint16_t now_distance,uint16_t goal_distance,uint8_t swsport)
{
	uint8_t status = 0;
	uint16_t nums;
    if((swsport & SW_CIRCLEDISTANCE_MASK) == SW_CIRCLEDISTANCE_MASK)
    {
        if ((now_distance >= (distance_nums*goal_distance) )&&(circle_remind_distance_last < (distance_nums*goal_distance)))
        {
            //自动计圈提醒
            nums =( now_distance - ( distance_nums - 1 ) * goal_distance ) / goal_distance;
            distance_nums+= nums; 			

			status = 1;
        }
        circle_remind_distance_last = now_distance;
    }
	
	return status;
}

//自动计圈提醒显示,需要先计算出上一圈运动数据
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
				//自动计圈提醒
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
*函数:高度提醒
*输入:now_altitude 当前高度
      goal_altitude  提醒高度
     swsport 各运动开关，如跑步开关:SetValue.SportSet.SwSwimmg 
*返回值 :无

*备注:登山
*/
void Remind_Altitude (uint16_t now_altitude,uint16_t goal_altitude,uint8_t swsport)
{

    if((swsport & SW_ALTITUDE_MASK) == SW_ALTITUDE_MASK)
    {
//		if ((now_altitude >= goal_altitude )&&(altitude_last < goal_altitude))//高于提醒高度
		if ((now_altitude >= goal_altitude )&&(altitude_last == 0))//高于提醒高度
        {
			//只提醒一次
			altitude_last = 1;
			
            //高度提醒
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
				msg.value = now_altitude; //现在版本只显示目标值 VB103
            //    msg.value= now_altitude | (goal_altitude <<16);//当前值低16位，目标值高16位
		        xQueueSend(DisplayQueue, &msg, portMAX_DELAY);							
	        }	
        }

//		else if ((now_altitude <goal_altitude )&&(altitude_last>= goal_altitude))//低于提醒高度
//		{
//			if( ScreenState == DISPLAY_SCREEN_NOTIFY_ALTITUDE )//正在高度提醒界面
//			{   
//				//退出高度提醒界面
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
*函数:速度提醒
*输入:now_speed 当前速度
      goal_speed 提醒速度
     swsport 各运动开关
*返回值 :无

*备注:骑行km/h，登山m/h
*/
void Remind_Speed (uint16_t now_speed,uint16_t goal_speed,uint8_t swsport)
{

    if((swsport & SW_PACE_MASK) == SW_PACE_MASK)
    {
        speed_nums++;
       if ((now_speed < goal_speed )&&(now_speed !=0))
        {    
            //速度提醒
            if (speed_nums > speed_nums_limit)//初始间隔1分钟提醒
            {
                speed_nums = 0;
				speed_nums_cycleindex++;
				if (speed_nums_cycleindex >= 3 )//连续提醒3次
				{
					speed_nums_limit = 300; //间隔5分钟
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
                    msg.value= now_speed | (goal_speed <<16);//当前值低16位，目标值高16位
		            xQueueSend(DisplayQueue, &msg, portMAX_DELAY);							
	           }	
            }

        }

    }

}

/*
*函数:配速提醒
*输入:now_pace 当前配速
      goal_pace 提醒配速
     swsport 各运动开关
*返回值 :无

*备注:跑步类+徒步越野
*/
void Remind_Pace (uint16_t now_pace,uint16_t goal_pace,uint8_t swsport)
{

    if((swsport & SW_PACE_MASK) == SW_PACE_MASK)
    {
        speed_nums++;
        if (now_pace > goal_pace)
        {
            //配速提醒
            if (speed_nums > speed_nums_limit)//初始间隔1分钟提醒
            {
                speed_nums = 0;
				speed_nums_cycleindex++;
				if (speed_nums_cycleindex >= 3 )//连续提醒3次
				{
					speed_nums_limit = 300; //间隔5分钟
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
                    msg.value= now_pace | (goal_pace <<16); //当前值低16位，目标值高16位
		            xQueueSend(DisplayQueue, &msg, portMAX_DELAY);							
	           }	
            }

        }

    }

}
/***************
以下为训练计划部分
*/
/*函数说明:训练计划是否有效 
判断依据有SetValue.TrainPlanFlag值:该值蓝牙下发训练计划命令打开
还有是训练计划下发内容的MD5头与结果的MD5头相同则认为训练计划命令打开有效
上述两个条件都满足,则认为当前训练计划有效*/
bool get_trainplan_valid_flag(void)
{
	_com_config_train_plan_MD5_UUID  MD5_UUIDStr,MD5_UUIDResultStr;
	bool m_md5str_status = false,m_uuidstr_status = false,status = false;
	dev_extFlash_enable();
	//从上传训练计划数据中取MD5和UUID
	dev_extFlash_read(TRAINING_DATA_START_ADDRESS,(uint8_t*)(&MD5_UUIDStr),sizeof(_com_config_train_plan_MD5_UUID));
	//写入到训练计划结果头部
	dev_extFlash_read(TRAINING_DATA_UPLOAD_ADDRESS,(uint8_t*)(&MD5_UUIDResultStr),sizeof(_com_config_train_plan_MD5_UUID));
	dev_extFlash_disable();
	m_md5str_status = strcmp((char *)MD5_UUIDStr.Md5Str,(char *)MD5_UUIDResultStr.Md5Str);
	m_uuidstr_status = strcmp((char *)MD5_UUIDStr.UUIDStr,(char *)MD5_UUIDResultStr.UUIDStr);
	status = m_md5str_status & m_uuidstr_status & SetValue.TrainPlanFlag;
//	COM_SPORT_PRINTF("[com_sport]: m_md5str_status=%d,m_uuidstr_status=%d,status=%d\n"
//	  ,m_md5str_status,m_uuidstr_status,status);
	
	return status;
}
/*函数说明:获取训练计划当天的提醒时间开关状态
1代表 今天的训练计划开启且训练计划未完成,0代表 未开启或者开启已完成 通知是否打开等
*/
uint8_t Get_TrainPlanFlag(void)
{
	return (get_trainplan_valid_flag() && (Upload[1].IsComplete != 1) && Is_TodayTimeTrainPlan()&&Get_TrainPlan_Notify_IsOpen());
}
/*函数描述:今天训练计划目标距离是否达标*/
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
/*函数描述:今天训练计划目标时长是否达标*/
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
/*函数描述:今天训练计划目标卡路里是否达标*/
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
/*函数描述:计算今天的训练活动是否达标 
注:不同类型运动评价标准不同
入参:  TrainPlanType类型值
返回值:true达标，false不达标
*/
uint8_t Get_IsCompleteBothTodayTrainPlan(TrainPlanType type)
{
	uint8_t isComplete = 0;
	
	//根据类型选择评价标准 卡路里标准待定
	switch(type)
	{
		case EasyToRun_Distance: //轻松跑-距离
		case NormalRun_Distance://跑步-距离
		case TempoRuns_Distance://节奏跑-距离
		case PacetoRun_Distance://配速跑-距离
		case Intervals_Distance://间歇训练-距离
		case Marathon_Distance://马拉松-距离
		case Rest_Run_Distance://休息或跑-距离
			isComplete = IsComplete_TargetDistance();//距离
			break;
			
		case EasyToRun_Time://轻松跑-时间
		case NormalRun_Time://跑步-时间
		case TempoRuns_Time://节奏跑-时间
		case PacetoRun_Time://配速跑-时间
		case Intervals_Time://间歇训练-时间
			isComplete = IsComplete_TargetHour();//时长
			break;
		case Rest://休息
		case StrengthTraining://力量训练
		case CrossTraining://交叉训练
		case Rest_StrengthTraining://休息或力量训练
		case Rest_CrossTraining://休息或交叉训练
			isComplete = 1;//默认完成
			break;
	
		default://默认距离
			isComplete = IsComplete_TargetDistance();//距离
			break;
	}
	
	return isComplete;
}

/*函数描述:设置今天的训练活动是否达标
入参:true达标 false不达标
*/
void Set_IsCompleteTodayTrainPlan(uint8_t isComplete)
{
	Upload[1].IsComplete = isComplete;
}
/*函数描述:获取今天的训练活动是否达标
返回值:true达标 false不达标
*/
uint8_t Get_IsCompleteTodayTrainPlan(void)
{
	return Upload[1].IsComplete;
}
/*函数描述:判断训练计划类型
入参: 如Download[1].Type
返回值:0 休息   只显示用
       1 力量训练等需要按键操作完成的
       2 休息或力量训练等 默认完成但可以运动
       3 配速跑和节奏跑等 有配速需求的
       4 有间歇训练 4x400类的提醒需求
			 5 其他跑步
       
*/
uint8_t Get_TrainPlanType(TrainPlanType type)
{
	uint8_t backType = 0;
	
	switch(type)
	{
		case Rest://休息
			backType = 0;
			break;
		case StrengthTraining://力量训练
		case CrossTraining://交叉训练
		case Rest_StrengthTraining://休息或力量训练
		case Rest_CrossTraining://休息或交叉训练
			backType = 1;
			break;
		
		case Rest_Run_Distance://休息或跑-距离
			backType = 2;
			break;
		case PacetoRun_Time://配速跑-时间
		case PacetoRun_Distance://配速跑-距离
		case TempoRuns_Distance://节奏跑-距离
		case TempoRuns_Time://节奏跑-时间
			backType = 3;
			break;
		case EasyToRun_Distance: //轻松跑-距离
		case NormalRun_Distance://跑步-距离
		case Intervals_Distance://间歇训练-距离
		case Marathon_Distance://马拉松-距离
		case EasyToRun_Time://轻松跑-时间
		case NormalRun_Time://跑步-时间
			backType = 5;
			break;
		case Intervals_Time://间歇训练-时间
			backType = 4;
			break;
		default:
			backType = 0;
			break;
	}
	return backType;
}
/*函数描述:获取间歇跑 4x400米中单次跑的400数据
由目标距离=(单次跑距离+单次走距离)x次数
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
/*函数描述:训练日提醒
分为处于非训练计划和处于非训练计划运动模式中时
前者直接提醒，后者运动模式结束后需要提醒,只要处于当天，训练计划均要提醒
提醒前需要从FLASH中获取相应的训练计划(3天)
*/
void Remind_TodayTrainPlan (void)
{
#ifndef COD 
	if(SetValue.IsNeedClearTrainPlanDayHint == 1 && ScreenState == DISPLAY_SCREEN_NOTIFY_TRAINING_DAY
		&& RTC_time.ui32Minute == 0 && RTC_time.ui32Hour == 0)
	{//如果昨天的训练计划提醒一直在,到了第二天的0点需要清除这个界面,退出到主界面
		SetValue.IsNeedClearTrainPlanDayHint = 0;
		DISPLAY_MSG  msg = {0,0};
		ScreenState = DISPLAY_SCREEN_HOME;
		msg.cmd = MSG_DISPLAY_SCREEN;
		xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
	}
	if(Download[1].Type == Rest && Get_IsCompleteTodayTrainPlan() != 1)
	{//如果今天是休息,默认保存
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
	{//训练计划是否打开且 今天处于训练计划中的某天 且目标未完成 通知开关打开
		if(Get_IsTrainPlanOnGoing() == false && IsSportMode(ScreenState) == false && IsSportReady(ScreenState) == false 
			&& Get_IsInSportMode() == false)
		{// 空闲时间 处于非运动模式
			if((TrainPlanDownloadSettingStr.Hint.Hour == RTC_time.ui32Hour 
			   && TrainPlanDownloadSettingStr.Hint.Minute == RTC_time.ui32Minute)
			   || (SetValue.TrainPlanHintFlag == 1 
			   && (RTC_time.ui32Hour*60 + RTC_time.ui32Minute ) > (TrainPlanDownloadSettingStr.Hint.Hour*60 + TrainPlanDownloadSettingStr.Hint.Minute)))
			{//提醒时间
				SetValue.TrainPlanHintFlag = 0;
				//获取训练计划
				if(get_trainplan_valid_flag())
				{//有训练计划同步
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
		{//处于非训练计划运动模式中
			static uint16_t hint_minute = 0,realtime_minute = 0;
			hint_minute = TrainPlanDownloadSettingStr.Hint.Hour * 60 + TrainPlanDownloadSettingStr.Hint.Minute;
			realtime_minute = RTC_time.ui32Hour * 60 + RTC_time.ui32Minute;
			if(hint_minute == realtime_minute)
			{
				SetValue.TrainPlanHintFlag = 1;//稍后提醒
			}
		}
	}
#endif
}
/*函数描述:训练目标达成提醒*/
void Remind_TrainPlan_Complete(void)
{
	//训练计划是否打开且 今天处于训练计划中的某天  达标 且在训练中
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
*函数:训练计划配速提醒
*输入:now_pace 当前配速
			now_distance 当前距离
      up_pace 上限配速
     down_pace 下限配速

在上下限范围内的配速不提醒，否则提醒，且每31s提醒一次，若训练计划完成，配速不再提醒
*返回值 :无

*备注:
*/
void Remind_Pace_TrainPlan (uint16_t now_pace,uint16_t now_distance,uint16_t up_pace,uint16_t down_pace)
{
	if(get_trainplan_valid_flag() && Is_TodayTimeTrainPlan() && (Get_IsCompleteTodayTrainPlan() != 1)
		&& Get_IsTrainPlanOnGoing())
	{
		if(Get_TrainPlanType(Download[1].Type) == 3)
		{//有配速需求的
			speed_trainplan_nums++;
			if (now_pace > up_pace || now_pace < down_pace)
			{
					//配速提醒
					if (speed_trainplan_nums > 31)//间隔31s提醒 防止覆盖训练计划完成提醒界面
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
		{//有间歇训练 4x400类的提醒需求
			for(uint8_t i=1;i<(Download[1].IntermittentRun*2);i++)
			{
				if((i+1)%2 == 0)
				{//跑提醒
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
				{//走提醒
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
以下轨迹纠偏算法
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
//纠偏
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

//初始化轨迹参数
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
         Last_GPS_Lon =(int32_t )(dLatLng.dlon*100000);//轨迹中gps点保留5位小数点，简化计算量第五位当做为米级
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
               if(xybackflag == 0)//判断返航状态
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

//画轨迹
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

//轨迹指针
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
//计算详情轨迹到显示数组
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
          xybacklen =2;//防止只有第一点也划线
    
      }

}

//运动详见轨迹
void DrawSaveTrack(void)
{
      uint16_t i;
      
      if(xybackflag == 0)//判断返航状态
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

//循迹返航轨迹标记 
void TrackBackFlag(void)
{
    //xybacklen = xylen;
    gpsbacklen = gpslen;
    xybackflag = 1;
    

}

//初始化加载轨迹参数,加载轨迹与退出运动时调用
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

//加载运动轨迹
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
    
    //通过等分的5个点计算中心点位置及缩放等级
    if (endaddress >= startaddress)//是否超出结束地址
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
        transform((double)(lonlat[0]/100000.0),(double)(lonlat[1]/100000.0));//存储的gps精度5位
        loadgpsstore[loadgpslen] =(int32_t)(dLatLng.dlon*100000);//轨迹中gps点保留5位小数点
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



//加载云迹导航轨迹
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
         tmp_lon = (int32_t)(lonlat[0]*100000);//轨迹中gps点保留5位小数点
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
              
       //起点
       if (i==0)
       {
           loadstartfalg =1;
                  
       }
       //终点
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
        
      //计算加载轨迹在显示范围内的点
			
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


//加载航迹标点
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
        //读取航迹标点
        dev_extFlash_enable();
        dev_extFlash_read(startaddress + sizeof(TrackMarkStr)*i,(uint8_t*)&(TrackMark[i]),sizeof(TrackMarkStr));   
        dev_extFlash_disable();
    }
#if defined WATCH_SPORT_NEW_CLOUD_NAVIGATION
		//刷新临时标注点
		memset(&m_compare_track_mark,0,sizeof(TrackMarkStr)*TRACK_MARK_NUM_MAX);
		memcpy(&m_compare_track_mark,&TrackMark,sizeof(TrackMarkStr)*TrackMarkNums);
#endif

}

//画航迹标点
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
距离当前点最近的标点，
输入参数当前经纬度
返回值，航迹标点数值标号
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
    //如果最近的距离过大不显示，大于约300米不显示。
    if(min_sum > 90000)
    {
        trackpointindex = 0xFF;
    }

    return trackpointindex;


}

/*获取铁人三项GPS最后一个地址*/
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
无数据刷新页面超过10分钟无任何操作（无按键操作）返回待机界面
运动类:运动过程中(含运动准备,除运动详情)
工具类:GPS定位、指北针、环境监测、秒表计时、寻找手机、倒计时等功能性测试(除日历天气)
快捷功能类:心率测试、计步、最大摄氧量测试中、乳酸阈测试
菜单类:配件搜索中、卫星授时界面
连接通知类:充电、常驻通知(训练计划、基础心率提醒)、闹钟
特殊功能类:SOS功能、CO检测功能等
其他:待机界面、自检等
返回:true 不在刷新界面可以自动返回待机界面,false在刷新界面不可以自动返回
*/
bool is_auto_back_home(ScreenState_t index)
{
	bool m_status = false;
	
	if(IsSportMode(index) == true || IsSportReady(index) == true || index == DISPLAY_SCREEN_SPORT)
	{//运动类:运动过程中(含运动准备)
		m_status = false;
	}
	else if((index >= DISPLAY_SCREEN_TOOL && index < DISPLAY_SCREEN_WEATHER_INFO)
		     || (index == DISPLAY_SCREEN_NOTIFY_FIND_WATCH))
	{//工具类:GPS定位、指北针等功能性测试
		m_status = false;
	}
	else if(index == DISPLAY_SCREEN_HEARTRATE || index == DISPLAY_SCREEN_HEARTRATE_STABLE
					|| index == DISPLAY_SCREEN_STEP)
	{//快捷功能类:心率测试、计步、最大摄氧量测试中、乳酸阈测试(运动中已包括)
		m_status = false;
	}
	
	else if(index == DISPLAY_SCREEN_ACCESSORY)
	{//菜单类:配件搜索中、卫星授时界面
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
	{//连接通知类:充电、常驻通知(训练计划、基础心率提醒)、闹钟
		m_status = false;
	}
	else if((index >= DISPLAY_SCREEN_POST_SETTINGS && index <= DISPLAY_SCREEN_PC_HWT) 
		|| (index >= DISPLAY_SCREEN_TIME_CALIBRATION && index <= DISPLAY_SCREEN_TIME_CALIBRATION_REALTIME)
		|| (index == DISPLAY_SCREEN_HOME) || (index == DISPLAY_SCREEN_LOGO)
	  || (index == DISPLAY_SCREEN_TEST_CALIBRATION_VIEW_STATUS) || (index == DISPLAY_SCREEN_TEST_REAL_DIFF_TIME)
	  || (index == DISPLAY_SCREEN_STEP_WEEK_STEPS) || (index == DISPLAY_SCREEN_STEP_WEEK_MILEAGE)
	  || (index == DISPLAY_SCREEN_CALORIES_WEEK_SPORTS))
	{//其他:待机界面、自检等
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
/*赛事ID是否有效*/
bool is_sport_event_id_valid(SportEventStr m_str)
{
	//年一定要大于0 区别存的是有效地址还是有效赛事
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
/*赛事是否有效*/
bool is_sport_event_schedule_valid(SportEventStatusStr m_sport_event_str)
{
	if(is_sport_status_str(m_sport_event_str) && m_sport_event_str.nums >= 1)
	{//有效赛事
		return true;
	}
	else
	{
		return false;
	}
}
/*获取赛事ID 年月日时分秒miao*/
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
函数说明:是否有该运动类型的赛事赛段
入参:活动类型
返回参数:SportEventStatusStr
*/
SportEventStatusStr get_sport_event_schedule_number(uint32_t m_type)
{
	SportEventStr m_sport_event,m_compare_str,m_lase_sport_event;
	SportEventStatusStr m_event;
	int64_t m_64_sport_event = 0,m_64_compare_str = 0,m_64_new_event_id_str = 0;
	
	uint8_t m_sport_type = 0,m_sport_schedule_compare = 0;//赛段比较
	
	memset(&m_event,0,sizeof(SportEventStatusStr));
	m_event.sport_type = m_type;//赛事活动类型
	if(is_sport_type_valid(m_type) == false)
	{//入参检测
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
			{//运动类型有效
				dev_extFlash_read(DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*i+ACTIVITY_DATA_START_ADDRESS+j*ACTIVITY_DAY_DATA_SIZE+SPORT_EVENT_BEFORE_BYTE
												 ,(uint8_t *)&m_64_sport_event,sizeof(m_sport_event));
				memcpy(&m_sport_event,&m_64_sport_event,sizeof(m_64_sport_event));
				m_64_new_event_id_str = get_sport_event_id(m_sport_event);
				if(is_sport_event_id_valid(m_sport_event))
				{//赛事ID有效 
					//获取最新的运动赛事ID
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
			//赛事结束 
			dev_extFlash_disable();
			
			memset(&m_event,0,sizeof(SportEventStatusStr));
			
			return m_event;
		}
		else
		{
			//赛事未结束
			m_event.event_id = m_64_compare_str;//赛事ID
			m_event.status = m_compare_str.U_EventSport.bits.Event_Status;
			m_event.nums = m_compare_str.U_EventSport.bits.Shedule_Nums;
			m_event.total_nums = m_compare_str.U_EventSport.bits.Shedule_Nums;
			
			memcpy(&m_event.event_str,&m_compare_str,sizeof(SportEventStr));
			//赛事ID找最新赛段
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
						
						m_64_sport_event = get_sport_event_id(m_sport_event);//赛事ID
						if(m_64_sport_event == m_event.event_id)
						{//赛事ID相同
							if(m_sport_event.U_EventSport.bits.Event_Status == 0)
							{//赛事结束标志 停止检索
								dev_extFlash_disable();
								memset(&m_event,0,sizeof(SportEventStatusStr));
								return m_event;
							}
							else
							{
								if(m_event.nums < m_sport_schedule_compare)
								{//获取最新的赛段
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
/*总结赛事运动 运动结束的运动详情显示
活动赛程必须大于等于1个
入参:运动类型,赛事ID,赛事 SportEventStatusStr
返回参数:SportScheduleCalStr
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
	//入参检查
	if(is_sport_event_schedule_valid(m_sport_event) == false)
	{//赛事ID有效、赛段数有效
		return m_sport_schedule_data;
	}
	//数据初始化
	memset(&m_sport_event_str,0,sizeof(SportEventStr));
	memset(&m_sport_schedule_data,0,sizeof(SportScheduleCalStr));
	memcpy(&m_sport_schedule_data.schedule,&m_sport_event,sizeof(SportEventStatusStr));//返回值赛事ID信息

	//有效运动中查找该赛事ID的赛段
	dev_extFlash_enable();
	for(uint16_t i = 0; i < 7; i++)
	{
		for(uint16_t j = 0; j < DAY_ACTIVITY_MAX_NUM; j++)
		{
			dev_extFlash_read(DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*i+ACTIVITY_DATA_START_ADDRESS+j*ACTIVITY_DAY_DATA_SIZE
			                   ,(uint8_t *)&m_sport_type,sizeof(m_sport_type));
			if(is_sport_type_valid(m_sport_type) && m_sport_type == m_sport_event.sport_type)
			{//运动类型有效
				dev_extFlash_read(DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*i+ACTIVITY_DATA_START_ADDRESS+j*ACTIVITY_DAY_DATA_SIZE+SPORT_EVENT_BEFORE_BYTE
												 ,(uint8_t *)&m_64_sport_event,sizeof(SportEventStr));
				memcpy(&m_sport_event_str,&m_64_sport_event,sizeof(SportEventStr));
				m_64_new_event_id_str = get_sport_event_id(m_sport_event_str);//赛事ID
				if(m_sport_event.event_id == m_64_new_event_id_str)
				{//赛事ID相同
					if(m_sport_event.nums >= m_sport_event_str.U_EventSport.bits.Shedule_Nums)
					{//读取有效赛段的活动详情
						dev_extFlash_read(DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*i+ACTIVITY_DATA_START_ADDRESS+j*ACTIVITY_DAY_DATA_SIZE
									 ,(uint8_t *)&m_activity_data,sizeof(ActivityDataStr));
						if(is_sport_type_valid(m_activity_data.Activity_Type) && m_activity_data.Activity_Type == m_sport_event.sport_type)
						{//重新检索的活动有效
							if(m_activity_data.ActTime != 0xFFFFFFFF 
								&& m_activity_data.CircleDistance != 0xFFFF)
							{//数据有效
								m_sport_schedule_data.total_time += m_activity_data.ActTime;//返回值赛事总时间
								m_sport_schedule_data.total_distance += m_activity_data.ActivityDetails.CrosscountryDetail.Distance;//返回值赛事总距离
								
								//返回值赛事总平均配速
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
	memcpy(&m_sport_schedule_data.schedule,&m_sport_event,sizeof(SportEventStatusStr));//返回值赛事ID信息
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
//新云迹导航

/*云迹导航 有效运动类型*/
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
/*运动中检测云迹导航数据是否有效*/
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
	//无效数据条件判断
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
赛事中是否下载了新的徒步越野等轨迹
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
		{//活动类型有效
			if(m_share_track_str.new_sign_value == SPORT_NEW_CLOUD_DISTINGUISH_SIGN_VALUE 
				 && is_has_sport_cloud_navigation_valid(&m_share_track_str))
			{//属于新云迹导航 且地址有效
				dev_extFlash_disable();
				return true;
			}
		}
	}
	dev_extFlash_disable();
	return false;
}
/*
距离当前点最近的标点
输入参数当前经纬度 
返回值，航迹标点数值标号
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
/*是否越过CPx(有关门时间的)点   算法返回是否越过CP点
num:标注点序号lon,lat已转化过的GPS点
true:越过,ShareTrack清掉cpx
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
		{//在轨迹上
			if(m_offtrack.RemainDistance == 0)
			{//越过
				return true;
			}
			else
			{//未越过
				return false;
			}
		}
		else
		{//未在轨迹上
			if(m_offtrack.NearestDistance == 0)
			{//越过
				return true;
			}
			else
			{//未越过
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
/*是否有最新关门时间
并获取关门时间给g_track_mark_data
num:存在数组ShareTrack[SHARED_TRACK_NUM_MAX]第几条轨迹
*/
bool is_sport_has_close_time(uint8_t num,uint8_t sport_type)
{
	uint8_t m_point_num_max = 0,m_point_position = 0;//标注点最大  标注点位置

	if(num >= SHARED_TRACK_NUM_MAX)
	{
		return false;
	}
	if(IsShareTrackDataValid(&ShareTrack[num]) == false)
	{//轨迹头数据是否有效
		memset(&g_track_mark_data,0,sizeof(SportTrackMarkDataStr));
		return false;
	}
	if(ShareTrack[num].point_end_address > ShareTrack[num].point_start_address
		&& ShareTrack[num].point_start_address > SHARE_TRACK_START_ADDR
		&& sizeof(TrackMarkStr) != 0)
	{//几个标注点
		m_point_num_max = (ShareTrack[num].point_end_address - ShareTrack[num].point_start_address)/sizeof(TrackMarkStr);
	}
	else
	{
		memset(&g_track_mark_data,0,sizeof(SportTrackMarkDataStr));
		return false;
	}
	
	m_point_position = is_track_mark_valid(Last_GPS_Lon,Last_GPS_Lat);
	//找到最近的标注点
	if( m_point_position < TRACK_MARK_NUM_MAX && m_point_position < m_point_num_max)
	{
		if(TrackMark[m_point_position].close_time_index > 0)
		{//有关门时间
			if(is_cross_cpx_valid(Last_GPS_Lon,Last_GPS_Lat,m_point_position) == true)
			{//越过CP点 找下一个CP点
				for(uint16_t i = m_point_position + 1; i < m_point_num_max;i++)
				{
					dev_extFlash_enable();
					dev_extFlash_read(ShareTrack[num].point_start_address + i * sizeof(TrackMarkStr)
															,(uint8_t *)&g_track_mark_data.track,sizeof(TrackMarkStr));
					dev_extFlash_disable();
					
					if(g_track_mark_data.track.close_time_index > 0)
					{
						g_track_mark_data.is_close_time_valid = 1;
						//获取到关门时间的CP后, m_compare_track_mark 从检索cp点buf中移除
						memset(&m_compare_track_mark[m_point_position],0,sizeof(TrackMarkStr));
						return true;
					}
				}
			}
			else
			{//没有越过CP点 获取当前CP点关门时间
				memcpy(&g_track_mark_data.track,&TrackMark[m_point_position],sizeof(TrackMarkStr));
				g_track_mark_data.is_close_time_valid = 1;
				return true;
			}
		}
		else
		{//该标注点无关门时间 寻找下一个标注点  两个CP标注点低于100m暂不考虑
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
/*云迹导航 flash数据读取 测试用*/
void test_share_cloud_detail_flash_print(void)
{
	ShareTrackStr m_test_share_title;
	TrackMarkStr m_track_str[20],m_test_track[3];
	GPS_Str m_gps_str[5];
	uint32_t m_data[1000],m_track_max = 0,m_gps_max = 0,m_size = 300;//若GPS超过1000个点，只读前300个
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



