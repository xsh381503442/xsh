//
// Created by Paint on 2019/7/23.
// copy from osip lib
//

#ifndef COD_SDK_COD_LIST_H
#define COD_SDK_COD_LIST_H

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

/**
 * Structure for referencing a node in a cod_list_t element.
 * @var __node_t
 */
typedef struct __node __node_t;

/**
 * Structure for referencing a node in a cod_list_t element.
 * @struct __node
 */
struct __node {
    __node_t *next;         /**< next __node_t containing element */
    void *element;          /**< element in Current node */
};

/**
 * Structure for referencing a list of elements.
 * @var cod_list_t
 */
typedef struct cod_list cod_list_t;

/**
  * Structure used to iterate list.
  * @var cod_list_iterator_t
  */
typedef struct cod_list_iterator cod_list_iterator_t;

/**
  * Structure used to iterate list.
  * @struct cod_list_iterator
  */
struct cod_list_iterator {
    __node_t *actual; /**< actual */
    __node_t **prev;  /**< prev */
    cod_list_t *li;  /**< li */
    int pos;          /**< pos */
};

/**
 * list中元素的释放函数
 */
typedef void (*cod_list_free_func_t) (void*);

/**
 * 必须使用COD_LIST_NEW宏来创建list
 * Structure for referencing a list of elements.
 * @struct cod_list
 */
struct cod_list {

    int num;                 /**< Number of element in the list */
    __node_t *node;             /**< Next node containing element  */
    uint8_t __list_init_check;     /** SDK内部用来 */
    cod_list_free_func_t __free_func;
};

cod_list_t * cod_list_new(void);

/**
 * 列表初始化，必须。
 * @param li 列表
 * @param free_func list中元素的释放函数指针。
 *        如果list中的元素内部没有动态分配的内存，可以传NULL进来，这时会直接free()。
 */
int cod_list_init (cod_list_t * li, cod_list_free_func_t free_func);


bool __cod_list_check (cod_list_t * li);

/**
 * Get the size of a list of element.
 * @param li The element to work on.
 */
int cod_list_size (const cod_list_t * li);
/**
 * Check if the end of list is detected .
 * @param li The element to work on.
 * @param pos The index of the possible element.
 */
int cod_list_eol (const cod_list_t * li, int pos);

/**
 * Add an element to end of a list.
 * @param li The element to work on.
 * @param element The pointer on the element to add.
 */
int cod_list_add2last (cod_list_t * li, void *element);

/**
 * Add an element in a list.
 * @param li The element to work on.
 * @param element The pointer on the element to add.
 * @param pos the index of the element to add. (or -1 to append the element at the end)
 */
int cod_list_add (cod_list_t * li, void *element, int pos);
/**
 * Get an element from a list.
 * @param li The element to work on.
 * @param pos the index of the element to get.
 */
void *cod_list_get (const cod_list_t * li, int pos);

/**
 * Get last element from a list.
 * @param li The element to work on.
 */
void *cod_list_get_last (const cod_list_t * li);

/**
 * Remove an element from a list.
 * @param li The element to work on.
 * @param pos the index of the element to remove.
 */
int cod_list_remove (cod_list_t * li, int pos);

/**
 * Remove all elements from a list.
 * @param li The element to work on.
 */
int cod_list_remove_all (cod_list_t * li);

/**
 * release list.
 * @param li The element to work on.
 */
int cod_list_release (cod_list_t * li);

/**
 * Check current iterator state.
 * @param it The element to work on.
 */
#define cod_list_iterator_has_elem( it ) ( 0 != (it).actual && (it).pos < (it).li->num )

/**
 * Get first iterator from list.
 * @param li The element to work on.
 * @param it The iterator.
 */
void *cod_list_get_first (const cod_list_t * li, cod_list_iterator_t * it);
/**
 * GEt next iterator.
 * @param it The element to work on.
 */
void *cod_list_get_next (cod_list_iterator_t * it);
/**
 * Remove current iterator.
 * @param it The element to work on.
 */
void *cod_list_iterator_remove (cod_list_iterator_t * it);

#endif //COD_SDK_COD_LIST_H
