#ifndef __TIME_NOTIFY_H__
#define __TIME_NOTIFY_H__



#include <stdint.h>
#include <stdbool.h>
#include "task_display.h"





enum
{
	NOTIFY_MODE_NORMAL = 0,//��������ģʽ��
	NOTIFY_MODE_SPORTS,//�˶�����ģʽ��
	NOTIFY_MODE_ALARM,//����ģʽ
	NOTIFY_MODE_SPECIFY_FUNC,//���⹦�ܲ�Ӱ��,��SOS��Ѱ�����
	NOTIFY_MODE_TOOLS,//�����๦��ʹ��ģʽ
	NOTIFY_MODE_SYSTEM_TIME_REACH,//ϵͳʱ�䵽/���
	NOTIFY_MODE_DIRECT_RESPONSE,//ֱ����Ӧ
	NOTIFY_MODE_MUTE,//����
	NOTIFY_MODE_PHONE_CALL,//��Ϣ����
};


void timer_notify_display_stop(bool is_isr);
void timer_notify_display_start(uint32_t timeout_msec, uint32_t repetition, bool is_isr);
void timer_notify_display_uninit(void);
void timer_notify_display_init(void);

void timer_notify_motor_stop(bool is_isr);
/**@brief ��ﶨʱ��.
	*@param interval  �������ʱ�䣬��λ����.
	*@param window    �����ʱ�䣬��λ����.
	*@param repetition ������ڴ�������λ����.
*/
void timer_notify_motor_start(uint32_t interval, uint32_t window, uint32_t repetition, bool is_isr, uint8_t type);
void timer_notify_motor_uninit(void);
void timer_notify_motor_init(void);

void timer_notify_buzzer_stop(bool is_isr);
void timer_notify_buzzer_start(uint32_t interval, uint32_t window, uint32_t repetition, bool is_isr, uint8_t type);
void timer_notify_buzzer_uninit(void);
void timer_notify_buzzer_init(void);

void timer_notify_remind_start(uint32_t interval, uint32_t window, uint32_t motor_interval, uint32_t motor_window, uint32_t buzzer_interval, uint32_t buzzer_window, uint32_t repetition, bool is_isr);
void timer_notify_remind_stop(bool is_isr);
void timer_notify_remind_uninit(void);
void timer_notify_remind_init(void);
void timer_notify_remind_motor_restart(void);
void timer_notify_remind_motor_stop(void);

void timer_notify_backlight_init(void);
void timer_notify_backlight_uninit(void);
void timer_notify_backlight_start(uint32_t timeout_msec, bool is_isr);
void timer_notify_backlight_stop(bool is_isr);
bool timer_notify_backlight_is_enable(void);

void timer_notify_keytone_start(void);

void timer_notify_rollback(ScreenState_t nowscreen, ScreenState_t destscreen);

uint8_t time_notify_is_ok(void);



#endif //__TIME_NOTIFY_H__
