#ifndef _TASK_SPORT_H__
#define _TASK_SPORT_H__

#include <stdint.h>
#include "app_config.h"
#include "task_display.h"
#include "watch_config.h"
#include "com_sport.h"
#include "task_config.h"

#if DEBUG_ENABLED == 1 && TASK_SPORT_LOG_ENABLED == 1
	#define TASK_SPORT_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define TASK_SPORT_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define TASK_SPORT_WRITESTRING(...)
	#define TASK_SPORT_PRINTF(...)		        
#endif

#define	STEP_SW_MASK		0x01    //??????λ
#define	COMPASS_SW_MASK		0x02    //?????λ
#define	HEARTRATE_SW_MASK	0x04    //??????λ
#define	HEIGHT_SW_MASK		0x08    //?????λ
#define	GPS_SW_MASK			0x10    //gps???λ
#define	GYRO_SW_MASK		0x20    //????????λ

extern TaskHandle_t TaskSportHandle;
extern SemaphoreHandle_t ActSemaphore;
extern uint16_t SportAngle;
extern volatile ScreenState_t Sport_Index;	//???????
#if defined WATCH_SPORT_NEW_CLOUD_NAVIGATION
extern uint8_t is_crosscountry;
#endif


extern void ReadySport(ScreenState_t index);
extern void ReadySportBack(void);
extern void StartSport(void);
extern void SaveSport( uint8_t sta );
extern void CloseSport(void);

extern uint32_t Get_PauseTotolTime(void);
extern bool Get_TaskSportSta(void);
extern void Set_TaskSportSta(bool sta);
extern void Set_PauseSta(bool sta);
extern bool Get_PauseSta(void);
extern void Set_SaveSta(uint8_t sta);
extern uint8_t Get_SaveSta(void);
extern uint8_t Get_SwimSta(void);
extern uint16_t get_ave_frequency(void);
extern uint8_t get_cyclingsta(void);
void set_cyclingsta(uint8_t sta);

extern void CreatSportTask(void);
extern void CloseSportTask(void);

extern void TaskSport(void* pvParameter);

extern uint8_t Get_Sport_Heart(void);
extern void OpenSensor(void);
extern void CloseSensor(void);

extern void sport_triathlon_change_start(void);
extern void sport_continue_later_start(void);
extern void sport_continue_later_end(void);
extern bool Get_IsInSportMode(void);
extern void sport_data_input(uint8_t pausesta,uint8_t stride,uint32_t steps,uint8_t activity_type);

extern uint32_t Get_Cycling_AvgCadence(void);
#if defined WATCH_SPORT_EVENT_SCHEDULE
extern void ReadyCloudSportBack(void);
extern void set_sport_pause_event_status(uint8_t status);
extern uint8_t get_sport_pause_event_status(void);
extern SportEventStatusStr g_sport_event_status;//赛事赛段状???
extern SportScheduleCalStr g_sport_event_schedule_detail_cal;//赛事赛段状???详??
extern uint8_t g_start_search_cloud_status;
#endif

extern void kf_initial(void);

#endif


