#ifndef GUI_SPORT_TRIATHLON_H
#define	GUI_SPORT_TRIATHLON_H

#include "bsp_rtc.h"

typedef struct
{
	rtc_time_t 	StartTime;			//铁人三项开始时间
	uint8_t 	FininshNum;			//完成项数
	uint8_t 	HeartAvg;			//平均心率
	uint32_t	SwimmingTime;		//游泳时长
	uint32_t 	SwimmingDistance;	//游泳里程
	uint32_t 	FirstWaitTime;		//第一次转项时长
	uint32_t	CyclingTime;		//骑行时长
	uint32_t 	CyclingDistance;	//骑行里程
	uint32_t 	SecondWaitTime;		//第二次转项时长
	uint32_t	RunningTime;		//跑步时长
	uint32_t 	RunningDistance;	//跑步里程
	uint8_t 	HeartMax;			//最大心率
}TriathlonDetailStr;

extern void gui_sport_triathlon_status_set(uint8_t status);
extern uint8_t gui_sport_triathlon_status_get(void);
extern void gui_sport_triathlon_data_get(void);

//Display
extern void gui_sport_triathlon_ready_paint(void);
extern void gui_sport_triathlon_swimming_paint(void);
extern void gui_sport_triathlon_first_change_paint(void);
extern void gui_sport_triathlon_cycling_ready_paint(void);
extern void gui_sport_triathlon_cycling_paint(void);
extern void gui_sport_triathlon_second_change_paint(void);
extern void gui_sport_triathlon_running_ready_paint(void);
extern void gui_sport_triathlon_running_paint(void);
extern void gui_sport_triathlon_finish_paint(uint8_t endangle);
extern void gui_sport_triathlon_pause_paint(void);
extern void gui_sport_triathlon_cancel_paint(void);
extern void gui_sport_triathlon_detail_paint(void);
extern void gui_sport_triathlon_detail_1(uint8_t x, uint8_t forecolor);
extern void gui_sport_triathlon_detail_1_paint(uint8_t backgroundcolor);
extern void gui_sport_triathlon_detail_2(uint8_t x, uint8_t forecolor);
extern void gui_sport_triathlon_detail_3(uint8_t x, uint8_t forecolor);
extern void gui_sport_triathlon_detail_7(uint8_t x, uint8_t forecolor);
extern void gui_sport_triathlon_detail_8(uint8_t x, uint8_t forecolor);
extern void gui_sport_triathlon_detail_4(uint8_t x, uint8_t forecolor);
extern void gui_sport_triathlon_detail_5(uint8_t x, uint8_t forecolor);
extern void gui_sport_triathlon_detail_6(uint8_t x, uint8_t forecolor);

//Key Event
extern void gui_sport_triathlon_ready_btn_evt(uint32_t Key_Value);
extern void gui_sport_triathlon_swimming_btn_evt(uint32_t Key_Value);
extern void gui_sport_triathlon_first_change_btn_evt(uint32_t Key_Value);
extern void gui_sport_triathlon_cycling_ready_btn_evt(uint32_t Key_Value);
extern void gui_sport_triathlon_cycling_btn_evt(uint32_t Key_Value);
extern void gui_sport_triathlon_second_change_btn_evt(uint32_t Key_Value);
extern void gui_sport_triathlon_running_ready_btn_evt(uint32_t Key_Value);
extern void gui_sport_triathlon_running_btn_evt(uint32_t Key_Value);
extern void gui_sport_triathlon_pause_btn_evt(uint32_t Key_Value);
extern void gui_sport_triathlon_cancel_btn_evt(uint32_t Key_Value);
extern void gui_sport_triathlon_detail_btn_evt(uint32_t Key_Value);
extern void gui_sport_triathlon_detail_track_paint(uint8_t backgroundcolor);
#endif
