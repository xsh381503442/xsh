#ifndef GUI_H
#define	GUI_H

#include "com_dial.h"

//Display
extern void gui_bottom_time(void);
extern void gui_bottom_time_battery(uint8_t x);
extern void gui_battery(uint8_t x, uint8_t forecolor);

extern void gui_page_up(uint8_t color);
extern void gui_page_down(uint8_t color);
extern void gui_button_up(uint8_t color);
extern void gui_button_down(uint8_t color);
extern void gui_button_ok(uint8_t color);
extern void gui_button_back(uint8_t color);
extern void gui_button_middle(uint8_t color);

extern void gui_daysofweek_paint(uint8_t day);

extern void gui_progress_circle(uint8_t centery, uint8_t centerx, uint8_t radius, uint8_t thickness, 
									uint16_t startangle, uint16_t rangeangle, float percent, uint8_t forecolor, uint8_t bckgrndcolor);
extern void gui_progress_loop(uint8_t centery, uint8_t centerx, uint8_t radius, uint8_t thickness, 
							uint16_t startangle, uint16_t rangeangle, float percent, uint8_t forecolor, uint8_t bckgrndcolor,uint8_t flag);

extern void gui_progess_bar(float percent, ProgressBarStr *bar);

extern void gui_progess_toothed(uint16_t rangeangle, uint8_t toothed_num);

extern void gui_distance_histogram(uint8_t starx, uint8_t height, uint8_t stary, uint8_t width);
extern void gui_VO2max_histogram(uint8_t starx);
extern void gui_12h_pressure(uint8_t starx, uint8_t height, 	uint8_t stary, uint8_t width, uint8_t forecolor, uint8_t bckgrndcolor);

extern void gui_page_progress_bar(uint8_t page, uint8_t total);
extern void gui_sport_layout_preview(uint16_t x_axis, uint16_t y_axis, uint8_t bckgrndcolor, uint8_t num, uint8_t total);

#endif
