#include "stm_os.h"

static list_t evtimer_head = {&evtimer_head, &evtimer_head};
static list_t cbtimer_head = {&cbtimer_head, &cbtimer_head};

/******************************************************************************
*   event timer function
*/
/*
 * 增加一个事件定时器
 * @me		状态机对象
 * @sig		事件
 * @para	事件参数
 * @ms		超时时间
 * @flag	定时器标志：单触发和重复触发，定时器启动和停止标志
 *
 * return
 */
uint8_t evtimer_add(TMsm *me, signal_t sig, void *para, uint32_t ms, uint8_t flag)
{
    evtimer_t *t;
    list_t *head;
    list_t *iter;

    //
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
    //

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

    //
    list_insert(head, &t->list);
    //

    return ERR_SUCCESS;
}

/*
 * 删除事件定时器
 */
uint8_t evtimer_del(TMsm *me, signal_t sig)
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
            os_free(t);
            return ERR_SUCCESS;
        }

        iter = iter->next;
    }

    return ERR_DELETE_FAILED;
}

/*
 * 事件定时器设置标志
 * @me		要设置的状态机对象
 * @sig		要设置的事件
 * @flag	设置标志:start,stop,one_shot,repeat
 */
uint8_t evtimer_set(TMsm *me, signal_t sig, uint8_t flag)
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
                    os_free(t);
                }
            }
        }

        iter = iter_temp;
    }
}


/******************************************************************************
*  callback function timer
*/
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
                switch (r)
                {
                case TIMER_RET_INC:
                    t->counter++;
                    break;
                case TIMER_RET_CLR:
                    t->counter = 0;
                    break;
                case TIMER_RET_DEL:
                    list_delete(&t->list);
                    os_free(t);
                    break;
                default:
                    break;
                }
            }
        }

        iter = iter_temp;
    }
}
