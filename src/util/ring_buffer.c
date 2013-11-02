#ifndef __ARCH_X86__
#include <type.h>
#include <linkage.h>
#include <mm.h>
#else
#include <stdlib.h>
#endif
#include "ring_buffer.h"
#include <printk.h>

struct ring_buffer *kernel_ring_buffer = NULL;
struct ring_buffer *user_ring_buffer = NULL;

int ring_buffer_enabled = 0;

void ring_buffer_init() {
  kernel_ring_buffer = create_ring_buffer();
  if (NULL == kernel_ring_buffer) {
	printk(PR_SS_INI, PR_LVL_ERR, "%s: kernel ring buffer init failure\n", __func__);
	while(1);
  }
  user_ring_buffer = create_ring_buffer();
  if (NULL == user_ring_buffer) {
	printk(PR_SS_INI, PR_LVL_ERR, "%s: user ring buffer init failure\n", __func__);
	while(1);
  }
  ring_buffer_enabled = 1;
}

struct ring_buffer *create_ring_buffer() {
#ifndef __ARCH_X86__
  struct ring_buffer *ring = (struct ring_buffer *)kmalloc(sizeof(struct ring_buffer));
#else
  struct ring_buffer *ring = (struct ring_buffer *)malloc(sizeof(struct ring_buffer));
#endif

  if (NULL == ring)
	return NULL;

#ifndef __ARCH_X86__
  ring->upper = (char *)kmalloc(RING_BUFFER_SIZE);
#else
  ring->upper = (char *)malloc(RING_BUFFER_SIZE);
#endif

  if (NULL == ring->upper)
	return NULL;
  else {
	ring->lower = ring->upper + RING_BUFFER_SIZE;
	ring->start = ring->upper;
	ring->end = ring->upper;
	ring->circled = 0;
	return ring;
  }
}

void ring_buffer_reset(struct ring_buffer *ring) {
  ring->start = ring->upper;
  ring->end = ring->upper;
  ring->circled = 0;
}

void ring_buffer_put_char(struct ring_buffer *ring, char ch) {
  if (0 == ring->circled) { /* no circle */
	*(ring->end++) = ch;
	if (ring->end >= ring->lower) {
	  ring->end = ring->upper;
	  ring->circled = 1;
	}
  } else { /* circled */
	*(ring->end++) = ch;
	if (ring->end > ring->start)
	  ring->start = ring->end;
	if (ring->end >= ring->lower)
	  ring->end = ring->upper;
	if (ring->start >= ring->lower)
	  ring->start = ring->upper;
  }
}

char ring_buffer_get_char(struct ring_buffer *ring) {
  char ch;
  if (0 == ring->circled) { /* no circle */
	if (ring->start == ring->end) /* empty buffer*/
	  ch = '\0';
	else {
	  ch = *(ring->start++);
	}
	if (ring->start >= ring->lower) {
	  ring->start = ring->upper;
	  ring->end = ring->upper;
	}
  } else { /* circled */
	ch = *(ring->start++);
	if (ring->start >= ring->lower) {
	  ring->start = ring->upper;
	  ring->circled = 0;
	}
  }
  return ch;
}

int ring_buffer_empty(struct ring_buffer *ring) {
  if ((0 == ring->circled) && (ring->start == ring->end))
	return 1;
  else
	return 0;
}

void ring_buffer_copy(struct ring_buffer *des, struct ring_buffer *src) {
  char ch;
  while (!ring_buffer_empty(src)) {
	ch = ring_buffer_get_char(src);
	ring_buffer_put_char(des, ch);
  }
}
