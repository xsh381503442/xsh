#ifndef __GUI_MOTIONSRECORD_H__
#define __GUI_MOTIONSRECORD_H__

#include <stdint.h>
#include <stdbool.h>
//display
extern void gui_motionsrecord_last_paint(void);
//key
extern void gui_motionsrecord_last_btn_evt(uint32_t Key_Value);

extern void GetSportDetail(uint32_t addr1,uint32_t addr2);
extern uint8_t getImgSport(uint8_t type);
extern uint32_t g_motion_record_addr;
#endif

