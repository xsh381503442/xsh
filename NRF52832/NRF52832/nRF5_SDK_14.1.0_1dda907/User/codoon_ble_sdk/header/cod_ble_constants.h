//
// Created by Paint on 2019/7/22.
//

#ifndef COD_SDK_COD_BLE_CONSTANTS_H
#define COD_SDK_COD_BLE_CONSTANTS_H

#define DEFAULT_TX_MTU          20
#define DEFAULT_CMD_HEAD        0xAA
#define OTA_DATA_CMD_HEAD       0xAB

/**
 * 调用SDK时，给外部的状态反馈
 * */
#define COD_STATUS_SUCCESS      0x01//协议解析 成功
#define COD_STATUS_FAIL         0x02//协议解析 失败

typedef enum {
    COD_BIG_ENDIAN,
    COD_LITTLE_ENDIAN
} cod_byte_order;

typedef enum {
    ANDROID = 1,
    IOS = 0
} cod_app_platform;

/**
 * 指令类型，一般而言，CMD_TYPE_SENSOR类型需要直接发送。
 */
typedef enum {
    CMD_TYPE_NORMAL = 1,
    CMD_TYPE_SETTING,
    CMD_TYPE_SENSOR,
    CMD_TYPE_SYNC,
    CMD_TYPE_OTA,
} cod_cmd_type;

/**
 * 默认设定的mtu的大小
 * */
typedef enum {
    MTU_509 = 509, MTU_155 = 155, MTU_20 = 20
} cod_mtu_size;

/**
 * 设备当前的绑定状态
 * */
typedef enum {
    STATUS_BINDING,
    STATUS_BIND_SUCCESS,
    STATUS_BIND_FAIL
} cod_bind_status;

/**
 * 设备当前的电量状态
 * */
typedef enum {
    STATUS_ELE_NOT_CHARGED = 0,
    STATUS_ELE_CHARGED ,
    STATUS_ELE_CHARGING
} cod_battery_status;

typedef enum{
    NORMAL_ALARM = 0,   //普通闹钟
    CONFERENCE_ALARM          //会议
} cod_ble_alarm_type;

typedef enum{
    FEMALE = 0, //女性
    MALE        //男性
} cod_ble_gender;


typedef enum{
    MSG_TYPE_PHONE_CALL,    //来电
    MSG_TYPE_PHONE_SMS,     //短信
    MSG_TYPE_QQ,
    MSG_TYPE_WECHAT,        //微信
    MSG_TYPE_WHATSAPP,
    MSG_TYPE_MESSENGER,
    MSG_TYPE_TWITTER,
    MSG_TYPE_LINKEDIN,
    MSG_TYPE_INSTAGRAM,
    MSG_TYPE_FACEBOOK,
    MSG_TYPE_LINE,
    MSG_TYPE_COMPANY_WECHAT, //企业微信
    MSG_TYPE_DINGDING        //钉钉
} cod_ble_msg_type;

/**
 * 运动键功能
 */
typedef enum{
    SPORT_FUNC_OUTDOOR_RUN = 1,    //户外跑
    SPORT_FUNC_INDOOR_RUN,         //室内跑
    SPORT_FUNC_OUTDOOR_WALK,       //户外健走
    SPORT_FUNC_INDOOR_WALK,        //室内健走
    SPORT_FUNC_RIDE,               //骑行
    SPORT_FUNC_SWIM,               //泳池游泳
    SPORT_FUNC_CLIMB,              //登山
} cod_ble_sport_btn_func;

/**
 * 运动类型
 */
typedef enum{
    SPORT_TYPE_UNKNOWN = 0,        //未知类型
    SPORT_TYPE_OUTDOOR_RUN = 1,    //户外跑
    SPORT_TYPE_OUTDOOR_WALK,       //户外健走
    SPORT_TYPE_RIDE,               //骑行
    SPORT_TYPE_CLIMB,              //登山
    SPORT_TYPE_INDOOR_RUN,         //室内跑
    SPORT_TYPE_INDOOR_WALK,        //室内健走
    SPORT_TYPE_SKI,                //滑雪
    SPORT_TYPE_SKATE,              //滑冰
} cod_ble_sport_type;


/**
 * 游泳类型
 */
typedef enum{
    SWIM_TYPE_INDOOR = 0,    //室内游泳
    SWIM_TYPE_OUTDOOR,       //室外游泳
} cod_ble_swim_type;

/**
 * 泳姿类型
 */
typedef enum{
    SWIM_STROKE_OTHER = 0,    //其他
    SWIM_STROKE_BREASTSTROKE, //蛙泳
    SWIM_STROKE_FRONT_CRAWL,  //自由泳
    SWIM_STROKE_BACK_CRAWL,   //仰泳
    SWIM_STROKE_BUTTERFLY,    //蝶泳
    SWIM_STROKE_MEDLEY = 8,   //混合泳
} cod_ble_swim_stroke;

/**
 * 距离单位
 */
typedef enum{
    UNIT_KILOMETER = 0,      //公里
    UNIT_MILE,               //英里
} cod_ble_distance_unit;

/**
 * 语言
 */
typedef enum{
    LAN_CHINESE = 0,      //中文
    LAN_ENGLISH,          //英文
} cod_ble_language;

/**
 * app类型
 */
typedef enum{
    APP_TYPE_CODOON_CHINESE = 0,      //咕咚中文版
    APP_TYPE_RUNTOPIA,                //咕咚国际版
} cod_ble_app_type;


/**
 * 运动或者训练的控制
 */
typedef enum{
    SPORT_CTRL_PAUSE = 1,      //暂停运动
    SPORT_CTRL_RESUME,         //继续运动
    SPORT_CTRL_STOP,           //结束运动
} cod_ble_sport_ctrl;

/**
 * 来电状态
 */
typedef enum{
    PHONE_CALL_STATUS_ANSWERED = 1,      //来电接听
    PHONE_CALL_STATUS_HANGUP,            //来电挂断
} cod_ble_phone_call_status;

/**
 * 来电控制
 */
typedef enum{
    ANSWER_PHONE_CALL = 1,      //接听。接听有些手机实现不了。
    REJECT_PHONE_CALL,          //拒接
    IGNORE_PHONE_CALL,          //忽略
} cod_ble_phone_call_ctrl;

/**
 * 运动状态
 */
typedef enum{
    SPORT_STATUS_START = 0,      //运动开始
    SPORT_STATUS_SPORTING,       //运动中
    SPORT_STATUS_PAUSE,          //运动暂停
    SPORT_STATUS_STOP,           //运动结束
} cod_ble_sport_status;

/**
 * 训练类型
 */
typedef enum{
    TRAINING_TYPE_UNKNOWN = 0,      //未知
    TRAINING_TYPE_NORMAL,           //普通训练
    TRAINING_TYPE_INTERACT,         //互动式课程
    TRAINING_TYPE_LIVE,             //直播训练
} cod_ble_training_type;

/**
 * 训练状态
 */
typedef enum{
    TRAINING_STATUS_UNKNOWN = 0,        //未知
    TRAINING_STATUS_PREPARE,            //准备
    TRAINING_STATUS_START,              //开始
    TRAINING_STATUS_INTRODUCT,          //讲解
    TRAINING_STATUS_TRAINING,           //训练中
    TRAINING_STATUS_REST,               //休息
    TRAINING_STATUS_PAUSE,              //暂停
    TRAINING_STATUS_RESUME,             //恢复
    TRAINING_STATUS_STOP,               //结束
} cod_ble_training_status;


/**
 * 训练动作类型
 */
typedef enum{
    TRAINING_ACTION_TYPE_TIME = 1,      //时间
    TRAINING_ACTION_TYPE_COUNT,         //个数
} cod_ble_training_action_type;


/**
 * 咕咚数据类型
 */
typedef enum{
    DATA_TYPE_STEP = 1,      //全天计步
    DATA_TYPE_SLEEP,         //睡眠数据
    DATA_TYPE_HEART,         //睡眠数据
    DATA_TYPE_BP,            //血压数据
    DATA_TYPE_SPORT,         //运动数据
    DATA_TYPE_LOG = 10       //日志数据
} cod_ble_data_type;

/**
 * 咕咚实时数据类型
 */
typedef enum{
    RT_DATA_TYPE_TODAY_SUMMARY = 2, //当天步数，距离，卡路里数据。每5秒钟发送一次。
    RT_DATA_TYPE_HEART_BP_BO,       //心率，血压，血氧数据，只当采集到值时才发送，时间间隔不固定。
    RT_DATA_TYPE_ROPE,              //跳绳数据，只当采集到值时才发送，时间间隔不固定。
} cod_ble_rt_data_type;

/**
 * 咕咚实时日志类型
 */
typedef enum{
    RT_LOG_TYPE_NORMAL,             //普通日志
    RT_LOG_TYPE_BT,                 //蓝牙协议栈日志
    RT_LOG_TYPE_SPORT,              //运动日志
    RT_LOG_TYPE_HRAET_RAW           //心率原始日志
} cod_ble_rt_log_type;

/**
 * 传感器类型
 */
typedef enum{
    SENSOR_TYPE_ACC = 1,    //加速度机
    SENSOR_TYPE_GYRO,       //陀螺仪
    SENSOR_TYPE_HEART,      //心率
    SENSOR_TYPE_MAG,        //地磁
    SENSOR_TYPE_PRESSURE,   //气压计
} cod_ble_sensor_type;

/**
 * FILE类型
 */
typedef enum{
    FILE_TYPE_OTA = 0,      //OTA文件
    FILE_TYPE_FONT,         //字库
    FILE_TYPE_GPS,          //星历文件
    FILE_TYPE_BP,           //表盘文件
} cod_ble_file_type;

/**
 * 点类型 表示暂停继续状态。
 */
typedef enum{
    COD_GPS_POINT_NORMAL = 0,     //普通点
    COD_GPS_POINT_PAUSE,          //暂停点
    COD_GPS_POINT_RESUME,         //恢复点
    COD_GPS_POINT_PAUSE_NO_GPS,   //无GPS信息的暂停点
    COD_GPS_POINT_VIRTUAL,        //虚拟点
} cod_gps_point_type;

/**
 * GPS正常/低功耗状态
 */
typedef enum{
    COD_GPS_STATUS_NORMAL = 0,     //正常状态
    COD_GPS_STATUS_LOW_POWER,      //低功耗状态
} cod_gps_mode;

/**
 * OTA失败原因
 */
typedef enum{
    COD_BLE_OTA_START_FAILED_TYPE_LOW_BAT = 2,     //电量低
    COD_BLE_OTA_START_FAILED_TYPE_SPORTING,        //正在独立运动，请结束运动后升级
    COD_BLE_OTA_START_FAILED_TYPE_UNKNOWN_TYPE,    //未知升级类型
} cod_ble_ota_start_failed_reason;

/**
 * 投屏结果
 */
typedef enum{
    COD_BLE_CAST_RESULT_OK = 0,          //投屏成功
    COD_BLE_CAST_RESULT_NOT_SUPPORT = 1, //不支持投屏
    COD_BLE_CAST_RESULT_SPORTING = 0xff, //当前正在独立运动，则回复错误码0xFF
} cod_ble_cast_result;


/**
 * 天气类型
 */
typedef enum{
    WEATHER_TYPE_SUNNY = 0,         //晴
    WEATHER_TYPE_SUNNY_NIGHT,       //晴-夜间
    WEATHER_TYPE_CLOUDY,            //多云
    WEATHER_TYPE_CLOUDY_NIGHT,      //多云-夜间
    WEATHER_TYPE_OVERCAST,          //阴
    WEATHER_TYPE_HEAVY_RAIN,        //大雨
    WEATHER_TYPE_LIGHT_RAIN,        //小雨
    WEATHER_TYPE_MODERATE_RAIN,     //中雨
    WEATHER_TYPE_TORRENTIAL_RAIN,   //暴雨
    WEATHER_TYPE_SNOW,              //雪
    WEATHER_TYPE_DUST,              //尘
    WEATHER_TYPE_FOGGY,             //雾
    WEATHER_TYPE_HAZE,              //霾
    WEATHER_TYPE_WINDY,             //风
    WEATHER_TYPE_UNKNOWN,           //未知
} cod_weather_type;

#endif //COD_SDK_COD_BLE_CONSTANTS_H
