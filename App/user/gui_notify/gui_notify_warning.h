#ifndef __GUI_NOTIFY_WARNING_H__
#define __GUI_NOTIFY_WARNING_H__


#include <stdint.h>



#define GUI_NOTIFY_WARNING_INDEX_WATCH_INFO_BLE_OFF             1
#define GUI_NOTIFY_WARNING_INDEX_ACCESSORY_BLE_OFF              2


void gui_notify_warning_paint(uint8_t index);
void gui_notify_warning_btn_evt(uint32_t evt);

void gui_notify_find_watch_piant(void);
void gui_notify_findwatch_timer_init(void);
void gui_notify_findwatch_timer_uninit(void);
void gui_notify_findwatch_timer_start(uint32_t timeout_msec);
void gui_notify_findwatch_timer_stop(void);

void gui_notify_find_watch_btn_evt(uint32_t evt);

void gui_notify_findwatch_response(void);







#endif //__GUI_NOTIFY_WARNING_H__
