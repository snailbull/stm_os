#include "stm_os.h"
#include "tetris.h"

static int field_idle(stm_t *me, evt_t *e);


int field_init(stm_t *me, evt_t *e)
{
    return STM_TRAN(field_idle);
}

static int field_idle(stm_t *me, evt_t *e)
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
