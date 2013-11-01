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

#endif /* _RING_BUFFER_H_ */
