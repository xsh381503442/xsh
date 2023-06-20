//Author: Jason Gong
//Time：20180130 V1.00
/*All Swimming Interfaces and Display Varibles in gui_swimming.c are defined in here, except some common variables and 
pre-defined functions/data strings from task_sport.c task_sport.h, task_display.c, task_display.h 
gui_sport.c and com_sport.h   V1.00  */

#ifndef _GUI_SWIMMING_H__
#define _GUI_SWIMMING_H__


#include <stdint.h>
#include "drv_lcd.h"
#include "task_display.h"
#include "task_key.h"
#include "com_data.h"

//#if defined WATCH_SIM_SPORT

//#define SWIM_DATA_DISPLAY_SUM		11   //游泳数据显示选项数
#define SWIM_DATA_DISPLAY_SUM		7   //咕咚项目游泳数据显示选项数

extern uint8_t m_swim_data_display_buf[SPORT_DATA_DISPLAY_SUM];
extern const uint8_t m_swim_data_display_option[SWIM_DATA_DISPLAY_SUM];

#define SWIM_REMIND_SET_SUM			5    	//游泳提醒设置选项数
extern const uint8_t m_swim_remind_set_option[SWIM_REMIND_SET_SUM];

extern void gui_sport_swim_laps_set(uint16_t laps);
extern void gui_sport_swim_ave_stroke_set(uint16_t stroke);

extern void gui_sport_swim_init(void);

extern void gui_sport_swim_paint(void);
extern void gui_sport_swim_pause_paint(void);
extern void gui_sport_swim_cancel_paint(void);
extern void gui_sport_swim_detail_paint(void);

extern void gui_sport_swim_btn_evt(uint32_t Key_Value);

extern void gui_sport_swim_pause_btn_evt(uint32_t Key_Value);
extern void gui_sport_swim_cancel_btn_evt(uint32_t Key_Value);
extern void gui_sport_swim_detail_btn_evt(uint32_t Key_Value);
extern void gui_sport_swim_heart_paint(void);

//#elif defined WATCH_COM_SPORT

/*extern void gui_sport_swim_init(void);

extern void gui_sport_swim_paint(void);
extern void gui_sport_swim_pause_paint(void);
extern void gui_sport_swim_cancel_paint(void);
extern void gui_sport_swim_detail_paint(void);

extern void gui_sport_swim_btn_evt(uint32_t Key_Value);

extern void gui_sport_swim_pause_btn_evt(uint32_t Key_Value);
extern void gui_sport_swim_cancel_btn_evt(uint32_t Key_Value);
extern void gui_sport_swim_detail_btn_evt(uint32_t Key_Value);
enum	
{
	SwimPoolLengthTen = 0U,
	SwimPoolLengthOne = 1U,
};

typedef struct {
	uint8_t ten;
	uint8_t one;
}Number_Below_Hundred_t;*/


//#else


//#endif
extern void gui_swimming_save_detail_1_paint(uint8_t backgroundcolor);
extern void gui_swimming_save_detail_2_paint(uint8_t backgroundcolor);

#if defined WATCH_SIM_SPORT

#else
/*extern void gui_swimming_poollength_paint(void);
extern void gui_swimming_save_heartratezone_paint(uint8_t backgroundcolor);

extern void gui_swich_msg(void);
extern void gui_sport_index_down(ScreenState_t FirstIndex,ScreenState_t LastIndex);
extern void gui_sport_index_up(ScreenState_t FirstIndex,ScreenState_t LastIndex);

extern void gui_swimming_data_btn_evt(uint32_t Key_Value);
extern void gui_swimming_stats_btn_evt(uint32_t Key_Value);
extern void gui_swimming_ready_btn_evt(uint32_t Key_Value);
extern void gui_swimming_pause_btn_evt(uint32_t Key_Value);
extern void gui_swimming_pool_btn_evt(uint32_t Key_Value);


extern void gui_sport_cancel_btn_evt(uint32_t Key_Value);
extern void gui_sport_feel_btn_evt(uint32_t Key_Value);

extern void gui_swimming_save_detail_1_btn_evt(uint32_t Key_Value);
extern void gui_swimming_save_detail_2_btn_evt(uint32_t Key_Value);
extern void gui_swimming_save_track_btn_evt(uint32_t Key_Value);
extern void gui_swimming_save_heartratezone_btn_evt(uint32_t Key_Value);

extern void gui_swimming_pause_screen_btn_evt(uint32_t Key_Value);*/

#endif
#endif
