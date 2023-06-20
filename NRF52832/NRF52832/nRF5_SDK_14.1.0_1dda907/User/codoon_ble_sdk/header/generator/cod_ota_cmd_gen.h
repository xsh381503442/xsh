//
// Created by Paint on 2019/7/22.
//

#ifndef COD_SDK_COD_OTA_CMD_GEN_H
#define COD_SDK_COD_OTA_CMD_GEN_H

#include <stdint.h>
#include <stdbool.h>
#include "../cod_ble_constants.h"

/**
 * 指令（resp）:文件传输(OTA)启动
 * @param device_mtu 设备单帧字节数.设备参考手机支持的最大字节数 ，评估自已可达到的最大字节数。
 * @param frame_num_per_group 设备根据自已的处理能力，一次可接受手机连续发送的帧数。如:发送帧数为10,即表示:
 *                            手机每次连续发送10帧后，随即发送组CRC 校验，再等待设备应答。
 * @param timeout 手机发送数据后，等待设备应答的时间。
 * @param file_type 文件类型
 */
void cod_cmd_ota_start_resp(uint16_t device_mtu, uint8_t frame_num_per_group,
                            uint8_t timeout, cod_ble_file_type file_type);

/**
 * 指令（resp）:文件传输(OTA)启动失败
 * @param reason 失败原因
 */
void cod_cmd_ota_start_failed_resp(cod_ble_ota_start_failed_reason reason);

/**
 * 指令（resp）:CRC组校验
 * @param is_success
 * @param data_type
 */
void cod_cmd_ota_group_crc_resp(bool is_success, uint8_t data_type);

/**
 * 指令（resp）:文件CRC校验
 * @param is_success
 * @param data_type
 */
void cod_cmd_ota_file_crc_resp(bool is_success, uint8_t data_type);


#endif //COD_SDK_COD_OTA_CMD_GEN_H
