#ifndef GUI_SPORT_MARATHON_H
#define	GUI_SPORT_MARATHON_H

#include "watch_config.h"
#include "gui_sport.h"

#if defined WATCH_SIM_SPORT
	#define MARATHON_DATA_DISPLAY_SUM		12   	//马拉松数据显示选项数
	extern uint8_t m_marathon_data_display_buf[SPORT_DATA_DISPLAY_SUM];
	extern const uint8_t m_marathon_data_display_option[MARATHON_DATA_DISPLAY_SUM];

	#define MARATHON_REMIND_SET_SUM			5		//马拉松提醒设置选项数
	extern const uint8_t m_marathon_remind_set_option[MARATHON_REMIND_SET_SUM];
	
	extern void gui_sport_marathon_last_time_set(uint32_t time);
#endif

extern void gui_sport_marathon_init(void);
extern void gui_sport_marathon_last_pace_set(uint16_t pace);

//Display
extern void gui_sport_marathon_paint(void);
extern void gui_sport_marathon_pause_paint(void);
extern void gui_sport_marathon_cancel_paint(void);
extern void gui_sport_marathon_detail_paint(void);

//Key Event
extern void gui_sport_marathon_btn_evt(uint32_t Key_Value);
extern void gui_sport_marathon_pause_btn_evt(uint32_t Key_Value);
extern void gui_sport_marathon_cancel_btn_evt(uint32_t Key_Value);
extern void gui_sport_marathon_detail_btn_evt(uint32_t Key_Value);

#endif
