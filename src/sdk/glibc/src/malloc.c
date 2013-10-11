#ifndef __ARCH_X86__
#include "../inc/stdio.h"
#else
#include <stdio.h>
#endif

#define BUFFER_LEADING_BYTES 8
#define MIN_ALLOCATION_SIZE 4
#define BUFFER_TRAILING_BYTES 4
#define MIN_BUFFER_SIZE (BUFFER_LEADING_BYTES + MIN_ALLOCATION_SIZE + BUFFER_TRAILING_BYTES)

unsigned long heap_start = 0;
unsigned long heap_end = 0;

struct memory_control_block {
  int allocated;
  int size;
};

#ifndef __ARCH_X86__
static unsigned long _brk(unsigned long brk) {
  register int res __asm__ ("r0");
  /* system call */
  asm("mov r0, %0  @ parameter 1\n"
	  "mov r1, #0  @ parameter 2\n"
	  "mov r2, #0  @ parameter 3\n"
	  "mov r3, #0  @ parameter 4\n"
	  "mov r7, #2  @ scno\n"
	  "swi #0 \n"
	  :
	  : "r"(brk)
	  : "r0");

  return res;
}
#endif

static int whole_buffer_size(struct memory_control_block *mcb) {
  return BUFFER_LEADING_BYTES + mcb->size + BUFFER_TRAILING_BYTES;
}

static struct memory_control_block *get_next_mcb(struct memory_control_block *mcb) {
  return (struct memory_control_block *)((unsigned long)mcb + BUFFER_LEADING_BYTES + mcb->size + BUFFER_TRAILING_BYTES);
}

#ifdef __ARCH_X86__
void *my_malloc(int size)
#else
void *malloc(int size)
#endif
{
  struct memory_control_block* mcb;

  /* align the size to 4 */
  size = (size + 3) & 0xfffffffc;

  /* allocator not initiated, query heap start address by brk(0) */
  if(0 == heap_start) {
    heap_start = _brk(0);
	if (0 == heap_start) {
	  printf("%s: heap can't be initiated.\n", __func__);
	  return 0;
	}
	heap_end = heap_start;
  }

  mcb = (struct memory_control_block*)heap_start;


  /* check heap_end */
  if (heap_end < heap_start) {
	printf("%s: heap error\n", __func__);
	return 0;
  } else if (heap_end == heap_start) {
	/* heap is still empty */
	unsigned long new_heap_end = heap_end + size;
	new_heap_end = _brk(new_heap_end);
	if ((0 != new_heap_end) && (heap_end != new_heap_end)) {
	  heap_end = new_heap_end;
	  mcb->allocated = 0;
	  mcb->size = heap_end - (unsigned long)mcb - BUFFER_LEADING_BYTES - BUFFER_TRAILING_BYTES;
	}
	else {
	  printf("%s: heap expand failure\n", __func__);
	  return 0;
	}
  }

 retry:
  while (((unsigned long)mcb < (heap_end - MIN_BUFFER_SIZE)) &&((mcb->allocated) || (mcb->size < size)))
	mcb = get_next_mcb(mcb);

  /* now found a proper buffer, or arrived heap_end */
  if ((unsigned long)mcb > heap_end) {
	printf("%s: heap corupt\n", __func__);
	return 0;
  } else if (((unsigned long)mcb == heap_end) || (mcb->size == 0)) {
	/* no more buffers */
	unsigned long new_heap_end = heap_end + size;
	new_heap_end = _brk(new_heap_end);
	if ((0 != new_heap_end) && (heap_end != new_heap_end)) {
	  heap_end = new_heap_end;
	  mcb->allocated = 0;
	  mcb->size = heap_end - (unsigned long)mcb - BUFFER_LEADING_BYTES - BUFFER_TRAILING_BYTES;
	  goto retry;
	}
	else {
	  printf("%s: heap expand failure\n", __func__);
	  return 0;
	}
  } else {
	/* found a buffer large enough */
	if (mcb->size >= (size + BUFFER_LEADING_BYTES + MIN_ALLOCATION_SIZE + BUFFER_TRAILING_BYTES)) {
	  /* need to split */
	  int remaining_size = mcb->size - size - BUFFER_LEADING_BYTES - BUFFER_TRAILING_BYTES;
	  mcb->size = size;
	  mcb = get_next_mcb(mcb);
	  mcb->allocated = 0;
	  mcb->size = remaining_size;
	}
	mcb->allocated = 1;
  }

  return (void *)((unsigned long)mcb + BUFFER_LEADING_BYTES);
}

#ifdef __ARCH_X86__
void my_free(void *ptr) 
#else
void free(void *ptr) 
#endif
{
  struct memory_control_block *current_mcb, *next_mcb;
  current_mcb = (struct memory_control_block *)((unsigned long)ptr - BUFFER_LEADING_BYTES);

  if (((unsigned long)current_mcb < heap_start) || ((unsigned long)current_mcb >= heap_end)) {
	printf("%s: invalid address\n", __func__);
	return;
  }

  current_mcb->allocated = 0;

  /* merge from beginning */
  current_mcb = (struct memory_control_block *)heap_start;
  next_mcb = get_next_mcb(current_mcb);

  while (((unsigned long)next_mcb < heap_end) && (0 == next_mcb->allocated)) {
	  current_mcb->size += whole_buffer_size(next_mcb);
	  next_mcb = get_next_mcb(current_mcb);	  
  }

  if ((unsigned long)next_mcb >= heap_end) {
	/* current_cmb is the last buffer, try to release it to kernel */
	heap_end = _brk((unsigned long)current_mcb);
	if (heap_end >= ((unsigned long)current_mcb + MIN_BUFFER_SIZE))
	  current_mcb->size = heap_end - (unsigned long)current_mcb - BUFFER_LEADING_BYTES - BUFFER_TRAILING_BYTES;
	else if (heap_end < (unsigned long)current_mcb) {
	  printf("%s: this should never happen\n", __func__);
	  return;
	}
  }
}
