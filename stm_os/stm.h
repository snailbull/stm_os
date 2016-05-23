#ifndef _STM_H_
#define _STM_H_

typedef struct TEvt
{
    signal_t sig;
    void     *para;
} TEvt;

typedef struct TMsm TMsm;
typedef uint8_t (*state_func_t) (TMsm *me, TEvt *e);

typedef struct TMsmVtbl TMsmVtbl;

struct TMsmVtbl
{
    void (*init)(TMsm *me, TEvt *e);		/* hsm_init or fsm_init */
    void (*dispatch)(TMsm *me, TEvt *e);	/* hsm_dispatch or fsm_dispatch */
};

struct TMsm
{
    const TMsmVtbl *vptr;
    state_func_t state;
    state_func_t temp;
};

enum STMReturn
{
    STM_SUCCESS = 0,
    STM_RET_HANDLED,
    STM_RET_IGNORED,			/* discard this event now */
    STM_RET_TRAN,
    STM_RET_FATHER,
    STM_RET_UNHANDLED = 5		/* move up to father state, to find event case to excute it. */
};

enum STMRsvdSig
{
    STM_EMPTY_SIG = 0,			/* just trig to father state. */
    STM_ENTRY_SIG,
    STM_EXIT_SIG,
    STM_INIT_SIG,
    STM_TICK_SIG,
    STM_USER_SIG = 5
};

#define STM_MAX_NEST_DEPTH      6

#define STM_STATE_CAST(handler) ( (state_func_t)(handler) )

#define STM_TRAN(state)         ( ((TMsm *)me)->temp = STM_STATE_CAST(state), STM_RET_TRAN )

#define STM_FATHER(father)      ( ((TMsm *)me)->temp = STM_STATE_CAST(father),  STM_RET_FATHER )

#define STM_TRIG(state, sig)    ( (*(state))(me, &STMGlobalEvent[sig]) )

#define STM_ENTER(state)        STM_TRIG(state, STM_ENTRY_SIG)

#define STM_EXIT(state)         STM_TRIG(state, STM_EXIT_SIG)


void        fsm_init    (TMsm *me, TEvt *e);
void        fsm_dispatch(TMsm *me, TEvt *e);
void        fsm_ctor    (TMsm *me, state_func_t initial);
uint8_t     hsm_top     (TMsm *me, TEvt *e);
void        hsm_init    (TMsm *me, TEvt *e);
void        hsm_dispatch(TMsm *me, TEvt *e);
void        hsm_ctor    (TMsm *me, state_func_t initial);
uint8_t is_hsm_in_state (TMsm *me, state_func_t state);

#endif
