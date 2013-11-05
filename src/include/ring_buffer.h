#ifndef _RING_BUFFER_H_
#define _RING_BUFFER_H_

#define RING_BUFFER_SIZE 0x1000 /* size = 4K */

struct ring_buffer {
  char *upper;
  char *lower;
  char *start;
  char *end;
  int circled;
};

struct ring_buffer *create_ring_buffer();
void ring_buffer_reset(struct ring_buffer *ring);
void ring_buffer_put_char(struct ring_buffer *ring, char ch);
char ring_buffer_get_char(struct ring_buffer *ring);
int ring_buffer_empty(struct ring_buffer *ring);
void ring_buffer_copy(struct ring_buffer *des, struct ring_buffer *src);

#endif /* _RING_BUFFER_H_ */
