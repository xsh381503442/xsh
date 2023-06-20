#ifndef GUI_HOME_H
#define	GUI_HOME_H
#include "task_display.h"

enum
{
	IMG_POINT_LEFT = 0U,//���ͷ
	IMG_POINT_RIGHT,    //�Ҽ�ͷ
	IMG_POINT_UP,       //�ϼ�ͷ
	IMG_POINT_DOWN,     //�¼�ͷ
};
enum
{
	INDEX_VO2MAX = 0U,//�����Ӱ��
	INDEX_MOTION_RECORD,//�˶���¼
	INDEX_NOTIFICATION,
	INDEX_CALORIES,
	INDEX_LACTATE_THRESHOLD,
	INDEX_TRAIN_PLAN,
	INDEX_RECOVERY,
};
typedef enum
{
	BACK_HOME_SCREEN = 0,//BCAK������������
	MONITOR_HOME_SCREEN //�����Ŀ�л���������
	
}SWITCH_HOME_SCREEN_T;
typedef enum
{
	HOME_SCREEN_SWITCH = 0,//��������������
	MINOR_SCREEN_SWITCH //�������л�
	
}HOME_OR_MINOR_SCREEN_T;

extern void Set_Select_Home_Index(void);
extern void set_home_index_up(void);
extern void set_home_index_down(void);
extern ScreenState_t Get_Curr_Home_Index(void);
extern void Set_Curr_Home_Num(uint8_t Num);
extern void set_select_monitor_index(void);
extern uint8_t is_custom_monitor(ScreenState_t index);

extern void gui_home_lock_hint_paint(void);
extern void gui_point_direction(uint8_t direction,uint8_t color);
extern void gui_home_btn_evt(uint32_t Key_Value);
extern void gui_home_lock_hint_btn_evt(uint32_t Key_Value);
extern ScreenState_t get_current_monitor_screen(void);
extern ScreenState_t get_current_switch_screen(uint32_t index);
extern uint16_t get_switch_screen_count(void);
extern bool is_switch_shortcut_screen(ScreenState_t m_screen);
extern void set_switch_screen_count_evt(uint32_t Key_Value);
extern ScreenState_t get_current_longtool_screen(void);

#endif

