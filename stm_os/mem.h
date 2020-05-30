#ifndef _MEM_H_
#define _MEM_H_

void os_mem_init(void *begin_addr, void *end_addr);
void os_mem_info(void);
void *os_malloc(int size);
void *os_realloc(void *rmem, int newsize);
void *os_calloc(int count, int size);
void os_free(void *rmem);

#endif
