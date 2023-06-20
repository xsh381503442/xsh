#ifndef GUI_DIAL_H
#define	GUI_DIAL_H

#include "com_dial.h"






static void gui_dial_background(DialDataStr *dial);
static void gui_dial_time(DialDataStr *dial, am_hal_rtc_time_t *time);
static void gui_dial_date(DialDataStr *dial);
static void gui_dial_widget(DialDataStr *dial);
static void gui_dial_status(DialDataStr *dial);
static void gui_dial_battery(DialDataStr *dial);
static void gui_dial_heart(DialDataStr *dial);
static void gui_dial_weather(DialDataStr *dial);
static void gui_dial_ambient(DialDataStr *dial);
static void gui_dial_graph(DialDataStr *dial);
static void gui_dial_kcal(DialDataStr *dial);

extern void gui_dial_preview(uint8_t number, DialDataStr *dial);

 static void gui_dial_get_weather(void);

extern void gui_dial_paint(void);
extern void gui_dial_combat_or_counting_down_paint(void);

#endif
