#ifndef _GUI_RUN_H__
#define _GUI_RUN_H__

#include <stdint.h>
#include "drv_lcd.h"
#include "task_display.h"
#include "task_key.h"
#include "gui_sport.h"


#define RUN_DATA_DISPLAY_SUM		8   //咕咚项目跑步数据显示选项数
extern uint8_t vice_screen_run_display[SPORT_DATA_DISPLAY_SUM];

extern uint8_t m_run_data_display_buf[SPORT_DATA_DISPLAY_SUM];
extern const uint8_t m_run_data_display_option[RUN_DATA_DISPLAY_SUM];

#define RUN_REMIND_SET_SUM			6    	//跑步提醒设置选项数
extern const uint8_t m_run_remind_set_option[RUN_REMIND_SET_SUM];


extern void gui_sport_run_paint(void);
extern void gui_sport_run_pause_paint(void);
extern void gui_sport_run_cancel_paint(void);
extern void gui_sport_run_detail_paint(void);

extern void gui_sport_run_init(void);
extern void gui_sport_run_btn_evt(uint32_t Key_Value);

extern void gui_sport_run_pause_btn_evt(uint32_t Key_Value);
extern void gui_sport_run_cancel_btn_evt(uint32_t Key_Value);
extern void gui_sport_run_detail_btn_evt(uint32_t Key_Value);

extern void gui_sport_pause_evt(void);
extern void gui_sport_run_track_paint(void);

//#elif defined WATCH_COM_SPORT
/*extern void gui_sport_run_paint(void);
extern void gui_sport_run_pause_paint(void);
extern void gui_sport_run_cancel_paint(void);
extern void gui_sport_run_detail_paint(void);

extern void gui_sport_run_init(void);
extern void gui_sport_pause_evt(void);

extern void gui_sport_run_btn_evt(uint32_t Key_Value);

extern void gui_sport_run_pause_btn_evt(uint32_t Key_Value);
extern void gui_sport_run_cancel_btn_evt(uint32_t Key_Value);
extern void gui_sport_run_detail_btn_evt(uint32_t Key_Value);*/

//#else
/*enum
{
	RunPauseContinue = 0U,	//继续
	RunPauseSave = 1U,		//结束运动
	RunPauseTrackback = 2U,	//循迹返航
	RunPauseCancel = 3U,	//放弃
};




extern void gui_run_distance_paint(uint32_t distance);
extern void gui_run_time_paint(uint8_t x,uint8_t y,uint8_t forecolor);
extern void gui_run_heartrate_paint(uint8_t heart);
extern void gui_run_speed_paint(uint16_t speed);
extern void gui_realtime_battery_paint(uint8_t percent);

extern void gui_run_prompt_paint(void);
extern void gui_run_track_update_paint(void);

extern void gui_run_track_paint(void);
extern void gui_run_pause_track_paint(void);
extern void gui_run_data_paint(void);
extern void gui_run_pause_data_paint(void);
extern void gui_run_ready_paint(void);
extern void gui_run_pause_paint(void);


extern void gui_sport_cancel_paint(void);
extern void gui_sport_feel_paint(void);
extern void gui_sport_saving_paint(uint8_t endangle);
extern void gui_sport_saved_paint(void);
*/
//#endif


extern void gui_run_save_detail_1_paint(uint8_t backgroundcolor);
extern void gui_run_save_detail_2_paint(uint8_t backgroundcolor);

#if defined WATCH_SIM_SPORT
#elif defined WATCH_COM_SPORT
#else
/*extern void gui_run_save_track_paint(uint8_t backgroundcolor);
extern void gui_run_save_heartratezone_paint(uint8_t backgroundcolor);

extern void gui_run_data_trackback_paint(void);
extern void gui_run_data_trackback_btn_evt(uint32_t Key_Value);
extern void gui_run_track_trackback_paint(void);
extern void gui_run_track_trackback_btn_evt(uint32_t Key_Value);
extern void gui_run_trackback_pause_paint(void);
extern void gui_run_trackback_pause_btn_evt(uint32_t Key_Value);*/
/*
extern void gui_swich_msg(void);
extern void gui_sport_index_down(ScreenState_t FirstIndex,ScreenState_t LastIndex);
extern void gui_sport_index_up(ScreenState_t FirstIndex,ScreenState_t LastIndex);
*/

/*extern void gui_run_data_btn_evt(uint32_t Key_Value);
extern void gui_run_pause_data_btn_evt(uint32_t Key_Value);
extern void gui_run_track_btn_evt(uint32_t Key_Value);
extern void gui_run_ready_btn_evt(uint32_t Key_Value);
extern void gui_run_pause_btn_evt(uint32_t Key_Value);
extern void gui_run_pause_track_btn_evt(uint32_t Key_Value);*/
/*
extern void gui_sport_cancel_btn_evt(uint32_t Key_Value);
extern void gui_sport_feel_btn_evt(uint32_t Key_Value);
extern void gui_sport_feel_init(void);
*/
/*extern void gui_run_save_detail_1_btn_evt(uint32_t Key_Value);
extern void gui_run_save_detail_2_btn_evt(uint32_t Key_Value);
extern void gui_run_save_track_btn_evt(uint32_t Key_Value);
extern void gui_run_save_heartratezone_btn_evt(uint32_t Key_Value);
extern void gui_run_pause_back_data_trackback_pause_paint(void);
extern void gui_run_pause_back_data_trackback_pause_btn_env(uint32_t Key_Value);
extern void gui_run_pause_back_track_trackback_pause_paint(uint32_t distance);
extern void gui_run_pause_back_track_trackback_pause_btn_env(uint32_t Key_Value);
extern void Go_to_Run_Pause(void);*/
#endif

#endif

