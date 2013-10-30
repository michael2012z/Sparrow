#include <type.h>
#include <linkage.h>
#include "shell.h"
#include <printk.h>
#include <process.h>
#include <mm.h>
#include <string.h>

extern unsigned long jiffies;

static int user_thread_seed(char *elf_file_name) {
  if (elf_file_name) {
	printk(PR_SS_INI, PR_LVL_DBG5, "%s: elf = %s\n", __func__, elf_file_name);
	run_kernel_process(elf_file_name);
  } else {
	printk(PR_SS_INI, PR_LVL_DBG5, "%s: error, no elf file specified\n", __func__);
	// need to exit
  }
  return 0;
}


void handle_cmd_elfs(char *primary_parameter, char **secondary_parameters) {
  return;
}

void handle_cmd_elf(char *primary_parameter, char **secondary_parameters) {
  handle_cmd_elfs(primary_parameter, secondary_parameters);
  return;
}

void handle_cmd_elfa(char *primary_parameter, char **secondary_parameters) {
  if (NULL == primary_parameter) {
	// print help
  } else {
	char *file_name = (char *)kmalloc(strlen(primary_parameter) + 1);
	memcpy(file_name, primary_parameter, strlen(primary_parameter) + 1);
	printk(PR_SS_INI, PR_LVL_DBG5, "%s: file_name = %s\n", __func__, file_name);
	create_user_thread(user_thread_seed, file_name, secondary_parameters);
  }
  return;
}

void handle_cmd_ls(char *primary_parameter, char **secondary_parameters) {
  return;
}

void handle_cmd_jiffies(char *primary_parameter, char **secondary_parameters) {
  printu("jiffies: %d\n", jiffies);
  return;
}

void handle_cmd_vruntime(char *primary_parameter, char **secondary_parameters) {
  return;
}

void handle_cmd_help(char *primary_parameter, char **secondary_parameters) {
  return;
}

