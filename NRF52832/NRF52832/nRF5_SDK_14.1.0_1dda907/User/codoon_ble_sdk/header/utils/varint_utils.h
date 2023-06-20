//
// Created by Paint on 2019/7/22.
//

#ifndef COD_SDK_VARINT_UTILS_H
#define COD_SDK_VARINT_UTILS_H

#include <stdint.h>
#include "cod_byte_buffer.h"

/**
 * SDK内部使用
 * @param v
 */
uint8_t cod_int2varint(int32_t v, uint8_t* output, uint8_t output_len);

/**
 * SDK内部使用
 * @param v
 */
int32_t cod_varint2int(uint8_t* input, uint8_t input_len);

#endif //COD_SDK_VARINT_UTILS_H
