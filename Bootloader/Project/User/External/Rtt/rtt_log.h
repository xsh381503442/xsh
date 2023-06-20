#ifndef __RTT_LOG_H__
#define __RTT_LOG_H__






#include <stdint.h>
#include <stddef.h>
#include <ctype.h>

#ifndef LAYER_UPDATEALGORITHM 
#ifndef RTT_LOG_ENABLED
#define RTT_LOG_ENABLED 0
#endif
#endif
#ifdef LAYER_UPDATEALGORITHM 
#ifndef RTT_LOG_ENABLED
#define RTT_LOG_ENABLED 0
#endif
#endif


#define LOG_FLOAT_MARKER "%s%d.%02d"


#define LOG_FLOAT(val) (uint32_t)(((val) < 0 && (val) > -1.0) ? "-" : ""),   \
                           (int32_t)(val),                                       \
                           (int32_t)((((val) > 0) ? (val) - (int32_t)(val)       \
                                                : (int32_t)(val) - (val))*100)

uint32_t rtt_log_hexdump(const uint8_t *p_data, uint16_t len);





#endif //__RTT_LOG_H__
