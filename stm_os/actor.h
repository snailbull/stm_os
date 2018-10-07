#ifndef _ACTOR_H_
#define _ACTOR_H_

#define STM_MSG_NEST_DEPTH		8

#define STM_DEBUG_ON
#ifdef STM_DEBUG_ON
#define ACT_PRINTF(format, ...)    printf("[actor.c,%d]:" format "\r\n", __LINE__, ##__VA_ARGS__)
#else
#define ACT_PRINTF(format, ...)
#endif

#include <semaphore.h>
#include <pthread.h>

typedef struct
{
    stm_t me;
    msg_t *queue;
    uint8_t size;
    uint8_t head;
    uint8_t tail;
    uint8_t used;
    uint8_t nesting_cnt;
    pthread_mutex_t mutex;  /* protect actor data. */
    list_t list;            /* protect by s_actor_mutex */
} actor_t;

enum
{
    SEND_TO_FRONT = 0,
    SEND_TO_BACK,
};

actor_t *actor_add(stm_func_t init, uint8_t size, uint8_t opt);
void actor_del(actor_t *act);
uint8_t actor_post_message(actor_t *act, int sig, void *para, uint8_t opt);
uint8_t actor_send_message(actor_t *act, int sig, void *para);
void actor_dispatch(void);
void actor_sleep(void);
void actor_wakeup(void);

#endif
