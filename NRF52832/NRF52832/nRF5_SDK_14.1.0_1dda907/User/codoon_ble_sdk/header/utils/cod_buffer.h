//
// Created by Paint on 2019-07-25.
//

#ifndef COD_SDK_COD_BUFFER_H
#define COD_SDK_COD_BUFFER_H

#include <stdint.h>
#include <stdbool.h>

#define DEFAULT_DYNAMIC_INIT_BUFFER_SIZE 64
#define COD_MALLOC(size, type) cod_malloc(type, size, __FUNCTION__, __LINE__)
#define COD_FIX_BUFFER_NEW(size) cod_fix_buffer_new(size, __FUNCTION__, __LINE__)
#define COD_DYNAMIC_BUFFER_NEW cod_dynamic_buffer_new(DEFAULT_DYNAMIC_INIT_BUFFER_SIZE, __FUNCTION__, __LINE__)


/**
 * 自动扩容的buffer
 */
typedef struct {
    uint8_t* buffer;        //数据区
    uint16_t len;           //数据区实际长度
    uint8_t _flag;          //SDK内部使用。bit 0表示是否自动扩容，bit 1表示是否是外部空间
    uint16_t _capacity;     //数据区总长度。SDK内部使用。
} cod_ble_buffer;

/**
 * 咕咚内存分配，会自动内存统计
 * @param size
 * @return
 */
void* cod_malloc(const char* type, uint32_t size, const char *p_func, unsigned int func_line);

/**
 * 咕咚内存释放，会自动内存统计
 * @param buffer
 */
void cod_free(void* buffer);

/**
 * 用于检测内存泄漏
 * @return 还未释放的内存分配次数
 */
int32_t cod_get_malloc_count(void);


/**
 * 用outer_space去初始化一个cod_ble_buffer
 * @param buffer
 * @param outer_space
 * @param len
 */
void cod_fix_buffer_with_outer_space(cod_ble_buffer* buffer, uint8_t* outer_space, uint32_t len);

/**
 * 申请一个咕咚固定容量buffer，不自动扩容。
 * 请通过宏COD_FIX_BUFFER_NEW来申请
 * ## 如果是第三方自己申请的buffer，记得要通过cod_buffer_release释放 ##
 * @param len
 * @return
 */
cod_ble_buffer* cod_fix_buffer_new(uint32_t len, const char *p_func, unsigned int func_line);

/**
 * 申请一个咕咚动态buffer，自动扩容。
 * 请通过宏COD_DYNAMIC_BUFFER_NEW来申请
 * ## 如果是第三方自己申请的buffer，记得要通过cod_buffer_release释放 ##
 * @param len
 * @return
 */
cod_ble_buffer* cod_dynamic_buffer_new(uint32_t len, const char *p_func, unsigned int func_line);

/**
 * 如果buffer空间不够，会自动扩容。扩容因子默认是0.75
 * @param buffer
 * @param p_data
 * @param start
 * @param len
 * @return
 */
int8_t cod_buffer_add(cod_ble_buffer *buffer, const uint8_t *p_data, uint16_t start, uint16_t len);

/**
 * 是否是自动扩容
 * @param buffer
 * @return
 */
bool cod_buffer_is_auto_extend(cod_ble_buffer *buffer);

/**
 * 是否使用的外部空间，这样就不用内部release
 * @param buffer
 * @return
 */
bool cod_buffer_is_use_outer_space(cod_ble_buffer *buffer);

/**
 * 释放内存，对于动态分配会free空间
 * @param buffer
 */
void cod_buffer_release(cod_ble_buffer *buffer);

/**
 * clear内存，len置0
 * @param buffer
 */
void cod_buffer_clear(cod_ble_buffer *buffer);

/**
 * buffer空间是否已满。只有固定空间才有可能已满。
 * @param buffer
 * @return
 */
bool cod_buffer_is_full(cod_ble_buffer *buffer);



#endif //COD_SDK_COD_BUFFER_H
