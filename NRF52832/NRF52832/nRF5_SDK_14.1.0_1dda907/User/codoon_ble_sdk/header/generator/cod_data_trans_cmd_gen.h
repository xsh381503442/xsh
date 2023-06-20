//
// Created by Paint on 2019/7/22.
//

#ifndef COD_SDK_COD_DATA_TRANS_CMD_GEN_H
#define COD_SDK_COD_DATA_TRANS_CMD_GEN_H

#include <stdint.h>
#include <stdbool.h>
#include "../cod_ble_constants.h"
#include "../cod_ble_bean.h"

/**
 * 指令（resp）:获取数据帧数指令
 * @param type 数据类型
 * @param num 数据帧数
 * @param success 获取数据帧数是否成功
 */
void cod_cmd_get_data_frame_num_resp(cod_ble_data_type type, uint16_t num, bool success);

/**
 * 指令（resp）:删除数据指令
 * @param type 数据类型
 * @param success 删除是否成功
 */
void cod_cmd_delete_data_resp(cod_ble_data_type type, bool success);

/**
 * 指令（resp）:获取数据指令。每次连续发10帧数据。
 * @param type 数据类型
 * @param cur_frame_id 当前帧序号
 * @param last_frame 是否是10帧数据的最后一帧
 * @param data 数据数组
 * @param data_len 数据数组长度
 */
void cod_cmd_get_data_resp(cod_ble_data_type type, uint16_t cur_frame_id, bool last_frame,
        uint8_t* data, uint16_t data_len);

/**
 * 指令（resp）:实时当天步数，距离，卡路里数据传输指令。
 * @param step 步数
 * @param distance 米
 * @param calorie 大卡
 */
void cod_cmd_rt_today_summary_data_resp(uint32_t step, uint32_t distance, float calorie);

/**
 * 指令（resp）:心率，血压，血氧数据指令。没有采集到的某项则填0。
 * @param heart
 * @param sp 收缩压
 * @param dp 舒张压
 * @param bo 血氧
 */
void cod_cmd_rt_heart_bp_bo_resp(uint8_t heart, uint8_t sp, uint8_t dp, uint8_t bo);

/**
 * 指令（resp）:查询硬件具备的Sensor能力
 * @param sensor_cap sensor能力数组
 * @param cap_num sensor能力数组长度
 */
void cod_cmd_get_sensor_capability_resp(cod_ble_sensor_cap* sensor_cap, uint8_t cap_num);

/**
 * 指令（resp）:传感器数据传输指令
 * @param data 传感器数据
 * @param data_len 传感器数据长度
 */
void cod_cmd_sensor_data_trans_resp(uint8_t* data, uint16_t data_len);

/**
 * 指令（resp）:心率原始数据传输指令
 * @param data 心率原始数据
 * @param data_len 心率原始数据长度
 */
void cod_cmd_rt_heart_log_resp(uint8_t *data, uint8_t data_len);


#endif //COD_SDK_COD_DATA_TRANS_CMD_GEN_H
