#ifndef __GUI_NOTIFY_TRAIN_H__
#define __GUI_NOTIFY_TRAIN_H__




#include <stdint.h>


extern void gui_notify_train_step_paint(void);
extern void gui_notify_train_step_btn_evt(uint32_t Key_Value);
extern void gui_notify_train_calory_paint(void);
extern void gui_notify_train_calory_btn_evt(uint32_t Key_Value);
extern void gui_notify_heartrate_warning_paint(uint32_t Heart);
extern void gui_notify_heartrate_warning_btn_evt(uint32_t Key_Value);
	
#endif

