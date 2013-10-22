#include <stdio.h>
#include <CUnit/Basic.h>
#include <stdlib.h>

extern int test_id;

static void cmd_parameters_check(char *primary_parameter, char **secondary_parameters) {
  switch(test_id) {
  case 0:
	CU_ASSERT(0);
	break;
  case 1:
	CU_ASSERT(0);
	break;
  case 2:
	CU_ASSERT(0);
	break;
  case 3:
	CU_ASSERT(0 == strcmp(primary_parameter, "/bin/demo_1"));
	CU_ASSERT(0 == strcmp(secondary_parameters[0], "abc"));
	CU_ASSERT(0 == strcmp(secondary_parameters[1], "def"));
	break;
  case 4:
  case 5:
  case 6:
	CU_ASSERT(0 == strcmp(primary_parameter, "/bin/demo_1"));
	CU_ASSERT(0 == strcmp(secondary_parameters[0], "abc"));
	CU_ASSERT(0 == strcmp(secondary_parameters[1], "def"));
	CU_ASSERT(0 == strcmp(secondary_parameters[2], "098"));
	CU_ASSERT(0 == strcmp(secondary_parameters[3], "&&"));
	break;
  case 7:
  case 8:
  case 9:
  case 10:
  case 11:
  case 12:
  case 13:
	CU_ASSERT(NULL == primary_parameter);
	CU_ASSERT(NULL == secondary_parameters[0]);
	CU_ASSERT(NULL == secondary_parameters[1]);
	CU_ASSERT(NULL == secondary_parameters[2]);
	CU_ASSERT(NULL == secondary_parameters[3]);
	break;
  case 14:
	CU_ASSERT(0 == strcmp(primary_parameter, "ls"));
	CU_ASSERT(NULL == secondary_parameters[0]);
	CU_ASSERT(NULL == secondary_parameters[1]);
	CU_ASSERT(NULL == secondary_parameters[2]);
	CU_ASSERT(NULL == secondary_parameters[3]);	
	break;
  }
  return;
}

void handle_cmd_elf(char *primary_parameter, char **secondary_parameters) {
  CU_ASSERT((3 == test_id) || (4 == test_id));
  cmd_parameters_check(primary_parameter, secondary_parameters);
  return;
}

void handle_cmd_elfs(char *primary_parameter, char **secondary_parameters) {
  CU_ASSERT(5 == test_id);
  cmd_parameters_check(primary_parameter, secondary_parameters);
  return;
}

void handle_cmd_elfa(char *primary_parameter, char **secondary_parameters) {
  CU_ASSERT(6 == test_id);
  cmd_parameters_check(primary_parameter, secondary_parameters);
  return;
}

void handle_cmd_ls(char *primary_parameter, char **secondary_parameters) {
  CU_ASSERT((7 == test_id) || (8 == test_id));
  cmd_parameters_check(primary_parameter, secondary_parameters);
  return;
}

void handle_cmd_jiffies(char *primary_parameter, char **secondary_parameters) {
  CU_ASSERT((9 == test_id) || (10 == test_id));
  cmd_parameters_check(primary_parameter, secondary_parameters);
  return;
}

void handle_cmd_vruntime(char *primary_parameter, char **secondary_parameters) {
  CU_ASSERT((11 == test_id) || (12 == test_id));
  cmd_parameters_check(primary_parameter, secondary_parameters);
  return;
}

void handle_cmd_help(char *primary_parameter, char **secondary_parameters) {
  CU_ASSERT((13 == test_id) || (14 == test_id));
  cmd_parameters_check(primary_parameter, secondary_parameters);
  return;
}

