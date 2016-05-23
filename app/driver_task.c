#include "stm_os.h"
#include "driver_task.h"
#include "app_task.h"
#include "user_event.h"

#include <stdio.h>

/******************************************************************************
* state function
*/
TActive driver_act;
TEvt driver_queue[10];

/******************************************************************************
* cbtimer function
*/
uint8_t adc_timer(uint32_t counter)
{
    printf("adc:%d\n", counter);

    if (counter == 2)
    {
        evtimer_del(&driver_act.super, KEY_SIG);
        os_post_message(&app_act, APP_NET_SIG, 0, SEND_TO_END);
        return TIMER_RET_DEL;
    }
    return TIMER_RET_INC;
}

/******************************************************************************
* state function
*/
uint8_t driver_init(TMsm *me, TEvt *e);
uint8_t driver_task(TMsm *me, TEvt *e);


void driver_ctor(void)
{
    hsm_ctor(&driver_act.super, driver_init);
}

uint8_t driver_init(TMsm *me, TEvt *e)
{
    if (e->sig == STM_INIT_SIG)
    {
        printf("driver_init!\n");
    }

    return STM_TRAN(driver_task);
}

uint8_t driver_task(TMsm *me, TEvt *e)
{
    uint8_t r = STM_RET_HANDLED;

    switch (e->sig)
    {
    case KEY_SIG:
        printf("key_sig\n");
        break;

    case LED_SIG:
        printf("led_sig\n");
        break;

    case STM_INIT_SIG:
        printf("driver_task:init\n");
        break;

    case STM_ENTRY_SIG:
        printf("driver_task:entry\n");
        evtimer_add(me, KEY_SIG, 0, 1000, TIMER_FLAG_REPEAT | TIMER_FLAG_START);
        cbtimer_add(adc_timer, 2000, TIMER_FLAG_START);
        break;

    case STM_EXIT_SIG:
        printf("driver_task:exit\n");
        break;

    default:
        r = STM_FATHER(hsm_top);
        break;
    }

    return r;
}

