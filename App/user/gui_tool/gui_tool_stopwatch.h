#ifndef	GUI_TOOL_STOPWATCH_H
#define	GUI_TOOL_STOPWATCH_H

#define	TOOL_STOPWATCH_TIME_MAX		(60000*100)	//最大计时时间100分钟

#define	TOOL_STOPWATCH_COUNT_MAX	8
#define	TOOL_STOPWATCH_LAP_MAX		8
#define	TOOL_STOPWATCH_RECORD_MAX	(TOOL_STOPWATCH_COUNT_MAX + TOOL_STOPWATCH_LAP_MAX)

enum
{
	TOOL_STOPWATCH_STATUS_RESET	= 0U,	//复位状态
	TOOL_STOPWATCH_STATUS_START,		//开始计时状态
	TOOL_STOPWATCH_STATUS_STOP,			//暂停状态
	TOOL_STOPWATCH_STATUS_OVERTIME,		//超时状态
};

typedef struct 
{
	uint8_t 	type;	//1-计次  2-计圈
	uint8_t 	num;	//序号
	uint8_t 	min;	//分
	uint8_t 	sec;	//秒
	uint8_t 	ms;		//毫秒
}Stopwatch_Record_t;

typedef struct
{
	uint32_t	time;
	uint32_t	last_time;
	uint8_t 	status;
	uint8_t		count;
	uint8_t 	lap;
	uint8_t 	page;
	Stopwatch_Record_t	record[TOOL_STOPWATCH_RECORD_MAX];
}tool_Stopwatch_t;

extern uint8_t tool_stopwatch_status_get(void);
extern uint32_t tool_stopwatch_time_get(void);
extern void tool_stopwatch_overtime(void);
extern void gui_tool_stopwatch_init(void);
extern void gui_tool_stopwatch_menu_init(void);

//Display
extern void gui_tool_stopwatch_paint(void);
extern uint8_t gui_tool_stopwatch_time(uint8_t color);
extern void gui_tool_stopwatch_record(void);
extern void gui_tool_stopwatch_button(void);
extern void gui_tool_stopwatch_menu_paint(void);
extern void gui_tool_stopwatch_record_paint(void);

//Key Event
extern void gui_tool_stopwatch_btn_evt(uint32_t Key_Value);
extern void gui_tool_stopwatch_menu_btn_evt(uint32_t Key_Value);
extern void gui_tool_stopwatch_record_btn_evt(uint32_t Key_Value);

#endif
