#include "stm_os.h"
#include "tetris.h"

static int block_idle(stm_t *me, evt_t *e);
static int block_ready(stm_t *me, evt_t *e);
static int block_falling(stm_t *me, evt_t *e);
static int block_lay(stm_t *me, evt_t *e);
static int block_pause(stm_t *me, evt_t *e);

int block_init(stm_t *me, evt_t *e)
{
    return STM_TRAN(block_idle);
}
static int block_idle(stm_t *me, evt_t *e)
{
    int r = STM_RET_HANDLED;

    switch (e->evt)
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
static int block_ready(stm_t *me, evt_t *e)
{
    int r = STM_RET_HANDLED;

    switch (e->evt)
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

static int block_falling(stm_t *me, evt_t *e)
{
    int r = STM_RET_HANDLED;

    switch (e->evt)
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
static int block_lay(stm_t *me, evt_t *e)
{
    int r = STM_RET_HANDLED;

    switch (e->evt)
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

static int block_pause(stm_t *me, evt_t *e)
{
    int r = STM_RET_HANDLED;

    switch (e->evt)
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
