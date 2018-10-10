#include "stm_os.h"
#include "tetris.h"

actor_t *director_act;

field_act_t field_act;
block_act_t block_act;
store_act_t store_act;
score_act_t score_act;
stage_act_t stage_act;


// director stm
static uint8_t director_idle(stm_t *me, msg_t *e);

uint8_t director_init(stm_t *me, msg_t *e)
{
    return STM_TRAN(director_idle);
}
static uint8_t director_idle(stm_t *me, msg_t *e)
{
    uint8_t r = STM_RET_HANDLED;

    switch (e->sig)
    {
    case STM_EVT_INIT:
        break;

    case STM_EVT_ENTRY:
        stage_act.act = actor_create(stage_init, 10,0);
        field_act.act = actor_create(field_init, 10,0);
        block_act.act = actor_create(block_init, 10,0);
        store_act.act = actor_create(store_init, 10,0);
        score_act.act = actor_create(score_init, 10,0);
        break;

    case STM_EVT_EXIT:
        break;

    default:
        r = STM_FATHER(hsm_top);
        break;
    }

    return r;
}
