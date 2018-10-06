#ifndef _MEM_H_
#define _MEM_H_

#define MEM_DEBUG_ON
#ifdef MEM_DEBUG_ON
#define MEM_DEBUG(fmt,...)    printf("[mem.c,%d]:" fmt "\r\n", __LINE__,##__VA_ARGS__)
#define MEM_ASSERT(e)				\
	if (!(e))						\
	{								\
		volatile char dummy = 0;	\
		/* CPU_SR_Save(); */		\
		MEM_DEBUG("%s:(%s) assert failed!",  __FUNCTION__,#e);\
		while (dummy == 0);			\
	}
#else
#define MEM_DEBUG(fmt, ...)
#define MEM_ASSERT(e)
#endif

void os_mem_init(void *begin_addr, void *end_addr);
void os_mem_info(void);
void *os_malloc(uint32_t size);
void *os_realloc(void *rmem, uint32_t newsize);
void *os_calloc(uint32_t count, uint32_t size);
void os_free(void *rmem);

#endif
