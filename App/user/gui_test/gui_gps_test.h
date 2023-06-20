#ifndef _GUI_TEST_GPS_H__
#define _GUI_TEST_GPS_H__

#include <stdint.h>
#include "drv_ublox.h"

extern void DisplayTestGPS(GPSMode *gps);
extern void DisplayTestHdr( uint8_t hdr_value );
extern void DisplayTestFlash(bool isTrue);
extern void DisplayTestGPSDetail(GPSMode *gps);

extern void MsgPostGPS(uint32_t Key_Value);
#if defined(DEBUG_FACTORY_HDR_AMBIENT_TEST)
extern void DisplayTestHdrAmbient_paint(float pres,float alt,float temp,uint8_t hdr_value);
extern void GoToDisplayTestHdrAmbient_btn_evt(void);
extern void DisplayTestHdrAmbient_btn_evt(uint32_t Key_Value);
#endif
extern uint16_t g_Second;//测试时，界面计时刷新次数，1秒1次,0~120秒一个计时圈
extern uint8_t g_status;
extern void gui_post_gps_evt(uint32_t Key_Value);
extern void gui_post_gps_details_evt(uint32_t Key_Value);
#endif


