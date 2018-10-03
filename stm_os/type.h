#ifndef _TYPE_H_
#define _TYPE_H_

#if 1
#include <stdint.h>
#else
typedef unsigned char      bool;
typedef signed   char      int8_t;
typedef unsigned char      uint8_t;
typedef signed   short     int16_t;
typedef unsigned short     uint16_t;
typedef signed   long      int32_t;
typedef unsigned long      uint32_t;
typedef unsigned long long uint64_t;
#endif
typedef unsigned char evt_t;

/*
 * error code
 */
enum
{
    ERR_SUCCESS    = 0,
    ERR_NO_MEMORY,
    ERR_NULL_POINTER,
    ERR_NULL_OBJECT,
    ERR_BLOCK_SIZE_ERROR,
    ERR_INVALID_ALIGN,
    ERR_QUEUE_FULLED,
    ERR_ADD_FAILED,
    ERR_DELETE_FAILED,
    ERR_EXISTED_TIMER,
    ERR_NESTING_FULLED,
};


/*
 * Double-link list
 */
typedef struct list_node
{
    struct list_node    *next;
    struct list_node    *prev;
} list_t;

#define list_entry(node, type, member)    ( (type *)( ( uint8_t *)(node) - (uint32_t)(&((type *)0)->member) ) )

/*
 * return TRUE if the list is empty
 */
#define list_is_empty(list)   ((list)->next == (list))

/*
 * List initialization
 */
#define list_init(list_head)  \
            do {\
                (list_head)->next = (list_head);\
                (list_head)->prev = (list_head);\
            } while (0)

/*
 * add elem to node's before.
 */
#define list_insert(node, elem) \
            do {\
                (elem)->prev       = (node)->prev;\
                (elem)->next       = (node);\
                (node)->prev->next = (elem);\
                (node)->prev       = (elem);\
             } while (0)

/*
 * add elem to node's after.
 */
#define list_insert_after(node, elem) \
            do {\
                (elem)->prev       = (node);\
                (elem)->next       = (node)->next;\
                (node)->next->prev = (elem);\
                (node)->next       = (elem);\
             } while (0)

/*
 * delete elem from list
 */
#define list_delete(elem) \
            do {\
                (elem)->prev->next = (elem)->next;\
                (elem)->next->prev = (elem)->prev;\
            } while (0)

#endif
