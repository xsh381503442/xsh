#ifndef __TASK_BLE_H__
#define __TASK_BLE_H__


#include <stdint.h>
#include <stdbool.h>


#define BLE_CONNECT        1
#define BLE_DISCONNECT     2
#define BLE_BONDED         3

typedef struct {
	bool is_on;
	uint8_t is_conn;


}_bluetooth_status;
/**@brief GAP Passkey Length. */
#define BLE_GAP_PASSKEY_LEN 6
typedef struct
{
	uint32_t command;
	uint8_t passkey[BLE_GAP_PASSKEY_LEN];

}_ble_passkey;

void task_ble_init(void);
void task_ble_uninit(void);





#endif //__TASK_BLE_H__
