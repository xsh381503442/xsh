#ifndef __GUI_XT_BLE_AUTH_H__
#define __GUI_XT_BLE_AUTH_H__

#include "watch_config.h"

#ifdef WATCH_HAS_XINGTANG_FUNC
#include <stdint.h>

void gui_xt_ble_auth_paint(void);
void gui_xt_ble_auth_btn_evt(uint32_t evt);

void gui_xt_ble_device_option_paint(void);
void gui_xt_ble_device_option_btn_evt(uint32_t evt);

void gui_xt_ble_pinkey_paint(void);
void gui_xt_ble_pinkey_btn_evt(uint32_t evt);
#endif

#endif //__GUI_XT_BLE_AUTH_H__
