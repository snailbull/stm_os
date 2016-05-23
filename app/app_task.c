#include "stm_os.h"
#include "app_task.h"
#include "user_event.h"

#include <stdio.h>

/******************************************************************************
*
*/
TActive app_act;
TEvt app_queue[10];

uint8_t lcd_timer(uint32_t counter)
{
    printf("app counter:%d\n", counter);
    if (counter == 4)
    {
        printf("return:%d\n", evtimer_del(&app_act.super, FLUSH_SIG));
        return TIMER_RET_CLR;
    }
    return TIMER_RET_INC;
}

/******************************************************************************
*
*/
static uint8_t app_init(TMsm *me, TEvt *e);
static uint8_t app_task(TMsm *me, TEvt *e);

void app_ctor(void)
{
    hsm_ctor(&app_act.super, app_init);
}

static uint8_t app_init(TMsm *me, TEvt *e)
{
    if (e->sig == STM_INIT_SIG)
    {
        printf("app_init!\n");
    }

    return STM_TRAN(app_task);
}

static uint8_t app_task(TMsm *me, TEvt *e)
{
    uint8_t r = STM_RET_HANDLED;

    switch (e->sig)
    {
    case APP_NET_SIG:
        printf("app:APP_NET_SIG\n");
        break;

    case FLUSH_SIG:
        printf("app:FLUSH_SIG\n");
        break;

    case STM_INIT_SIG:
        printf("app_task:init\n");
        break;

    case STM_ENTRY_SIG:
        printf("app_task:entry\n");

        evtimer_add(me, FLUSH_SIG, 0, 1000, TIMER_FLAG_START | TIMER_FLAG_REPEAT);
        cbtimer_add(lcd_timer, 500, TIMER_FLAG_START);
        break;

    case STM_EXIT_SIG:
        printf("app_task:exit\n");
        break;

    default:
        r = STM_FATHER(hsm_top);
        break;
    }

    return r;
}

