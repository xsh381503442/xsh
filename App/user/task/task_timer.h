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
	CMD_HOME_STEP,		//待机界面步数更新
	CMD_AMBIENT_DATA,	//环境监测数据更新
	CMD_MOTION_TRAIL_LOAD, //运动轨迹加载
	CMD_NAVIGATION_LOAD,    //云迹导航加载
	CMD_MINUTE_DATA,        //每分钟数据更新
	CMD_SPORTS_TRAIL_LOAD,//运动记录中运动轨迹加载
};

typedef struct{
	uint32_t	cmd;
	uint32_t	value;
} TIMER_MSG;

#endif
