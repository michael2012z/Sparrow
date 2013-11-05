#include <type.h>
#include <linkage.h>
#include "shell.h"
#include <printk.h>
#include <process.h>
#include <mm.h>
#include <string.h>
#include <exception.h>
#include <vfs.h>
#include <uart.h>
#include <ring_buffer.h>

extern struct sched_class *scheduler;

extern unsigned long jiffies;
extern struct sched_class *scheduler;
extern struct task_struct *current_task;
extern struct ring_buffer *kernel_ring_buffer;
extern struct ring_buffer *user_ring_buffer;

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
  if (NULL == primary_parameter) {
	// print help
  } else {
	int pid = -1;
	struct task_struct* new_task = NULL;
	printk(PR_SS_INI, PR_LVL_DBG5, "%s: primary_parameter = %s\n", __func__, primary_parameter);
	pid = create_user_thread(user_thread_seed, primary_parameter, secondary_parameters);
	// change current process state to waiting
	new_task = find_task_by_pid(pid);
	new_task->sched_en.blocked_pid = current_task->pid;
	current_task->sched_en.state = PROCESS_STATE_WAITING;
	current_task->sched_en.waiting_type = PROCESS_WAITING_TYPE_THREAD;
	current_task->sched_en.blocking_pid = pid;
	exception_disable();
	schedule();
	exception_enable();
  }
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
	printk(PR_SS_INI, PR_LVL_DBG5, "%s: primary_parameter = %s\n", __func__, primary_parameter);
	create_user_thread(user_thread_seed, primary_parameter, secondary_parameters);
  }
  return;
}

void handle_cmd_ls(char *primary_parameter, char **secondary_parameters) {
  print_fs_tree();
  return;
}

void handle_cmd_tree(char *primary_parameter, char **secondary_parameters) {
  print_fs_tree();
  return;
}

void handle_cmd_jiffies(char *primary_parameter, char **secondary_parameters) {
  printu("jiffies: %d\n", jiffies);
  return;
}

void handle_cmd_cat(char *primary_parameter, char **secondary_parameters) {
  struct file file;
  char *ch;
  vfs_node* file_node;

  if (NULL == primary_parameter)
	return;

  file_node = vfs_find_node(primary_parameter);
  if (NULL == file_node) {
	printu("file %s not found\n", primary_parameter);
  } else {  
	file.buf = file_node->file.addr;
	file.size = file_node->file.size;
	
	ch = file.buf;
	while(ch < ((char *)file.buf + file.size))
	  printu("%c", *(ch++));
	printu("\n");
  }
  return;
}

void handle_cmd_kmsg(char *primary_parameter, char **secondary_parameters) {
  ring_buffer_copy(user_ring_buffer, kernel_ring_buffer);
  uart0_tx_start();
  return;
}

void handle_cmd_ps(char *primary_parameter, char **secondary_parameters) {
  scheduler->dump();
  return;
}

void handle_cmd_help(char *primary_parameter, char **secondary_parameters) {
  return;
}

