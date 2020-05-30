#include "stm_os.h"


#define STM_MSG_NEST_DEPTH		8

#define STM_DEBUG_ON
#ifdef STM_DEBUG_ON
#define ACT_PRINTF(format, ...)    printf("[actor.c,%d]:" format "\r\n", __LINE__, ##__VA_ARGS__)
#else
#define ACT_PRINTF(format, ...)
#endif


static list_t s_actor_head = {&s_actor_head, &s_actor_head};
static int s_msg_total;
static sem_t s_actor_sem;
static pthread_mutex_t s_actor_mutex;

void actor_init(void)
{
	if (sem_init(&s_actor_sem, 0, 0) == -1) {
		ACT_PRINTF("s_actor_sem err!");
	}
	pthread_mutex_init(&s_actor_mutex, NULL);

	pthread_mutex_lock(&s_actor_mutex);
	s_msg_total = 0;
	pthread_mutex_unlock(&s_actor_mutex);
}

actor_t *actor_create(stm_func_t init, uint8_t size, uint8_t opt)
{
	actor_t *act;

	act = (actor_t *)os_malloc(sizeof(actor_t));
	if (act == NULL) {
		ACT_PRINTF("act mem err!");
		return NULL;
	}
	act->queue = (evt_t *)os_malloc(sizeof(evt_t) * size);
	if (act->queue == NULL) {
		os_free(act);
		act = NULL;
		ACT_PRINTF("queue mem err!");
		return NULL;
	}
	pthread_mutex_init(&act->mutex, NULL);
	act->size = size;
	act->head = 0;
	act->tail = 0;
	act->used = 0;
	if (opt == 0) {
		fsm_ctor(&act->me, init);
	} else {
		hsm_ctor(&act->me, init);
	}
	list_insert(&s_actor_head, &act->list);
	return act;
}

void actor_del(actor_t *act)
{
	pthread_mutex_lock(&s_actor_mutex);
	list_delete(&act->list);
	pthread_mutex_unlock(&s_actor_mutex);

	pthread_mutex_destroy(&act->mutex);
	os_free(act->queue);
	os_free(act);
}

/*
 * Post event to specify active object
 *
 * @act   active object
 * @e     signal num
 * @para  signal param
 * @opt   send to front or back of msg queue, SEND_TO_FRONT  SEND_TO_BACK
 *
 * Note(s) it might be called in interrupt. MESSAGE WILL PROCESSING ASYNCHRONOUS.
 */
uint8_t actor_post_message(actor_t *act, int evt, void *para, uint8_t opt)
{
	pthread_mutex_lock(&act->mutex);
	if (act->used >= act->size) {
		pthread_mutex_unlock(&act->mutex);
		return ERR_QUEUE_FULLED;
	}

	if (opt == SEND_TO_BACK) {
		act->queue[act->tail].evt = evt;
		act->queue[act->tail].para = para;
		act->tail++;
		if (act->tail >= act->size) {
			act->tail = 0;
		}
	} else {
		if (act->head == 0) {
			act->head = act->size;
		}
		act->head--;
		act->queue[act->head].evt = evt;
		act->queue[act->head].para = para;
	}
	act->used++;
	pthread_mutex_unlock(&act->mutex);

	pthread_mutex_lock(&s_actor_mutex);
	s_msg_total++;
	pthread_mutex_unlock(&s_actor_mutex);

	actor_wakeup();
	return ERR_SUCCESS;
}

/*
 * Send event to specify active object
 *
 * @act   active object
 * @evt   signal num
 * @para  signal param
 *
 * Note(s) it might be called in interrupt. MESSAGE WILL PROCESSING SYNCHRONOUS.
 */
uint8_t actor_send_message(actor_t *act, int evt, void *para)
{
	evt_t e = {evt, para};

	pthread_mutex_lock(&act->mutex);
	act->nesting_cnt++;	/* ++ and -- always in pairs */
	if (act->nesting_cnt > STM_MSG_NEST_DEPTH) {
		if (act->nesting_cnt) {
			act->nesting_cnt--;
		}
		pthread_mutex_unlock(&act->mutex);
		return ERR_NESTING_FULLED;
	}
	pthread_mutex_unlock(&act->mutex);

	/* recursive dispatch */
	act->me.vptr->dispatch(&act->me, &e);

	pthread_mutex_lock(&act->mutex);
	if (act->nesting_cnt) {
		act->nesting_cnt--;
	}
	pthread_mutex_unlock(&act->mutex);

	return ERR_SUCCESS;
}

/**
 * roll poling dispatch
 */
void actor_dispatch(void)
{
	actor_t *t;
	list_t *head;
	list_t *iter;
	evt_t e;

	pthread_mutex_lock(&s_actor_mutex);
	if (s_msg_total > 0) {
		head = &s_actor_head;
		iter = head->next;
		pthread_mutex_unlock(&s_actor_mutex);

		while (iter && (iter != head)) {
			t = list_entry(iter, actor_t, list);

			pthread_mutex_lock(&t->mutex);
			if (t->used > 0) {
				/* pick a event from queue head. */
				t->used--;

				pthread_mutex_lock(&s_actor_mutex);
				s_msg_total--;
				pthread_mutex_unlock(&s_actor_mutex);

				e.evt = t->queue[t->head].evt;
				e.para = t->queue[t->head].para;
				t->head++;
				if (t->head >= t->size) {
					t->head = 0;
				}
				pthread_mutex_unlock(&t->mutex);

				t->me.vptr->dispatch(&t->me, &e);
			} else {
				pthread_mutex_unlock(&t->mutex);
			}

			iter = iter->next;
		}
	} else {
		pthread_mutex_unlock(&s_actor_mutex);
		actor_sleep();
	}
}

void actor_sleep(void)
{
	sem_wait(&s_actor_sem);
}

void actor_wakeup(void)
{
	sem_post(&s_actor_sem);
}
