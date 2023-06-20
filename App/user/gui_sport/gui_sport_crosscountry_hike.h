#ifndef __GUI_SPORT_CROSSCOUNTRY_HIKE_H
#define	__GUI_SPORT_CROSSCOUNTRY_HIKE_H

#include <stdint.h>
#include "com_sport.h"
#include "gui_sport.h"


#define HIKE_DATA_DISPLAY_SUM		8		//������Ŀͽ��ԽҰ������ʾѡ����
#define HIKE_REMIND_SET_SUM			6		//ͽ��ԽҰ��������ѡ����



/*ͽ��ԽҰ������ʾ��ǰ��*/
extern uint8_t m_hike_data_display_buf[SPORT_DATA_DISPLAY_SUM];
/*ͽ��ԽҰ������ʾѡ��*/
extern const uint8_t m_hike_data_display_option[HIKE_DATA_DISPLAY_SUM];

/*ͽ��ԽҰ��������ѡ��*/
extern const uint8_t m_hike_remind_set_option[HIKE_REMIND_SET_SUM];

extern uint8_t	vice_screen_walk_display[SPORT_DATA_DISPLAY_SUM];



extern void gui_sport_crosscountry_hike_init(void);

//Display
extern void gui_sport_hike_paint(void);
extern void gui_sport_hike_pause_paint(void);
extern void gui_sport_hike_cancel_paint(void);
extern void gui_sport_hike_detail_paint(void);

//Key Event
extern void gui_sport_hike_btn_evt(uint32_t Key_Value);
extern void gui_sport_hike_pause_btn_evt(uint32_t Key_Value);
extern void gui_sport_hike_cancel_btn_evt(uint32_t Key_Value);
extern void gui_sport_hike_detail_btn_evt(uint32_t Key_Value);



#if defined WATCH_SPORT_NEW_CLOUD_NAVIGATION
extern void gui_sport_event_crosscountry_hike_detail_1_paint(ActivityDataStr m_ActivityData
																										 ,SportScheduleCalStr m_sport_str
																										 ,uint8_t backgroundcolor);
extern void gui_sport_event_crosscountry_hike_detail_3_paint(ActivityDataStr m_ActivityData
																										 ,SportScheduleCalStr m_sport_str
																										 ,uint8_t backgroundcolor);

//extern void gui_sport_event_hike_three_grid_paint(void);
//extern void gui_sport_event_hike_four_grid_paint(void);
//�ӹ���GPS�н�����Ƽ����� ͽ��ԽҰ��ͣ�˵����水���¼�
extern void gui_sport_cloud_navigation_hike_pause_btn_evt(uint32_t Key_Value);
extern void gui_sport_hike_track_paint(void);



#endif
#endif
