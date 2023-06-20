#ifndef __GUI_ALARM_H__
#define __GUI_ALARM_H__



#include <stdint.h>
#include <stdbool.h>


#define GUI_ALARM_SET_REPITION_SINGLE       0
#define GUI_ALARM_SET_REPITION_DAILY        1
#define GUI_ALARM_SET_REPITION_WEEK_DAY     2
#define GUI_ALARM_SET_REPITION_WEEKEND      3
typedef struct {
	int8_t hour;
	int8_t minute;
} _alarm_time;
typedef struct {
	bool enable;
	_alarm_time time;
	uint8_t repetition_type;
	bool buzzer_enable;
	bool motor_enable;
	
} _alarm_instance;

extern void gui_alarm_paint(void);
extern void gui_alarm_btn_evt(uint32_t evt);

extern void gui_alarm_set_paint(void);
extern void gui_alarm_set_btn_evt(uint32_t evt);

extern void gui_alarm_set_time_btn_evt(uint32_t evt);
extern void gui_alarm_set_time_paint(void);

extern void gui_alarm_set_sound_N_viberate_paint(void);
extern void gui_alarm_set_sound_N_viberate_btn_evt(uint32_t evt);

extern void gui_alarm_set_repition_btn_evt(uint32_t evt);
extern void gui_alarm_set_repition_paint(void);

#endif //__GUI_ALARM_H__
