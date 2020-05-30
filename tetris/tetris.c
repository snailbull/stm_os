#include "stm_os.h"
#include "tetris.h"

actor_t *director_act;

field_act_t field_act;
block_act_t block_act;
store_act_t store_act;
score_act_t score_act;
stage_act_t stage_act;


// director stm
static int director_idle(stm_t *me, evt_t *e);

int director_init(stm_t *me, evt_t *e)
{
    return STM_TRAN(director_idle);
}
static int director_idle(stm_t *me, evt_t *e)
{
    int r = STM_RET_HANDLED;

    switch (e->evt)
    {
    case STM_EVT_INIT:
        break;

    case STM_EVT_ENTRY:
        memset(&stage_act, 0, sizeof(stage_act));
        memset(&field_act, 0, sizeof(field_act));
        memset(&block_act, 0, sizeof(block_act));
        memset(&store_act, 0, sizeof(store_act));
        memset(&score_act, 0, sizeof(score_act));

        stage_act.width = 40;
        stage_act.height = 25;
        stage_act.x = 0;
        stage_act.y = 0;

        field_act.width = 10;
        field_act.height = 20;
        field_act.x = 2;
        field_act.y = 3;

        store_act.x = 29;
        store_act.y = 3;

        score_act.x = 25;
        score_act.y = 18;
        
        stage_act.act = actor_create(stage_init, 10,0); // the whole game scene
        field_act.act = actor_create(field_init, 10,0); // the area to place block
        block_act.act = actor_create(block_init, 10,0); // the falling block
        store_act.act = actor_create(store_init, 10,0); // the block in store
        score_act.act = actor_create(score_init, 10,0); // the game score
        break;

    case STM_EVT_EXIT:
        break;

    default:
        r = STM_FATHER(hsm_top);
        break;
    }

    return r;
}
