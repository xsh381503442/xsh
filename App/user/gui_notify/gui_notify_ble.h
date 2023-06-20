#ifndef __GUI_NOTIFY_BLE_H__
#define __GUI_NOTIFY_BLE_H__



#include <stdint.h>
#include "com_data.h"

#include "com_data.h"



void gui_notify_ble_connect_paint(void);
void gui_notify_ble_connect_btn_evt(uint32_t evt);

void gui_notify_ble_disconnect_paint(void);
void gui_notify_ble_disconnect_btn_evt(uint32_t evt);

void gui_notify_ble_bond_paint(void);
void gui_notify_ble_bond_btn_evt(uint32_t evt);

void gui_notify_ble_passkey_paint(void);
void gui_notify_ble_passkey_btn_evt(uint32_t evt);
#ifdef COD 
void gui_notify_on_bind_device_paint(void);
void gui_notify_on_bind_device_btn_evt(uint32_t evt);
void gui_notify_bind_status_paint(cod_bind_status status);
void gui_notify_bind_status_btn_evt(uint32_t evt);
void gui_training_cast_start_remind_paint(void);
void gui_training_cast_press_remind_paint(void);
void gui_training_cast_end_remind_paint(void);
void gui_training_cast_end_remind_btn_evt(uint32_t evt);
void gui_training_cast_pause_paint(void);
void gui_training_cast_pause_btn_evt(uint32_t evt);
void gui_training_cast_paint(void);
void gui_training_cast_btn_evt(uint32_t evt);
void gui_train_pause_time(void);
void set_train_pause_time(void);
void gui_vice_sport_btn_evt(uint32_t evt);
void gui_sport_cast_pause_paint(void);
void gui_training_cast_remind_btn_evt(uint32_t evt);
void gui_sport_cast_pause_btn_evt(uint32_t evt);
extern void gui_notify_locatin_ok_btn_evt(uint32_t evt);
extern void gui_notify_locatin_ok_paint(void);


#endif




#endif //__GUI_NOTIFY_BLE_H__
