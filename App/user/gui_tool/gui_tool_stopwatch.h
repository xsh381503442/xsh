#ifndef	GUI_TOOL_STOPWATCH_H
#define	GUI_TOOL_STOPWATCH_H

#define	TOOL_STOPWATCH_TIME_MAX		(60000*100)	//����ʱʱ��100����

#define	TOOL_STOPWATCH_COUNT_MAX	8
#define	TOOL_STOPWATCH_LAP_MAX		8
#define	TOOL_STOPWATCH_RECORD_MAX	(TOOL_STOPWATCH_COUNT_MAX + TOOL_STOPWATCH_LAP_MAX)

enum
{
	TOOL_STOPWATCH_STATUS_RESET	= 0U,	//��λ״̬
	TOOL_STOPWATCH_STATUS_START,		//��ʼ��ʱ״̬
	TOOL_STOPWATCH_STATUS_STOP,			//��ͣ״̬
	TOOL_STOPWATCH_STATUS_OVERTIME,		//��ʱ״̬
};

typedef struct 
{
	uint8_t 	type;	//1-�ƴ�  2-��Ȧ
	uint8_t 	num;	//���
	uint8_t 	min;	//��
	uint8_t 	sec;	//��
	uint8_t 	ms;		//����
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
