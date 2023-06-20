//
// Created by Paint on 2019-11-08.
//

#ifndef COD_SDK_COD_SPORT_DATA_CONVERTER_H
#define COD_SDK_COD_SPORT_DATA_CONVERTER_H


#include <stdint.h>
#include "../cod_ble_bean.h"
#include "cod_sport_bean.h"

/**
 * 1、运动数据分为两部分，summary+详细数据。summary在详细数据之前。
 * 由于数据是实时转化，所以详细数据的转化回调on_sport_detail_data_converted会先被调用。
 * 最后才会调用on_sport_summary_converted。
 * 这一点保存或者是发送数据的时候需要注意。
 * 2、数据转化会根据buffer_size的设置，当buffer满了之后，就会把buffer的数据回调出来。
 * 3、输入gps、step、heart等数据，理论上可以交叉输入，但最好是输入完某类型数据之后，再输入下一个类型的数据。
 * 这样转化出来的数据占用的空间会更小。不同的输入顺序转化出来的数据大小不一样。
 */
typedef struct {

    /**
     * 运动详细数据转化回调，包括gps、计步、心率、公里牌.
     * 每次回调的data_len为初始化时候设置的buffer_size，如果不足buffer_size，则回调实际的大小。
     */
    void (*on_sport_detail_data_converted)(uint8_t *data, uint16_t data_len);

    /**
     * 运动summary数据转化回调。
     * 一次性回调所有的summary数据，data_len和cod_get_sport_summary_data_size()一致。
     */
    void (*on_sport_summary_converted)(uint8_t *data, uint16_t data_len);
} cod_sport_data_convert_callback;


/**
 * 数据转化器。结构体内部的数据，仅供SDK内部使用，外部只需要等待callback回调数据即可。
 */
typedef struct {
    uint16_t _buffer_size;
    cod_sport_data_convert_callback *_callback;
    cod_ble_buffer *_convert_data;
    cod_ble_buffer *_summary_data;
    cod_ble_buffer *_temp_buffer;
    cod_sport_input_data_type _last_input_type;
    uint32_t _last_input_count;
    uint32_t _last_input_total_size;
    cod_gps_point _last_gps;
    cod_step_min _last_step;
    cod_heart_rate _last_heart;
    cod_kilometer_record _last_kilometer_record;
    uint32_t _total_bytes;
} cod_sport_data_converter;

/**
 * 获取summary数据需要占用的空间。方便预留空间。
 * @return
 */
uint32_t cod_get_sport_summary_data_size(void);

 /**
  * 获取转换后的summary数据。
  * @param summary
  * @param total_size 本次运动转换后的详细数据和summary数据的总大小
  * @param output 外部用于接收转换后的summary数据的内存
  * @param output_size 外部用于接收转换后的summary数据的内存大小。必须大于等于cod_get_sport_summary_data_size
  */
void cod_get_sport_summary_data(cod_sport_summary* summary, uint32_t total_size,
        uint8_t* output, uint32_t output_size);

/**
 * 初始化(数据转化使用动态内存)。需要加上宏COD_DATA_CONV_DYNAMIC_MEM。
 * 两种初始化方法只能二选一
 * @param converter
 * @param buffer_size 当转化出来的数据量满了buffer_size之后，会回调一次on_sport_data_converted。
 *                    最小是12字节。
 * @param callback 数据转化回调
 */
void cod_sport_data_conv_init_by_dynamic(cod_sport_data_converter *converter, uint16_t buffer_size,
                                         cod_sport_data_convert_callback *callback);

/**
 * 初始化(数据转化使用外部内存)。不能定义宏COD_DATA_CONV_DYNAMIC_MEM
 * 两种初始化方法只能二选一
 * @param converter
 * @param buffer 外部buffer
 * @param buffer_size 当转化出来的数据量满了buffer_size之后，会回调一次on_sport_data_converted。
 *                    最小是12字节。
 * @param callback 数据转化回调
 */
void cod_sport_data_conv_init_by_outer_buffer(cod_sport_data_converter *converter,
                                              uint8_t* buffer,
                                              uint16_t buffer_size,
                                              cod_sport_data_convert_callback *callback);

/**
 * 开始转化
 * @param converter
 */
void cod_sport_data_conv_start(cod_sport_data_converter *converter);

/**
 * 输入gps点数组。
 * @param total_list_size 数组总大小
 * @param gps_list 本次输入的数组
 * @param cur_list_size 本次输入的数组大小
 */
void cod_sport_data_conv_input_gps(cod_sport_data_converter *converter, uint32_t total_list_size,
                                   cod_gps_point *gps_list, uint16_t cur_list_size);

/**
 * 输入计步数组.
 * @param total_list_size 数组总大小
 * @param step_list 本次输入的数组
 * @param cur_list_size 本次输入的数组大小
 */
void cod_sport_data_conv_input_step(cod_sport_data_converter *converter, uint32_t total_list_size,
                                    cod_step_min *step_list, uint16_t cur_list_size);

/**
 * 输入心率数组.
 * @param total_list_size 数组总大小
 * @param heart_list 本次输入的数组
 * @param cur_list_size 本次输入的数组大小
 */
void cod_sport_data_conv_input_heart(cod_sport_data_converter *converter, uint32_t total_list_size,
                                     cod_heart_rate *heart_list, uint16_t cur_list_size);

/**
 * 输入公里牌数组.
 * @param total_list_size 数组总大小
 * @param kr_list 本次输入的数组
 * @param cur_list_size 本次输入的数组大小
 */
void cod_sport_data_conv_input_kilometer_record(cod_sport_data_converter *converter,
                                                uint32_t total_list_size,
                                                cod_kilometer_record *kr_list,
                                                uint16_t cur_list_size);

/**
 * 结束转化
 */
void cod_sport_data_conv_finish(cod_sport_data_converter *converter, cod_sport_summary *summary);

#endif //COD_SDK_COD_SPORT_DATA_CONVERTER_H
