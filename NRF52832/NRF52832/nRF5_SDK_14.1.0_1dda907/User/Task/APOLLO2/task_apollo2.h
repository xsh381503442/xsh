#ifndef __TASK_APOLLO2_H__
#define __TASK_APOLLO2_H__


#include "ble_gap.h"

#define UUID16_EXTRACT(DST, SRC) \
    do                           \
    {                            \
        (*(DST))   = (SRC)[1];   \
        (*(DST)) <<= 8;          \
        (*(DST))  |= (SRC)[0];   \
    } while (0)
		

typedef struct
{
    uint8_t  * p_data;      /**< Pointer to data. */
    uint16_t   data_len;    /**< Length of data. */
} data_t;

typedef void (*task_apollo2_ht_rssi_callback) (ble_gap_evt_adv_report_t const * p_adv_report);





void task_apollo2_init(void * p_context);


#define SOS_SUBCMD_START            1
#define SOS_SUBCMD_FINISH           2

typedef struct __attribute__ ((packed))
{
	uint8_t heartrate;
	uint8_t reserve[7];
	uint32_t longitude;
	uint32_t latitude;
	

}_sos_msg;

typedef struct __attribute__ ((packed))
{
	uint32_t cmd;
	uint8_t subcmd;
	_sos_msg msg;

}_sos_cmd;




#endif //__TASK_APOLLO2_H__
