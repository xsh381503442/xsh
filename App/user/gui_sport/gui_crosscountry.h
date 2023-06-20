#ifndef _GUI_CROSSCOUNTRY_H__
#define _GUI_CROSSCOUNTRY_H__

#include <stdint.h>
#include "drv_lcd.h"
#include "task_display.h"
#include "task_key.h"
#include "com_data.h"

//extern bool cancel_index;//放弃选项索引(否,是)
#define CROSSCOUNTRY_DATA_DISPLAY_SUM		8		//越野跑数据显示选项数
#define CROSSCOUNTRY_REMIND_SET_SUM			6		//越野跑提醒设置选项数
extern uint8_t m_crosscountry_data_display_buf[SPORT_DATA_DISPLAY_SUM];
extern const uint8_t m_crosscountry_data_display_option[CROSSCOUNTRY_DATA_DISPLAY_SUM];
extern const uint8_t m_crosscountry_remind_set_option[CROSSCOUNTRY_REMIND_SET_SUM];



extern void gui_sport_crosscountry_paint(void);
extern void gui_sport_crosscountry_pause_paint(void);
extern void gui_sport_crosscountry_cancel_paint(void);
extern void gui_sport_crosscountry_detail_paint(void);

extern void gui_sport_crosscountry_init(void);

extern void gui_sport_crosscountry_btn_evt(uint32_t Key_Value);

extern void gui_sport_crosscountry_pause_btn_evt(uint32_t Key_Value);
extern void gui_sport_crosscountry_cancel_btn_evt(uint32_t Key_Value);
extern void gui_sport_crosscountry_detail_btn_evt(uint32_t Key_Value);
extern void gui_crosscountry_prompt_paint(void);


extern void gui_crosscountry_save_detail_1_paint(uint8_t backgroundcolor);
extern void gui_crosscountry_save_detail_2_paint(uint8_t backgroundcolor);
extern void gui_crosscountry_save_detail_3_paint(uint8_t backgroundcolor);
extern void gui_crosscountry_cloud_navigation_paint(void);


void gui_sport_crosscountry_track_paint(void);

#endif

