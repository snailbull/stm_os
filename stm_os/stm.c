#include "stm_os.h"


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
	{ STM_EVT_EMPTY, 0},
	{ STM_EVT_INIT,  0},
	{ STM_EVT_ENTRY, 0},
	{ STM_EVT_EXIT,  0}
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
	if (me->next == 0) {
		STM_ASSERT (0);
	}

	ret = (*me->next) (me, e);          /* do the fsm constructor init function */
	if (ret != STM_RET_TRAN) {          /* transition must happen here */
		STM_ASSERT (0);
	}

	STM_TRIG (me->next, STM_EVT_ENTRY); /* trig the STM_EVT_ENTRY to the new transitioned state */
	me->state = me->next;               /*change to new state*/
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

	if (me->state != me->next) {        	/* State must be stable here */
		STM_ASSERT (0);
	}

	ret = (*me->state) (me, e);         	/* exceute the state function with new event */
	if (ret == STM_RET_TRAN) {
		STM_TRIG(me->state, STM_EVT_EXIT);	/* exit the original state */
		STM_TRIG(me->next, STM_EVT_ENTRY);	/* enter the new state */
		me->state = me->next;           	/* change to new state */
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
	me->next = init;

	STM_TRIG(me->vptr->init, STM_EVT_INIT);
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

	if (me->next == 0) {
		STM_ASSERT (0);
	}

	if (t != hsm_top) { /* if state is not equal to the hsm top state, just assert */
		STM_ASSERT (0);
	}

	ret = (*me->next) (me, e);          /* do the hsm constructor init function */
	if (ret != STM_RET_TRAN) {          /* transition must happen here */
		STM_ASSERT (0);
	}

	/*Becareful STM_EVT_INIT must trig state to the nested children state, otherwise hsm crash*/
	do {
		ip = 0;
		path[0] = me->next;

		STM_TRIG (me->next, STM_EVT_EMPTY);/* Find all the father state until to hsm_top */

		while (me->next != t) {
			++ip;
			path[ip] = me->next;
			STM_TRIG (me->next, STM_EVT_EMPTY);
		}

		me->next = path[0];

		if (ip >= STM_MAX_NEST_DEPTH) {
			STM_ASSERT (0);
		}

		/* trig STM_EVT_ENTRY from father source state to nested children state */
		do {
			STM_TRIG(path[ip], STM_EVT_ENTRY);
			--ip;
		} while (ip >= 0);

		t = path[0];

		/* trig the STM_EVT_INIT to the new transitioned state, if new transion happened again, then we need do int init again */

	} while (STM_TRIG (t, STM_EVT_INIT) == STM_RET_TRAN);

	me->state = t;                      /* change to new state */
	me->next  = t;
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

	if (t != me->next) {            /* state must be stable here */
		STM_ASSERT (0);
	}

	do {
		s = me->next;
		r = (*s) (me, e);           /* exceute the state function with new event */

		if (r == STM_RET_UNHANDLED) {
			r = STM_TRIG (s, STM_EVT_EMPTY);/* Move up to father state */
		}

		/* move up to the father state to find suitable state to handle the sig */

	} while (r == STM_RET_FATHER);

	/* if state transition happened then process it */
	if (r == STM_RET_TRAN) {
		ip = -1;

		path[0] = me->next;         /* save the transitioned state */
		path[1] = t;

		/* t is the source state, and s is the state which cause new state change
		   for example s is the father state of t */
		while (t != s) {
			/* if STM_EVT_EXIT is handled, trig STM_EVT_EMPTY to find the father state
			   if STM_EVT_EXIT not handled , then me->next hold the father state */
			if (STM_TRIG (t, STM_EVT_EXIT) == STM_RET_HANDLED) {
				STM_TRIG (t, STM_EVT_EMPTY);
			}

			t = me->next;           /* move to one father state up */
		}

		t = path[0];                /* t is the target transition state */

		/* all the following code is try to find the LCA and exit from the source state to LCA state
		   Be careful LCA state is either not entered not exited.
		   all the father state of the target transition state is stored to path from hight to low etc,
		   path[0] is the target transition state */
		if (s == t) {
			STM_TRIG (s, STM_EVT_EXIT);
			ip = 0;
		} else {
			STM_TRIG (t, STM_EVT_EMPTY);
			t = me->next;

			if (s == t) {
				ip = 0;
			} else {
				STM_TRIG (s, STM_EVT_EMPTY);

				if (me->next == t) {
					STM_TRIG (s, STM_EVT_EXIT);
					ip = 0;
				} else {
					if (me->next == path[0]) {
						STM_TRIG (s, STM_EVT_EXIT);
					} else {
						iq = 0;
						ip = 1;
						path[1] = t;
						t = me->next;

						r = STM_TRIG (path[1], STM_EVT_EMPTY);

						while (r == STM_RET_FATHER) {
							++ip;
							path[ip] = me->next;
							if (me->next == s) {
								iq = 1;

								if (ip >= STM_MAX_NEST_DEPTH) {
									STM_ASSERT (0);
								}

								--ip;
								r = STM_RET_HANDLED;
							} else {
								r = STM_TRIG (me->next, STM_EVT_EMPTY);
							}
						}

						if (iq == 0) {
							if (ip >= STM_MAX_NEST_DEPTH) {
								STM_ASSERT (0);
							}

							STM_TRIG (s, STM_EVT_EXIT);

							iq = ip;
							r = STM_RET_IGNORED;
							do {
								if (t == path[iq]) {
									r = STM_RET_HANDLED;

									ip = iq - 1;
									iq = -1;
								} else {
									--iq;
								}

							} while (iq >= 0);

							if (r != STM_RET_HANDLED) {
								r = STM_RET_IGNORED;
								do {
									if (STM_TRIG (t, STM_EVT_EXIT) == STM_RET_HANDLED) {
										STM_TRIG (t, STM_EVT_EMPTY);
									}

									t = me->next;
									iq = ip;
									do {
										if (t == path[iq]) {
											ip = iq - 1;
											iq = -1;
											r = STM_RET_HANDLED;
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

		for (; ip >= 0; --ip) {     /* trig STM_EVT_ENTRY from LCA to transioned state */
			STM_TRIG(path[ip], STM_EVT_ENTRY);
		}

		t = path[0];
		me->next = t;

		/* trig the STM_EVT_INIT to the new transitioned state, if new transion happened again, then we need do it again
		   Becareful STM_EVT_INIT must trig t state to the nested children state, otherwise hsm crash */
		while (STM_TRIG (t, STM_EVT_INIT) == STM_RET_TRAN) {
			ip = 0;
			path[0] = me->next;

			STM_TRIG (me->next, STM_EVT_EMPTY);/* Find all the father state until to source t state */

			while (me->next != t) {
				++ip;
				path[ip] = me->next;
				STM_TRIG (me->next, STM_EVT_EMPTY);
			}

			me->next = path[0];

			if (ip >= STM_MAX_NEST_DEPTH) {
				STM_ASSERT (0);
			}

			/* trig STM_EVT_ENTRY from father source state to nested transition children state */
			do {
				STM_TRIG (path[ip], STM_EVT_ENTRY);
				--ip;
			} while (ip >= 0);

			t = path[0];            /* remember the target transitoned state */
		}
	}

	me->state = t;                  /* change to new state */
	me->next  = t;
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

	STM_ASSERT (me->next == me->state);
	do {
		if (me->next == state) {
			inState = 1;
			break;
		} else {
			r = STM_TRIG (me->next, STM_EVT_EMPTY);
		}
	} while (r != STM_RET_IGNORED);

	me->next = me->state;

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
	me->next = init;

	STM_TRIG(me->vptr->init, STM_EVT_INIT);
}
