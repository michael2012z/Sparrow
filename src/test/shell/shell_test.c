#include <stdio.h>
#include <CUnit/Basic.h>
#include <stdlib.h>

int test_id = 0;
int char_index = 0;


char* test_cmds[15] = {
  "\n",
  "     \n   ",
  "  sfdds   \n",
  "elf /bin/demo_1 abc def\n",
  "   elf    /bin/demo_1  abc    def  098   &&   \n",
  "elfs /bin/demo_1 abc def 098 && xx xx\n",
  "elfa /bin/demo_1 abc def 098 && x       \n",
  "ls\n",
  "   ls  /bin\n",
  "jiffies\n",
  "    jiffies  \n",
  "vruntime\n",
  " vruntime                    \n",
  "help\n",
  "    help    ls  \n",
};

char inputc() {
  return test_cmds[test_id][char_index++];
}

static void test_case_common(int id) {
  test_id = id;
  char_index = 0; 
  kernel_shell();
}

static void test_case_01(void) {
  test_case_common(0);
  return;
}

static void test_case_02(void) {
  test_case_common(1);
  return;
}

static void test_case_03(void) {
  test_case_common(2);
  return;
}

static void test_case_04(void) {
  test_case_common(3);
  return;
}

static void test_case_05(void) {
  test_case_common(4);
  return;
}

static void test_case_06(void) {
  test_case_common(5);
  return;
}

static void test_case_07(void) {
  test_case_common(6);
  return;
}

static void test_case_08(void) {
  test_case_common(7);
  return;
}

static void test_case_09(void) {
  test_case_common(8);
  return;
}

static void test_case_10(void) {
  test_case_common(9);
  return;
}

static void test_case_11(void) {
  test_case_common(10);
  return;
}

static void test_case_12(void) {
  test_case_common(11);
  return;
}

static void test_case_13(void) {
  test_case_common(12);
  return;
}

static void test_case_14(void) {
  test_case_common(13);
  return;
}

static void test_case_15(void) {
  test_case_common(14);
  return;
}


static CU_TestInfo test_cases_array[] = {
  {"Shell malloc: test_case_01", test_case_01},
  {"Shell malloc: test_case_02", test_case_02},
  {"Shell malloc: test_case_03", test_case_03},
  {"Shell malloc: test_case_04", test_case_04},
  {"Shell malloc: test_case_05", test_case_05},
  {"Shell malloc: test_case_06", test_case_06},
  {"Shell malloc: test_case_07", test_case_07},
  {"Shell malloc: test_case_08", test_case_08},
  {"Shell malloc: test_case_09", test_case_09},
  {"Shell malloc: test_case_10", test_case_10},
  {"Shell malloc: test_case_11", test_case_11},
  {"Shell malloc: test_case_12", test_case_12},
  {"Shell malloc: test_case_13", test_case_13},
  {"Shell malloc: test_case_14", test_case_14},
  {"Shell malloc: test_case_15", test_case_15},
};

CU_TestInfo* get_shell_test_cases_array() {
  return test_cases_array;
}

int get_shell_test_cases_count() {
  return sizeof(test_cases_array)/sizeof(CU_TestInfo);
}
