/**
 * Copyright (c) 2025 Rose-Hulman Institute of Technology. All Rights Reserved.
 *
 * @author Noah James
 * @date   4/8/26
 */
#ifndef _RHMALLOC_H
#define _RHMALLOC_H

#include <stddef.h>

/**
 * This constant determines the maximum size of our heap, restricted to 1MB for
 * now.
 *
 * A real library would allocate some memory up front, then call to the OS to
 * request additional memory chunks as that initial supply is depleted. On the
 * contrary, in this assignment, we will only allocate one large blob of memory
 * upfront and then fail if that blob is fully consumed.
 */
#define MAX_HEAP_SIZE 1024 * 1024

/**
 * Align things to the next unsigned long boundary.
 */
#define ALIGNMENT sizeof(unsigned long)
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))

struct __attribute__((aligned(8))) metadata {
  unsigned size;         /* The size of the block. */
  unsigned in_use : 1;   /* Flag to indicate if the block is in use.*/
  struct metadata *next; /* The next pointer in the linked list. */
  struct metadata *prev; /* The prev pointer in the linked list. */
};

/**
 * Get the pointer to the metadata of the head of the free list.
 *
 * @return the freelist pointer.
 */
struct metadata *freelist_head(void);

/**
 * Helper to get the pointer to the start of the memory region.
 *
 * @return the pointer to the start of the heap region.
 */
void *heap_start(void);

/**
 * Initializa the RH memory allocator system.
 *
 * @return 0 on success, -1 on failure and sets errno.
 */
int rhmalloc_init(void);

/**
 * Deallocates everything and frees back all the memory to the operating system.
 *
 * This routine is useful to do between tests so that we can reset everything.
 * You should not need to modify this routine though if you use global
 * variables, it might be useful to reset their values here.
 *
 * @return 0 on success, -1 on failure and sets errno.
 */
int rhfree_all(void);

/**
 * Allocate size bytes and return a pointer to start of the region.
 *
 * @param size  The size (in bytes) of the memory area to allocate.
 *
 * @return A valid void ptr if there is enough room, 0 on error.
 */
void *rhmalloc(size_t size);

/**
 * Free a memory region and return it to the memory allocator.
 *
 * @param ptr The pointer to free.
 *
 * @warning
 *  This routine is not responsible for making sure that the free
 *  operation will not result in an error. If freeing a pointer that has already
 *  been freed, undefined behavior may occur.
 */
void rhfree(void *p);

#endif // rhmalloc.h
