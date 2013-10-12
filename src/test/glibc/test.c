#include <stdio.h>
#include <CUnit/Basic.h>
#include <stdlib.h>

#define MAX_TEST_CASES 50

static int test_suite_init(void);
static int test_suite_clean(void);
static void test_suite_setup(void);
static void test_suite_teardown(void);

static CU_TestInfo test_cases_array[MAX_TEST_CASES] = {CU_TEST_INFO_NULL};

CU_TestInfo* get_glibc_malloc_test_cases_array();
int get_glibc_malloc_test_cases_count();

CU_SuiteInfo glibc_test_suite = {
  "Glibc test suite",
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



void glibc_test_initialize() {
  _add_test_cases(get_glibc_malloc_test_cases_array(), get_glibc_malloc_test_cases_count());
}

void glibc_test_finalize() {

}

extern unsigned long sim_heap_start;
extern unsigned long sim_heap_end;

static int test_suite_init(void) {
  /* allocate 128K memory used for brk */
  sim_heap_start = (unsigned long)malloc(128*1024);
  if ((unsigned long)NULL == sim_heap_start)
    return -1;
  sim_heap_end = sim_heap_start;

  return 0;
}

static int test_suite_clean(void) {
  free((void *)sim_heap_start);
  return 0;
}

static void test_suite_setup(void) {}

static void test_suite_teardown(void) {}

#undef MAX_TEST_CASES
