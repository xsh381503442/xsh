#ifndef __GUI_CALORIES_H__
#define __GUI_CALORIES_H__

#include <stdint.h>
//display
extern void gui_calories_sports_foundation_paint(void);
extern void gui_calories_week_sports_paint(void);
//key
extern void gui_calories_sports_foundation_btn_evt(uint32_t Key_Value);
extern void gui_calories_week_sports_btn_evt(uint32_t Key_Value);
#endif
