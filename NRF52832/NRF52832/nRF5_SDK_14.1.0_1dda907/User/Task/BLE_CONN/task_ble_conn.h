#ifndef __TASK_BLE_CONN_H__
#define __TASK_BLE_CONN_H__



#include <stdint.h>
#include "watch_config.h"



#define DISCOVERY_ARRAY_SIZE         4      //NRF_SDH_BLE_CENTRAL_LINK_COUNT+1(ANCS)
#define DISCONVER_ARRAY_ANCS         0
#define DISCONVER_ARRAY_HEARTRATE    1
#define DISCONVER_ARRAY_RUNNING      2
#define DISCONVER_ARRAY_CYCLING      3




#define TASK_BLE_CONN_EVT_CONNECT                  1
#define TASK_BLE_CONN_EVT_DISCONNECT               2
#define TASK_BLE_CONN_EVT_BOND                     3
#define TASK_BLE_CONN_EVT_PASSKEY                  4
#define TASK_BLE_CONN_EVT_BLE_UART                 5
#define TASK_BLE_CONN_EVT_ANCS_DISC_COMPLETE       6
#define TASK_BLE_CONN_EVT_ANCS_DISC_FAIL           7
#define TASK_BLE_CONN_EVT_BOND_ANDROID             8
#define TASK_BLE_CONN_PARAM_UPDATE                 9
#define TASK_BLE_CONN_HWT_RSSI_READ                10
#define TASK_BLE_CONN_DEV_NAME_UPDATE              11
#define TASK_BLE_CONN_ANCS_ATTR_REQUEST            12
#define TASK_BLE_CONN_ANCS_ATTR_ID_APP_IDENTIFY    13
#define TASK_BLE_CONN_ANCS_MESSAGE_SEND            14
#define TASK_BLE_CONN_WECHAT_SPORT_DATA_REQUEST    15
#define TASK_BLE_CONN_EVT_CONNECT_HEARTRATE        16
#define TASK_BLE_CONN_EVT_CONNECT_RUNNING          17
#define TASK_BLE_CONN_EVT_CONNECT_CYCLING          18
#define TASK_BLE_CONN_EVT_DISCONNECT_HEARTRATE     19
#define TASK_BLE_CONN_EVT_DISCONNECT_RUNNING       20
#define TASK_BLE_CONN_EVT_DISCONNECT_CYCLING       21
#define TASK_BLE_CONN_EVT_ACCESSORY_DATA_UPLOAD    22

#define TASK_BLE_CONN_EVT_PAIRING_FAILED        	25

#define TASK_BLE_CONN_DISCONNECT_REASON_WRONG_PASSKEY             1


void task_ble_conn_init(void * p_context);
void task_ble_conn_handle(uint32_t evt, uint8_t *p_data, uint32_t length);


#endif //__TASK_BLE_CONN_H__
