//
// Created by Paint on 2019/7/22.
//

#ifndef COD_SDK_BYTES_UTILS_H
#define COD_SDK_BYTES_UTILS_H

#include "stdint.h"
#include "../cod_ble_constants.h"

int32_t cod_bytes2int(const uint8_t *input, uint16_t start, uint16_t len, cod_byte_order order);

uint32_t cod_bytes2uint(const uint8_t *input, uint16_t start, uint16_t len, cod_byte_order order);

void cod_int2bytes(const int32_t input, uint8_t len, cod_byte_order order, uint8_t *output);

void cod_uint2bytes(const uint32_t input, uint8_t len, cod_byte_order order, uint8_t *output);

/**
 *  将二进制输出为16机制字符串，并以空格分割
 * @param input
 * @param start
 * @param len
 * @param output
 */
uint32_t cod_bytes2hexstr(const uint8_t *input, uint16_t start, int32_t len, char *output);

/**
 * 将字节数组复制到output，并添加\0结尾。所以output的长度要比input大一
 * @param input
 * @param start
 * @param len
 * @param output
 */
void cod_bytes2str(const uint8_t *input, uint16_t start, int32_t len, char *output);

uint8_t cod_byte2enum(uint8_t value, uint8_t min, uint8_t max);

#endif //COD_SDK_BYTES_UTILS_H
