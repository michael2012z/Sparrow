
#define BUFFER_LEADING_BYTES 8
#define MIN_ALLOCATION_SIZE 4
#define BUFFER_TRAILING_BYTES 4

unsigned long heap_start = 0;
unsigned long heap_end = 0;

struct memory_control_block {
  int allocated;
  int size;
};

unsigned long _brk(unsigned brk) {
  return 0;
}

TODO: merge

void *malloc(int size) {
  struct memory_control_block* mcb;

  /* align the size to 4 */
  size = (size + 3) & 4;

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
	  mcb->allocate = 0;
	  mcb->size = heap_end - (unsigned long)mcb - BUFFER_LEADING_BYTES - BUFFER_TRAILING_BYTES;
	}
	else {
	  printf("%s: heap expand failure\n", __func__);
	  return 0;
	}
  }

 retry:
  while (((unsigned long)mcb < heap_end) &&((mcb->allocated) || (mcb->size < size)))
	mcb = (unsigned long)mcb + BUFFER_LEADING_BYTES + mcb->size + BUFFER_TRAILING_BYTES;

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
	  mcb->allocate = 0;
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
	  mcb = (unsigned long)mcb + BUFFER_LEADING_BYTES + mcb->size + BUFFER_TRAILING_BYTES;
	  mcb->allocated = 0;
	  mcb->size = remaining_size;
	}
	mcb->allocated = 1;
  }

  return (unsigned long)mcb + BUFFER_LEADING_BYTES;
}


