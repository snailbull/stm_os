#ifndef _TIMER_H_
#define _TIMER_H_

/*
 * timer flag
 */
#define TIMER_FLAG_STOP			0x00
#define TIMER_FLAG_START		0x01
#define TIMER_FLAG_ONESHOT		0x00
#define TIMER_FLAG_REPEAT		0x02

typedef uint8_t (*timer_func_t)(uint32_t counter);

typedef struct
{
    stm_t *me;
    list_t list;
    uint8_t flag;
    uint32_t timeout;		/* ms */
    uint32_t reload_timeout;
    evt_t e;
} evtimer_t;

typedef struct
{
    list_t list;
    uint8_t flag;
    uint32_t timeout;
    uint32_t reload_timeout;
    uint32_t counter;
    timer_func_t func;
} cbtimer_t;

enum
{
    TIMER_RET_INC = 0,
    TIMER_RET_CLR,
    TIMER_RET_DEL,
};

uint8_t evtimer_add(stm_t *me, evt_t sig, void *para, uint32_t ms, uint8_t flag);
uint8_t evtimer_del(stm_t *me, evt_t sig);
uint8_t evtimer_set(stm_t *me, evt_t sig, uint8_t flag);
void evtimer_update(uint32_t elapse_ms);
uint8_t cbtimer_add(timer_func_t func, uint32_t ms, uint8_t flag);
uint8_t cbtimer_del(timer_func_t func);
uint8_t cbtimer_set(timer_func_t func, uint8_t flag);
void cbtimer_update(uint32_t elapse_ms);

#endif
