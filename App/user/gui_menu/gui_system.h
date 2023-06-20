#ifndef __GUI_SYSTEM_H__
#define __GUI_SYSTEM_H__



#include <stdint.h>
#include <stdbool.h>
#include "watch_config.h"


typedef struct {
	bool enable;
	int8_t hour;
	int8_t minute;
} _freq_set;


typedef struct {
	bool enable;
	int8_t hour;
	int8_t minute;
} _base_heartrate;

typedef struct {
	int8_t start_hour;
	int8_t start_minute;
	int8_t end_hour;
	int8_t end_minute;
} _time_set;






void gui_system_paint(void);
void gui_system_btn_evt(uint32_t evt);

void gui_system_base_heartrate_btn_evt(uint32_t evt);
void gui_system_base_heartrate_paint(void);

void gui_system_language_paint(void);
void gui_system_language_btn_evt(uint32_t evt);

void gui_system_backlight_paint(void);
void gui_system_backlight_btn_evt(uint32_t evt);

void gui_system_sound_paint(void);
void gui_system_sound_btn_evt(uint32_t evt);

extern void system_silent_mode_set(void);
extern void system_silent_mode_clear(void);
extern uint8_t system_silent_mode_get(void);

extern void gui_system_silent_paint(void);
extern void gui_system_silent_time_paint(void);
extern void gui_system_silent_btn_evt(uint32_t evt);
extern void gui_system_silent_time_btn_evt(uint32_t Key_Value);

extern uint8_t system_raiselight_mode_get(void);
extern void gui_system_autolight_paint(void);
extern void gui_system_autolight_time_paint(void);
extern void gui_system_autolight_btn_evt(uint32_t evt);
extern void gui_system_autolight_time_btn_evt(uint32_t Key_Value);





#endif //__GUI_SYSTEM_H__


