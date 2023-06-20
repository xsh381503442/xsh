//
// Created by Paint on 2019-08-19.
//

#ifndef COD_SDK_COD_BYTE_BUFFER_H
#define COD_SDK_COD_BYTE_BUFFER_H

#include <stdint.h>
#include <stdbool.h>

/**
 * 根据read_pos可以对数据区进行便利读取
 */
typedef struct {
    uint8_t* buffer;        //数据区
    uint16_t len;           //数据区长度
    uint16_t read_pos;      //当前读写的问题
} cod_ble_byte_buffer;

// ======================= for byte buffer =========================
/**
 * 用data初始化一个buffer
 * @param buffer
 * @param data
 * @param len
 * @return
 */
void cod_byte_buffer_init(cod_ble_byte_buffer* buffer, uint8_t* data, uint16_t len);

/**
 * 重置read_pos
 * @param buffer
 */
void cod_byte_buffer_rewind(cod_ble_byte_buffer* buffer);

uint16_t cod_byte_buffer_remain_read(cod_ble_byte_buffer* buffer);

/**
 * 从buffer的当前读取位置，读取指定长度的字节到output
 * @param buffer
 * @param len
 * @param output
 * @return 读取操作是否成功
 */
bool cod_byte_buffer_read(cod_ble_byte_buffer* buffer, uint16_t len, uint8_t* output);

/**
 * 从buffer的当前读取位置读取一个bool
 */
bool cod_byte_buffer_read_bool(cod_ble_byte_buffer* buffer);
/**
 * 从buffer的当前读取位置读取一个enum值，并用最大和最小值限制它。
 * 如果枚举有增删，需要记得调整最大或最小值。
 */
uint8_t cod_byte_buffer_read_enum(cod_ble_byte_buffer* buffer, uint8_t min, uint8_t max);

/**
 * 从buffer的当前读取位置读取一个uint8
 */
uint8_t cod_byte_buffer_read_uint8(cod_ble_byte_buffer* buffer);
/**
 * 从buffer的当前读取位置读取一个int16
 */
uint16_t cod_byte_buffer_read_int16(cod_ble_byte_buffer* buffer);
/**
 * 从buffer的当前读取位置读取一个uint16
 */
uint16_t cod_byte_buffer_read_uint16(cod_ble_byte_buffer* buffer);
/**
 * 从buffer的当前读取位置读取一个uint32
 */
uint32_t cod_byte_buffer_read_uint32(cod_ble_byte_buffer* buffer);

/**
 * 从buffer的当前读取位置读取一个int8
 */
int8_t cod_byte_buffer_read_int8(cod_ble_byte_buffer* buffer);

/**
 * 从buffer的当前读取位置读取一个varint
 */
int32_t cod_byte_buffer_read_varint(cod_ble_byte_buffer* buffer);

/**
 * 从buffer的当前读取位置跳过指定字节的数据
 * @return skip是否成功。buffer为空，或者剩余字节数不足都会返回false
 */
bool cod_byte_buffer_skip(cod_ble_byte_buffer* buffer, uint8_t skip_num);

/**
 * SDK内部使用，外部一般不要用。
 * 从buffer的当前读取位置读取一个字符串，自动拼接'\0'。
 *
 * @param bytes_of_len 长度占几个字节
 * @return 字符串, NULL表示读取失败。用完之后记得通过cod_free()释放内存。
 */
char* cod_byte_buffer_read_len_char(cod_ble_byte_buffer* buffer, uint8_t bytes_of_len,
        uint16_t* output_actual_read_len);

/**
 * SDK内部使用，外部一般不要用。
 * 从buffer的当前读取位置读取一个变长字符串，自动拼接'\0'。
 *
 * @return 字符串, NULL表示读取失败。用完之后记得通过cod_free()释放内存。
 */
char* cod_byte_buffer_read_varlen_char(cod_ble_byte_buffer* buffer, uint16_t* output_actual_read_len);

#endif //COD_SDK_COD_BYTE_BUFFER_H
