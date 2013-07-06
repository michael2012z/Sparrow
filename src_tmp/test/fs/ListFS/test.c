#include <stdio.h>
#include <string.h>
#include <CUnit/Basic.h>

CU_TestInfo* get_fs_listfs_parser_test_cases_array();
int get_fs_listfs_parser_test_cases_count();



#define MAX_TEST_CASES 20

static CU_TestInfo test_cases_array[MAX_TEST_CASES] = {CU_TEST_INFO_NULL};


CU_TestInfo* get_fs_listfs_test_cases_array() {
  return test_cases_array;
}

int get_fs_listfs_test_cases_count() {
  return sizeof(test_cases_array)/sizeof(CU_TestInfo);
}

static _add_test_cases(CU_TestInfo *cases_array, int cases_count) {
  static int i = 0;
  int j;

  if((i + cases_count) >= MAX_TEST_CASES)
	return;

  for (j = 0; j < cases_count; j++)
	test_cases_array[i++] = cases_array[j];

  test_cases_array[i] = (CU_TestInfo)CU_TEST_INFO_NULL;;
}

void fs_listfs_test_initialize() {
  _add_test_cases(get_fs_listfs_parser_test_cases_array(), get_fs_listfs_parser_test_cases_count());
}

void fs_listfs_test_finalize() {

}
