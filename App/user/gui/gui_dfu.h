#ifndef __GUI_DFU_H__
#define __GUI_DFU_H__





#include <stdint.h>





void gui_dfu_paint(void);
void gui_dfu_btn_evt(uint32_t Key_Value);
void gui_dfu_timer_stop(void);
void gui_dfu_timer_start(uint32_t timeout_msec);
void gui_dfu_timer_uninit(void);
void gui_dfu_timer_init(void);
#ifdef COD 
void gui_update_cod_dial_paint(void);
#endif 






#endif //__GUI_DFU_H__
