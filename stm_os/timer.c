#include "stm_os.h"

static list_t evtimer_head = {&evtimer_head, &evtimer_head};
static list_t cbtimer_head = {&cbtimer_head, &cbtimer_head};

/*
 * Add a evt timer
 * @me		state-machine
 * @evt		evt wanto send
 * @para	pointer para wanto send
 * @ms		timeout
 * @flag	oneshot&repeat,start&stop
 *
 * return
 */
int evtimer_add(stm_t *me, int evt, void *para, int ms, int flag)
{
	evtimer_t *t;
	list_t *head;
	list_t *iter;

	head = &evtimer_head;
	iter = head->next;
	while (iter != head) {
		t = list_entry(iter, evtimer_t, list);

		if ((t->me == me) && (t->e.evt == evt)) {
			return ERR_EXISTED_TIMER;
		}

		iter = iter->next;
	}

	t = (evtimer_t *)os_malloc(sizeof(evtimer_t));
	if (t == NULL) {
		return ERR_NO_MEMORY;
	}
	t->me      = me;
	t->e.evt   = evt;
	t->e.para  = para;
	t->timeout = ms;
	t->flag    = flag;
	if (flag & TIMER_FLAG_REPEAT) {
		t->reload_timeout = ms;
	} else {
		t->reload_timeout = 0;
	}

	list_insert(head, &t->list);

	return ERR_SUCCESS;
}

int evtimer_del(stm_t *me, int evt)
{
	evtimer_t *t;
	list_t *head;
	list_t *iter;

	head = &evtimer_head;
	iter = head->next;

	while (iter != head) {
		t = list_entry(iter, evtimer_t, list);

		if ((t->me == me) && (t->e.evt == evt)) {
			list_delete(&t->list);
			if (t->e.para) {
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
 * @e		evt
 * @flag	set flag:start,stop,one_shot,repeat
 */
int evtimer_set(stm_t *me, int evt, int flag)
{
	evtimer_t *t;
	list_t *head;
	list_t *iter;

	head = &evtimer_head;
	iter = head->next;
	while (iter != head) {
		t = list_entry (iter, evtimer_t, list);

		if ((t->me == me) && (t->e.evt == evt)) {
			t->flag = flag;
			return ERR_SUCCESS;
		}

		iter = iter->next;
	}
	return ERR_NULL_OBJECT;
}

void evtimer_update(int elapse_ms)
{
	evtimer_t *t;
	list_t *head;
	list_t *iter;
	list_t *iter_temp;

	head = &evtimer_head;
	iter = head->next;
	while (iter != head) {
		t = list_entry (iter, evtimer_t, list);
		iter_temp = iter->next;

		if (t->flag & TIMER_FLAG_START) {
			if (t->timeout > elapse_ms) {
				t->timeout -= elapse_ms;
			} else {
				actor_post_message((actor_t *)(t->me), t->e.evt, t->e.para, SEND_TO_BACK);
				if (t->flag & TIMER_FLAG_REPEAT) {
					t->timeout = t->reload_timeout;
				} else {
					list_delete(&t->list);
					if (t->e.para) {
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

int cbtimer_add(timer_func_t func, int ms, int flag)
{
	cbtimer_t *t;
	list_t *head;
	list_t *iter;

	head = &cbtimer_head;
	iter = head->next;
	while (iter != head) {
		t = list_entry(iter, cbtimer_t, list);
		if (t->func == func) {
			return ERR_EXISTED_TIMER;
		}
		iter = iter->next;
	}

	t = (cbtimer_t *)os_malloc(sizeof(cbtimer_t));
	if (t == NULL) {
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

int cbtimer_del(timer_func_t func)
{
	cbtimer_t *t;
	list_t *head;
	list_t *iter;

	head = &cbtimer_head;
	iter = head->next;
	while (iter != head) {
		t = list_entry(iter, cbtimer_t, list);
		if (t->func == func) {
			list_delete(&t->list);
			os_free(t);
			return ERR_SUCCESS;
		}
		iter = iter->next;
	}

	return ERR_DELETE_FAILED;
}

int cbtimer_set(timer_func_t func, int flag)
{
	cbtimer_t *t;
	list_t *head;
	list_t *iter;

	head = &cbtimer_head;
	iter = head->next;
	while (iter != head) {
		t = list_entry (iter, cbtimer_t, list);
		if (t->func == func) {
			t->flag = flag;
			return ERR_SUCCESS;
		}
		iter = iter->next;
	}
	return ERR_NULL_OBJECT;
}

void cbtimer_update(int elapse_ms)
{
	cbtimer_t *t;
	list_t *head;
	list_t *iter;
	list_t *iter_temp;

	head = &cbtimer_head;
	iter = head->next;
	while (iter != head) {
		t = list_entry (iter, cbtimer_t, list);
		iter_temp = iter->next;

		if (t->flag & TIMER_FLAG_START) {
			if (t->timeout > elapse_ms) {
				t->timeout -= elapse_ms;
			} else {
				int r = t->func(t->counter);
				t->timeout = t->reload_timeout;
				if (r == TIMER_RET_INC) {
					t->counter++;
				} else if (r == TIMER_RET_CLR) {
					t->counter = 0;
				} else if (r == TIMER_RET_DEL) {
					list_delete(&t->list);
					os_free(t);
				} else {
					;
				}
			}
		}

		iter = iter_temp;
	}
}
