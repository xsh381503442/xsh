#ifndef __GUI_SPORTS_RECORD_H__
#define __GUI_SPORTS_RECORD_H__


#include <stdint.h>

#include "com_sport.h"

typedef struct {
	uint8_t hour;
	uint8_t minute;
  uint8_t percentage;
} _gui_sports_record_hr_section;

typedef struct {
	float hour;
	uint8_t minute;
	uint8_t second;
} _gui_sports_record_time;
typedef struct {
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
} _gui_sports_time;

typedef struct {
	uint8_t type;
	uint16_t year;
	uint8_t month;
	uint8_t date;
	_gui_sports_time           time_stamp;
	_gui_sports_time           time_elapse;
	float distance;
	float avg_speed;
	float max_speed;
	float calory;
	uint8_t avg_hr;
	uint8_t max_hr;
	float oxygen_efficiency;
	float non_oxygen_efficiency;
	uint32_t	steps;
	uint16_t riseHeight;
	uint16_t dropHeight;
	_gui_sports_record_time           time_recovery;
	_gui_sports_record_hr_section     hr_section_one;
	_gui_sports_record_hr_section     hr_section_two;
	_gui_sports_record_hr_section     hr_section_three;
	_gui_sports_record_hr_section     hr_section_four;
  _gui_sports_record_hr_section     hr_section_five;
	
	
} _gui_sports_record;


void gui_sports_record_paint(void);
void gui_sports_record_btn_evt(uint32_t evt);

void gui_sports_record_detail_btn_evt(uint32_t evt);
void gui_sports_record_detail_paint(void);

void gui_sports_record_delete_btn_evt(uint32_t evt);
void gui_sports_record_delete_paint(void);

void gui_sports_record_delete_notify_btn_evt(uint32_t evt);
void gui_sports_record_delete_notify_paint(void);	

extern uint8_t data_up_down_flag;
extern uint32_t sport_data_addr[2];
extern uint32_t g_SportRecord_Going_Screen_Flag;
extern uint8_t LOADINGFLAG;
extern ActivityDataStr g_gpstrack_data;
extern void Get_SportTrack_GpsAddress(uint32_t addr);
extern void gui_sports_track_paint(uint8_t backgroundcolor);
extern void List_None(void);
extern uint32_t activity_data_index;//用于运动记录界面上下键切换标记
#if defined WATCH_SPORT_EVENT_SCHEDULE
/*查找赛事 徒步越野 越野跑有赛事 运动记录详情不同*/
extern void search_sport_event_schedule_record(ActivityDataStr m_ActivityDataStr);
#endif
#endif //__GUI_SPORTS_RECORD_H__
