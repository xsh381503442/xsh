#ifndef __BOARD_H__
#define __BOARD_H__


/*
*
* 不同的PCB选择不同的BSP.H文件，IO口的宏定义名与JW901_BSP.h保持一致
* 如果函数需要提取可以在不同项目的BSP.C中提取
*/

#if defined(YZL)
  #include "YZL_BSP.h"
#else
#error "Board is not defined"
#endif













#endif
