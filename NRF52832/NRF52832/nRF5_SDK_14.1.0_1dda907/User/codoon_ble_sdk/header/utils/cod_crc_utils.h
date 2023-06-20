//
// Created by Paint on 2019-07-25.
//

#ifndef COD_SDK_COD_CRC_UTILS_H
#define COD_SDK_COD_CRC_UTILS_H

#define COD_CRC_INIT  0xffffffffL
#define COD_CRC_XOROT 0xffffffffL


#include <stdint.h>
#include "cod_crc_utils.h"
#include "stdbool.h"

typedef struct {
    uint32_t _crc32;
    bool _has_calculated;
}cod_crc32_calculator;

uint8_t cod_get_sum_check(uint8_t* p_data, uint16_t len);

/**
 * 初始化CRC32计算器
 * @param calculator
 */
void cod_crc32_init(cod_crc32_calculator* calculator);

/**
 * 往CRC32计算器里添加数据，可以多次添加
 * @param calculator
 * @param data
 * @param size
 */
void cod_crc32_update(cod_crc32_calculator* calculator, uint8_t *data, uint32_t size);

/**
 * 从CRC32计算器里取出最终的CRC32计算值
 * @param calculator
 * @return
 */
uint32_t cod_crc32_get_final(cod_crc32_calculator* calculator);

/**
 * 直接获取本次数据的CRC32
 * @param data
 * @param size
 * @return
 */
uint32_t cod_get_crc32_direct(uint8_t *data, uint32_t size);

#endif //COD_SDK_COD_CRC_UTILS_H
