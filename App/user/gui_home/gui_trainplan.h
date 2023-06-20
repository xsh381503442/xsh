#ifndef __GUI_TRAINPLAN_H__
#define __GUI_TRAINPLAN_H__

#include <stdint.h>
enum{
	WAYS_ENTER_TRAINPLAN_SPORT_STANDBUY = 0U,//待机快捷界面进入训练计划运动
	WAYS_ENTER_TRAINPLAN_SPORT_RUN,          //跑步运动进入训练计划运动
	WAYS_ENTER_TRAINPLAN_SPORT_REMIND,       //提醒界面进入训练计划运动
};
//display
extern void gui_trainplan_target_paint(void);
extern void gui_trainplan_today_hint_paint(void);
extern void gui_trainplan_target_hint_paint(void);
extern void gui_trainplan_today_type_content_paint(void);
extern void gui_trainplan_iscomplete_paint(void);
extern void gui_trainplan_pace_up_down_paint(uint16_t now_pace);
extern void gui_trainplan_pace_inter_paint(uint16_t num);
extern void gui_trainplan_sw_hint_paint(uint16_t hintFlag);
extern void gui_trainplan_run_select_paint(void);
//key
extern void gui_trainplan_target_btn_evt(uint32_t Key_Value);
extern void gui_trainplan_target_hint_btn_evt(uint32_t Key_Value);
extern void gui_trainplan_home_today_hint_btn_evt(uint32_t Key_Value);
extern void gui_trainplan_sport_today_hint_btn_evt(uint32_t Key_Value);
extern void gui_trainplan_is_complete_btn_evt(uint32_t Key_Value);
extern void gui_trainplan_today_hint_btn_evt(uint32_t Key_Value);
extern void gui_trainplan_pace_up_down_btn_evt(uint32_t Key_Value);
extern void gui_trainplan_pace_inter_btn_evt(uint32_t Key_Value);
extern void gui_trainplan_sw_hint_btn_evt(uint32_t Key_Value);
extern void gui_trainplan_run_select_btn_evt(uint32_t Key_Value);

extern void set_enter_trainplan_way(uint8_t ways);
extern uint8_t get_enter_trainplan_way(void);
#endif
