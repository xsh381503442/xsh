#ifndef __TIME_NOTIFY_H__
#define __TIME_NOTIFY_H__



#include <stdint.h>
#include <stdbool.h>
#include "task_display.h"





enum
{
	NOTIFY_MODE_NORMAL = 0,//待机正常模式下
	NOTIFY_MODE_SPORTS,//运动功能模式下
	NOTIFY_MODE_ALARM,//闹钟模式
	NOTIFY_MODE_SPECIFY_FUNC,//特殊功能不影响,如SOS、寻找腕表
	NOTIFY_MODE_TOOLS,//工具类功能使用模式
	NOTIFY_MODE_SYSTEM_TIME_REACH,//系统时间到/完成
	NOTIFY_MODE_DIRECT_RESPONSE,//直接响应
	NOTIFY_MODE_MUTE,//静音
	NOTIFY_MODE_PHONE_CALL,//消息提醒
};


void timer_notify_display_stop(bool is_isr);
void timer_notify_display_start(uint32_t timeout_msec, uint32_t repetition, bool is_isr);
void timer_notify_display_uninit(void);
void timer_notify_display_init(void);

void timer_notify_motor_stop(bool is_isr);
/**@brief 马达定时器.
	*@param interval  马达周期时间，单位毫秒.
	*@param window    马达震动时间，单位毫秒.
	*@param repetition 马达周期次数，单位次数.
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
