//
// Created by Paint on 2019/7/22.
//

#ifndef COD_SDK_COD_BLE_BEAN_H
#define COD_SDK_COD_BLE_BEAN_H

#include <stdint.h>
#include "utils/cod_list.h"
#include "cod_ble_constants.h"
#include "utils/cod_buffer.h"
#include <stdbool.h>


#define COD_MAC_LEN   6
#define COD_MAX_ALARM_NUM   20
#define COD_MAX_SIT_REMIND_NUM   10
#define COD_MAX_DRINK_REMIND_NUM   10
#define COD_MAX_NAME_LEN   41   //20个字
#define COD_MAX_PHONE_NUMBER_LEN   41   //20个字
#define COD_MAX_EMERGENCY_CONTACTS_NUM   5

/**
 * mac地址的结构体,COD_MAC_LEN个字节
 * */
typedef struct {
    uint8_t content[COD_MAC_LEN];
} cod_ble_mac_address;

/**
 * 时间
 */
typedef struct {
    uint16_t year;          //年:2018~2099
    uint8_t month;          //月:1~12
    uint8_t day;            //日:1~31
    uint8_t hour;           //时:0~23
    uint8_t minute;         //分:0~59
    uint8_t second;         //秒:0~59
    uint8_t week;           //星期: 0:星期1 1:星期2 2:星期3 3:星期4 4:星期5 5:星期6 6:星期日
    uint8_t time_format;    //0:24小时制, 1:12小时制, 其他值:不修改当前时间进制
    int8_t time_zone;       //有符号数，值为:-12~+12 (未表示有符号数，传输时将时区 + 12，解析时 -12 ) 手机下发的时间为零时区时间，本地时间为:手机下发的时间+时区值
} cod_ble_time;


/**
 * 闹钟
 */
typedef struct {
    uint8_t hour;           //时:0~23
    uint8_t minute;         //分:0~59
    uint8_t repeat;         //每一bit代表某一日是否重复，1表示重复，0表示不重复。bit0表示周一，bit6表示周日。所有都为0，表示只提醒一次。
    cod_ble_alarm_type type;
    bool enable;            //开关
} cod_ble_alarm;

/**
 * 闹钟数组结构，里面包含一个数组指针，及数组大小
 */
typedef struct {
    cod_ble_alarm *list;
    uint8_t num;
} cod_ble_alarm_list;

/**
 * 久坐提醒
 */
typedef struct {
    uint8_t start_hour;     //开始时间:采用24小时制，以小时为单位。
    uint8_t stop_hour;      //结束时间:采用24小时制，以小时为单位。
    //久坐时间: 以分钟为单位，超过这个时间即提醒
    //例 :设计的久坐时间为:30分钟，若9:20出提醒事件后，则一下个判定时间则是9:20~9:50，而不是按自然时间。
    uint8_t sitting_long_time;
    uint8_t sitting_long_step_threshold;    //以小时为单位久坐时间内，步数低于这个阈值才提醒。单位:步数。
    uint8_t repeat;         //每一bit代表某一日是否重复，1表示重复，0表示不重复。bit0表示周一，bit6表示周日。所有都为0，表示只提醒一次。
    bool enable;            //开关
} cod_ble_sit_remind;

/**
 * 久坐提醒数组结构，里面包含一个数组指针，及数组大小
 */
typedef struct {
    cod_ble_sit_remind *list;
    uint8_t num;
} cod_ble_sit_remind_list;

/**
 * 睡眠检测设置
 * 默认为晚上22点~早上6点
 * 对于睡眠时间长度的计算可以采用如下方式:
 *      按照24小时制式，差值 = 结束时间-开始时间
 *      如果差值>=0，则睡眠时间长度 = 差值
 *      如果差值<0，则睡眠时间长度 = 24+差值
 */
typedef struct {
    uint8_t start_hour;     //开始时间:采用24小时制，以小时为单位。
    uint8_t stop_hour;      //结束时间:采用24小时制，以小时为单位。
    bool enable;            //开关
} cod_ble_sleep_setting;


/**
 * 喝水提醒
 */
typedef struct {
    uint8_t start_hour;     //开始hour:采用24小时制
    uint8_t start_minute;
    uint8_t stop_hour;      //结束hour:采用24小时制
    uint8_t stop_minute;
    uint8_t drink_interval; //喝水间隔，单位: 分钟。多少分钟不喝水，则提醒。
    uint8_t repeat;         //每一bit代表某一日是否重复，1表示重复，0表示不重复。bit0表示周一，bit6表示周日。所有都为0，表示只提醒一次。
    bool enable;            //开关
} cod_ble_drink_remind;

/**
 * 喝水提醒数组结构，里面包含一个数组指针，及数组大小
 */
typedef struct {
    cod_ble_drink_remind *list;
    uint8_t num;
} cod_ble_drink_remind_list;

/**
 * 用户信息
 */
typedef struct {
    cod_ble_gender gender;  //性别
    uint8_t age;
    uint8_t height;         //身高:单位CM
    uint8_t weight;         //体重:kg
} cod_ble_user_info;

/**
 * 消息推送设置
 */
typedef struct {
    bool phone_call;    //来电
    bool phone_sms;     //短信
    bool qq;
    bool wechat;        //微信
    bool whatsapp;
    bool messenger;
    bool twitter;
    bool linkedin;
    bool instagram;
    bool facebook;
    bool line;
    bool company_wechat;//企业微信
    bool dingding;      //钉钉
} cod_ble_msg_push_setting;


/**
 * 独⽴运动公⾥牌设置
 */
typedef struct {
    bool distance_notify_enable;    //距离播报开关
    float notify_distance;          //单位公⾥/英⾥，每多少公⾥/英⾥提醒
    bool time_notify_enable;        //时间播报开关
    uint8_t notify_time;            //单位分钟，每多少分钟提醒
} cod_ble_sport_notify_setting;

/**
 * 抬腕亮屏设置
 */
typedef struct {
    bool enable;             //开关
    uint8_t start_hour;      //开始时间
    uint8_t duration;        //持续时间
} cod_ble_raise_to_wake_setting;


/**
 * 联系人
 */
typedef struct {
    uint8_t name_len;           //联系人姓名长度，包含'\0'
    char *name;                 //联系人姓名，包含'\0'。UTF-16⼩端编码
    uint8_t phone_number_len;   //联系人号码长度，包含'\0'
    char *phone_number;         //联系人号码，包含'\0'。UTF-16⼩端编码
} cod_ble_contact;

/**
 * 所有紧急联系人
 */
typedef struct {
    bool has_emergency_contacts;    //是否有紧急联系人
    cod_ble_contact *contact_list;  //联系人数组指针
    uint8_t num;                    //联系人数组个数
} cod_ble_emergency_contacts;

/**
 * 地理信息
 */
typedef struct {
    float latitude;             //纬度
    float longitude;            //经度
    uint32_t city_code;         //城市码，0表示没有获取到城市码信息，则本次从端不不⽤用更更新城市信息。
    char *city_name;            //城市名称，包含'\0'
    uint8_t city_name_len;      //城市名称的长度，包含'\0'
} cod_ble_location_info;

/**
 * 客户端信息
 */
typedef struct {
    cod_ble_app_type app_type;
    cod_app_platform platform;
    char *nick_name;            //昵称，包含'\0'.UTF-16小端编码
    uint8_t nick_name_len;     //昵称的长度，包含'\0'
    uint8_t reserved[2];        //预留字段
} cod_ble_app_info;

/**
 * 硬件日志设置
 */
typedef struct {
    bool enable;
    uint8_t auto_stop_hour;     //自动结束时间(小时)。 0: 不自动结束；>0:超过此小时则自动关闭日志记录，但不清除已经记录日志。
    uint8_t reserved[2];        //预留字段
} cod_ble_log_setting;

/**
 * 运动投屏信息
 */
typedef struct {
    cod_ble_sport_status sport_status;
    cod_ble_sport_type sport_type;
    float distance;                 //米
    uint32_t time;                  //时长，秒
    uint16_t pace;                  //配速，秒
    uint8_t heart_rate;             //实时心率
    uint8_t avg_heart_rate;         //平均心率
    uint32_t step;                  //步数
    float rt_speed;                 //实时速度, 千米/小时
    float avg_speed;                //平均速度, 千米/小时
    float calorie;                  //卡路里, 大卡
    int16_t altitude;              //海拔
    int16_t altitude_diff;         //海拔上升
    int16_t max_altitude;          //最高海拔
    uint32_t total_altitude_increase;//累积爬升
    uint32_t climb_distance;        //上坡里程
} cod_ble_sport_cast;

/**
 * 训练投屏信息
 */
typedef struct {
    cod_ble_training_status training_status;
    cod_ble_training_type training_type;
    cod_ble_training_action_type action_type;
    char *course_name;          //课程名称，包括'\0'
    uint16_t course_name_len;   //课程名称长度，包括'\0'
    char *action_name;          //动作名称，包括'\0'
    uint16_t action_name_len;   //动作名称长度，包括'\0'
    uint16_t action_target;     //动作目标值, 当type为时间时，单位为秒，当type技术时，单位为个
    uint16_t action_actual;     //动作实际值
    float calorie;              //卡路里, 大卡
    uint32_t time;              //时长，秒
    uint8_t avg_heart_rate;         //平均心率,结束状态时会传，范围0-255
} cod_ble_training_cast;


/**
 * 天气
 */
typedef struct {
    cod_weather_type weather;   //天气
    int8_t temperature;         //温度
    uint16_t aqi;
} cod_ble_weather;

/**
 * 版本信息
 * firmware_h 固件版本，高位
 * firmware_m 固件版本，中位
 * firmware_l 固件版本，低位
 * res_h 资源版本，高位
 * res_m 资源版本，中位
 * res_l 资源版本，低位
 * ble_h 蓝牙协议栈版本，高位
 * ble_m 蓝牙协议栈版本，中位
 * ble_l 蓝牙协议栈版本，低位
 */
typedef struct {
    uint8_t firmware_h;
    uint8_t firmware_m;
    uint8_t firmware_l;

    uint8_t res_h;
    uint8_t res_m;
    uint8_t res_l;

    uint8_t ble_h;
    uint8_t ble_m;
    uint8_t ble_l;
} cod_version;

/**
 * 勿扰模式设置
 */
typedef struct {
    bool enable;            //开关
    uint8_t start_hour;     //开始时间:采用24小时制
    uint8_t start_minute;
    uint8_t end_hour;       //结束时间:采用24小时制
    uint8_t end_minute;
} cod_ble_no_disturb;


/**
 * Sensor能力
 */
typedef struct {
    cod_ble_sensor_type type;  //传感器类型
    uint8_t max_freq;          //传感器数据的最大请求频率
} cod_ble_sensor_cap;

/**
 * Sensor数据请求
 */
typedef struct {
    cod_ble_sensor_type type;  //传感器类型
    uint8_t freq;              //传感器数据的请求频率，采集频率一般为 1hz, 50hz 100hz。心率一般1Hz，6轴数据一般50Hz。
} cod_ble_sensor_req;

// ============= utils for cod_ble_alarm =============
/**
 * 将闹钟列表转化为字节数组，方便硬件保存。
 * 字节数组格式为二字节表示len+咕咚的协议格式。
 * 后续可以通过cod_bytes2alarm_list转化为闹钟列表。
 * @param alarm_list
 * @return 字节数组，用完之后记得通过cod_buffer_release释放内存
 *
 */
cod_ble_buffer *cod_alarm_list2bytes(cod_ble_alarm_list *alarm_list);

/**
 * 将闹钟字节数组转化为闹钟列表
 * @param has_head 第三方开发者的数据，一般是有head，这里填true
 * @param alarm_bytes
 * @param len alarm_bytes的长度
 * @return 闹钟列表，用完之后记得通过cod_ble_alarm_list_release释放内存
 */
cod_ble_alarm_list *cod_bytes2alarm_list(bool has_head, uint8_t *alarm_bytes, uint16_t len);

/**
 * 释放闹钟列表的内存
 * @param list
 */
void cod_ble_alarm_list_release(cod_ble_alarm_list *list);

// ============= utils for cod_ble_sit_remind =============
/**
 * 将久坐提醒列表转化为字节数组，方便硬件保存。
 * 字节数组格式为二字节表示len+咕咚的协议格式。
 * 后续可以通过cod_bytes2sit_remind_list转化为久坐提醒列表。
 * @param sit_remind_list
 * @return 字节数组，用完之后记得通过cod_buffer_release释放内存
 */
cod_ble_buffer *cod_sit_remind_list2bytes(cod_ble_sit_remind_list *sit_remind_list);

/**
 * 将久坐提醒字节数组转化为久坐提醒列表
 * @param has_head 第三方开发者的数据，一般是有head，这里填true
 * @param sit_remind_bytes
 * @param len sit_remind_bytes的长度
 * @return 久坐提醒列表，用完之后记得通过cod_ble_sit_remind_list_release释放内存
 */
cod_ble_sit_remind_list *cod_bytes2sit_remind_list(bool has_head, uint8_t *sit_remind_bytes, uint16_t len);

/**
 * 释放久坐提醒列表的内存
 * @param list
 */
void cod_ble_sit_remind_list_release(cod_ble_sit_remind_list *list);

// ============= utils for cod_ble_drink_remind =============
/**
 * 将喝水提醒列表转化为字节数组，方便硬件保存。
 * 字节数组格式为二字节表示len+咕咚的协议格式。
 * 后续可以通过cod_bytes2drink_remind_list转化为喝水提醒列表。
 * @param drink_remind_list
 * @return 字节数组，用完之后记得通过cod_buffer_release释放内存
 */
cod_ble_buffer *cod_drink_remind_list2bytes(cod_ble_drink_remind_list *drink_remind_list);

/**
 * 将喝水提醒字节数组转化为喝水提醒列表
 * @param drink_remind_bytes
 * @param len drink_remind_bytes的长度
 * @return 喝水提醒列表，用完之后记得通过cod_ble_drink_remind_list_release释放内存
 */
cod_ble_drink_remind_list *cod_bytes2drink_remind_list(bool has_head, uint8_t *drink_remind_bytes, uint16_t len);

/**
 * 释放喝水提醒列表的内存
 * @param list
 */
void cod_ble_drink_remind_list_release(cod_ble_drink_remind_list *list);

// ============= utils for cod_ble_msg_push_setting =============
/**
 * 将消息推送设置转化为字节数组，方便硬件保存。
 * 字节数组格式为二字节表示len+咕咚的协议格式。
 * 后续可以通过cod_bytes2msg_push_setting转化为消息推送设置。
 * @return 字节数组，用完之后记得通过cod_buffer_release释放内存
 */
cod_ble_buffer *cod_msg_push_setting2bytes(cod_ble_msg_push_setting *msg_push_setting);

/**
 * 将消息推送设置字节数组转化为消息推送设置
 * @param msg_push_setting_bytes
 * @param len msg_push_setting_bytes的长度
 * @param output
 */
void cod_bytes2msg_push_setting(bool has_head, uint8_t *msg_push_setting_bytes, uint16_t len,
                                cod_ble_msg_push_setting *output);

// ============= utils for cod_ble_sport_notify_setting =============
/**
 * 将运动中提醒设置转化为字节数组，方便硬件保存。
 * 字节数组格式为二字节表示len+咕咚的协议格式。
 * 后续可以通过cod_bytes2sport_notify_setting转化为运动中提醒设置。
 * @return 字节数组，用完之后记得通过cod_buffer_release释放内存
 */
cod_ble_buffer *cod_sport_notify_setting2bytes(cod_ble_sport_notify_setting *sport_notify_setting);

/**
 * 将运动中提醒字节数组转化为运动中提醒设置
 * @param sport_notify_setting_bytes
 * @param len sport_notify_setting_bytes的长度
 * @param output
 */
void cod_bytes2sport_notify_setting(bool has_head, uint8_t *sport_notify_setting_bytes, uint16_t len,
                                    cod_ble_sport_notify_setting *output);

// ============= utils for cod_ble_location_info =============
/**
 * 将地理信息转化为字节数组，方便硬件保存。
 * 字节数组格式为二字节表示len+咕咚的协议格式。
 * 后续可以通过cod_bytes2location_info转化为地理信息。
 * @return 字节数组，用完之后记得通过cod_buffer_release释放内存
 */
cod_ble_buffer *cod_location_info2bytes(cod_ble_location_info *location_info);

/**
 * 将地理信息字节数组转化为地理信息
 * @param location_info_bytes
 * @param len location_info_bytes的长度
 * @return 地理信息，用完之后记得通过cod_ble_location_info_release释放内存
 */
cod_ble_location_info *cod_bytes2location_info(bool has_head, uint8_t *location_info_bytes, uint16_t len);

/**
 * 释放cod_ble_location_info的内存
 * @param location_info
 */
void cod_ble_location_info_release(cod_ble_location_info *location_info);

// ============= utils for cod_ble_emergency_contacts =============
/**
 * 将紧急联系人信息转化为字节数组，方便硬件保存。
 * 字节数组格式为二字节表示len+咕咚的协议格式。
 * 后续可以通过cod_bytes2emergency_contacts_list转化为紧急联系人信息。
 * @param emergency_contacts
 * @return 字节数组，用完之后记得通过cod_buffer_release释放内存
 */
cod_ble_buffer *cod_emergency_contacts_list2bytes(cod_ble_emergency_contacts *emergency_contacts);

/**
 * 将紧急联系人信息字节数组转化为紧急联系人信息
 * @param emergency_contacts_bytes
 * @param len emergency_contacts_bytes的长度
 * @return 紧急联系人信息，用完之后记得通过cod_ble_emergency_contacts释放内存
 */
cod_ble_emergency_contacts *
cod_bytes2emergency_contacts_list(bool has_head, uint8_t *emergency_contacts_bytes, uint16_t len);

/**
 * 释放cod_ble_emergency_contacts的内存
 * @param emergency_contacts
 */
void cod_ble_emergency_contacts_release(cod_ble_emergency_contacts *emergency_contacts);

///**
// * 释放cod_ble_contact的内存
// * @param contact
// */
//void cod_ble_contacts_release(cod_ble_contact* contact);


// ============= utils for cod_ble_app_info =============
/**
 * 将客户端信息转化为字节数组，方便硬件保存。
 * 字节数组格式为二字节表示len+咕咚的协议格式。
 * 后续可以通过cod_bytes2app_info转化为客户端信息。
 * @return 字节数组，用完之后记得通过cod_buffer_release释放内存
 */
cod_ble_buffer *cod_app_info2bytes(cod_ble_app_info *app_info);

/**
 * 将客户端信息字节数组转化为客户端信息
 * @param app_info_bytes
 * @param len app_info_bytes的长度
 * @return 客户端信息，用完之后记得通过cod_ble_app_info_release释放内存
 */
cod_ble_app_info *cod_bytes2app_info(bool has_head, uint8_t *app_info_bytes, uint16_t len);

/**
 * 释放cod_ble_app_info的内存
 * @param app_info
 */
void cod_ble_app_info_release(cod_ble_app_info *app_info);

#endif //COD_SDK_COD_BLE_BEAN_H
