#include "stm_os.h"
#include "driver_task.h"
#include "app_task.h"
#include "user_event.h"

#include <stdio.h>
#include <curses.h>

#define DRV_PRINTF(fmt,...)  mvprintw(2,5,"drv:" fmt, ##__VA_ARGS__)

uint8_t driver_task(stm_t *me, msg_t *e);

uint8_t driver_init(stm_t *me, msg_t *e)
{
    if (e->sig == STM_EVT_INIT)
    {
        DRV_PRINTF("driver_init!");
    }

    return STM_TRAN(driver_task);
}

uint8_t driver_task(stm_t *me, msg_t *e)
{
    uint8_t r = STM_RET_HANDLED;

    switch (e->sig)
    {
    case KEY_SIG:
        DRV_PRINTF("driver:key_sig");
        break;

    case STM_EVT_INIT:
        DRV_PRINTF("driver_task:init");
        break;

    case STM_EVT_ENTRY:
        DRV_PRINTF("driver_task:entry");
        evtimer_add(me, KEY_SIG, 0, 1000, TIMER_FLAG_REPEAT | TIMER_FLAG_START);
        break;

    case STM_EVT_EXIT:
        DRV_PRINTF("driver_task:exit");
        break;

    default:
        r = STM_FATHER(hsm_top);
        break;
    }

    return r;
}
