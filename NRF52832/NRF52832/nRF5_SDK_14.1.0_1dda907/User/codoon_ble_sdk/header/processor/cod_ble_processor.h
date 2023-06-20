//
// Created by Paint on 2019/7/19.
//

#ifndef COD_SDK_COD_BLE_PROCESS_H
#define COD_SDK_COD_BLE_PROCESS_H

#include "../cod_ble_cmd.h"
#include "../cod_ble_api.h"


/**
 * 蓝牙协议处理
 * @param p_data 蓝牙底层上报的原始数据
 * @param data_len p_data指向数据的长度
 */
void cod_cmd_process(uint8_t *p_data, uint16_t data_len);



#endif //COD_SDK_COD_BLE_PROCESS_H
