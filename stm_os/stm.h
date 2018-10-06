#ifndef _STM_H_
#define _STM_H_

typedef struct msg_t
{
    int sig;
    void *para;
} msg_t;

typedef struct stm_t stm_t;
typedef struct stm_vtbl_t stm_vtbl_t;
typedef uint8_t (*stm_func_t) (stm_t *me, msg_t *e);

struct stm_vtbl_t
{
    void (*init)(stm_t *me, msg_t *e);		/* hsm_init or fsm_init */
    void (*dispatch)(stm_t *me, msg_t *e);	/* hsm_dispatch or fsm_dispatch */
};

struct stm_t
{
    const stm_vtbl_t *vptr;
    stm_func_t state;
    stm_func_t temp;
};

enum
{
    STM_RET_OK = 0,
    STM_RET_HANDLED,
    STM_RET_IGNORED,			/* discard this event now */
    STM_RET_TRAN,               /* transition state */
    STM_RET_FATHER,
    STM_RET_UNHANDLED = 5		/* move up to father state, to find event case to excute it. */
};

enum
{
    STM_EVT_EMPTY = 0,			/* just trig to father state. */
    STM_EVT_ENTRY,
    STM_EVT_EXIT,
    STM_EVT_INIT,
    STM_EVT_TICK,
    STM_EVT_USER = 5
};

#define STM_MAX_NEST_DEPTH      8

#define STM_TRAN(state)         (me->temp = state,  STM_RET_TRAN)
#define STM_FATHER(father)      (me->temp = father, STM_RET_FATHER)

#define STM_TRIG(state, sig)    ( (*(state))(me, &s_stm_global_evt[sig]) )
#define STM_ENTER(state)        STM_TRIG(state, STM_EVT_ENTRY)
#define STM_EXIT(state)         STM_TRIG(state, STM_EVT_EXIT)


#define STM_DEBUG_ON
#ifdef STM_DEBUG_ON
#define STM_PRINTF(format, ...)    printf("[stm.c,%d]:" format "\r\n", __LINE__, ##__VA_ARGS__)
#define STM_ASSERT(EX)                                                       \
	if (!(EX))                                                                \
	{                                                                         \
		volatile char dummy = 0;                                              \
		/* CPU_SR_Save(); */                                                        \
		STM_PRINTF("(%s) assert failed at %s:%d \n", #EX, __FUNCTION__, __LINE__);\
		while (dummy == 0);                                                   \
	}
#else
#define OS_PRINTF(format, ...)
#define STM_ASSERT(ex)
#endif

void    fsm_init    (stm_t *me, msg_t *e);
void    fsm_dispatch(stm_t *me, msg_t *e);
void    fsm_ctor    (stm_t *me, stm_func_t init);
uint8_t hsm_top     (stm_t *me, msg_t *e);
void    hsm_init    (stm_t *me, msg_t *e);
void    hsm_dispatch(stm_t *me, msg_t *e);
void    hsm_ctor    (stm_t *me, stm_func_t init);
uint8_t hsm_in_state(stm_t *me, stm_func_t state);

#endif
