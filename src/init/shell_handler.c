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
#include <setup.h>

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
	printu("executable file not specified\n");
  } else {
	vfs_node* file;
	file = vfs_find_node(primary_parameter);
	if (NULL == file)
	  printu("file not exist\n");
	else {
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
  }
  return;
}

void handle_cmd_elf(char *primary_parameter, char **secondary_parameters) {
  handle_cmd_elfs(primary_parameter, secondary_parameters);
  return;
}

void handle_cmd_elfa(char *primary_parameter, char **secondary_parameters) {
  if (NULL == primary_parameter) {
	printu("executable file not specified\n");
	// print help
  } else {
	vfs_node* file;
	file = vfs_find_node(primary_parameter);
	if (NULL == file)
	  printu("file not exist\n");
	else
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
  exception_disable();
  ring_buffer_copy(user_ring_buffer, kernel_ring_buffer);
  exception_enable();
  uart0_tx_start();
  return;
}

void handle_cmd_ps(char *primary_parameter, char **secondary_parameters) {
  scheduler->dump();
  return;
}

void handle_cmd_kill(char *primary_parameter, char **secondary_parameters) {
  if (NULL == primary_parameter)
	printu("invalid parameter\n");
  else {
	int pid;
	struct task_struct *task;
	pid = atoi(primary_parameter);
	if (pid >= 0) {
	  task = find_task_by_pid(pid);
	  if (NULL != task) {
		task->sched_en.state = PROCESS_STATE_DEAD;
		printu("process %d killed\n", pid);
	  }
	  else
		printu("process not found\n");
	} else
	  printu("process not found\n");
  }
  return;
}

void handle_cmd_log(char *primary_parameter, char **secondary_parameters) {
  if ((NULL != primary_parameter) &&
      (NULL != secondary_parameters[0]) && 
      (NULL == secondary_parameters[1]) && 
      (NULL == secondary_parameters[2]) && 
      (NULL == secondary_parameters[3])) {
    int n, i;
    /* clean all log type and level */
    log_type_ini = 0;
    log_type_fs = 0;
    log_type_mm = 0;
    log_type_proc = 0;
    log_type_irq = 0;
    log_level_err = 0;
    log_level_inf = 0;
    log_level_wrn = 0;
    log_level_0 = 0;
    log_level_1 = 0;
    log_level_2 = 0;
    log_level_3 = 0;
    log_level_4 = 0;
    log_level_5 = 0;
    log_level_6 = 0;
    log_level_7 = 0;
    log_level_8 = 0;
    log_level_9 = 0;
    
    n = strlen(primary_parameter);
    for (i = 0; i < n; i++)
      switch (primary_parameter[i]) {
      case 'i':
	log_type_ini = 1;
	break;
      case 'm':
	log_type_mm = 1;
	break;
      case 'p':
	log_type_proc = 1;
	break;
      case 'q':
	log_type_irq = 1;
	break;
      case 'f':
	log_type_fs = 1;
	break;
      default:
	printu("wrong log type: %c\n", primary_parameter[i]);
	break;
      }
    n = strlen(secondary_parameters[0]);
    for (i = 0; i < n; i++)
      switch (secondary_parameters[0][i]) {
      case 'w':
	log_level_wrn = 1;
	break;
      case 'e':
	log_level_err = 1;
	break;
      case 'n':
	log_level_inf = 1;
	break;
      case '0':
	log_level_0 = 1;
	break;
      case '1':
	log_level_1 = 1;
	break;
      case '2':
	log_level_2 = 1;
	break;
      case '3':
	log_level_3 = 1;
	break;
      case '4':
	log_level_4 = 1;
	break;
      case '5':
	log_level_5 = 1;
	break;
      case '6':
	log_level_6 = 1;
	break;
      case '7':
	log_level_7 = 1;
	break;
      case '8':
	log_level_8 = 1;
	break;
      case '9':
	log_level_9 = 1;
	break;
      default:
	printu("wrong log level: %c\n", secondary_parameters[0][i]);
	break;
      }

    primary_parameter = NULL;
    secondary_parameters[0] = NULL;
  } 
  
  if ((NULL == primary_parameter) &&
      (NULL == secondary_parameters[0]) && 
      (NULL == secondary_parameters[1]) && 
      (NULL == secondary_parameters[2]) && 
      (NULL == secondary_parameters[3])) {
    char buf[16] = {0};
    int i = 0;
    
    for (i = 0; i < 16; i++)
      buf[i] = '\0';
    
    i = 0;
    /* check log type */
    if (1 == log_type_ini)
      buf[i++] = 'i';
    if (1 == log_type_mm)
      buf[i++] = 'm';
    if (1 == log_type_proc)
      buf[i++] = 'p';
    if (1 == log_type_irq)
      buf[i++] = 'q';
    if (1 == log_type_fs)
      buf[i++] = 'f';
    printu("  type : %s\n", buf);
    
    
    for (i = 0; i < 16; i++)
      buf[i] = '\0';
    
    i = 0;
    /* check log level */
    if (1 == log_level_wrn)
      buf[i++] = 'w';
    if (1 == log_level_inf)
      buf[i++] = 'n';
    if (1 == log_level_err)
      buf[i++] = 'e';
    if (1 == log_level_0)
      buf[i++] = '0';
    if (1 == log_level_1)
      buf[i++] = '1';
    if (1 == log_level_2)
      buf[i++] = '2';
    if (1 == log_level_3)
      buf[i++] = '3';
    if (1 == log_level_4)
      buf[i++] = '4';
    if (1 == log_level_5)
      buf[i++] = '5';
    if (1 == log_level_6)
      buf[i++] = '6';
    if (1 == log_level_7)
      buf[i++] = '7';
    if (1 == log_level_8)
      buf[i++] = '8';
    if (1 == log_level_9)
      buf[i++] = '9';
    printu("  level: %s\n", buf);
    
  } else
    printu("wrong number of parameters\n");
  
  return;
}

void handle_cmd_reset(char *primary_parameter, char **secondary_parameters) {
  printu("reseting ... \n");
  arm_cpu_reset();
}

void handle_cmd_help(char *primary_parameter, char **secondary_parameters) {
  int i = 0;
  for (i = 0; i < 4; i++)
	if (NULL != secondary_parameters[i]) {
	  printu("wrong number of parameters\n");
	  return;
	}

  if (NULL == primary_parameter) {
	printu("Usage: help <command>\n");
	printu("Get help of all shell commands or specified command.\n");
	printu("Supported commands:\n");
	printu("    elfs <file> [parameters...]       execute ELF file in synchronized mode\n");
	printu("    elfa <file> [parameters...]       execute ELF file in asynchronized mode\n");
	printu("    elf  <file> [parameters...]       same as 'elfs' command\n");
	printu("    tree                              display tree view of file system\n");
	printu("    ls                                same as 'tree' command\n");
	printu("    jiffies                           display jiffies/ticks since system start\n");
	printu("    cat  <file>                       dump content of specified file\n");
	printu("    kmsg                              dump kernel log\n");
	printu("    ps                                display live process information\n");
	printu("    kill <pid >                       terminate specified process\n");
	printu("    log  [type] [level]               enable kernel log types and levels\n");
	printu("    reset                             restart the system\n");
	printu("    help [command]                    print help information\n");
  } else {
	char *cmd = primary_parameter;
	if (0 == strcmp(cmd, "elf")) {
	  printu("Usage: elf <file> [param1] [param2] [param3] [param4]\n");
	  printu("Execute ELF file in synchronized mode.\n");
	} else if (0 == strcmp(cmd, "elfs")) {
	  printu("Usage: elfs <file> [param1] [param2] [param3] [param4]\n");
	  printu("Execute ELF file in synchronized mode.\n");
	} else if (0 == strcmp(cmd, "elfa")) {
	  printu("Usage: elfa <file> [param1] [param2] [param3] [param4]\n");
	  printu("Execute ELF file in asynchronized mode.\n");
	} else if (0 == strcmp(cmd, "ls")) {
	  printu("Usage: ls\n");
	  printu("Display tree view of file system.\n");
	} else if (0 == strcmp(cmd, "tree")) {
	  printu("Usage: tree\n");
	  printu("Display tree view of file system.\n");
	} else if (0 == strcmp(cmd, "jiffies")) {
	  printu("Usage: jiffies\n");
	  printu("Display jiffies/ticks since system start.\n");
	} else if (0 == strcmp(cmd, "cat")) {
	  printu("Usage: cat <file>\n");
	  printu("Dump content of specified file.\n");
	} else if (0 == strcmp(cmd, "kmsg")) {
	  printu("Usage: kmsg\n");
	  printu("Dump kernel log to screen.\n");
	} else if (0 == strcmp(cmd, "ps")) {
	  printu("Usage: ps\n");
	  printu("Display live process information.\n");
	} else if (0 == strcmp(cmd, "kill")) {
	  printu("Usage: kill <pid>\n");
	  printu("Terminate specified process by PID.\n");
	} else if (0 == strcmp(cmd, "reset")) {
	  printu("Usage: reset\n");
	  printu("Restart the system.\n");
	} else if (0 == strcmp(cmd, "log")) {
	  printu("Usage: log [log types] [log levels]\n");
	  printu("Enable kernel log types and levels, or display enabled log if no parameter.\n");
	  printu("Supported log type:\n");
	  printu("  i - initialization\n");
	  printu("  m - memory management\n");
	  printu("  p - process management\n");
	  printu("  q - interrupt(IRQ) handling\n");
	  printu("  f - file system management\n");
	  printu("Supported log level:\n");
	  printu("  e - error\n");
	  printu("  w - warning\n");
	  printu("  n - information\n");
	  printu("  0 - level 0\n");
	  printu("  1 - level 1\n");
	  printu("  2 - level 2\n");
	  printu("  3 - level 3\n");
	  printu("  4 - level 4\n");
	  printu("  5 - level 5\n");
	  printu("  6 - level 6\n");
	  printu("  7 - level 7\n");
	  printu("  8 - level 8\n");
	  printu("  9 - level 9\n");
	  printu("  Read user manual for content of each log level\n");
	  printu("Example: log ipq we261\n");
	} else if (0 == strcmp(cmd, "help")) {
	  printu("Usage: help [command]\n");
	  printu("Print help information.\n");
	} else {
	  printu("unknown command\n");
	}
  }

  return;
}

