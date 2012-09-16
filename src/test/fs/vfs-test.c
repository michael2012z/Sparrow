#include <stdio.h>
#include <string.h>
#include <CUnit/Basic.h>


static void test_case_1 (void)
{
  CU_ASSERT(1);
}

static void test_case_2 (void)
{
  CU_ASSERT(1);
}

static void test_case_3 (void)
{
  CU_ASSERT(1);
}


static CU_TestInfo test_cases_array[] = {

};

CU_TestInfo* get_fs_vfs_test_cases_array() {
  return test_cases_array;
}

int get_fs_vfs_test_cases_count() {
  return sizeof(test_cases_array)/sizeof(CU_TestInfo);
}

