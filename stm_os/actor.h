#ifndef _ACTOR_H_
#define _ACTOR_H_

#include <semaphore.h>
#include <pthread.h>

typedef struct {
	stm_t me;
	evt_t *queue;
	uint8_t size;
	uint8_t head;
	uint8_t tail;
	uint8_t used;
	uint8_t nesting_cnt;
	pthread_mutex_t mutex;  /* protect actor data. */
	list_t list;            /* protect by s_actor_mutex */
} actor_t;

enum {
	SEND_TO_FRONT = 0,
	SEND_TO_BACK,
};

actor_t *actor_create(stm_func_t init, uint8_t size, uint8_t opt);
void actor_del(actor_t *act);
uint8_t actor_post_message(actor_t *act, int evt, void *para, uint8_t opt);
uint8_t actor_send_message(actor_t *act, int evt, void *para);
void actor_dispatch(void);
void actor_sleep(void);
void actor_wakeup(void);

#endif
