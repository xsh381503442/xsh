#ifndef GUI_SPORT_INDOORRUN_H
#define	GUI_SPORT_INDOORRUN_H

#include "watch_config.h"
#include "gui_sport.h"


#if defined WATCH_SIM_SPORT
	#define INDOORRUN_DATA_DISPLAY_SUM		10   	//室内跑跑步数据显示选项数
	extern uint8_t m_indoorrun_data_display_buf[SPORT_DATA_DISPLAY_SUM];
	extern const uint8_t m_indoorrun_data_display_option[INDOORRUN_DATA_DISPLAY_SUM];

	#define INDOORRUN_REMIND_SET_SUM		3		//跑步提醒设置选项数
	extern const uint8_t m_indoorrun_remind_set_option[INDOORRUN_REMIND_SET_SUM];
	
	extern void gui_sport_indoorrun_init(void);
#elif defined WATCH_COM_SPORT
extern void gui_sport_indoorrun_init(void);
extern void gui_sport_set_indoorleng_paint(void);
extern void gui_sport_set_indoorleng_btn_evt(uint32_t Key_Value);
typedef struct {
	int8_t tens;
	int8_t ones;
} _indoorleng;

#endif

//Display
extern void gui_sport_indoorrun_paint(void);
extern void gui_sport_indoorrun_pause_paint(void);
extern void gui_sport_indoorrun_cancel_paint(void);
extern void gui_sport_indoorrun_detail_paint(void);

//Key Event
extern void gui_sport_indoorrun_btn_evt(uint32_t Key_Value);
extern void gui_sport_indoorrun_pause_btn_evt(uint32_t Key_Value);
extern void gui_sport_indoorrun_cancel_btn_evt(uint32_t Key_Value);
extern void gui_sport_indoorrun_detail_btn_evt(uint32_t Key_Value);

#endif
