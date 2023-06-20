#ifndef TASK_KEY_H
#define	TASK_KEY_H

#include <stdint.h>
#include <stdbool.h>
#include "watch_config.h"
void TaskKey( void *pvParameters );
void KeyEventHandler( uint32_t Key_Value );
extern void CreateKeyTask(void);


#endif
