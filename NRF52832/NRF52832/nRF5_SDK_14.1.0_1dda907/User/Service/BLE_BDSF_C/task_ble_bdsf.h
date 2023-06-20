#ifndef TASK_BLE_BDSF_H__
#define TASK_BLE_BDSF_H__

#include <stdint.h>
#include "ble_gap.h"

#define TASK_BLE_BDSF_EVT_SCAN_ENABLE                      1
#define TASK_BLE_BDSF_EVT_NAME_MATCH                       2
#define TASK_BLE_BDSF_EVT_DEVICE_NAME_HANDLE               3
#define TASK_BLE_BDSF_EVT_DEVICE_NAME_SEND              	 4
#define TASK_BLE_BDSF_EVT_BD_SRV_DISC											 5
#define TASK_BLE_BDSF_EVT_DATA_NOTIFY_SEND              	 6
#define TASK_BLE_BDSF_EVT_SCAN_DISABLE                     7
#define TASK_BLE_BDSF_EVT_WRITE_COMMAND										 8
#define TASK_BLE_BDSF_EVT_SCAN_AGAIN                       9
#define TASK_BLE_BDSF_EVT_RECONNECT_LAST_DEVICE            10
//#define TASK_BLE_BDSF_EVT_DATA_TIMER_START                8
//#define TASK_BLE_BDSF_EVT_DATA_TIMER_STOP                 9

//#define BDSF_DEVICE_NAME_MAX_LEN	7

typedef struct __attribute__ ((packed))
{
	uint32_t command;
	uint8_t	 len;
	uint8_t	 *dev_name;
}ble_bdsf_dev_name;

typedef struct __attribute__ ((packed))
{
  uint8_t addr_id_peer : 1;       /**< Only valid for peer addresses.
                                       Reference to peer in device identities list (as set with @ref sd_ble_gap_device_identities_set) when peer is using privacy. */
  uint8_t addr_type    : 7;       /**< See @ref BLE_GAP_ADDR_TYPES. */
  uint8_t addr[BLE_GAP_ADDR_LEN]; /**< 48-bit address, LSB format. */
} ble_gap_addr_m;

typedef struct __attribute__ ((packed)) {
	uint8_t rssi;
	uint8_t dev_name[8];
	ble_gap_addr_m peer_addr;
}ble_bdsf_name_peer_addr;

extern uint8_t SENDING_DEV_NAME;

#define MAX_DEV_NUM			6

extern ble_bdsf_name_peer_addr DEVICE_ADV_LIST[MAX_DEV_NUM];

extern uint8_t VALID_DEVICE_LIST;

typedef enum{
	BD_FKI_E,
	RNSS_GNRMC_E,
	RNSS_BDRMC_E,
	RNSS_GNGGA_E,
	RNSS_BDGGA_E,
	RDSS_DATA_E,
	RESCUE_CENTER_INFO_E,
	DEVICE_INFO_E,
	DEV_BDTXR_E,
	VERSION_INFO_E,
	LOC_STRATEGY_REQUEST_E,
	LOC_STRATEGY_CHANGE_E,
	LOC_SEND_TIMES_E,
	LOW_POWER_INF_E
}MESSAGE_RECV_TYPE;




extern void task_ble_bdsf_init(void * p_context);
extern void task_ble_bdsf_handle(uint32_t evt, uint8_t *p_data, uint32_t length);



#endif 			//TASK_BLE_BDSF_H__
