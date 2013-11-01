#include <type.h>
#include <linkage.h>
#include <mm.h>
#include "ring_buffer.h"

struct ring_buffer *kernel_ring_buffer = NULL;
struct ring_buffer *user_ring_buffer = NULL;

struct ring_buffer *create_ring_buffer() {
  struct ring_buffer *ring = (struct ring_buffer *)kmalloc(sizeof(struct ring_buffer));
  if (NULL == ring)
	return NULL;

  ring->upper = (char *)kmalloc(RING_BUFFER_SIZE);
  if (NULL == ring->upper)
	return NULL;
  else {
	ring->lower = ring->upper + RING_BUFFER_SIZE;
	ring->start = ring->upper;
	ring->end = ring->upper;
	ring->circled = 0;
	return 0;
  }
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
