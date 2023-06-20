#ifndef _BSP_TIMER_H__
#define _BSP_TIMER_H__

#include <stdint.h>

extern void SetupTimerB0( uint32_t	Sec);
extern void StopTimerB0( void );
extern void ResetTimerB0( uint32_t	Sec );

extern void SetupTimerB2( uint32_t	Sec);
extern void StopTimerB2( void );
extern void ResetTimerB2( uint32_t	Sec );

#endif

