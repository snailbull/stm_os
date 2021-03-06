#include "stm_os.h"


static uint8_t store_idle(stm_t *me, msg_t *e);

uint8_t store_init(stm_t *me, msg_t *e)
{
    return STM_TRAN(store_idle);
}

static uint8_t store_idle(stm_t *me, msg_t *e)
{
    uint8_t r = STM_RET_HANDLED;

    switch (e->sig)
    {
    case STM_EVT_INIT:
        break;

    case STM_EVT_ENTRY:
        break;

    case STM_EVT_EXIT:
        break;

    default:
        r = STM_FATHER(hsm_top);
        break;
    }

    return r;
}
