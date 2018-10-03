#include "stm_os.h"

static list_t evtimer_head = {&evtimer_head, &evtimer_head};
static list_t cbtimer_head = {&cbtimer_head, &cbtimer_head};

/*
 * Add a evt timer
 * @me		state-machine
 * @sig		evt wanto send
 * @para	pointer para wanto send
 * @ms		timeout
 * @flag	oneshot&repeat,start&stop
 *
 * return
 */
uint8_t evtimer_add(stm_t *me, evt_t sig, void *para, uint32_t ms, uint8_t flag)
{
    evtimer_t *t;
    list_t *head;
    list_t *iter;

    head = &evtimer_head;
    iter = head->next;
    while (iter != head)
    {
        t = list_entry(iter, evtimer_t, list);

        if ((t->me == me) && (t->e.sig == sig))
        {
            return ERR_EXISTED_TIMER;
        }

        iter = iter->next;
    }

    t = (evtimer_t *)os_malloc(sizeof(evtimer_t));
    if (t == NULL)
    {
        return ERR_NO_MEMORY;
    }
    t->me      = me;
    t->e.sig   = sig;
    t->e.para  = para;
    t->timeout = ms;
    if (flag & TIMER_FLAG_REPEAT)
    {
        t->reload_timeout = ms;
    }
    else
    {
        t->reload_timeout = 0;
    }
    t->flag    = flag;

    list_insert(head, &t->list);

    return ERR_SUCCESS;
}

uint8_t evtimer_del(stm_t *me, evt_t sig)
{
    evtimer_t *t;
    list_t *head;
    list_t *iter;

    head = &evtimer_head;
    iter = head->next;

    while (iter != head)
    {
        t = list_entry(iter, evtimer_t, list);

        if ((t->me == me) && (t->e.sig == sig))
        {
            list_delete(&t->list);
			if (t->e.para)
			{
				os_free(t->e.para);
				t->e.para = 0;
			}
            os_free(t);
			t = 0;
            return ERR_SUCCESS;
        }

        iter = iter->next;
    }

    return ERR_DELETE_FAILED;
}

/*
 * @me		state-machine
 * @sig		evt
 * @flag	set flag:start,stop,one_shot,repeat
 */
uint8_t evtimer_set(stm_t *me, evt_t sig, uint8_t flag)
{
    evtimer_t *t;
    list_t *head;
    list_t *iter;

    head = &evtimer_head;
    iter = head->next;
    while (iter != head)
    {
        t = list_entry (iter, evtimer_t, list);

        if ((t->me == me) && (t->e.sig == sig))
        {
            t->flag = flag;
            return ERR_SUCCESS;
        }

        iter = iter->next;
    }
    return ERR_NULL_OBJECT;
}

void evtimer_update(uint32_t elapse_ms)
{
    evtimer_t *t;
    list_t *head;
    list_t *iter;
    list_t *iter_temp;

    head = &evtimer_head;
    iter = head->next;
    while (iter != head)
    {
        t = list_entry (iter, evtimer_t, list);
        iter_temp = iter->next;

        if (t->flag & TIMER_FLAG_START)
        {
            if (t->timeout > elapse_ms)
            {
                t->timeout -= elapse_ms;
            }
            else
            {
                os_post_message((TActive *)(t->me), t->e.sig, t->e.para, SEND_TO_END);
                if (t->flag & TIMER_FLAG_REPEAT)
                {
                    t->timeout = t->reload_timeout;
                }
                else
                {
                    list_delete(&t->list);
					if (t->e.para)
					{
						os_free(t->e.para);
						t->e.para = 0;
					}
                    os_free(t);
                }
            }
        }

        iter = iter_temp;
    }
}

uint8_t cbtimer_add(timer_func_t func, uint32_t ms, uint8_t flag)
{
    cbtimer_t *t;
    list_t *head;
    list_t *iter;

    head = &cbtimer_head;
    iter = head->next;
    while (iter != head)
    {
        t = list_entry(iter, cbtimer_t, list);
        if (t->func == func)
        {
            return ERR_EXISTED_TIMER;
        }
        iter = iter->next;
    }

    t = (cbtimer_t *)os_malloc(sizeof(cbtimer_t));
    if (t == NULL)
    {
        return ERR_NO_MEMORY;
    }
    t->flag    = flag;
    t->timeout = ms;
    t->reload_timeout = ms;
    t->counter = 0;
    t->func    = func;
    list_insert(head, &t->list);

    return ERR_SUCCESS;
}

uint8_t cbtimer_del(timer_func_t func)
{
    cbtimer_t *t;
    list_t *head;
    list_t *iter;

    head = &cbtimer_head;
    iter = head->next;
    while (iter != head)
    {
        t = list_entry(iter, cbtimer_t, list);
        if (t->func == func)
        {
            list_delete(&t->list);
            os_free(t);
            return ERR_SUCCESS;
        }
        iter = iter->next;
    }

    return ERR_DELETE_FAILED;
}

uint8_t cbtimer_set(timer_func_t func, uint8_t flag)
{
    cbtimer_t *t;
    list_t *head;
    list_t *iter;

    head = &cbtimer_head;
    iter = head->next;
    while (iter != head)
    {
        t = list_entry (iter, cbtimer_t, list);
        if (t->func == func)
        {
            t->flag = flag;
            return ERR_SUCCESS;
        }
        iter = iter->next;
    }
    return ERR_NULL_OBJECT;
}

void cbtimer_update(uint32_t elapse_ms)
{
    cbtimer_t *t;
    list_t *head;
    list_t *iter;
    list_t *iter_temp;

    head = &cbtimer_head;
    iter = head->next;
    while (iter != head)
    {
        t = list_entry (iter, cbtimer_t, list);
        iter_temp = iter->next;

        if (t->flag & TIMER_FLAG_START)
        {
            if (t->timeout > elapse_ms)
            {
                t->timeout -= elapse_ms;
            }
            else
            {
                uint8_t r = t->func(t->counter);
                t->timeout = t->reload_timeout;
                if (r == TIMER_RET_INC)
				{
                    t->counter++;
				}
                else if (r == TIMER_RET_CLR)
				{
                    t->counter = 0;
				}
                else if (e == TIMER_RET_DEL)
				{
                    list_delete(&t->list);
                    os_free(t);
                }
				else
				{
					;
				}
            }
        }

        iter = iter_temp;
    }
}
