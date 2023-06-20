//
// Created by Paint on 2019-10-14.
//

#ifndef COD_SDK_COD_RING_ARRAY_H
#define COD_SDK_COD_RING_ARRAY_H

#include <stdint.h>
#include <stdbool.h>

/**
 * 环形数组，可以代替list。
 * 优点是一次性申请内存或者用静态区内存（避免动态申请潜在失败），索引快。
 * 缺点是不能动态扩容。
 * 注意：目前先配合静态内存使用，动态一次性申请的动态内存连续性还未验证过。
 */
typedef struct {
    void *elements;      //item数据，SDK内部使用，不要修改
    uint16_t capacity;   //array的item容量，SDK内部使用，不要修改
    uint16_t item_count; //array中已有的item数量，SDK内部使用，不要修改
    uint16_t _item_size; //单个item占用大小，SDK内部使用，不要修改
    uint16_t _head;      //item的起始pos，SDK内部使用，不要修改
    uint16_t _tail;      //item的结束pos，SDK内部使用，不要修改
    bool _use_outer_space;//使用外部的空间存储item
} cod_ring_array;

/**
 *
 * @param array
 * @param capacity array的item容量
 * @param item_size 单个item占用大小
 * @param space 此空间大小应该等于capacity*item_size，否则会出问题。
 * @return
 */
bool cod_ring_array_init_with_exist_space(cod_ring_array *array, uint16_t capacity,
                                          uint16_t item_size, void *space);

bool cod_ring_array_init(cod_ring_array *array, uint16_t capacity, uint16_t item_size);

bool cod_ring_array_is_full(cod_ring_array *array);

bool cod_ring_array_is_empty(cod_ring_array *array);

void cod_ring_array_clear(cod_ring_array *array);

uint16_t cod_ring_array_get_size(cod_ring_array *array);

bool cod_ring_array_put(cod_ring_array *array, void *element);

uint16_t cod_ring_array_remove_from_front(cod_ring_array *array, uint16_t count);

uint16_t cod_ring_array_remove_from_end(cod_ring_array *array, uint16_t count);

bool cod_ring_array_remove_by_index(cod_ring_array *array, uint16_t index);

void *cod_ring_array_get(cod_ring_array *array, uint8_t index);

void *cod_ring_array_get_last(cod_ring_array *array);

void cod_ring_array_release(cod_ring_array *array);

#endif //COD_SDK_COD_RING_ARRAY_H
