#include <stdio.h>
#include "stm.h"


#define STM_MAX_NEST_DEPTH      8

#define STM_TRIG(state, e)      ( (*(state))(me, &s_stm_global_evt[e]) )

#define STM_DEBUG_ON
#ifdef STM_DEBUG_ON
#define STM_PRINTF(format, ...)    printf("[stm.c,%d]:" format "\r\n", __LINE__, ##__VA_ARGS__)
#define STM_ASSERT(EX)                                                       \
	if (!(EX))                                                                \
	{                                                                         \
		volatile int dummy = 0;                                              \
		STM_PRINTF("(%s) assert failed at %s:%d \n", #EX, __FUNCTION__, __LINE__);\
		while (dummy == 0);                                                   \
	}
#else
#define STM_PRINTF(format, ...)
#define STM_ASSERT(ex)
#endif


static evt_t s_stm_global_evt[4] = {
	{ STM_SIG_EMPTY, 0},
	{ STM_SIG_INIT,  0},
	{ STM_SIG_ENTRY, 0},
	{ STM_SIG_EXIT,  0}
};

/*
 * Init the finit state machine.
 *
 * @me is the state machine
 * @e is the trig event
 *
 * Note(s)
 */
void fsm_init (stm_t *me, evt_t *e)
{
	int ret;
	STM_ASSERT (me->temp != 0);

	ret = (*me->temp) (me, e);          /* do the fsm constructor init function */
	STM_ASSERT (ret == STM_RET_TRAN);	/* transition must happen here */

	STM_TRIG (me->temp, STM_SIG_ENTRY); /* trig the STM_SIG_ENTRY to the new transitioned state */
	me->state = me->temp;               /* change to new state */
}

/*
 * This function is used to exceute the finit state machine.
 * @me is the meta state machine
 * @e is the trig event
 *
 * Note(s)
 */
void fsm_dispatch(stm_t *me, evt_t *e)
{
	int ret;

	STM_ASSERT (me->state == me->temp);	/* State must be stable here */

	ret = (*me->state) (me, e);         	/* exceute the state function with new event */
	if (ret == STM_RET_TRAN) {
		STM_TRIG(me->state, STM_SIG_EXIT);	/* exit the original state */
		STM_TRIG(me->temp, STM_SIG_ENTRY);	/* enter the new state */
		me->state = me->temp;           	/* change to new state */
	}
}

/*
 * fsm constructor
 */
void fsm_ctor(stm_t *me, stm_func_t init)
{
	static stm_vtbl_t vtbl = {fsm_init, fsm_dispatch};		/* set visual table */

	me->vptr = &vtbl;
	me->state = 0;
	me->temp = init;

	STM_TRIG(me->vptr->init, STM_SIG_INIT);
}

/*
 * This function is used to init the finit state machine.
 * @me is the state machine
 * @e is the trig event
 *
 * Note(s)
 */
void hsm_init (stm_t *me, evt_t *e)
{
	int ret;
	int ip;
	stm_func_t path[STM_MAX_NEST_DEPTH]; /* Max nested state levels */
	stm_func_t t = me->state;

	STM_ASSERT (me->temp != 0);
	STM_ASSERT (t == hsm_top);	/* if state is not equal to the hsm top state, just assert */

	ret = (*me->temp) (me, e);          /* do the hsm constructor init function */
	STM_ASSERT (ret == STM_RET_TRAN);	/* transition must happen here */

	/*Becareful STM_SIG_INIT must trig state to the nested children state, otherwise hsm crash*/
	do {
		ip = 0;
		path[0] = me->temp;

		STM_TRIG (me->temp, STM_SIG_EMPTY);/* Find all the father state until to hsm_top */

		while (me->temp != t) {
			++ip;
			path[ip] = me->temp;
			STM_TRIG (me->temp, STM_SIG_EMPTY);
		}

		me->temp = path[0];

		STM_ASSERT (ip < STM_MAX_NEST_DEPTH);

		/* trig STM_SIG_ENTRY from father source state to nested children state */
		do {
			STM_TRIG(path[ip], STM_SIG_ENTRY);
			--ip;
		} while (ip >= 0);

		t = path[0];

		/* trig the STM_SIG_INIT to the new transitioned state, if new transion happened again, then we need do int init again */
	} while (STM_TRIG (t, STM_SIG_INIT) == STM_RET_TRAN);

	me->state = t;                      /* change to new state */
	me->temp  = t;
}

/*
 * Exceute the hsm state machine.
 *
 * @me is the state machine
 * @e is the trig event
 *
 */
void hsm_dispatch(stm_t *me, evt_t *e)
{
	int r;
	int ip, iq;
	stm_func_t path[STM_MAX_NEST_DEPTH];
	stm_func_t s;
	stm_func_t t = me->state;

	STM_ASSERT (t == me->temp);		/* state must be stable here */

	do {
		s = me->temp;
		r = (*s) (me, e);           /* exceute the state function with new event */

		if (r == STM_RET_UNHANDLED) {
			r = STM_TRIG (s, STM_SIG_EMPTY);/* Move up to father state */
		}

		/* move up to the father state to find suitable state to handle the sig */
	} while (r == STM_RET_FATHER);

	/* if state transition happened then process it */
	if (r == STM_RET_TRAN) {
		ip = -1;

		path[0] = me->temp;         /* save the transitioned state */
		path[1] = t;

		/* t is the source state, and s is the state which cause new state change
		   for example s is the father state of t */
		while (t != s) {
			/* if STM_SIG_EXIT is handled, trig STM_SIG_EMPTY to find the father state
			   if STM_SIG_EXIT not handled , then me->temp hold the father state */
			if (STM_TRIG (t, STM_SIG_EXIT) == STM_RET_HANDLED) {
				STM_TRIG (t, STM_SIG_EMPTY);
			}

			t = me->temp;           /* move to one father state up */
		}

		t = path[0];                /* t is the target transition state */

		/* all the following code is try to find the LCA and exit from the source state to LCA state
		   Be careful LCA state is either not entered not exited.
		   all the father state of the target transition state is stored to path from hight to low etc,
		   path[0] is the target transition state */
		if (s == t) {								/* (a) check source==target (transition to self) */
			STM_TRIG (s, STM_SIG_EXIT);
			ip = 0;
		} else {
			STM_TRIG (t, STM_SIG_EMPTY);			/* superstate of target */
			t = me->temp;
			if (s == t) {							/* (b) check source==target->super */
				ip = 0;
			} else {
				STM_TRIG (s, STM_SIG_EMPTY);		/* superstate of src */
				if (me->temp == t) {				/* (c) check source->super==target->super */
					STM_TRIG (s, STM_SIG_EXIT);
					ip = 0;
				} else {
					if (me->temp == path[0]) {		/* (d) check source->super==target */
						STM_TRIG (s, STM_SIG_EXIT);
					} else {						/* (e) check rest of source==target->super->super..
                            						* and store the entry path along the way
                            						*/
						iq = 0;
						ip = 1;
						path[1] = t;
						t = me->temp;

						r = STM_TRIG (path[1], STM_SIG_EMPTY);	/* find target->super->super */
						while (r == STM_RET_FATHER) {
							++ip;
							path[ip] = me->temp;
							if (me->temp == s) {
								iq = 1;
								STM_ASSERT (ip < STM_MAX_NEST_DEPTH);	/* entry path must not overflow */
								--ip;
								r = STM_RET_HANDLED;			/* terminate the loop */
							} else {			/* it is not the source, keep going up */
								r = STM_TRIG (me->temp, STM_SIG_EMPTY);
							}
						}

						if (iq == 0) {
							STM_ASSERT (ip < STM_MAX_NEST_DEPTH);
							STM_TRIG (s, STM_SIG_EXIT);
							/* (f) check the rest of source->super == target->super->super... */
							iq = ip;
							r = STM_RET_IGNORED;
							do {
								if (t == path[iq]) {
									r = STM_RET_HANDLED;/* indicate LCA found */
									ip = iq - 1;		/*do not enter LCA*/
									iq = -1;			/* terminate the loop */
								} else {
									--iq;				/* try lower superstate of target */
								}
							} while (iq >= 0);

							if (r != STM_RET_HANDLED) {	/* LCA not found yet? */
								/* (g) check each source->super->... 
								   (h) for each target->super...*/
								r = STM_RET_IGNORED;
								do {
									if (STM_TRIG (t, STM_SIG_EXIT) == STM_RET_HANDLED) {
										STM_TRIG (t, STM_SIG_EMPTY);
									}

									t = me->temp;
									iq = ip;
									do {
										if (t == path[iq]) {
											ip = iq - 1;
											iq = -1;			/* breaker inner */
											r = STM_RET_HANDLED;/* breaker outer */
										} else {
											--iq;
										}
									} while (iq >= 0);
								} while (r != STM_RET_HANDLED);
							}
						}
					}
				}
			}
		}
									/* retrace the entry path in reverse (desired) order... */
		for (; ip >= 0; --ip) {     /* trig STM_SIG_ENTRY from LCA to transioned state */
			STM_TRIG(path[ip], STM_SIG_ENTRY);
		}

		t = path[0];
		me->temp = t;

		/* drill into the target hierarchy... 
		   trig the STM_SIG_INIT to the new transitioned state, if new transion happened again, then we need do it again
		   Becareful STM_SIG_INIT must trig t state to the nested children state, otherwise hsm crash */
		while (STM_TRIG (t, STM_SIG_INIT) == STM_RET_TRAN) {
			ip = 0;
			path[0] = me->temp;

			STM_TRIG (me->temp, STM_SIG_EMPTY);/* Find all the father state until to source t state */

			while (me->temp != t) {
				++ip;
				path[ip] = me->temp;
				STM_TRIG (me->temp, STM_SIG_EMPTY);
			}

			me->temp = path[0];

			STM_ASSERT (ip < STM_MAX_NEST_DEPTH);

			/* trig STM_SIG_ENTRY from father source state to nested transition children state */
			do {
				STM_TRIG (path[ip], STM_SIG_ENTRY);
				--ip;
			} while (ip >= 0);

			t = path[0];            /* remember the target transitoned state */
		}
	}

	me->state = t;                  /* change to new state */
	me->temp  = t;
}

/*
 * If the current state is in state or not
 *
 * @me is the state machine
 * @state is to compared with currenet state.
 *
 * Returns   if the state is the father state of current state, it also return 1.
 */
int hsm_in_state(stm_t *me, stm_func_t state)
{
	int inState = 0;
	int r;

	STM_ASSERT (me->temp == me->state);
	do {
		if (me->temp == state) {
			inState = 1;
			break;
		} else {
			r = STM_TRIG (me->temp, STM_SIG_EMPTY);
		}
	} while (r != STM_RET_IGNORED);

	me->temp = me->state;

	return inState;
}

/*
 * exec top of state machine, top of hsm ignore all event
 */
int hsm_top (stm_t *me, evt_t *e)
{
	me = me;
	e = e;
	return STM_RET_IGNORED;
}

/*
 * hsm constructor
 */
void hsm_ctor(stm_t *me, stm_func_t init)
{
	static stm_vtbl_t vtbl = {hsm_init, hsm_dispatch};		/* set visual table */

	me->vptr = &vtbl;
	me->state = hsm_top;
	me->temp = init;

	STM_TRIG(me->vptr->init, STM_SIG_INIT);
}
