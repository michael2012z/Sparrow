#include <stdio.h>
#include <string.h>
#include <CUnit/Basic.h>
#include <stdlib.h>
#include <type.h>
#include <mm.h>

#define MAX_TEST_CASES 50

static int test_suite_init(void);
static int test_suite_clean(void);
static void test_suite_setup(void);
static void test_suite_teardown(void);

static CU_TestInfo test_cases_array[MAX_TEST_CASES] = {CU_TEST_INFO_NULL};

CU_TestInfo* get_mm_page_allocator_test_cases_array();
int get_mm_page_allocator_test_cases_count();
CU_TestInfo* get_mm_slab_allocator_test_cases_array();
int get_mm_slab_allocator_test_cases_count();
CU_TestInfo* get_mm_allocator_test_cases_array();
int get_mm_allocator_test_cases_count();

CU_SuiteInfo mm_test_suite = {
  "Memory Management test suite",
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



void mm_test_initialize() {
  _add_test_cases(get_mm_page_allocator_test_cases_array(), get_mm_page_allocator_test_cases_count());
  _add_test_cases(get_mm_slab_allocator_test_cases_array(), get_mm_slab_allocator_test_cases_count());
  _add_test_cases(get_mm_allocator_test_cases_array(), get_mm_allocator_test_cases_count());
}

void mm_test_finalize() {

}

extern struct page* pages_map;
void * test_page_offset;
static int test_suite_init(void) {
  /* page allocator */
  pages_map = (struct page *)malloc(MEMORY_PAGES * sizeof(struct page));
  if (NULL == pages_map)
    return -1;

  /* page allocator ~ */

  /* slab allocator */
  test_page_offset = (void *)malloc(MEMORY_PAGES * PAGE_SIZE);
  if (NULL == test_page_offset)
    return -1;

  /* slab allocator ~ */

  return 0;
}

static int test_suite_clean(void) {
  /* De-allocate memory for page allocator. */
  free(pages_map);
  return 0;
}

static void test_suite_setup(void) {}

static void test_suite_teardown(void) {}

#undef MAX_TEST_CASES
