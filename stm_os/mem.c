#include "stm_os.h"

/*
 * memory config
 */
#define MEM_STATISTIC
#define MEM_ALIGN_BYTE	8
#define MEM_SAFETY
#define MEM_ALIGN_UP(size, align)	(((size) + (align) - 1) & ~((align) - 1))
#define MEM_ALIGN_DOWN(size, align)	((size) & ~((align) - 1))
#define MEM_BLOCK_MIN_SIZE      12
#define MEM_BLOCK_ALIGN_BYTE    MEM_ALIGN_UP(MEM_BLOCK_MIN_SIZE, MEM_ALIGN_BYTE)		/* 16byte */
#define MEM_BLOCK_SIZE          MEM_ALIGN_UP(sizeof(struct heap_mem), MEM_ALIGN_BYTE)	/* 16byte */

/*
 * memory block(12byte)
 */
#define HEAP_MAGIC 0x1ea0
struct heap_mem {
	/* magic and used flag */
	uint16_t magic;
	uint16_t used;

	uint32_t next;
	uint32_t prev;
};

static uint8_t *heap_ptr;			/* memory pool pointer. */

static struct heap_mem *heap_end;/* the last entry, always unused! */
static struct heap_mem *lfree;   /* pointer to the lowest free block */
#ifdef MEM_SAFETY
static sem_t heap_sem; /* mem semaphore */
#endif
static uint32_t mem_size_aligned;	/* Currently maximum available memory block */

#ifdef MEM_STATISTIC
static uint32_t used_mem, max_mem;
#endif

static void plug_holes(struct heap_mem *mem)
{
	struct heap_mem *nmem;
	struct heap_mem *pmem;

	MEM_ASSERT((uint8_t *)mem >= heap_ptr);
	MEM_ASSERT((uint8_t *)mem < (uint8_t *)heap_end);
	MEM_ASSERT(mem->used == 0);

	/* plug hole forward */
	nmem = (struct heap_mem *)&heap_ptr[mem->next];
	if (mem != nmem &&
	        nmem->used == 0 &&
	        (uint8_t *)nmem != (uint8_t *)heap_end) {
		/* if mem->next is unused and not end of heap_ptr,
		 * combine mem and mem->next
		 */
		if (lfree == nmem) {
			lfree = mem;
		}
		mem->next = nmem->next;
		((struct heap_mem *)&heap_ptr[nmem->next])->prev = (uint8_t *)mem - heap_ptr;
	}

	/* plug hole backward */
	pmem = (struct heap_mem *)&heap_ptr[mem->prev];
	if (pmem != mem && pmem->used == 0) {
		/* if mem->prev is unused, combine mem and mem->prev */
		if (lfree == mem) {
			lfree = pmem;
		}
		pmem->next = mem->next;
		((struct heap_mem *)&heap_ptr[mem->next])->prev = (uint8_t *)pmem - heap_ptr;
	}
}

/**
 * This function will initialize system heap memory.
 *
 * @begin_addr the beginning address of heap memory.
 * @end_addr   the end address of heap memory.
 */
void os_mem_init(void *begin_addr, void *end_addr)
{
	struct heap_mem *mem;
	uint32_t begin_align = MEM_ALIGN_UP((uint32_t)begin_addr, MEM_ALIGN_BYTE);
	uint32_t end_align = MEM_ALIGN_DOWN((uint32_t)end_addr, MEM_ALIGN_BYTE);

	/*RT_DEBUG_NOT_IN_INTERRUPT;*/

	/* alignment addr */
	if ((end_align > (2 * MEM_BLOCK_SIZE)) &&
	        ((end_align - 2 * MEM_BLOCK_SIZE) >= begin_align)) {
		/* calculate the aligned memory size */
		mem_size_aligned = end_align - begin_align - 2 * MEM_BLOCK_SIZE;
	} else {
		MEM_DEBUG("mem init, error begin address 0x%x, and end address 0x%x",
		          (uint32_t)begin_addr, (uint32_t)end_addr);
		return;
	}

	/* point to begin address of heap */
	heap_ptr = (uint8_t *)begin_align;

	MEM_DEBUG("mem init, heap begin address 0x%x, size %d",
	          (uint32_t)heap_ptr, mem_size_aligned);

	/* initialize the start of the heap */
	mem        = (struct heap_mem *)heap_ptr;
	mem->magic = HEAP_MAGIC;
	mem->next  = mem_size_aligned + MEM_BLOCK_SIZE;
	mem->prev  = 0;
	mem->used  = 0;

	/* initialize the end of the heap */
	heap_end        = (struct heap_mem *)&heap_ptr[mem->next];
	heap_end->magic = HEAP_MAGIC;
	heap_end->used  = 1;
	heap_end->next  = mem_size_aligned + MEM_BLOCK_SIZE;
	heap_end->prev  = mem_size_aligned + MEM_BLOCK_SIZE;

#ifdef MEM_SAFETY
	if (sem_init(&heap_sem, 0, 1) == -1) {
		MEM_DEBUG("heap_sem intitialization failed\n");
		MEM_ASSERT(0);
	}
#endif

	/* initialize the lowest-free pointer to the start of the heap */
	lfree = (struct heap_mem *)heap_ptr;
}

/**
 * Allocate a block of memory with a minimum of 'size' bytes.
 *
 * @size is the minimum size of the requested block in bytes.
 *
 * @return pointer to allocated memory or NULL if no free memory was found.
 */
void *os_malloc(uint32_t size)
{
	uint32_t ptr, ptr2;
	struct heap_mem *mem, *mem2;

	/* Note:shall not used in ISR, I never do this. */
	/*RT_DEBUG_NOT_IN_INTERRUPT;*/

	if (size == 0) {
		return NULL;
	}

	if (size != MEM_ALIGN_UP(size, MEM_ALIGN_BYTE)) {
		MEM_DEBUG("malloc size %d, but align to %d", size, MEM_ALIGN_UP(size, MEM_ALIGN_BYTE));
	} else {
		MEM_DEBUG("malloc size %d", size);
	}

	/* alignment size */
	size = MEM_ALIGN_UP(size, MEM_ALIGN_BYTE);

	if (size > mem_size_aligned) {
		MEM_DEBUG("no memory");
		return NULL;
	}

	/* every data block must be at least MEM_BLOCK_ALIGN_BYTE long */
	if (size < MEM_BLOCK_ALIGN_BYTE) {
		size = MEM_BLOCK_ALIGN_BYTE;
	}

#ifdef MEM_SAFETY
	/* take memory semaphore */
	sem_wait(&heap_sem);
#endif

	for (ptr = (uint8_t *)lfree - heap_ptr;
	        ptr < mem_size_aligned - size;
	        ptr = ((struct heap_mem *)&heap_ptr[ptr])->next) {
		mem = (struct heap_mem *)&heap_ptr[ptr];

		if ((!mem->used) && (mem->next - (ptr + MEM_BLOCK_SIZE)) >= size) {
			/* mem is not used and at least perfect fit is possible:
			 * mem->next - (ptr + MEM_BLOCK_SIZE) gives us the 'user data size' of mem */

			if (mem->next - (ptr + MEM_BLOCK_SIZE) >=
			        (size + MEM_BLOCK_SIZE + MEM_BLOCK_ALIGN_BYTE)) {
				/* (in addition to the above, we test if another struct heap_mem (MEM_BLOCK_SIZE) containing
				 * at least MEM_BLOCK_ALIGN_BYTE of data also fits in the 'user data space' of 'mem')
				 * -> split large block, create empty remainder,
				 * remainder must be large enough to contain MEM_BLOCK_ALIGN_BYTE data: if
				 * mem->next - (ptr + (2*MEM_BLOCK_SIZE)) == size,
				 * struct heap_mem would fit in but no data between mem2 and mem2->next
				 * @todo we could leave out MEM_BLOCK_ALIGN_BYTE. We would create an empty
				 *       region that couldn't hold data, but when mem->next gets freed,
				 *       the 2 regions would be combined, resulting in more free memory
				 */
				ptr2 = ptr + MEM_BLOCK_SIZE + size;

				/* create mem2 struct */
				mem2       = (struct heap_mem *)&heap_ptr[ptr2];
				mem2->used = 0;
				mem2->next = mem->next;
				mem2->prev = ptr;

				/* and insert it between mem and mem->next */
				mem->next = ptr2;
				mem->used = 1;

				if (mem2->next != mem_size_aligned + MEM_BLOCK_SIZE) {
					((struct heap_mem *)&heap_ptr[mem2->next])->prev = ptr2;
				}
#ifdef MEM_STATISTIC
				used_mem += (size + MEM_BLOCK_SIZE);
				if (max_mem < used_mem) {
					max_mem = used_mem;
				}
#endif
			} else {
				/* (a mem2 struct does no fit into the user data space of mem and mem->next will always
				 * be used at this point: if not we have 2 unused structs in a row, plug_holes should have
				 * take care of this).
				 * -> near fit or excact fit: do not split, no mem2 creation
				 * also can't move mem->next directly behind mem, since mem->next
				 * will always be used at this point!
				 */
				mem->used = 1;
#ifdef MEM_STATISTIC
				used_mem += mem->next - ((uint8_t *)mem - heap_ptr);
				if (max_mem < used_mem) {
					max_mem = used_mem;
				}
#endif
			}
			/* set memory block magic */
			mem->magic = HEAP_MAGIC;

			if (mem == lfree) {
				/* Find next free block after mem and update lowest free pointer */
				while (lfree->used && lfree != heap_end) {
					lfree = (struct heap_mem *)&heap_ptr[lfree->next];
				}

				MEM_ASSERT(((lfree == heap_end) || (!lfree->used)));
			}
#ifdef MEM_SAFETY
			sem_post(&heap_sem);
#endif
			MEM_ASSERT((uint32_t)mem + MEM_BLOCK_SIZE + size <= (uint32_t)heap_end);
			MEM_ASSERT((uint32_t)((uint8_t *)mem + MEM_BLOCK_SIZE) % MEM_ALIGN_BYTE == 0);
			MEM_ASSERT((((uint32_t)mem) & (MEM_ALIGN_BYTE - 1)) == 0);

			MEM_DEBUG("allocate memory at 0x%x, size: %d",
			          (uint32_t)((uint8_t *)mem + MEM_BLOCK_SIZE),
			          (uint32_t)(mem->next - ((uint8_t *)mem - heap_ptr)));

			/* return the memory data except mem struct */
			return (uint8_t *)mem + MEM_BLOCK_SIZE;
		}
	}
#ifdef MEM_SAFETY
	sem_post(&heap_sem);
#endif
	return NULL;
}

/**
 * This function will change the previously allocated memory block.
 *
 * @rmem pointer to memory allocated by os_malloc
 * @newsize the required new size
 *
 * @return the changed memory block address
 */
void *os_realloc(void *rmem, uint32_t newsize)
{
	uint32_t size;
	uint32_t ptr, ptr2;
	struct heap_mem *mem, *mem2;
	void *nmem;

	/*RT_DEBUG_NOT_IN_INTERRUPT;*/

	/* alignment size */
	newsize = MEM_ALIGN_UP(newsize, MEM_ALIGN_BYTE);
	if (newsize > mem_size_aligned) {
		MEM_DEBUG("realloc: out of memory");
		return NULL;
	}

	/* allocate a new memory block */
	if (rmem == NULL) {
		return os_malloc(newsize);
	}

#ifdef MEM_SAFETY
	sem_wait(&heap_sem);
#endif

	if ((uint8_t *)rmem < (uint8_t *)heap_ptr ||
	        (uint8_t *)rmem >= (uint8_t *)heap_end) {
#ifdef MEM_SAFETY
		/* illegal memory */
		sem_post(&heap_sem);
#endif

		return rmem;
	}

	mem = (struct heap_mem *)((uint8_t *)rmem - MEM_BLOCK_SIZE);

	ptr = (uint8_t *)mem - heap_ptr;
	size = mem->next - ptr - MEM_BLOCK_SIZE;
	if (size == newsize) {
#ifdef MEM_SAFETY
		/* the size is the same as */
		sem_post(&heap_sem);
#endif
		return rmem;
	}

	if (newsize + MEM_BLOCK_SIZE + MEM_BLOCK_MIN_SIZE < size) {
		/* split memory block */
#ifdef MEM_STATISTIC
		used_mem -= (size - newsize);
#endif

		ptr2 = ptr + MEM_BLOCK_SIZE + newsize;
		mem2 = (struct heap_mem *)&heap_ptr[ptr2];
		mem2->magic = HEAP_MAGIC;
		mem2->used = 0;
		mem2->next = mem->next;
		mem2->prev = ptr;
		mem->next = ptr2;
		if (mem2->next != mem_size_aligned + MEM_BLOCK_SIZE) {
			((struct heap_mem *)&heap_ptr[mem2->next])->prev = ptr2;
		}

		plug_holes(mem2);
#ifdef MEM_SAFETY
		sem_post(&heap_sem);
#endif
		return rmem;
	}
#ifdef MEM_SAFETY
	sem_post(&heap_sem);
#endif

	/* expand memory */
	nmem = os_malloc(newsize);
	if (nmem != NULL) { /* check memory */
		memcpy(nmem, rmem, size < newsize ? size : newsize);
		os_free(rmem);
	}

	return nmem;
}

/**
 * This function will contiguously allocate enough space for count objects
 * that are size bytes of memory each and returns a pointer to the allocated
 * memory.
 *
 * The allocated memory is filled with bytes of value zero.
 *
 * @count number of objects to allocate
 * @size size of the objects to allocate
 *
 * @return pointer to allocated memory / NULL pointer if there is an error
 */
void *os_calloc(uint32_t count, uint32_t size)
{
	void *p;

	/*RT_DEBUG_NOT_IN_INTERRUPT;*/

	/* allocate 'count' objects of size 'size' */
	p = os_malloc(count * size);

	/* zero the memory */
	if (p) {
		memset(p, 0, count * size);
	}

	return p;
}

/**
 * This function will release the previously allocated memory block by
 * os_malloc. The released memory block is taken back to system heap.
 *
 * @rmem the address of memory which will be released
 */
void os_free(void *rmem)
{
	struct heap_mem *mem;

	/*RT_DEBUG_NOT_IN_INTERRUPT;*/

	if (rmem == NULL) {
		return;
	}
	MEM_ASSERT((((uint32_t)rmem) & (MEM_ALIGN_BYTE - 1)) == 0);
	MEM_ASSERT((uint8_t *)rmem >= (uint8_t *)heap_ptr &&
	           (uint8_t *)rmem < (uint8_t *)heap_end);

	if ((uint8_t *)rmem < (uint8_t *)heap_ptr ||
	        (uint8_t *)rmem >= (uint8_t *)heap_end) {
		MEM_DEBUG("illegal memory");
		return;
	}

	/* Get the corresponding struct heap_mem ... */
	mem = (struct heap_mem *)((uint8_t *)rmem - MEM_BLOCK_SIZE);

	MEM_DEBUG("release memory 0x%x, size: %d",
	          (uint32_t)rmem,
	          (uint32_t)(mem->next - ((uint8_t *)mem - heap_ptr)));

#ifdef MEM_SAFETY
	/* protect the heap from concurrent access */
	sem_wait(&heap_sem);
#endif

	/* ... which has to be in a used state ... */
	MEM_ASSERT(mem->used);
	MEM_ASSERT(mem->magic == HEAP_MAGIC);
	/* ... and is now unused. */
	mem->used  = 0;
	mem->magic = 0;

	if (mem < lfree) {
		/* the newly freed struct is now the lowest */
		lfree = mem;
	}

#ifdef MEM_STATISTIC
	used_mem -= (mem->next - ((uint8_t *)mem - heap_ptr));
#endif

	/* finally, see if prev or next are free also */
	plug_holes(mem);
#ifdef MEM_SAFETY
	sem_post(&heap_sem);
#endif
}

#ifdef MEM_STATISTIC
void os_mem_info(void)
{
	MEM_DEBUG("struct heap_mem=%d, MEM_BLOCK_ALIGN_BYTE=%d", sizeof(struct heap_mem), MEM_BLOCK_ALIGN_BYTE);
	MEM_DEBUG("total memory: %d", mem_size_aligned);
	MEM_DEBUG("used memory : %d", used_mem);
	MEM_DEBUG("maximum allocated memory: %d", max_mem);
}
#endif
