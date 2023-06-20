#ifndef __GUI_HWT_H__
#define __GUI_HWT_H__


#include <stdint.h>
#include <stdbool.h>
#include "watch_config.h"

typedef struct __attribute__ ((packed)){
	bool enable;
	uint8_t press_cnt;
} _gui_hwt_button;

#define HWT_EXTFLASH_TYPE_SECTOR       1


typedef struct __attribute__ ((packed)){
	bool enable;
	uint8_t type;
	uint32_t address;
	uint32_t size;
} _gui_hwt_extFlash;



typedef struct __attribute__ ((packed)){
	bool enable;
	short x;
	short y;
	short z;
	bool int1;
	bool int2;	
} _gui_hwt_motion;


typedef struct __attribute__ ((packed)){
	bool enable;
	uint32_t value;
} _gui_hwt_hr;

typedef struct __attribute__ ((packed)){
	bool enable;
	bool read_info;
	bool read_extFlash;
} _gui_hwt_ble;



typedef struct __attribute__ ((packed)){

	_gui_hwt_button button_pwr;
	_gui_hwt_button button_bl;
	_gui_hwt_button button_up;
	_gui_hwt_button button_ok;
	_gui_hwt_button button_dwn;
	
	bool lcd_enable;
	bool backlight_enable;
	bool motor_enable;
	bool buzzer_enable;
	_gui_hwt_extFlash extFlash;
	bool font_enable;

	_gui_hwt_motion g_sensor;
	_gui_hwt_motion gyro;
	_gui_hwt_motion compass;
	
	_gui_hwt_hr hr;
	
	bool gps_enable;
	_gui_hwt_ble ble;
#ifdef WATCH_CHIP_TEST
	bool hardware_test_enable;
#endif
} _gui_hwt;








void gui_hwt_paint(void);
void gui_hwt_btn_evt(uint32_t evt);



#endif //__GUI_HWT_H__

