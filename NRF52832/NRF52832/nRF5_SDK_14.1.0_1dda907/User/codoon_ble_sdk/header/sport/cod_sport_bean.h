//
// Created by Paint on 2019-10-15.
//

#ifndef COD_SDK_COD_SPORT_BEAN_H
#define COD_SDK_COD_SPORT_BEAN_H


#include <stdint.h>
#include "../utils/cod_list.h"

#include "../cod_ble_bean.h"


typedef struct {
    uint32_t id;            //记录ID，唯一的标明一条记录，自增。到最大值后+1回到0。
    cod_ble_sport_type sport_type;
    float distance;         //总距离，m
    uint32_t total_time;    //包括暂停，s
    uint32_t sport_time;    //不包括暂停，s
    uint64_t start_time;    //起始时间戳，ms
    uint64_t end_time;      //结束时间戳，ms
    uint64_t last_pause_time;//暂停的时间戳，ms，如果当前不处于暂停状态，则为0
    uint32_t pause_total_time;//暂停的总时间，s
    float speed;            //m/s
    float speed_max;        //m/s
    float speed_avg;        //m/s
    uint32_t pace;          //配速, 秒/公里
    uint32_t pace_avg;      //平均配速, 秒/公里
    float calorie;          //大卡
    float altitude;         //m
    float altitude_diff;    //m，海拔上升，最后一个海拔 - 第一个海拔
    float altitude_max;     //m
    float altitude_min;     //m
    float climb_altitude;   //m，累计爬升
    float climb_distance;   //m，爬坡里程
    uint16_t step_freq_avg; //平均步频
    uint32_t total_step;    //总步数

    uint8_t heart_avg;      //平均心率，等于0则表示没有心率数据。

} cod_sport_info;

typedef struct {
    uint32_t id;            //记录ID，唯一的标明一条记录，自增。到最大值后+1回到0。
    cod_ble_sport_type sport_type;
    float distance;         //总距离，m
    uint32_t total_time;    //包括暂停，s
    uint32_t sport_time;    //不包括暂停，s
    uint64_t start_time;    //起始时间戳，ms
    uint64_t end_time;      //结束时间戳，ms
    float speed_max;        //m/s
    float speed_avg;        //m/s
    float calorie;          //大卡

    uint32_t half_marathon_time;    //半马时间，不包括暂停，s
    uint32_t full_marathon_time;    //全马时间，不包括暂停，s

    uint16_t max_oxygen_uptake;     //最大摄氧量。TODO 运动结束的时候由外部提供
    uint8_t version_h;              //运动记录时固件版本号(注意不是上传时版本号),大版本
    uint8_t version_l;              //运动记录时固件版本号(注意不是上传时版本号),小版本

    uint64_t agps_update_time;      //离线星历更新时间戳ms，如果没有离线星历，填2018年1月1日0时0分0秒

    uint32_t total_step;            //总步数
    uint8_t heart_avg;              //平均心率
    float climb_attitude;           //累积爬升
    float climb_distance;           //爬坡距离

} cod_sport_summary;

typedef struct {
    uint32_t id;            //记录ID，唯一的标明一条记录，自增。到最大值后+1回到0。
    cod_ble_swim_type swim_type;
    float distance;         //总距离，m
    uint32_t total_time;    //包括暂停，s
    uint32_t sport_time;    //不包括暂停，s
    uint64_t start_time;    //起始时间戳，ms
    uint64_t end_time;      //结束时间戳，ms
    float calorie;          //大卡
    cod_ble_swim_stroke stroke;

    uint16_t laps;          //趟数
    uint16_t pool_length;   //泳池长度，单位米。

    uint32_t total_strokes; //总划水次数
    uint8_t stroke_freq;    //划水频率，单位 次/分
    uint16_t pace_avg;      //平均配速(100米用时)，单位秒。
    uint16_t swolf_avg;     //平均SWOLF

} cod_swim_summary;

/**
 * 游泳泳姿点信息
 */
typedef struct {
    uint64_t time;              //手表系统时间戳，而非gps时间
    cod_ble_swim_stroke stroke;
    uint8_t lap_id;             //趟序号,第一趟为0，255后又回到0即可
} cod_swim_stroke_point;

/**
 * GPS点，26bytes
 */
typedef struct {
    uint64_t time;              //手表系统时间戳，而非gps时间
    float latitude;             //纬度，度，精确至小数点第5位，
    float longitude;            //经度，度，精确至小数点第5位，
    float altitude;             //高度，m
    uint8_t accuracy;           //定位精度，1到128m

    float distance2pre_point;   //距离上一点距离 单位米。
    float merged_distance;      //合并点累计的距离 单位米。
    cod_gps_point_type point_type;  //点类型

    uint32_t to_start_time;     //距离起点时间 秒。不包括暂停时间.外部不需要set，可以get
    uint32_t to_pre_time;       //距离上一点时间 秒。不包括暂停时间.外部不需要set，可以get
} cod_gps_point;

/**
 * 公里牌
 */
typedef struct {
    uint32_t total_time;        //本公里用时 单位秒。不包括暂停，s
    float latitude;             //本公里达成时的纬度, 精确至小数点第5位，如果是步数记录的距离不存在经纬度则传256。
    float longitude;            //本公里达成时的经度, 精确至小数点第5位，如果是步数记录的距离不存在经纬度则传256。
} cod_kilometer_record;

/**
 * 心率
 */
typedef struct {
    uint64_t time;      //时间戳，ms
    uint8_t heart_rate; //单位:bpm
} cod_heart_rate;

/**
 * 每分钟步数信息
 */
typedef struct {
    uint64_t start_time;    //起始时间戳，ms
    uint16_t step;          //此分钟步数 步
    float distance;         //此分钟距离 m
} cod_step_min;

typedef enum {
    COD_SP_INPUT_DATA_TYPE_NONE = 0,
    COD_SP_INPUT_DATA_TYPE_GPS,
    COD_SP_INPUT_DATA_TYPE_STEP,
    COD_SP_INPUT_DATA_TYPE_HEART,
    COD_SP_INPUT_DATA_TYPE_KILOMETER_RECORD
} cod_sport_input_data_type;
#endif //COD_SDK_COD_SPORT_BEAN_H
