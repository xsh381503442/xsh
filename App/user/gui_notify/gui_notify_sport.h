#ifndef __GUI_NOTIFY_SPORT_H__
#define __GUI_NOTIFY_SPORT_H__




#include <stdint.h>

//自动计圈上一圈数据
typedef struct {
	uint32_t time;		//耗时
	uint16_t lap;		//圈数
	uint16_t pace;		//配速、速度
	uint16_t frequency;	//步频
	uint16_t riseheight;//上升高度
	uint16_t dropheight;//下降高度	
	uint8_t  heart;		//心率
} last_circle_t;

extern void gui_notify_sport_circle_paint(uint32_t circle);
extern void gui_notify_sport_time_paint(uint32_t time);
extern void gui_notify_sport_distance_paint(uint32_t distance);
extern void gui_notify_sport_pace_paint(uint32_t pace);
extern void gui_notify_sport_height_paint(uint32_t height);
extern void gui_notify_sport_speed_paint(uint32_t speed);
extern void gui_notify_sport_calory_paint(uint32_t speed);
extern void gui_notify_sport_atuocircle_paint(void);

extern void gui_notify_sport_circle_btn_evt(uint32_t Key_Value);
extern void gui_notify_sport_time_btn_evt(uint32_t Key_Value);
extern void gui_notify_sport_distance_btn_evt(uint32_t Key_Value);
extern void gui_notify_sport_pace_btn_evt(uint32_t Key_Value);
extern void gui_notify_sport_height_btn_evt(uint32_t Key_Value);
extern void gui_notify_sport_speed_btn_evt(uint32_t Key_Value);
extern void gui_notify_sport_calory_btn_evt(uint32_t Key_Value);
extern void gui_notify_sport_autocircle_btn_evt(uint32_t Key_Value);
#endif 
