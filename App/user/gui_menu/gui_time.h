#ifndef __GUI_TIME_H__
#define __GUI_TIME_H__



#include <stdint.h>
#include "com_data.h"

#define	GUI_TIME_UTC_TIMEOUT	60   				//…Ë÷√Œ™10√Î≥¨ ±
void gui_time_paint(void);
void gui_time_btn_evt(uint32_t evt);

#define GUI_TIME_UTC_GET_EVT_SEARCHING         0
#define GUI_TIME_UTC_GET_EVT_SUCCESS           1
#define GUI_TIME_UTC_GET_EVT_FAIL              2


typedef struct {
	uint8_t evt;
	uint8_t progress;

} _gui_time_utc_get_param;


void gui_time_utc_get_init(void);
void gui_time_utc_get_paint(void);
void gui_time_utc_get_btn_evt(uint32_t evt);




#include <stdbool.h>





extern void gui_time_alarm_paint(void);
extern void gui_time_alarm_btn_evt(uint32_t evt);

extern void gui_time_alarm_set_paint(void);
extern void gui_time_alarm_set_btn_evt(uint32_t evt);

extern void gui_time_alarm_set_time_btn_evt(uint32_t evt);
extern void gui_time_alarm_set_time_paint(void);

extern void gui_time_alarm_set_sound_N_viberate_paint(void);
extern void gui_time_alarm_set_sound_N_viberate_btn_evt(uint32_t evt);

extern void gui_time_alarm_set_repition_btn_evt(uint32_t evt);
extern void gui_time_alarm_set_repition_paint(void);

#if defined WATCH_TIMEZONE_SET
extern void gui_time_zone_set_paint(void);
extern void gui_time_zone_select_paint(void);

extern void gui_time_zone_set_btn_evt(uint32_t Key_Value);
extern void gui_time_zone_select_btn_evt(uint32_t Key_Value);

#endif

#endif //__GUI_TIME_H__
