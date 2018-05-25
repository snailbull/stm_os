#ifndef _OS_H_
#define _OS_H_

typedef struct
{
    TMsm super;

    uint8_t head;
    uint8_t tail;
    uint8_t used;

    uint8_t prio;
    uint8_t prio_bit_x;
    uint8_t prio_bit_y;
    uint8_t prio_x;
    uint8_t prio_y;
} TActive;

typedef struct
{
    TActive *act;
    TEvt    *queue;
    uint8_t  end;
} TActiveCB;

enum
{
    SEND_TO_FRONT = 0,
    SEND_TO_END,
};

extern TActiveCB activeCBs[];
extern uint8_t max_active_object;

void os_init(void);
uint8_t os_post_message(TActive *act, signal_t sig, void *para, uint8_t opt);
uint8_t os_send_message(TActive *act, signal_t sig, void *para);
void os_dispatch(void);
void os_power(void);
void os_sleep(void);
void os_wakeup(void);

#endif
