#include "stm_os.h"

static msg_t s_stm_global_evt[4] =
{
    { STM_EVT_EMPTY, 0},
    { STM_EVT_ENTRY, 0},
    { STM_EVT_EXIT,  0},
    { STM_EVT_INIT,  0}
};

/*
 * Init the finit state machine.
 *
 * @me is the state machine
 * @e is the trig event
 *
 * Note(s)
 */
void fsm_init (stm_t *me, msg_t *e)
{
    uint8_t ret;
    if (me->temp == 0)
    {
        STM_ASSERT (0);
    }

    ret = (*me->temp) (me, e);          /* do the fsm constructor init function */
    if (ret != STM_RET_TRAN)            /* transition must happen here */
    {
        STM_ASSERT (0);
    }

    STM_TRIG (me->temp, STM_EVT_ENTRY); /* trig the STM_EVT_ENTRY to the new transitioned state */
    me->state = me->temp;               /*change to new state*/
}

/*
 * This function is used to exceute the finit state machine.
 * @me is the meta state machine
 * @e is the trig event
 *
 * Note(s)
 */
void fsm_dispatch(stm_t *me, msg_t *e)
{
    uint8_t ret;

    if (me->state != me->temp)          /* State must be stable here */
    {
        STM_ASSERT (0);
    }

    ret = (*me->state) (me, e);         /* exceute the state function with new event */
    if (ret == STM_RET_TRAN)
    {
        STM_EXIT (me->state);           /* exit the original state */
        STM_ENTER (me->temp);           /* enter the new state */
        me->state = me->temp;           /* change to new state */
    }
}

/*
 * fsm constructor
 */
void fsm_ctor(stm_t *me, stm_func_t init)
{
    static const stm_vtbl_t vtbl = {fsm_init, fsm_dispatch};		/* set visual table */

    me->vptr = &vtbl;
    me->state = 0;
    me->temp = init;
}

/*
 * This function is used to init the finit state machine.
 * @me is the state machine
 * @e is the trig event
 *
 * Note(s)
 */
void hsm_init (stm_t *me, msg_t *e)
{
    uint8_t ret;
    int8_t ip;
    stm_func_t path[STM_MAX_NEST_DEPTH]; /* Max nested state levels */
    stm_func_t t = me->state;

    if (me->temp == 0)
    {
        STM_ASSERT (0);
    }

    if (t != hsm_top) /* if state is not equal to the hsm top state, just assert */
    {
        STM_ASSERT (0);
    }

    ret = (*me->temp) (me, e);          /* do the hsm constructor init function */
    if (ret != STM_RET_TRAN)            /* transition must happen here */
    {
        STM_ASSERT (0);
    }

    /*Becareful STM_EVT_INIT must trig state to the nested children state, otherwise hsm crash*/
    do
    {
        ip = 0;
        path[0] = me->temp;

        STM_TRIG (me->temp, STM_EVT_EMPTY);/* Find all the father state until to hsm_top */

        while (me->temp != t)
        {
            ++ip;
            path[ip] = me->temp;
            STM_TRIG (me->temp, STM_EVT_EMPTY);
        }

        me->temp = path[0];

        if (ip >= STM_MAX_NEST_DEPTH)
        {
            STM_ASSERT (0);
        }

        /* trig STM_EVT_ENTRY from father source state to nested children state */
        do
        {
            STM_ENTER (path[ip]);
            --ip;
        }
        while (ip >= 0);

        t = path[0];

        /* trig the STM_EVT_INIT to the new transitioned state, if new transion happened again, then we need do int init again */

    }
    while (STM_TRIG (t, STM_EVT_INIT) == STM_RET_TRAN);

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
void hsm_dispatch(stm_t *me, msg_t *e)
{
    uint8_t r;
    int8_t ip;
    int8_t iq;
    stm_func_t path[STM_MAX_NEST_DEPTH];
    stm_func_t s;
    stm_func_t t = me->state;

    if (t != me->temp)              /* state must be stable here */
    {
        STM_ASSERT (0);
    }

    do
    {
        s = me->temp;
        r = (*s) (me, e);           /* exceute the state function with new event */

        if (r == STM_RET_UNHANDLED)
        {
            r = STM_TRIG (s, STM_EVT_EMPTY);/* Move up to father state */
        }

        /* move up to the father state to find suitable state to handle the sig */

    }while (r == STM_RET_FATHER);

    /* if state transition happened then process it */
    if (r == STM_RET_TRAN)
    {
        ip = -1;

        path[0] = me->temp;         /* save the transitioned state */
        path[1] = t;

        /* t is the source state, and s is the state which cause new state change
           for example s is the father state of t */
        while (t != s)
        {
            /* if STM_EVT_EXIT is handled, trig STM_EVT_EMPTY to find the father state
               if STM_EVT_EXIT not handled , then me->temp hold the father state */
            if (STM_TRIG (t, STM_EVT_EXIT) == STM_RET_HANDLED)
            {
                STM_TRIG (t, STM_EVT_EMPTY);
            }

            t = me->temp;           /* move to one father state up */
        }

        t = path[0];                /* t is the target transition state */

        /* all the following code is try to find the LCA and exit from the source state to LCA state
           Be careful LCA state is either not entered not exited.
           all the father state of the target transition state is stored to path from hight to low etc,
           path[0] is the target transition state */
        if (s == t)
        {
            STM_EXIT (s);
            ip = 0;
        }
        else
        {
            STM_TRIG (t, STM_EVT_EMPTY);
            t = me->temp;

            if (s == t)
            {
                ip = 0;
            }
            else
            {
                STM_TRIG (s, STM_EVT_EMPTY);

                if (me->temp == t)
                {
                    STM_EXIT (s);
                    ip = 0;
                }
                else
                {
                    if (me->temp == path[0])
                    {
                        STM_EXIT (s);
                    }
                    else
                    {
                        iq = 0;
                        ip = 1;
                        path[1] = t;
                        t = me->temp;

                        r = STM_TRIG (path[1], STM_EVT_EMPTY);

                        while (r == STM_RET_FATHER)
                        {
                            ++ip;
                            path[ip] = me->temp;
                            if (me->temp == s)
                            {
                                iq = 1;

                                if (ip >= STM_MAX_NEST_DEPTH)
                                {
                                    STM_ASSERT (0);
                                }

                                --ip;
                                r = STM_RET_HANDLED;
                            }
                            else
                            {
                                r = STM_TRIG (me->temp, STM_EVT_EMPTY);
                            }
                        }

                        if (iq == 0)
                        {
                            if (ip >= STM_MAX_NEST_DEPTH)
                            {
                                STM_ASSERT (0);
                            }

                            STM_EXIT (s);

                            iq = ip;
                            r = STM_RET_IGNORED;
                            do
                            {
                                if (t == path[iq])
                                {
                                    r = STM_RET_HANDLED;

                                    ip = iq - 1;
                                    iq = -1;
                                }
                                else
                                {
                                    --iq;
                                }

                            }
                            while (iq >= 0);

                            if (r != STM_RET_HANDLED)
                            {
                                r = STM_RET_IGNORED;
                                do
                                {
                                    if (STM_TRIG (t, STM_EVT_EXIT) == STM_RET_HANDLED)
                                    {
                                        STM_TRIG (t, STM_EVT_EMPTY);
                                    }

                                    t = me->temp;
                                    iq = ip;
                                    do
                                    {
                                        if (t == path[iq])
                                        {
                                            ip = iq - 1;
                                            iq = -1;
                                            r = STM_RET_HANDLED;
                                        }
                                        else
                                        {
                                            --iq;
                                        }

                                    }
                                    while (iq >= 0);
                                }
                                while (r != STM_RET_HANDLED);
                            }
                        }
                    }
                }
            }
        }

        for (; ip >= 0; --ip)       /* trig STM_EVT_ENTRY from LCA to transioned state */
        {
            STM_ENTER (path[ip]);
        }

        t = path[0];
        me->temp = t;

        /* trig the STM_EVT_INIT to the new transitioned state, if new transion happened again, then we need do it again
           Becareful STM_EVT_INIT must trig t state to the nested children state, otherwise hsm crash */
        while (STM_TRIG (t, STM_EVT_INIT) == STM_RET_TRAN)
        {
            ip = 0;
            path[0] = me->temp;

            STM_TRIG (me->temp, STM_EVT_EMPTY);/* Find all the father state until to source t state */

            while (me->temp != t)
            {
                ++ip;
                path[ip] = me->temp;
                STM_TRIG (me->temp, STM_EVT_EMPTY);
            }

            me->temp = path[0];

            if (ip >= STM_MAX_NEST_DEPTH)
            {
                STM_ASSERT (0);
            }

            /* trig STM_EVT_ENTRY from father source state to nested transition children state */
            do
            {
                STM_ENTER (path[ip]);
                --ip;
            }
            while (ip >= 0);

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
 *
 */
uint8_t hsm_in_state(stm_t *me, stm_func_t state)
{
    uint8_t inState = 0;
    uint8_t r;

    STM_ASSERT (me->temp == me->state);
    do
    {
        if (me->temp == state)
        {
            inState = 1;
            break;
        }
        else
        {
            r = STM_TRIG (me->temp, STM_EVT_EMPTY);
        }
    }
    while (r != STM_RET_IGNORED);

    me->temp = me->state;

    return inState;
}

/*
 * exec top of state machine, top of hsm ignore all event
 */
uint8_t hsm_top (stm_t *me, msg_t *e)
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
    static const stm_vtbl_t vtbl = {hsm_init, hsm_dispatch};		/* set visual table */

    me->vptr = &vtbl;
    me->state = hsm_top;
    me->temp = init;
}
