#ifndef _PORT_H_
#define _PORT_H_

#define CFG_LOWEST_PRIO         63u

#define CFG_RDY_TBL_SIZE        ((CFG_LOWEST_PRIO) / 8u + 1u)

#define CFG_MSG_NEST_DEPTH		6

#define CFG_POWER_SAVING		1

#define CFG_LITTLE_ENDIEN		1

#define CFG_DEBUG_ON

/*
 * os的全局变量互斥访问
 */
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

#define PORT_ASSERT(EX)                                                       \
	if (!(EX))                                                                \
	{                                                                         \
		volatile char dummy = 0;                                              \
		/* CPU_SR_Save(); */                                                        \
		PORT_PRINTF("(%s) assert failed at %s:%d \n", #EX, __FUNCTION__, __LINE__);\
		while (dummy == 0);                                                   \
	}

#define MEM_ALIGN_UP(size, align)	(((size) + (align) - 1) & ~((align) - 1))

#define MEM_ALIGN_DOWN(size, align)	((size) & ~((align) - 1))

#define ARRAY_SIZE(array)           (sizeof(array) / sizeof(array[0]))

#endif


