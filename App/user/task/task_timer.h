#ifndef _TASK_TIMER_H__
#define _TASK_TIMER_H__
#include <stdint.h>

extern void CTimer1TickHandler(void );
extern void CTimerLightHandler( void );
extern void CTimerB0TickHandler(void);
extern void Send_Timer_Cmd(uint32_t cmd);
extern void CTimerB2TickHandler(void);

extern void CreatTimerTask(void);
void TaskTimer(void* pvParameter);

enum
{
	CMD_DEFAULT = 0,    
	CMD_HOME_STEP,		//�������沽������
	CMD_AMBIENT_DATA,	//����������ݸ���
	CMD_MOTION_TRAIL_LOAD, //�˶��켣����
	CMD_NAVIGATION_LOAD,    //�Ƽ���������
	CMD_MINUTE_DATA,        //ÿ�������ݸ���
	CMD_SPORTS_TRAIL_LOAD,//�˶���¼���˶��켣����
};

typedef struct{
	uint32_t	cmd;
	uint32_t	value;
} TIMER_MSG;

#endif
