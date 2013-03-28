#include <stdio.h>
#include <string.h>
#include <CUnit/Basic.h>

#define MAX_TEST_CASES 50

static int test_suite_init(void);
static int test_suite_clean(void);
static void test_suite_setup(void);
static void test_suite_teardown(void);

static CU_TestInfo test_cases_array[MAX_TEST_CASES] = {CU_TEST_INFO_NULL};

CU_SuiteInfo fs_test_suite = {
  "File System test suite",
  test_suite_init, 
  test_suite_clean, 
  //  test_suite_setup,
  //  test_suite_teardown,
  test_cases_array
};


void fs_listfs_test_initialize();
void fs_listfs_test_finalize();

CU_TestInfo* get_fs_node_test_cases_array();
int get_fs_node_test_cases_count();
CU_TestInfo* get_fs_listfs_test_cases_array();
int get_fs_listfs_test_cases_count();
CU_TestInfo* get_fs_vfs_test_cases_array();
int get_fs_vfs_test_cases_count();


static _add_test_cases(CU_TestInfo *cases_array, int cases_count) {
  static int i = 0;
  int j;

  if((i + cases_count) >= MAX_TEST_CASES)
	return;

  for (j = 0; j < cases_count; j++)
	test_cases_array[i++] = cases_array[j];

  test_cases_array[i] = (CU_TestInfo)CU_TEST_INFO_NULL;;
}

void fs_test_initialize() {
  fs_listfs_test_initialize();
  _add_test_cases(get_fs_node_test_cases_array(), get_fs_node_test_cases_count());
  _add_test_cases(get_fs_listfs_test_cases_array(), get_fs_listfs_test_cases_count());
  _add_test_cases(get_fs_vfs_test_cases_array(), get_fs_vfs_test_cases_count());
}

void fs_test_finalize() {
  fs_listfs_test_finalize();
}

static int test_suite_init(void) {
  return 0;
}

static int test_suite_clean(void) {
  return 0;
}

static void test_suite_setup(void) {

}

static void test_suite_teardown(void) {

}

#undef MAX_TEST_CASES
