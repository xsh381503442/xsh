#ifndef __GUI_UTC_GET_H__
#define __GUI_UTC_GET_H__



#include <stdint.h>

#define GUI_UTC_GET_EVT_SEARCHING         0
#define GUI_UTC_GET_EVT_SUCCESS           1
#define GUI_UTC_GET_EVT_FAIL              2

typedef struct {
	uint8_t evt;
	uint8_t progress;

} _gui_utc_get_param;


void gui_utc_get_paint(void);
void gui_utc_get_btn_evt(uint32_t evt);




#endif //__GUI_UTC_GET_H__
