#include "timer_traceback.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "portmacro.h"
#include "portable.h"
#include "event_groups.h"
#include "semphr.h"


#include "task_display.h"
#include "gui_time.h"
#include "drv_ublox.h"
#include "lib_error.h"
#include "gui_sport.h"
#include "com_sport.h"
#include "algo_trackjudge.h"
#include "task_gps.h"
#include "task_sport.h"
#include "gui_tool_gps.h"

#define TIME_PROGRESS_LOG_ENABLED 1

#if DEBUG_ENABLED == 1 && TIME_PROGRESS_LOG_ENABLED == 1
	#define TIME_PROGRESS_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define TIME_PROGRESS_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define TIME_PROGRESS_WRITESTRING(...)
	#define TIME_PROGRESS_PRINTF(...)		        
#endif


#define TIMER_INIT_INTERVAL_MS        1000
#define TRACE_BACK_TIMER_PERIED    (3)    //10秒钟

static TimerHandle_t TracebackTimer = NULL;    //运动循迹返航timer
extern int32_t Last_GPS_Lon;               //上次经纬度
extern int32_t Last_GPS_Lat;

//运动循迹返航界面或导航切换定时器callback
static void vTimerTracebackCallback(TimerHandle_t xTimer)
{
	OfftrackDetailStr offtrack;
	
	switch(ActivityData.Activity_Type)
	{
		case ACT_RUN:
		case ACT_CROSSCOUNTRY:
		case ACT_HIKING:
		case ACT_CLIMB:
		case ACT_CYCLING:
		case ACT_WALK:
		case ACT_MARATHON:
		{
			if((Get_PauseSta() == false) && (GetGpsStatus()== true))
			{   	    
				//在运动状态下,gps定位无效时判断是否在轨迹上无意义只显示时间
				TRACE_BACK_DATA.sport_traceback_flag = !TRACE_BACK_DATA.sport_traceback_flag;
#if defined WATCH_SPORT_NEW_CLOUD_NAVIGATION
				memset(&offtrack,0,sizeof(OfftrackDetailStr));
#endif
				if (TRACE_BACK_DATA.is_traceback_flag ==1)
                {            
				    offtrack = get_back_OfftrackDetail(Last_GPS_Lon, Last_GPS_Lat, TRACE_BACK_DATA.sport_distance);
                }

                else if (loadflag ==1)
                {
#if defined WATCH_SPORT_NEW_CLOUD_NAVIGATION
									int32_t m_track_lon = 0,m_track_lat = 0;
										
									m_track_lon =(int32_t )(g_track_mark_data.track.lon*100000);//轨迹中gps点保留5位小数点，简化计算量第五位当做为米级
									m_track_lat = (int32_t )(g_track_mark_data.track.lat*100000);
									//获取关门时间
                	if (g_track_mark_data.is_close_time_valid == 1 && TRACE_BACK_DATA.is_traceback_flag == 0) //有cp关门时间
                	{//非循迹返航时
										if(m_track_lon != 0 && m_track_lat != 0 && Last_GPS_Lon != 0 && Last_GPS_Lat != 0)
										{
											offtrack = get_distCP_OfftrackDetail(Last_GPS_Lon,Last_GPS_Lat
																															,m_track_lon
																															,m_track_lat
																															,TRACE_BACK_DATA.sport_distance);
										}
                	}
									else
									{
										if(Last_GPS_Lon != 0 && Last_GPS_Lat != 0)
										{
											offtrack = get_navi_OfftrackDetail(Last_GPS_Lon,Last_GPS_Lat, TRACE_BACK_DATA.sport_distance);
										}
										
									}
#else
								offtrack = get_navi_OfftrackDetail(Last_GPS_Lon,Last_GPS_Lat, TRACE_BACK_DATA.sport_distance);		
#endif

                }
                else
                {
                    TRACE_BACK_DATA.sport_traceback_flag = 1;
                }
//					TIME_PROGRESS_PRINTF("[timer_tracback]:Last_GPS_Lon=0%x,Last_GPS_Lat=0%x,track.lon=0%x,track.lat=0%x,distance=%d,loadflag=%d,traceback_flag=%d\r\n"
//													,Last_GPS_Lon,Last_GPS_Lat,g_track_mark_data.track.lon,g_track_mark_data.track.lat,TRACE_BACK_DATA.sport_distance
//													,loadflag,TRACE_BACK_DATA.is_traceback_flag);
				TRACE_BACK_DATA.remaining_distance = offtrack.RemainDistance;
				TRACE_BACK_DATA.whether_in_track = offtrack.WhetherinTrack;
				TRACE_BACK_DATA.nearest_distance = offtrack.NearestDistance;

                if (TrackMarkNums != 0)
				{
                  TrackMarkIndex =  min_track_mark(Last_GPS_Lon,Last_GPS_Lat);
                }
			}
			else
			{
				TRACE_BACK_DATA.sport_traceback_flag = 1;
			}
		}
			break;		
		default:
			TRACE_BACK_DATA.sport_traceback_flag = 1;
			break;
	}
		
	//SEGGER_RTT_printf(0,"TRACE_BACK_DATA.sport_traceback_flag is %u\n", TRACE_BACK_DATA.sport_traceback_flag);
	//SEGGER_RTT_printf(0,"REMAINING_DISTANCE is %u\n", REMAINING_DISTANCE);

}

//运动循迹返航界面或导航切换定时器start
void trace_back_timer_start(void)
{
     if (TracebackTimer ==NULL)
      {
	        TracebackTimer = xTimerCreate("TracebackTimer", TRACE_BACK_TIMER_PERIED * ( TIMER_INIT_INTERVAL_MS / portTICK_PERIOD_MS ),
																 pdTRUE,
																 NULL,
																 vTimerTracebackCallback);
      }
	TRACE_BACK_DATA.sport_traceback_flag = 1;
	if(NULL != TracebackTimer)
	{
		if(xTimerStart(TracebackTimer, 100) != pdPASS)
		{
			
		}
	}
}

//运动循迹返航界面或导航切换定时器delete
void trace_back_timer_delete(void)
{
	//SEGGER_RTT_printf(0,"IN trace_back_timer_delete()\n");
	if(TracebackTimer != NULL)
	{
//		if(xTimerIsTimerActive(TracebackTimer) != pdFALSE)
		{
			//SEGGER_RTT_printf(0,"TimerActive..\n");
			xTimerDelete(TracebackTimer, 100);
			TracebackTimer = NULL;
		}
	}
}

void trace_back_timer_stop(void)
{
	if(TracebackTimer != NULL)
		xTimerStop(TracebackTimer, 100);
}

void trace_back_timer_restart(void)
{
	if(TracebackTimer != NULL)
		xTimerStart(TracebackTimer, 100);
}







