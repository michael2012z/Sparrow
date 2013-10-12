#include <stdio.h>
#include <string.h>
#include <CUnit/Basic.h>

#define TEST_CUITES_COUNT 3
extern CU_SuiteInfo fs_test_suite;
extern CU_SuiteInfo mm_test_suite;
extern CU_SuiteInfo glibc_test_suite;

void fs_test_initialize();
void fs_test_finalize();
void mm_test_initialize();
void mm_test_finalize();
void glibc_test_initialize();
void glibc_test_finalize();

void test_initialize() {
  fs_test_initialize();
  mm_test_initialize();
  glibc_test_initialize();
}

void test_finalize() {
  fs_test_finalize();
  mm_test_finalize();
  glibc_test_finalize();
}


int main() {
  int i = 0;
  CU_ErrorCode error;
  
  test_initialize();

  CU_SuiteInfo all_test_suites[TEST_CUITES_COUNT + 1];
  all_test_suites[i++] = fs_test_suite;
  all_test_suites[i++] = mm_test_suite;
  all_test_suites[i++] = glibc_test_suite;
  all_test_suites[i++] = (CU_SuiteInfo)CU_SUITE_INFO_NULL;

  if (CUE_SUCCESS != CU_initialize_registry())
	return CU_get_error();

  error = CU_register_suites(all_test_suites);

  CU_basic_set_mode(CU_BRM_VERBOSE);
  CU_basic_run_tests();
  CU_cleanup_registry();
  error = CU_get_error();

  test_finalize();

  return error;
}
