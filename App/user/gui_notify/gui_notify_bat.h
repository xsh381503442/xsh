#ifndef __GUI_NOTIFY_BAT_H__
#define __GUI_NOTIFY_BAT_H__






#include <stdint.h>




void gui_monitor_bat_low_paint(void);
void gui_monitor_bat_low_btn_evt(uint32_t Key_Value);


void gui_monitor_bat_chg_paint(void);
void gui_monitor_bat_chg_btn_evt(uint32_t Key_Value);


#endif //__GUI_NOTIFY_LOW_BAT_H__

