#ifndef _DRIVER_TASK_H_
#define _DRIVER_TASK_H_


extern TActive driver_act;
extern evt_t driver_queue[10];
void driver_ctor(void);

#endif
