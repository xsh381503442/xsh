#ifndef GUI_SPORT_CYCLING_H
#define	GUI_SPORT_CYCLING_H

#include "watch_config.h"
#include "gui_sport.h"

//#define CYCLING_DATA_DISPLAY_SUM		11   	//骑行数据显示选项数
	
#define CYCLING_DATA_DISPLAY_SUM		6		//咕咚项目骑行数据显示选项数
	extern uint8_t m_cycling_data_display_buf[SPORT_DATA_DISPLAY_SUM];
	extern const uint8_t m_cycling_data_display_option[CYCLING_DATA_DISPLAY_SUM];
	extern uint8_t vice_screen_cycling_display[SPORT_DATA_DISPLAY_SUM];

	#define CYCLING_REMIND_SET_SUM			6		//骑行提醒设置选项数
	extern const uint8_t m_cycling_remind_set_option[CYCLING_REMIND_SET_SUM];
	
	extern void gui_sport_cycling_last_time_set(uint32_t time);

extern void gui_sport_cycling_init(void);

//Display
extern void gui_sport_cycling_paint(void);
extern void gui_sport_cycling_pause_paint(void);
extern void gui_sport_cycling_cancel_paint(void);
extern void gui_sport_cycling_detail_paint(void);
extern void gui_sport_cycling_track_paint(void);


//Key Event
extern void gui_sport_cycling_btn_evt(uint32_t Key_Value);
extern void gui_sport_cycling_pause_btn_evt(uint32_t Key_Value);
extern void gui_sport_cycling_cancel_btn_evt(uint32_t Key_Value);
extern void gui_sport_cycling_detail_btn_evt(uint32_t Key_Value);

#endif
