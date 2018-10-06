#include "stm_os.h"
#include "app_task.h"
#include "user_event.h"

#include <stdio.h>
#include <curses.h>

#define APP_PRINTF(fmt,...)  mvprintw(1,5,"app:" fmt, ##__VA_ARGS__)

static uint8_t app_task(stm_t *me, msg_t *e);

uint8_t app_init(stm_t *me, msg_t *e)
{
    if (e->sig == STM_EVT_INIT)
    {
        APP_PRINTF("app_init!");
    }

    return STM_TRAN(app_task);
}

static uint8_t app_task(stm_t *me, msg_t *e)
{
    uint8_t r = STM_RET_HANDLED;

    switch (e->sig)
    {
    case FLUSH_SIG:
        APP_PRINTF("app:FLUSH_SIG");
        break;

    case STM_EVT_INIT:
        APP_PRINTF("app_task:init");
        break;

    case STM_EVT_ENTRY:
        APP_PRINTF("app_task:entry");
        evtimer_add(me, FLUSH_SIG, 0, 1000, TIMER_FLAG_START | TIMER_FLAG_REPEAT);
        break;

    case STM_EVT_EXIT:
        APP_PRINTF("app_task:exit");
        break;

    default:
        r = STM_FATHER(hsm_top);
        break;
    }

    return r;
}
