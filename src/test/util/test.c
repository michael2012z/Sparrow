#include <stdio.h>
#include <CUnit/Basic.h>
#include <stdlib.h>
#include "ring_buffer.h"

#define MAX_TEST_CASES 50

static int test_suite_init(void);
static int test_suite_clean(void);
static void test_suite_setup(void);
static void test_suite_teardown(void);

static CU_TestInfo test_cases_array[MAX_TEST_CASES] = {CU_TEST_INFO_NULL};

CU_TestInfo* get_ring_buffer_test_cases_array();
int get_ring_buffer_test_cases_count();

CU_SuiteInfo ring_buffer_test_suite = {
  "Ring Buffer test suite",
  test_suite_init, 
  test_suite_clean, 
  //  test_suite_setup,
  //  test_suite_teardown,
  test_cases_array
};


static _add_test_cases(CU_TestInfo *cases_array, int cases_count) {
  static int i = 0;
  int j;

  if((i + cases_count) >= MAX_TEST_CASES)
	return;

  for (j = 0; j < cases_count; j++)
	test_cases_array[i++] = cases_array[j];

  test_cases_array[i] = (CU_TestInfo)CU_TEST_INFO_NULL;;
}



void ring_buffer_test_initialize() {
  _add_test_cases(get_ring_buffer_test_cases_array(), get_ring_buffer_test_cases_count());
}

void ring_buffer_test_finalize() {

}

extern struct ring_buffer *kernel_ring_buffer;
extern struct ring_buffer *user_ring_buffer;

static int test_suite_init(void) {
  kernel_ring_buffer = create_ring_buffer();
  user_ring_buffer = create_ring_buffer();
  return 0;
}

static int test_suite_clean(void) {
  free(kernel_ring_buffer->upper);
  free(kernel_ring_buffer);
  free(user_ring_buffer->upper);
  free(user_ring_buffer);
  return 0;
}

static void test_suite_setup(void) {}

static void test_suite_teardown(void) {}

#undef MAX_TEST_CASES
