//
// Created by Paint on 2019/7/22.
//

#ifndef COD_SDK_COD_BLE_API_H
#define COD_SDK_COD_BLE_API_H

#include <stdbool.h>
#include "cod_ble_cmd.h"
#include "cod_ble_constants.h"
#include "cod_ble_bean.h"
#include "cod_ble_log.h"
#include "processor/cod_ble_processor.h"
#include "generator/cod_cmd_gen.h"
#include "sport/cod_sport_data_converter.h"
#include "sport/cod_swim_data_converter.h"
#include "utils/cod_list.h"

typedef enum {
    COD_NULL_CMD = 0,
    COD_UNKNOWN_CMD,
    COD_CMD_LEN_IS_TOO_SHORT
} cod_cmd_process_error;


/**
 * 基础指令处理回调
 */
typedef struct {

    /**
     * 指令处理异常
     * @param error
     */
    void (*on_process_error)(cod_cmd_process_error error);

    /**
     * 连接测试请求
     * @param phone_tx_max 手机的最大发送MTU
     * @param phone_rx_max 手机的最大接收MTU
     */
    void (*on_connection_test)(uint16_t phone_tx_max, uint16_t phone_rx_max);

    /**
     * 获取设备版本请求
     */
    void (*on_get_device_version)(void);

    /**
     * 获取设备电量请求
     */
    void (*on_get_device_battery)(void);

    /**
     * 获取mac地址请求
     */
    void (*on_get_device_mac)(void);

    /**
     * 获取星历状态
     */
    void (*on_check_agps)(void);

    /**
     * 手机端请求同步IMEI和IMSI
     */
    void (*on_sync_imei_imsi)(void);

    void (*on_set_ios_ancs)(bool open);

    void (*on_set_device_mac)(uint16_t product_type, cod_ble_mac_address *mac_address);

    /**
     * 手机绑定请求
     * @param platform app 平台
     * @param name 用户名 UTF-16小端编码
     * @param user_id 用户id ASCII编码
     */
    void (*on_bind_device)(cod_app_platform platform,
                           char *name, uint8_t name_len,
                           char *user_id, uint8_t user_id_len);

    /**
     * 设备解绑
     */
    void (*on_unbind_device)(void);

    /**
     * 查询设备绑定关系
     */
    void (*on_check_bind)(void);

    /**
     * 手机端对硬件的主动同步IMEI和IMSI的回复
     */
    void (*on_sync_imei_imsi_resp)(void);

    /**
     * 发送绑定状态
     * @param status
     */
    void (*on_send_bind_status)(cod_bind_status status);

    /**
     * 时间设置
     * @param time
     */
    void (*on_set_time)(cod_ble_time *time);

    /**
     * 时间读取
     */
    void (*on_get_time)(void);

    /**
     * 闹钟设置
     * @param alarm_list
     */
    void (*on_set_alarm)(cod_ble_alarm_list *alarm_list);

    /**
     * 闹钟读取
     */
    void (*on_get_alarm)(void);

    /**
     * 用户信息设置
     * @param userInfo
     */
    void (*on_set_user_info)(cod_ble_user_info *userInfo);

    /**
     * 读取用户信息
     */
    void (*on_get_user_info)(void);

    /**
     * 久坐提醒设置
     * @param sit_remind_list
     */
    void (*on_set_sit_remind)(cod_ble_sit_remind_list *sit_remind_list);

    /**
     * 读取久坐提醒
     */
    void (*on_get_sit_remind)(void);

    /**
     * 睡眠设置
     * @param sleep_setting
     */
    void (*on_set_sleep_setting)(cod_ble_sleep_setting *sleep_setting);

    /**
     * 读取睡眠设置
     */
    void (*on_get_sleep_setting)(void);

    /**
     * 喝水提醒设置
     * @param drink_remind_list
     */
    void (*on_set_drink_remind)(cod_ble_drink_remind_list *drink_remind_list);

    /**
     * 读取喝水提醒
     */
    void (*on_get_drink_remind)(void);

    /**
     * 消息推送设置
     * @param msg_push_setting
     */
    void (*on_set_msg_push_setting)(cod_ble_msg_push_setting *msg_push_setting);

    /**
     * 读取消息推送设置
     */
    void (*on_get_msg_push_setting)(void);

    /**
     * 表盘设置
     * @param dial_plate_index
     */
    void (*on_set_dial_plate)(uint8_t dial_plate_index);

    /**
     * 表盘读取
     */
    void (*on_get_dial_plate)(void);

    /**
     * 运动自动暂停设置
     * @param auto_pause
     */
    void (*on_set_auto_pause)(bool auto_pause);

    /**
     * 读取运动自动暂停设置
     */
    void (*on_get_auto_pause)(void);

    /**
     * 运动公里牌/时间提醒设置
     * @param sport_notify_setting
     */
    void (*on_set_sport_notify_setting)(cod_ble_sport_notify_setting *sport_notify_setting);

    /**
     * 读取运动公里牌/时间提醒设置
     */
    void (*on_get_sport_notify_setting)(void);

    /**
     * 运动功能键设置
     * @param sport_btn_func
     */
    void (*on_set_sport_btn_func)(cod_ble_sport_btn_func sport_btn_func);

    /**
     * 读取运动功能键设置
     */
    void (*on_get_sport_btn_func)(void);

    /**
     * 同步咕咚计步目标
     * @param step_target
     */
    void (*on_set_step_target)(uint32_t step_target);

    /**
     * 抬腕亮屏设置
     * @param step_target
     */
    void (*on_set_raise_to_wake)(cod_ble_raise_to_wake_setting *setting);

    /**
     * 全天心率检测开关
     * @param enable
     */
    void (*on_set_all_day_heart_check)(bool enable);

    /**
     * 自动记录运动开关设置
     * @param enable
     * @param distance_threshold 阈值:单位(m)
     */
    void (*on_set_auto_record_sport)(bool enable, uint16_t distance_threshold);

    /**
     * 公制和英制的设置
     * @param enable
     * @param distance_threshold
     */
    void (*on_set_distance_unit)(cod_ble_distance_unit unit);

    /**
     * 独立运动心率检测开关设置.这个开关对应的是app上的省电模式
     * @param enable
     */
    void (*on_set_heart_check_in_standalone_sport)(bool enable);

    /**
     * 设置语言
     * @param language
     */
    void (*on_set_language)(cod_ble_language language);

    /**
     * 获取当前语言
     */
    void (*on_get_cur_language)(void);

    /**
     * 获取支持的语言
     */
    void (*on_get_support_language)(void);

    /**
     * 地理信息设置
     * @param info
     */
    void (*on_set_location_info)(cod_ble_location_info *info);

    /**
     * 紧急联系人设置
     * @param emergency_contacts
     */
    void (*on_set_emergency_contacts)(cod_ble_emergency_contacts *emergency_contacts);

    /**
     * 设置安静心率
     * @param heart_rate
     */
    void (*on_set_rest_heart_rate)(uint8_t heart_rate);

    /**
     * 设置客户端信息
     * @param info
     */
    void (*on_set_app_info)(cod_ble_app_info *info);

    /**
     * 开关硬件日志记录
     * @param log_setting
     */
    void (*on_set_log_setting)(cod_ble_log_setting *log_setting);

    /**
     * 运动或训练控制的回复
     * @param success 控制是否成功。一般不需要管。
     */
    void (*on_sport_ctrl_resp)(bool success);

    /**
     * 设置极限心率
     * @param enable 开关极限心率
     * @param heart_rate
     */
    void (*on_set_limit_heart_rate)(bool enable, uint8_t heart_rate);

    /**
     * 同步手机来电状态.
     * 来电指令，⼿机来电后，发指令让⼿表震动。
     * 在⼿机上拒接或忽略或接听，⼿表停止震动也不再显示来电图标。
     */
    void (*on_sync_phone_call_status)(cod_ble_phone_call_status status);

    /**
     * 设备控制主端来电的回复
     */
    void (*on_phone_call_ctrl_resp)(cod_ble_phone_call_ctrl ctrl);


    /**
     * 设置设备端来电震动延迟
     * @param delay_second 秒
     */
    void (*on_set_phone_call_virb_delay)(float delay_second);

    /**
     * 主动触发SOS的回复
     */
    void (*on_sos_resp)(bool success);

    /**
     * 手机端提醒类型设置
     * @param msg_type
     * @param title UTF-16小端编码,包括'\0'，不会为空，但如果只有'\0'，表示没有标题
     * @param title_len msg的长度，包括'\0'
     * @param msg UTF-16小端编码,包括'\0'
     * @param msg_len msg的长度，包括'\0'
     */
    void (*on_notify_msg)(cod_ble_msg_type msg_type, char *title, uint16_t title_len,
                          char *msg, uint16_t msg_len);

    /**
     * 勿扰模式设置
     * @param no_disturb
     */
    void (*on_set_no_disturb)(cod_ble_no_disturb *no_disturb);

    /**
     * 运动投屏
     * @param cast
     */
    void (*on_sport_cast)(cod_ble_sport_cast *cast);

    /**
     * 训练投屏
     * @param cast
     */
    void (*on_training_cast)(cod_ble_training_cast *cast);

    /**
     * 同步24小时天气
     * @param weather_list 天气数组。第一为当前小时的天气，后面每个天气的时间递增1小时。
     * @param num 天气个数
     */
    void (*on_sync_weather)(cod_ble_weather *weather_list, uint8_t num);

} cod_ble_basic_process_cb;

typedef struct {

    /**
     * 获取数据帧数
     * @param type 数据类型
     */
    void (*on_get_data_frame_num)(cod_ble_data_type type);

    /**
     * 删除数据
     * @param type 数据类型
     */
    void (*on_delete_data)(cod_ble_data_type type);

    /**
     * 获取数据
     * @param type 数据类型
     * @param start_frame 需要传输的起始帧号
     */
    void (*on_get_data)(cod_ble_data_type type, uint16_t start_frame);

    /**
     * 实时数据传输指令
     * @param type 数据类型
     * @param start 开始或者结束
     */
    void (*on_real_time_data_trans)(bool start, cod_ble_rt_data_type type);

    /**
     * 查询硬件具备的Sensor能力
     */
    void (*on_get_sensor_capability)(void);

    /**
     * 传感器数据传输指令。
     * @param start 开始或者结束
     * @param sensor_req_array 传感器请求数组
     * @param req_num 数组长度
     */
    void (*on_sensor_data_trans)(bool start, cod_ble_sensor_req *sensor_req_array, uint8_t req_num);

    void (*on_real_time_log_trans)(bool start, cod_ble_rt_log_type type);

} cod_ble_data_trans_process_cb;

typedef struct {
    /**
     * OTA传输开始请求
     * @param phone_mtu 手机本次的传输MTU
     * @param file_size 文件大小
     * @param file_type 文件类型
     */
    void (*on_ota_start)(uint16_t phone_mtu, uint32_t file_size, cod_ble_file_type file_type);

    /**
     * 组CRC校验请求
     * @param is_success sdk判断组的crc校验是否成功
     * @param data_type 数据类型
     */
    void (*on_ota_group_crc)(bool is_success, uint8_t data_type);

    /**
     * 文件CRC校验请求
     * @param is_success sdk判断文件的crc校验是否成功。SDK判断只是一个预判，
     * 实际要以硬件重新读取文件数据计算得到的CRC校验为准。
     * @param file_crc
     * @param data_type
     */
    void (*on_ota_file_crc)(bool is_success, uint32_t file_crc, uint8_t data_type);

    /**
     * 每帧的文件数据返回
     * @param file_data
     * @param data_len
     */
    void (*on_ota_data)(uint8_t *file_data, uint16_t data_len);
} cod_ble_ota_process_cb;

typedef struct {

    /**
     * 指令发生器的回调。SDK使用者可以根据cod_cmd_type决定当前指令是否是直接发送，还是放到任务队列。
     * 一般而言，CMD_TYPE_SENSOR类型需要直接发送。
     * @param cmd_type 指令类型
     * @param cmds 组装好的指令数组
     * @param cmd_num 组装好的指令数组长度
     */
    void (*on_cod_cmd_gen)(cod_cmd_type cmd_type, cod_ble_buffer *cmds, uint8_t cmd_num);
} cod_ble_cmd_gen_cb;

typedef struct {
    void (*print_log)(int level, char *tag, char *content);
} cod_ble_log;

typedef struct {

    void *(*malloc)(uint32_t size);

    void (*free)(void *p);
} cod_ble_memory_delegate;

/**
 * Codoon Ble SDK 初始化
 * @param tx_mtu 硬件发送指令每帧的数据长度，此长度包括codoon协议壳，不包括蓝牙底层L2CAP的协议壳。
 * @param basic_process_cb 基础指令处理回调
 * @param data_trans_process_cb 数据传输指令处理回调
 * @param ota_process_cb OTA指令处理回调
 * @param cmd_gen_cb 指令组装回调
 * @param log_delegate 日志代理
 * @param memory_delegate 设置外部内存代理，设置之后SDK的动态内存分配和是否都会通过代理实现。
 */
void cod_ble_init(uint16_t tx_mtu,
                  cod_ble_basic_process_cb *basic_process_cb,
                  cod_ble_data_trans_process_cb *data_trans_process_cb,
                  cod_ble_ota_process_cb *ota_process_cb,
                  cod_ble_cmd_gen_cb *cmd_gen_cb,
                  cod_ble_log *log_delegate,
                  cod_ble_memory_delegate *memory_delegate);

/**
 * 设置硬件发送指令每帧的数据长度。初始化之后，如果协商的mtu发生变化，需要调用此函数更新sdk内部使用mtu。
 * @param tx_mtu 硬件发送指令每帧的数据长度，此长度包括Codoon协议壳，不包括蓝牙底层L2CAP的协议壳。
 */
void cod_ble_set_tx_mtu(int tx_mtu);

/**
 * 设置版本信息
 */
void cod_ble_set_version(cod_version* version);

/**
 * 设置上次AGPS更新的时间戳，如果没有更新过，则填0。
 */
void cod_ble_set_agps_update_time(uint64_t time);

/**
 * 设置用户信息
 * @param user
 */
void cod_ble_set_user_info(cod_ble_user_info* user);

/**
 * 开关sdk内部的日志输出，默认开
 * @param open
 */
void cod_ble_open_log(bool open);

extern cod_ble_basic_process_cb *g_basic_process_cb;
extern cod_ble_data_trans_process_cb *g_data_trans_process_cb;
extern cod_ble_ota_process_cb *g_ota_process_cb;
extern cod_ble_cmd_gen_cb *g_cmd_gen_cb;
extern int g_tx_mtu;
extern bool g_cod_open_log;
extern cod_ble_log *clog_delegate;
extern cod_ble_memory_delegate *g_memory_delegate;
extern cod_version g_cod_version;
extern cod_ble_user_info g_user_info;
extern uint64_t g_agps_update_time;

#endif //COD_SDK_COD_BLE_API_H
