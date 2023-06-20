#ifndef GUI_CLIMBING_H__
#define GUI_CLIMBING_H__

#include <stdint.h>
#include "drv_lcd.h"
#include "task_display.h"
#include "task_key.h"

#define CLIMBING_DATA_DISPLAY_SUM		7		//咕咚项目登山数据显示选项数
#define CLIMBING_REMIND_SET_SUM			6		//登山提醒设置选项数

extern uint8_t m_climbing_data_display_buf[SPORT_DATA_DISPLAY_SUM];
extern const uint8_t m_climbing_data_display_option[CLIMBING_DATA_DISPLAY_SUM];
extern const uint8_t m_climbing_remind_set_option[CLIMBING_REMIND_SET_SUM];

extern uint8_t vice_screen_climbing_display[SPORT_DATA_DISPLAY_SUM];

extern void gui_sport_climbing_init(void);

extern void gui_sport_climbing_paint(void);
extern void gui_sport_climbing_pause_paint(void);
extern void gui_sport_climbing_cancel_paint(void);
extern void gui_sport_climbing_detail_paint(void);
extern void gui_sport_climbing_heart_paint(void);

	
extern void gui_sport_climbing_btn_evt(uint32_t Key_Value);
	
extern void gui_sport_climbing_pause_btn_evt(uint32_t Key_Value);
extern void gui_sport_climbing_cancel_btn_evt(uint32_t Key_Value);
extern void gui_sport_climbing_detail_btn_evt(uint32_t Key_Value);



extern void gui_climbing_save_detail_1_paint(uint8_t backgroundcolor);
extern void gui_climbing_save_detail_2_paint(uint8_t backgroundcolor);
void gui_sport_climbing_track_paint(void);


#endif
