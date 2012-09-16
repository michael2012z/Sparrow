#include <stdio.h>
#include "/usr/include/string.h"
#include <CUnit/Basic.h>
#include <stdlib.h>
#include <vfs.h>

static void test_case_vfs_new_node(void) {
  vfs_node *node = NULL;
  node = vfs_new_node(VFS_NODE_TYPE_DIR);
  CU_ASSERT(NULL != node);
  free(node);
}

static void test_case_vfs_new_dir(void) {
  vfs_node *node = NULL;
  node = vfs_new_dir();
  CU_ASSERT(NULL != node);
  CU_ASSERT(VFS_NODE_TYPE_DIR == node->type);
  free(node);  
}

static void test_case_vfs_new_file(void) {
  vfs_node *node = NULL;
  node = vfs_new_file();
  CU_ASSERT(NULL != node);
  CU_ASSERT(VFS_NODE_TYPE_FILE == node->type);
  free(node);  
}

static void test_case_vfs_delete_node_single(void) {
  vfs_node *node = NULL;
  node = vfs_new_file();
  vfs_delete_node(node);

  node = vfs_new_dir();
  vfs_delete_node(node);

  CU_ASSERT(1);
}

static void test_case_vfs_add_node(void) {
  int ret = 0;
  vfs_node *dir = vfs_new_dir();
  vfs_node *file = vfs_new_file();
  ret = vfs_add_node_to_dir(dir, file);
  CU_ASSERT(0 == ret);
}

static void test_case_vfs_remove_node(void) {
  int ret = 0;
  vfs_node *dir = vfs_new_dir();
  vfs_node *file = vfs_new_file();
  ret = vfs_add_node_to_dir(dir, file);
  ret = vfs_remove_node_from_dir(dir, file);
  CU_ASSERT(0 == ret);
}

static void test_case_vfs_delete_node_re(void) {
  int ret = 0;
  vfs_node *dir = vfs_new_dir();
  vfs_node *file = vfs_new_file();
  ret = vfs_add_node_to_dir(dir, file);
  file = vfs_new_file();
  ret = vfs_add_node_to_dir(dir, file);

  vfs_delete_node(dir);
  CU_ASSERT(1);
}

static void test_case_vfs_print_file(void) {
  vfs_node *file = vfs_new_file();
  strcpy(file->name, "file 1");
  vfs_print_file(file);
  CU_ASSERT(1);
}

static void test_case_vfs_print_dir(void) {
  vfs_node *dir = vfs_new_dir();
  strcpy(dir->name, "dir 1");
  vfs_print_dir(dir);
  CU_ASSERT(1);
}

static void test_case_vfs_print_re(void) {
  int ret = 0;
  vfs_node *dir, *root, *file;

  //  printf("\n");

  root = vfs_new_dir();
  strcpy(root->name, "root");

  dir = vfs_new_dir();
  strcpy(dir->name, "dir 1");
  ret = vfs_add_node_to_dir(root, dir);

  file = vfs_new_file();
  strcpy(file->name, "file 11");
  ret = vfs_add_node_to_dir(dir, file);

  file = vfs_new_file();
  strcpy(file->name, "file 12");
  ret = vfs_add_node_to_dir(dir, file);

  dir = vfs_new_dir();
  strcpy(dir->name, "dir 2");
  ret = vfs_add_node_to_dir(root, dir);

  file = vfs_new_file();
  strcpy(file->name, "file 21");
  ret = vfs_add_node_to_dir(dir, file);

  file = vfs_new_file();
  strcpy(file->name, "file 22");
  ret = vfs_add_node_to_dir(dir, file);

  vfs_print_node_re(root);
  CU_ASSERT(1);
}

static CU_TestInfo test_cases_array[] = {
  {"VFS node: test_case_vfs_new_node", test_case_vfs_new_node},
  {"VFS node: test_case_vfs_new_dir", test_case_vfs_new_dir},
  {"VFS node: test_case_vfs_new_file", test_case_vfs_new_file},
  {"VFS node: test_case_vfs_delete_node_single", test_case_vfs_delete_node_single},
  {"VFS node: test_case_vfs_add_node", test_case_vfs_add_node},
  {"VFS node: test_case_vfs_remove_node", test_case_vfs_remove_node},
  {"VFS node: test_case_vfs_delete_node_re", test_case_vfs_delete_node_re},
  {"VFS node: test_case_vfs_print_file", test_case_vfs_print_file},
  {"VFS node: test_case_vfs_print_dir", test_case_vfs_print_dir},
  {"VFS node: test_case_vfs_print_re", test_case_vfs_print_re},
};

CU_TestInfo* get_fs_node_test_cases_array() {
  return test_cases_array;
}

int get_fs_node_test_cases_count() {
  return sizeof(test_cases_array)/sizeof(CU_TestInfo);
}
