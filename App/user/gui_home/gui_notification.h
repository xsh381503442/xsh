#ifndef __GUI_NOTIFICATION_H__
#define __GUI_NOTIFICATION_H__

#include <stdint.h>
#include "watch_config.h"

//extern int32_t m_msg_cnt;

extern uint16_t gui_notification_unread_msg_get(void);
extern uint8_t gui_notification_unread_msg_recogn(void);

#ifdef WATCH_HAS_WIFI
extern void gui_wifi_notification_paint(void);
extern void gui_wifi_notification_info_paint(void);
#else
extern void gui_notification_paint(void);
extern void gui_notification_info_paint(void);
#endif
extern void gui_notification_btn_evt(uint32_t Key_Value);
extern void gui_notification_info_btn_evt(uint32_t Key_Value);

#endif

