#ifndef __GUI_NOTIFY_SPORT_H__
#define __GUI_NOTIFY_SPORT_H__




#include <stdint.h>

//�Զ���Ȧ��һȦ����
typedef struct {
	uint32_t time;		//��ʱ
	uint16_t lap;		//Ȧ��
	uint16_t pace;		//���١��ٶ�
	uint16_t frequency;	//��Ƶ
	uint16_t riseheight;//�����߶�
	uint16_t dropheight;//�½��߶�	
	uint8_t  heart;		//����
} last_circle_t;

extern void gui_notify_sport_circle_paint(uint32_t circle);
extern void gui_notify_sport_time_paint(uint32_t time);
extern void gui_notify_sport_distance_paint(uint32_t distance);
extern void gui_notify_sport_pace_paint(uint32_t pace);
extern void gui_notify_sport_height_paint(uint32_t height);
extern void gui_notify_sport_speed_paint(uint32_t speed);
extern void gui_notify_sport_calory_paint(uint32_t speed);
extern void gui_notify_sport_atuocircle_paint(void);

extern void gui_notify_sport_circle_btn_evt(uint32_t Key_Value);
extern void gui_notify_sport_time_btn_evt(uint32_t Key_Value);
extern void gui_notify_sport_distance_btn_evt(uint32_t Key_Value);
extern void gui_notify_sport_pace_btn_evt(uint32_t Key_Value);
extern void gui_notify_sport_height_btn_evt(uint32_t Key_Value);
extern void gui_notify_sport_speed_btn_evt(uint32_t Key_Value);
extern void gui_notify_sport_calory_btn_evt(uint32_t Key_Value);
extern void gui_notify_sport_autocircle_btn_evt(uint32_t Key_Value);
#endif 
