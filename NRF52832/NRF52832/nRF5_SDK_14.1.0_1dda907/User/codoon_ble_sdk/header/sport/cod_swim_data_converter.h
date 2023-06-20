//
// Created by Paint on 2019-11-28.
//

#ifndef COD_SDK_COD_SWIM_DATA_CONVERTER_H
#define COD_SDK_COD_SWIM_DATA_CONVERTER_H


#include <stdint.h>
#include "../cod_ble_bean.h"
#include "cod_sport_bean.h"
#include "cod_sport_data_converter.h"

/**
 * 数据转化器。结构体内部的数据，仅供SDK内部使用，外部只需要等待callback回调数据即可。
 */
typedef struct {
    uint16_t _buffer_size;
    cod_sport_data_convert_callback *_callback;
    cod_ble_buffer *_convert_data;
    cod_ble_buffer *_summary_data;
    cod_ble_buffer *_temp_buffer;
    uint32_t _total_bytes;
    cod_swim_stroke_point _last_stroke;
    uint32_t _cur_input_total_list_size;  //for check
    uint32_t _cur_input_list_size;        //for check
} cod_swim_data_converter;

/**
 * 获取summary数据需要占用的空间。方便预留空间。
 * @return
 */
uint32_t cod_get_swim_summary_data_size(void);

/**
 * 获取转换后的summary数据。
 * @param summary
 * @param total_size 本次运动转换后的详细数据和summary数据的总大小
 * @param output 外部用于接收转换后的summary数据的内存
 * @param output_size 外部用于接收转换后的summary数据的内存大小。必须大于等于cod_get_swim_summary_data_size
 */
void cod_get_swim_summary_data(cod_swim_summary *summary, uint32_t total_size,
                               uint8_t *output, uint32_t output_size);

/**
 * 初始化(数据转化使用动态内存)
 * @param converter
 * @param buffer_size 当转化出来的数据量满了buffer_size之后，会回调一次on_swim_data_converted。
 *                    最小是12字节。
 * @param callback 数据转化回调
 */
void cod_swim_data_conv_init_by_dynamic(cod_swim_data_converter *converter, uint16_t buffer_size,
                                        cod_sport_data_convert_callback *callback);

/**·
 * 初始化(数据转化使用外部内存)
 * @param converter
 * @param buffer 外部buffer
 * @param buffer_size 当转化出来的数据量满了buffer_size之后，会回调一次on_swim_data_converted。
 *                    最小是12字节。
 * @param callback 数据转化回调
 */
void cod_swim_data_conv_init_by_outer_buffer(cod_swim_data_converter *converter,
                                             uint8_t *buffer,
                                             uint16_t buffer_size,
                                             cod_sport_data_convert_callback *callback);

/**
 * 开始转化
 * @param converter
 */
void cod_swim_data_conv_start(cod_swim_data_converter *converter);

/**
 * 输入手划水动作数组。
 * @param total_list_size 数组总大小
 * @param stroke_list 本次输入的数组
 * @param cur_list_size 本次输入的数组大小
 */
void cod_swim_data_conv_input_stroke(cod_swim_data_converter *converter, uint32_t total_list_size,
                                     cod_swim_stroke_point *stroke_list, uint16_t cur_list_size);


/**
 * 结束转化
 */
void cod_swim_data_conv_finish(cod_swim_data_converter *converter, cod_swim_summary *summary);


#endif //COD_SDK_COD_SWIM_DATA_CONVERTER_H
