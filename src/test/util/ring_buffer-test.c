#include <stdio.h>
#include <CUnit/Basic.h>
#include <stdlib.h>
#include "ring_buffer.h"

extern struct ring_buffer *kernel_ring_buffer;
extern struct ring_buffer *user_ring_buffer;

static void ring_buffer_check(struct ring_buffer *ring) {
  CU_ASSERT((ring->start >= ring->upper) && (ring->start < ring->lower));
  CU_ASSERT((ring->end >= ring->upper) && (ring->end < ring->lower));
}

static void test_case_01(void) {
  /* put 10 chars */
  struct ring_buffer *ring = user_ring_buffer;
  int i = 0;
  ring_buffer_reset(ring);
  for(i = 0; i < 10; i++) {
	ring_buffer_put_char(ring, 'c');
	ring_buffer_check(ring);	
  }
  CU_ASSERT(ring->start == ring->upper);
  CU_ASSERT(ring->end == (ring->upper + 10));
  return;
}

static void test_case_02(void) {
  /* put 10 chars, get 8 chars */
  struct ring_buffer *ring = user_ring_buffer;
  int i = 0;
  ring_buffer_reset(ring);
  for(i = 0; i < 10; i++) {
	ring_buffer_put_char(ring, 'c');
	ring_buffer_check(ring);	
  }
  for(i = 0; i < 8; i++) {
	ring_buffer_get_char(ring);
	ring_buffer_check(ring);	
  }

  CU_ASSERT(ring->start == (ring->upper + 8));
  CU_ASSERT(ring->end == (ring->upper + 10));
  return;
}

static void test_case_03(void) {
  /* put 20 chars, get 30 chars */
  struct ring_buffer *ring = user_ring_buffer;
  int i = 0;
  ring_buffer_reset(ring);
  for(i = 0; i < 20; i++) {
	ring_buffer_put_char(ring, 'c');
	ring_buffer_check(ring);	
  }
  for(i = 0; i < 30; i++) {
	ring_buffer_get_char(ring);
	ring_buffer_check(ring);	
  }

  CU_ASSERT(ring->start == (ring->upper + 20));
  CU_ASSERT(ring->end == (ring->upper + 20));
  return;
}

static void test_case_04(void) {
  /* put 4K chars, get 4K chars */
  struct ring_buffer *ring = user_ring_buffer;
  int i = 0;
  ring_buffer_reset(ring);
  for(i = 0; i < RING_BUFFER_SIZE; i++) {
	ring_buffer_put_char(ring, 'c');
	ring_buffer_check(ring);	
  }
  CU_ASSERT(ring->start == (ring->upper));
  CU_ASSERT(ring->end == (ring->upper));
  CU_ASSERT(ring->circled == 1);

  for(i = 0; i < RING_BUFFER_SIZE; i++) {
	ring_buffer_get_char(ring);
	ring_buffer_check(ring);	
  }
  CU_ASSERT(ring->start == (ring->upper));
  CU_ASSERT(ring->end == (ring->upper));
  CU_ASSERT(ring->circled == 0);

  return;
}

static void test_case_05(void) {
  /* put 5K chars, get 2K chars */
  struct ring_buffer *ring = user_ring_buffer;
  int i = 0;
  ring_buffer_reset(ring);
  for(i = 0; i < 5*1024; i++) {
	ring_buffer_put_char(ring, 'c');
	ring_buffer_check(ring);	
  }
  CU_ASSERT(ring->start == (ring->upper + 1024));
  CU_ASSERT(ring->end == (ring->upper + 1024));
  CU_ASSERT(ring->circled == 1);

  for(i = 0; i < 2*1024; i++) {
	ring_buffer_get_char(ring);
	ring_buffer_check(ring);	
  }
  CU_ASSERT(ring->start == (ring->upper + 3*1024));
  CU_ASSERT(ring->end == (ring->upper + 1024));
  CU_ASSERT(ring->circled == 1);

  return;
}

static void test_case_06(void) {
  /* put 5K chars, get 3K chars */
  struct ring_buffer *ring = user_ring_buffer;
  int i = 0;
  ring_buffer_reset(ring);
  for(i = 0; i < 5*1024; i++) {
	ring_buffer_put_char(ring, 'c');
	ring_buffer_check(ring);	
  }
  CU_ASSERT(ring->start == (ring->upper + 1024));
  CU_ASSERT(ring->end == (ring->upper + 1024));
  CU_ASSERT(ring->circled == 1);

  for(i = 0; i < 3*1024; i++) {
	ring_buffer_get_char(ring);
	ring_buffer_check(ring);	
  }
  CU_ASSERT(ring->start == (ring->upper));
  CU_ASSERT(ring->end == (ring->upper + 1024));
  CU_ASSERT(ring->circled == 0);

  return;
}

static void test_case_07(void) {
  /* put 5K chars, get 4K chars */
  struct ring_buffer *ring = user_ring_buffer;
  int i = 0;
  ring_buffer_reset(ring);
  for(i = 0; i < 5*1024; i++) {
	ring_buffer_put_char(ring, 'c');
	ring_buffer_check(ring);	
  }
  CU_ASSERT(ring->start == (ring->upper + 1024));
  CU_ASSERT(ring->end == (ring->upper + 1024));
  CU_ASSERT(ring->circled == 1);

  for(i = 0; i < 4*1024; i++) {
	ring_buffer_get_char(ring);
	ring_buffer_check(ring);	
  }
  CU_ASSERT(ring->start == (ring->upper + 1024));
  CU_ASSERT(ring->end == (ring->upper + 1024));
  CU_ASSERT(ring->circled == 0);

  return;
}

static void test_case_08(void) {
  /* put 5K chars, get 6K chars */
  struct ring_buffer *ring = user_ring_buffer;
  int i = 0;
  ring_buffer_reset(ring);
  for(i = 0; i < 5*1024; i++) {
	ring_buffer_put_char(ring, 'c');
	ring_buffer_check(ring);	
  }
  CU_ASSERT(ring->start == (ring->upper + 1024));
  CU_ASSERT(ring->end == (ring->upper + 1024));
  CU_ASSERT(ring->circled == 1);

  for(i = 0; i < 6*1024; i++) {
	ring_buffer_get_char(ring);
	ring_buffer_check(ring);	
  }
  CU_ASSERT(ring->start == (ring->upper + 1024));
  CU_ASSERT(ring->end == (ring->upper + 1024));
  CU_ASSERT(ring->circled == 0);

  return;
}

static void test_case_09(void) {
  struct ring_buffer *ring = user_ring_buffer;
  ring_buffer_reset(ring);
  ring_buffer_check(ring);
  CU_ASSERT(ring_buffer_empty(ring));
  return;
}

static void ring_buffer_presure_test(struct ring_buffer *ring, int repeat) {
  int write_read = 0;
  int count = 0;

  ring_buffer_reset(ring);

  while(repeat--) {
	write_read = random()%2;
	if (write_read == 0) { /* write */
	  count = random()%64;
	  printf("count = %d\n", count);
	  while(count--)
		ring_buffer_put_char(ring, 'x');
	  ring_buffer_check(ring);
	} else { /* read */
	  count = random()%64;
	  printf(" count= %d\n", count);
	  while(count--)
		ring_buffer_get_char(ring);
	  ring_buffer_check(ring);
	}
  }

  count = RING_BUFFER_SIZE;
  while(count--)
	ring_buffer_get_char(ring);

  CU_ASSERT(ring->start == ring->end);
  CU_ASSERT(ring->circled == 0);
  CU_ASSERT(ring_buffer_empty(ring));
  return;
}

static void test_case_10(void) {
  ring_buffer_presure_test(user_ring_buffer, 32);
  return;
}

static void test_case_11(void) {
  ring_buffer_presure_test(user_ring_buffer, 256);
  return;
}

static void test_case_12(void) {
  ring_buffer_presure_test(user_ring_buffer, 4094);
  return;
}

static CU_TestInfo test_cases_array[] = {
  {"Ring Buffer: test_case_01", test_case_01},
  {"Ring Buffer: test_case_02", test_case_02},
  {"Ring Buffer: test_case_03", test_case_03},
  {"Ring Buffer: test_case_04", test_case_04},
  {"Ring Buffer: test_case_05", test_case_05},
  {"Ring Buffer: test_case_06", test_case_06},
  {"Ring Buffer: test_case_07", test_case_07},
  {"Ring Buffer: test_case_08", test_case_08},
  {"Ring Buffer: test_case_09", test_case_09},
  {"Ring Buffer: test_case_10", test_case_10},
  {"Ring Buffer: test_case_11", test_case_11},
  {"Ring Buffer: test_case_12", test_case_12},
};

CU_TestInfo* get_ring_buffer_test_cases_array() {
  return test_cases_array;
}

int get_ring_buffer_test_cases_count() {
  return sizeof(test_cases_array)/sizeof(CU_TestInfo);
}
