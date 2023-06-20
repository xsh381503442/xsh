//
// Created by Paint on 2019/7/22.
// 基础模块 指令组装
//

#ifndef COD_SDK_COD_BASIC_CMD_GEN_H
#define COD_SDK_COD_BASIC_CMD_GEN_H

#include <stdint.h>
#include <stdbool.h>
#include "../cod_ble_constants.h"
#include "../cod_ble_bean.h"

/**
 * 指令(resp)：连接测试
 * @param read_max_b 设备最大可读单帧字节数
 * @param write_max_b 设备最大可写单帧字节数
 * */
void cod_cmd_connect_test_resp(cod_mtu_size read_max_s, cod_mtu_size write_max_s);

/**
 * 指令(resp)：读取设备软硬件版本
 * 如硬件版本v2.11 则硬件版本 高位为 2，低位为 11
 * @param hard_v_h 硬件版本 高位
 * @param hard_v_l 硬件版本 低位
 * @param soft_v_h 软件版本 高位
 * @param soft_v_l 软件版本 低位
 * */
void cod_cmd_device_version_resp(uint8_t hard_v_h, uint8_t hard_v_l,
                                 uint8_t soft_v_h, uint8_t soft_v_l);

/**
 * 指令(resp)：设备的电量
 * @param ele 设备当前电量(0 - 100)
 * @param ele_status 当前状态 STATUS_ELE_NOT_CHARGED,STATUS_ELE_CHARGED,STATUS_ELE_CHARGING
 * */
void cod_cmd_device_ele_resp(uint8_t ele, cod_battery_status ele_status);

/**
 * 指令(resp)：设备解绑
 * @param is_success 解绑成功 true ;解绑失败 false
 * */
void cod_cmd_unbind_device_resp(bool is_success);


/**
 * 指令(resp)：检测发送mtu
 * @param read_b 硬件读取到的字节数
 * */
void cod_cmd_tx_mtu_test_resp(uint16_t read_b);

/**
 * 指令(resp)：检测接受mtu
 * @param write_b 需要写入的字节数支持
 * */
void cod_cmd_rx_mtu_test_resp(uint16_t write_b);

/**
 * 指令(resp)：获取设备mac地址
 * @param device_type cod定义的，每款设备有自己对应的一个product_type
 * @param mac_address 硬件的mac地址
 * */
void cod_cmd_device_mac_resp(uint16_t product_type, cod_ble_mac_address *mac_address);

/**
 * 指令(resp)：获取星历状态
 * @param has_agps 有无星历
 * @param update_time 更新时间，单位：小时（距离当前的时间）
 * */
void cod_cmd_agps_status_resp(bool has_agps, uint16_t update_time);

/**
 * 指令(resp)：同步IMEI和IMSI
 * @param imei 15-17位长 包含'\0'
 * @param imsi 15位长 包含'\0'
 * */
void cod_cmd_sync_imei_imsi_resp(char *imei, uint8_t imei_len, char *imsi, uint8_t imsi_len);

/**
 * 指令(resp)：设置iOS ANCS开关
 * */
void cod_cmd_ios_ancs_resp(void);

/**
 * 指令(resp)：设置设备mac地址
 *
 * */
void cod_cmd_set_device_mac_resp(void);

/**
 * 指令(resp)：设备绑定
 * @param is_success 绑定成功 true ；绑定失败false
 * */
void cod_cmd_bind_device_resp(bool is_success);

/**
 * 指令（resp）:查询设备绑定关系
 * @param is_bind 是否绑定 true 已绑定；false 未绑定
 * @param platform 安卓/苹果，若未绑定，可以传任意值
 * @param user_nick_len 昵称长度  包含'\0'
 * @param user_nick 用户昵称  包含'\0'，UTF-16小端编码
 * @param user_id_len 用户ID长度  包含'\0'
 * @param user_id 用户ID 包含'\0'，ASCII编码
 * */
void cod_cmd_check_bind_resp(bool is_bind, cod_app_platform platform,
                             uint8_t user_nick_len, char *user_nick,
                             uint8_t user_id_len, char *user_id);

/**
 * 指令(req)：硬件主动同步IMEI和IMSI到手机
 * @param imei 15-17位长 包含'\0'
 * @param imei_len 包含'\0'
 * @param imsi 15位长 包含'\0'
 * @param imsi_len 包含'\0'
 * */
void cod_cmd_sync_imei_imsi(char *imei, uint8_t imei_len, char *imsi, uint8_t imsi_len);

/**
 * 指令(resp)：发送绑定状态到device
 * */
void cod_cmd_send_bind_status_resp(void);


//------------------------------------------
/**
 * 指令（resp）:时间设置
 * @param success 设置是否成功。比如时间格式错误，则回复失败。
 */
void cod_cmd_set_time_resp(bool success);

/**
 * 指令（resp）:时间读取
 * @param time
 */
void cod_cmd_get_time_resp(cod_ble_time* time);

/**
 * 指令（resp）:闹钟设置
 * @param success 设置是否成功。比如闹钟格式错误，则回复失败。
 */
void cod_cmd_set_alarm_resp(bool success);

/**
 * 指令（resp）:闹钟读取
 * @param data 如果之前硬件存的数据是通过cod_alarm_list2bytes转化的，那么硬件直接将这个数据传过来
 * @param len 数据长度
 */
void cod_cmd_get_alarm_resp(uint8_t* data, uint16_t len);

/**
 * 指令（resp）:用户信息设置
 */
void cod_cmd_set_user_info_resp(void);

/**
 * 指令（resp）:用户信息读取
 * @param data 如果之前硬件存的数据是通过cod_alarm_list2bytes转化的，那么硬件直接将这个数据传过来
 * @param len 数据长度
 */
void cod_cmd_get_user_info_resp(cod_ble_user_info* userInfo);


/**
 * 指令（resp）:久坐提醒设置
 * @param success 设置是否成功。比如久坐提醒格式错误，则回复失败。
 */
void cod_cmd_set_sit_remind_resp(bool success);

/**
 * 指令（resp）:久坐提醒读取
 * @param data 如果之前硬件存的数据是通过cod_sit_remind_list2bytes转化的，那么硬件直接将这个数据传过来
 * @param len 数据长度
 */
void cod_cmd_get_sit_remind_resp(uint8_t* data, uint16_t len);

/**
 * 指令（resp）:睡眠时间段设置
 */
void cod_cmd_set_sleep_setting_resp(void);

/**
 * 指令（resp）:睡眠时间段读取
 */
void cod_cmd_get_sleep_setting_resp(cod_ble_sleep_setting* sleep_setting);

/**
 * 指令（resp）:喝水提醒设置
 * @param success 设置是否成功。比如喝水提醒格式错误，则回复失败。
 */
void cod_cmd_set_drink_remind_resp(bool success);

/**
 * 指令（resp）:喝水提醒读取
 * @param data 如果之前硬件存的数据是通过cod_drink_remind_list2bytes转化的，那么硬件直接将这个数据传过来
 * @param len 数据长度
 */
void cod_cmd_get_drink_remind_resp(uint8_t* data, uint16_t len);

/**
 * 指令（resp）:消息推送设置
 */
void cod_cmd_set_msg_push_setting_resp(void);

/**
 * 指令（resp）:消息推送设置读取
 * @param data 如果之前硬件存的数据是通过cod_msg_push_setting2bytes转化的，那么硬件直接将这个数据传过来
 * @param len 数据长度
 */
void cod_cmd_get_msg_push_setting_resp(uint8_t* data, uint16_t len);

/**
 * 指令（resp）:表盘设置
 */
void cod_cmd_set_dial_plate_resp(void);

/**
 * 指令（resp）:表盘读取
 */
void cod_cmd_get_dial_plate_resp(uint8_t dial_plate_index);

/**
 * 指令（resp）:独⽴立运动，自动暂停设置
 */
void cod_cmd_set_auto_pause_resp(void);

/**
 * 指令（resp）:独⽴立运动，自动暂停读取
 */
void cod_cmd_get_auto_pause_resp(bool auto_pause);

/**
 * 指令（resp）:独⽴立运动，运动中提醒设置
 */
void cod_cmd_set_sport_notify_setting_resp(void);

/**
 * 指令（resp）:独⽴立运动，运动中提醒设置读取
 * @param data 如果之前硬件存的数据是通过cod_sport_notify_setting2bytes转化的，那么硬件直接将这个数据传过来
 * @param len 数据长度
 */
void cod_cmd_get_sport_notify_setting_resp(uint8_t* data, uint16_t len);

/**
 * 指令（resp）:运动键功能设置
 */
void cod_cmd_set_sport_btn_func_resp(void);

/**
 * 指令（resp）:运动键功能读取
 */
void cod_cmd_get_sport_btn_func_resp(cod_ble_sport_btn_func sport_btn_func);

/**
 * 指令（resp）:同步咕咚计步目标
 */
void cod_cmd_set_step_target_resp(void);
/**
 * 指令（resp）:抬腕亮屏设置
 */
void cod_cmd_set_raise_to_wake_resp(void);
/**
 * 指令（resp）:全天心率检测开关设置
 */
void cod_cmd_set_all_day_heart_check_resp(void);
/**
 * 指令（resp）:自动记录运动开关设置
 */
void cod_cmd_set_auto_record_sport_resp(void);
/**
 * 指令（resp）:公制和英制的设置
 */
void cod_cmd_set_distance_unit_resp(void);
/**
 * 指令（resp）:独立运动心率检测开关设置
 */
void cod_cmd_set_heart_check_in_standalone_sport_resp(void);

/**
 * 指令（resp）:语言设置
 */
void cod_cmd_set_language_resp(void);

/**
 * 指令（resp）:获取当前语言
 */
void cod_cmd_get_cur_language_resp(cod_ble_language language);

/**
 * 指令（resp）:获取支持的语言
 * @param language_array 语言数组
 * @param num 语言个数
 */
void cod_cmd_get_support_language_resp(cod_ble_language* language_array, uint8_t num);

/**
 * 指令（resp）:设置地理信息
 */
void cod_cmd_set_location_info_resp(void);

/**
 * 指令（resp）:设置紧急联系人
 */
void cod_cmd_set_emergency_contacts_resp(void);

/**
 * 指令（resp）:设置安静心率
 */
void cod_cmd_set_rest_heart_rate_resp(void);

/**
 * 指令（resp）:设置客户端信息
 */
void cod_cmd_set_app_info_resp(void);

/**
 * 指令（resp）:开关硬件日志记录
 */
void cod_cmd_set_log_setting_resp(void);

//----------------------------
/**
 * 请求（req）:设备控制运动或者训练的暂停、恢复、停止
 * @param ctrl
 */
void cod_cmd_sport_ctrl_req(cod_ble_sport_ctrl ctrl);

/**
 * 指令（resp）:设置极限心率
 */
void cod_cmd_set_limit_heart_rate_resp(void);

/**
 * 指令（resp）:同步手机来电状态
 */
void cod_cmd_sync_phone_call_status_resp(void);

/**
 * 请求（req）:设备控制主端来电
 * @param ctrl
 */
void cod_cmd_phone_call_ctrl_req(cod_ble_phone_call_ctrl ctrl);

/**
 * 指令（resp）:设置设备端来电震动延迟
 */
void cod_cmd_set_phone_call_virb_delay_resp(void);

/**
 * 请求（req）:主动触发SOS
 * @param latitude 将浮点数保留留⼩小数点后⾯面7位. 未获取到位置信息则填0f。
 * @param longitude 将浮点数保留留⼩小数点后⾯面7位. 未获取到位置信息则填0f。
 */
void cod_cmd_sos_req(float latitude, float longitude);

//-------------------------------------------
/**
 * 指令（resp）:推送消息
 */
void cod_cmd_notify_msg_resp(void);

/**
 * 指令（resp）:勿扰模式设置
 */
void cod_cmd_set_no_disturb_resp(void);

//-------------------------------------------
/**
 * 指令（resp）:投屏数据传送指令
 * @param result 投屏结果
 */
void cod_cmd_cast_resp(cod_ble_cast_result result);

/**
 * 指令（resp）:同步天气数据指令
 */
void cod_cmd_sync_weather_resp(void);

#endif //COD_SDK_COD_BASIC_CMD_GEN_H
