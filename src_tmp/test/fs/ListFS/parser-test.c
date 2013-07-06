#include <stdio.h>
#include "/usr/include/string.h"
#include <CUnit/Basic.h>
#include <stdlib.h>
#include <vfs.h>
#include <fs/ListFS/parser.h>

static char* read_file(char *file_name) {
  FILE * fp;
  //printf("read file : %s\n", file_name);

  if((fp=fopen(file_name,"r"))== NULL) {
	CU_ASSERT(0);
	return NULL;
  } else {
	char *buffer = (char *)malloc(1024*8);
	char block[65];
	int bytes_reading, bytes_read = 0;
	memset(buffer, '\0', 1024*8);
	do {
	  memset(block, '\0', 65);
	  bytes_reading = fread(block, 1, 64, fp);
	  block[bytes_reading] = '\0';
	  // printf("reading %d bytes : %s\n", bytes_reading, block);
	  memcpy((buffer+bytes_read), block, bytes_reading);
	  bytes_read += bytes_reading;
	  if (bytes_read > 1024*8)
		CU_ASSERT(0);
	} while (bytes_reading > 0);
	fclose(fp);
	return buffer;
  }
}

static void test_case_sample_common(char* file_name) {
  char * buffer = read_file(file_name);
  vfs_node* root;
  //  printf("\nFile content (%s): \b%s\n", file_name, buffer);

  listfs_parser_set_buffer(buffer);
  listfs_parser_parse_buffer();
  root = listfs_parser_get_root();

  /*
  printf("\n");
  vfs_print_node_re(root);
  */

  free(buffer);
  return;
}

static void test_case_sample_01(void) {
  test_case_sample_common("fs/ListFS/sample-01.xml");
  CU_ASSERT(1);
}

static void test_case_sample_02(void) {
  test_case_sample_common("fs/ListFS/sample-02.xml");
  CU_ASSERT(1);
}

static void test_case_sample_03(void) {
  test_case_sample_common("fs/ListFS/sample-03.xml");
  CU_ASSERT(1);
}

static void test_case_sample_04(void) {
  test_case_sample_common("fs/ListFS/sample-04.xml");
  CU_ASSERT(1);
}

static void test_case_sample_05(void) {
  test_case_sample_common("fs/ListFS/sample-05.xml");
  CU_ASSERT(1);
}

static void test_case_sample_06(void) {
  test_case_sample_common("fs/ListFS/sample-06.xml");
  CU_ASSERT(1);
}

static void test_case_sample_07(void) {
  test_case_sample_common("fs/ListFS/sample-07.xml");
  CU_ASSERT(1);
}

static void test_case_sample_08(void) {
  test_case_sample_common("fs/ListFS/sample-08.xml");
  CU_ASSERT(1);
}

static CU_TestInfo test_cases_array[] = {
  {"ListFS parser: test_case_sample_01", test_case_sample_01},
  {"ListFS parser: test_case_sample_02", test_case_sample_02},
  {"ListFS parser: test_case_sample_03", test_case_sample_03},
  {"ListFS parser: test_case_sample_04", test_case_sample_04},
  {"ListFS parser: test_case_sample_05", test_case_sample_05},
  {"ListFS parser: test_case_sample_06", test_case_sample_06},
  {"ListFS parser: test_case_sample_07", test_case_sample_07},
  {"ListFS parser: test_case_sample_08", test_case_sample_08},
};

CU_TestInfo* get_fs_listfs_parser_test_cases_array() {
  return test_cases_array;
}

int get_fs_listfs_parser_test_cases_count() {
  return sizeof(test_cases_array)/sizeof(CU_TestInfo);
}
