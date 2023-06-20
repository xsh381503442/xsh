#ifdef LAYER_APPLICATION 
#ifndef __GUI_NOTIFY_H__
#define __GUI_NOTIFY_H__


#include <stdint.h>

void gui_notify_rollback_paint(uint32_t gui_index);
void gui_notify_btn_evt(uint32_t evt);
void gui_notify_passkey_paint(void);
void gui_notify_bond_paint(void);
void gui_notify_disconnect_paint(void);
void gui_notify_connect_paint(void);
void gui_notify_sleep_mode_paint(void);

#endif //__GUI_NOTIFY_H__
#endif
