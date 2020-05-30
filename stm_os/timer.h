#ifndef _TIMER_H_
#define _TIMER_H_

/*
 * timer flag
 */
#define TIMER_FLAG_STOP			0x00
#define TIMER_FLAG_START		0x01
#define TIMER_FLAG_ONESHOT		0x00
#define TIMER_FLAG_REPEAT		0x02

typedef int (*timer_func_t)(uint32_t counter);

typedef struct {
	stm_t *me;
	list_t list;
	int flag;
	int timeout;		/* ms */
	int reload_timeout;
	evt_t e;
} evtimer_t;

typedef struct {
	list_t list;
	int flag;
	int timeout;
	int reload_timeout;
	int counter;
	timer_func_t func;
} cbtimer_t;

enum {
	TIMER_RET_INC = 0,
	TIMER_RET_CLR,
	TIMER_RET_DEL,
};

int evtimer_add(stm_t *me, int e, void *para, int ms, int flag);
int evtimer_del(stm_t *me, int e);
int evtimer_set(stm_t *me, int e, int flag);
void evtimer_update(int elapse_ms);
int cbtimer_add(timer_func_t func, int ms, int flag);
int cbtimer_del(timer_func_t func);
int cbtimer_set(timer_func_t func, int flag);
void cbtimer_update(int elapse_ms);

#endif
