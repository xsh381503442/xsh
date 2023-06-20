//
// Created by cj on 2019/7/22.
//

#include <stdio.h>
#include <time.h>

#ifndef COD_SDK_COD_BLE_LOG_H
#define COD_SDK_COD_BLE_LOG_H


//TODO log需要支持格式化打印。不管是通过传递变长数据，还是动态字符串buffer
/**
 * 打印日志的等级分布
 * */
typedef enum
{
    LOG_VERBOSE = 0,
    LOG_DEBUG  = 1,
    LOG_INFO  = 2,
    LOG_WARNING = 3,
    LOG_ERROR = 4
} LogLevel;

static char* s_log_info[] = {"VERBOSE",
                                   "DEBUG", "INFO","WARNING", "ERROR",
};



#ifndef _LOG_LEVEL_
#define _LOG_LEVEL_ LOG_INFO //如果没有设置 默认是LOG_INFO级别的日志
#endif


void print_log(uint8_t level, char* tag, char* content);

char* get_bytes_str(uint8_t *data, uint16_t len);

void print_bytes(char* tag, uint8_t *data, uint16_t len);

#ifdef COD_LOG_ENABLE
#define clogv(tag, format, ...) _clogf(LOG_VERBOSE, tag, format, ##__VA_ARGS__)
#define clogd(tag, format, ...) _clogf(LOG_DEBUG, tag, format, ##__VA_ARGS__)
#define clogi(tag, format, ...) _clogf(LOG_INFO, tag, format, ##__VA_ARGS__)
#define clogw(tag, format, ...) _clogf(LOG_WARNING, tag, format, ##__VA_ARGS__)
#define cloge(tag, format, ...) _clogf(LOG_ERROR, tag, format, ##__VA_ARGS__)
#else
#define clogv(tag, format, ...)
#define clogd(tag, format, ...)
#define clogi(tag, format, ...)
#define clogw(tag, format, ...)
#define cloge(tag, format, ...)
#endif

void _clogf(uint8_t level, char *tag, char *format, ...);


#ifdef COD_SDK_TEST

//#define PRINT_LOG(level,tag,content)\
//        char* ll = s_log_info[level];\
//        if (level == LOG_VERBOSE || level == LOG_DEBUG){\
//            printf("\033[37m [%s %s] [%s] [%s]：%s \033[0m\n",__DATE__,__TIME__,ll,tag,content);\
//        }else if(level == LOG_INFO){\
//            printf("\033[32m [%s %s] [%s] [%s]：%s \033[0m\n",__DATE__,__TIME__,ll,tag,content);\
//        }else{\
//            printf("\033[31m [%s %s] [%s] [%s]：%s \033[0m\n",__DATE__,__TIME__,ll,tag,content);\
//        }
//#define PRINT_LOG(level,tag,content)\
//				char* ll = s_log_info[level];\
//				if (level == LOG_VERBOSE || level == LOG_DEBUG){\
//						printf("[%s %s] [%s] [%s]: %s \n",__DATE__,__TIME__,ll,tag,content);\
//				}else if(level == LOG_INFO){\
//						printf("[%s %s] [%s] [%s]: %s \n",__DATE__,__TIME__,ll,tag,content);\
//				}else{\
//						printf("[%s %s] [%s] [%s]: %s \n",__DATE__,__TIME__,ll,tag,content);\
//				}
//short log
#define PRINT_LOG(level,tag,content)\
				char* ll = s_log_info[level];\
				if (level == LOG_VERBOSE || level == LOG_DEBUG){\
						printf("[%s]: %s \n",tag,content);\
				}else if(level == LOG_INFO){\
						printf("[%s]: %s \n",tag,content);\
				}else{\
						printf("[%s]: %s \n",tag,content);\
				}
#else

//#define PRINT_LOG(level,tag,content)\
//				char* ll = s_log_info[level];\
//				if (level == LOG_VERBOSE || level == LOG_DEBUG){\
//						printf("[%s %s] [%s] [%s]: %s \n",__DATE__,__TIME__,ll,tag,content);\
//				}else if(level == LOG_INFO){\
//						printf("[%s %s] [%s] [%s]: %s \n",__DATE__,__TIME__,ll,tag,content);\
//				}else{\
//						printf("[%s %s] [%s] [%s]: %s \n",__DATE__,__TIME__,ll,tag,content);\
//				}
#define PRINT_LOG(level,tag,content) 
#endif





#endif //COD_SDK_COD_BLE_LOG_H
