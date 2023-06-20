#ifndef GUI_SPORT_WALK_H
#define	GUI_SPORT_WALK_H
#include "gui_sport.h"

#if defined WATCH_SIM_SPORT

#define WALK_DATA_DISPLAY_SUM		11    //跑步数据显示选项数
extern uint8_t m_walk_data_display_buf[SPORT_DATA_DISPLAY_SUM];
extern const uint8_t m_walk_data_display_option[WALK_DATA_DISPLAY_SUM];

#define WALK_REMIND_SET_SUM			4    	//跑步提醒设置选项数
extern const uint8_t m_walk_remind_set_option[WALK_REMIND_SET_SUM];

extern void gui_sport_walk_last_time_set(uint32_t time);

#endif
extern void gui_sport_walk_init(void);

//Display
extern void gui_sport_walk_paint(void);
extern void gui_sport_walk_pause_paint(void);
extern void gui_sport_walk_cancel_paint(void);
extern void gui_sport_walk_detail_paint(void);

//Key Event
extern void gui_sport_walk_btn_evt(uint32_t Key_Value);
extern void gui_sport_walk_pause_btn_evt(uint32_t Key_Value);
extern void gui_sport_walk_cancel_btn_evt(uint32_t Key_Value);
extern void gui_sport_walk_detail_btn_evt(uint32_t Key_Value);

#endif
