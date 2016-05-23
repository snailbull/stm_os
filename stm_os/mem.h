#ifndef _MEM_H_
#define _MEM_H_

void os_mem_init(void *begin_addr, void *end_addr);
void os_mem_info(void);
void *os_malloc(uint32_t size);
void *os_realloc(void *rmem, uint32_t newsize);
void *os_calloc(uint32_t count, uint32_t size);
void os_free(void *rmem);

#endif
