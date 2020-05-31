#include "stm_os.h"


static int store_idle(stm_t *me, evt_t *e);

int store_init(stm_t *me, evt_t *e)
{
    return STM_TRAN(store_idle);
}

static int store_idle(stm_t *me, evt_t *e)
{
    int r = STM_RET_HANDLED;

    switch (e->sig)
    {
    case STM_SIG_INIT:
        break;

    case STM_SIG_ENTRY:
        break;

    case STM_SIG_EXIT:
        break;

    default:
        r = STM_FATHER(hsm_top);
        break;
    }

    return r;
}
