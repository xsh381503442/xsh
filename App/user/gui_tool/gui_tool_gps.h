#ifndef GUI_TOOL_GPS_H
#define	GUI_TOOL_GPS_H

#include "drv_ublox.h"
#include "com_sport.h"

enum{
	GOTO_SHARE_TRACK_BY_NONE = 0U,//0.一般运动 非云迹导航
	GOTO_SHARE_TRACK_BY_GPS,//1.云迹导航
	GOTO_SHARE_TRACK_BY_COUNTRYRUN,//2：越野跑界面云迹导航入口
	GOTO_SHARE_TRACK_BY_HIKING,//3：徒步越野界面云迹入口
};
extern uint8_t ENTER_NAVIGATION;    //进入云迹导航标记，用于从次界面进入运动ready界面按返回键情况判断
extern uint8_t ENTER_MOTION_TRAIL;  //进入运动轨迹导航标记，用于从次界面进入运动ready界面按返回键情况判断


extern uint8_t get_navigation_index(void);
//Display
extern void gui_tool_gps_paint(GPSMode *gps);
extern void gui_gps_detail_paint(void);
extern void gui_projection_paint(void);
extern void gui_cloud_navigation_paint(void);
extern void gui_motion_trail_paint(void);
extern void gui_navigation_ops_paint(void);
extern void gui_navigation_item_paint(void);
extern void gui_navigation_trail_paint(void);
extern void gui_navigation_del_paint(void);
extern void gui_motion_trail_detail_paint(void);
extern void gui_view_motion_trail_paint(void);
extern void gui_gps_trail_remind_paint(void);



//Key Event
extern void gui_tool_gps_btn_evt(uint32_t Key_Value);
extern void gui_gps_detail_btn_evt(uint32_t Key_Value);
extern void gui_projection_btn_evt(uint32_t Key_Value);
extern void gui_cloud_navigation_btn_evt(uint32_t Key_Value);
extern void gui_motion_trail_btn_evt(uint32_t Key_Value);
extern void gui_navigation_ops_btn_evt(uint32_t Key_Value);
extern void gui_navigation_item_btn_evt(uint32_t Key_Value);
extern void gui_navigation_trail_btn_evt(uint32_t Key_Value);
extern void gui_navigation_del_btn_evt(uint32_t Key_Value);
extern void gui_motion_trail_detail_btn_evt(uint32_t Key_Value);
extern void gui_view_motion_trail_btn_evt(uint32_t Key_Value);
extern uint32_t get_navigation_gps_startaddress(void);
extern uint32_t get_navigation_gps_endaddress(void);
extern uint32_t get_track_point_startaddress(void);
extern uint32_t get_track_point_endaddress(void);
extern void gui_cloud_navigation_btn_item (void);
extern void gui_cloud_navigation_load(void);

extern uint32_t get_motion_trail_startaddress(void);
extern uint32_t get_motion_trail_endaddress(void);
extern void gui_track__trail_load_paint(void);
extern void gui_track_load_btn_evt(uint32_t Key_Value);
extern void gui_motion_trail_btn_detail(void);
extern void gui_gps_trail_remind_btn_evt(uint32_t Key_Value);

extern uint8_t ActivityDataGetFlash(uint32_t address_s, ActivityDataStr *actdata);
#if defined WATCH_SPORT_NEW_CLOUD_NAVIGATION
extern bool IsShareTrackDataValid(ShareTrackStr *share_track_data);
extern int8_t ShareTrackDataTraverse(uint32_t address_s, ShareTrackStr *sport_data_rec, uint8_t num);
extern void init_cloud_navigation(uint32_t type);
extern void set_share_track_back_discreen(uint8_t status);
extern uint8_t get_share_track_back_discreen(void);
#endif


#endif
