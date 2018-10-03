#ifndef _OS_H_
#define _OS_H_

#define CFG_LOWEST_PRIO         63u

#define CFG_RDY_TBL_SIZE        ((CFG_LOWEST_PRIO) / 8u + 1u)

#define CFG_MSG_NEST_DEPTH		8

#define CFG_POWER_SAVING		1

#define CFG_LITTLE_ENDIEN		1

#define CFG_DEBUG_ON

#include <pthread.h>

extern pthread_mutex_t os_mutex;

#define PORT_SR_ALLOC()         //CPU_SR cpu_sr = (CPU_SR)0
#define PORT_CPU_DISABLE()      {pthread_mutex_lock(&os_mutex);}
//{ cpu_sr = CPU_SR_Save();}      /* disable cpu interrupt */
#define PORT_CPU_ENABLE()       {pthread_mutex_unlock(&os_mutex);}
//{ CPU_SR_Restore(cpu_sr);}      /* enable cpu interrupt */
#define PORT_OS_SLEEP()			{pthread_mutex_lock(&wkup_mutex);}
#define PORT_OS_WKUP()			{pthread_mutex_unlock(&wkup_mutex);}

#ifdef CFG_DEBUG_ON
#define PORT_PRINTF(format, ...)    printf(format, ##__VA_ARGS__)
#else
#define PORT_PRINTF(format, ...)
#endif

#define OS_ASSERT(EX)	\
	if (!(EX))			\
	{					\
		volatile char dummy = 0;	\
		/* CPU_SR_Save(); */		\
		PORT_PRINTF("(%s) assert failed at %s:%d \n", #EX, __FUNCTION__, __LINE__);\
		while (dummy == 0);			\
	}

typedef struct
{
    stm_t super;

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
    evt_t    *queue;
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
uint8_t os_post_message(TActive *act, evt_t sig, void *para, uint8_t opt);
uint8_t os_send_message(TActive *act, evt_t sig, void *para);
void os_dispatch(void);
void os_power(void);
void os_sleep(void);

#endif
