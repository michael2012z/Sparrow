#include <linkage.h>
#include <printk.h>
#include <type.h>
#include <mm.h>
#include <exception.h>
#include <timer.h>
#include <uart.h>
#include <process.h>
#include <vfs.h>
#include <irq.h>
#include <head.h>
#include <setup.h>
#include "shell.h"

extern struct task_struct *current_task;

#define UFCON0	((volatile unsigned int *)(0x7f005020))

void __init helloworld(void){
	const char *p="helloworld\n";
	while(*p){
		*(volatile unsigned int *)0xef005020=*p++;
	};
}

extern unsigned int _kernel_end;
void __init store_registers() {

  asm("mov %0, r8\n"
	  : "=r"(_kernel_end)
	  : );
  printk(PR_SS_INI, PR_LVL_INF, "kernel_end = %x\n", _kernel_end);
}

extern struct file_system_type listfs_file_system_type;

static void __init cpu_init() {
  arm_cpu_init();
}

static int __init kernel_init(void *unused) {
	/*
	if (!run_init_process("/bin/init"))
	  return 0;
	else {
	  printk(PR_SS_INI, PR_LVL_ERR, "Init process can't be created, kernel is killed.\n");
	  while(1);
	}
	*/
  while(1) {
	int i = 0;
	for (i = 0; i < 1024; i++){}
	//	return 0;
	continue;
	printk(PR_SS_INI, PR_LVL_INF, "%s\n", __func__);
  }
  return 0;
}

static int __init kernel_demo(void *unused) {
  printk(PR_SS_INI, PR_LVL_INF, "%s\n", __func__);
  run_kernel_process("/demo_6");
  //  process_test();
  return 0;
}

static void __init rest_init(void) {
  create_kernel_thread(kernel_init);
  create_kernel_thread(process_cleaner);
  if (0)  create_kernel_thread(kernel_demo);
  create_kernel_thread(kernel_shell);
}

static void __init health_check(void) {
  arm_health_check();

  CRASHIF(TI_TASK != offsetof(struct thread_info, task));
  CRASHIF(TI_CPU_DOMAIN != offsetof(struct thread_info, cpu_domain));
  CRASHIF(TI_TP_VALUE != offsetof(struct thread_info, tp_value));
  CRASHIF(TI_CPU_CONTEXT != offsetof(struct thread_info, cpu_context));

}

void __init start_kernel(void) {

  printk(PR_SS_INI, PR_LVL_INF, "Enter start_kernel().\n");

  store_registers();

  cpu_init();

  printk(PR_SS_INI, PR_LVL_INF, "Enter registers stored.\n");

  exception_disable();
  printk(PR_SS_INI, PR_LVL_INF, "Enter irq diabled.\n");

  health_check();
  printk(PR_SS_INI, PR_LVL_INF, "Passed health check.\n");

  mm_init();

  printk(PR_SS_INI, PR_LVL_INF, "MM initialization finish.\n");

  exception_init();

  exception_disable();

  register_file_system(&listfs_file_system_type);

  map_fs_to_ram();

  mount_file_system("ListFS");

  printk(PR_SS_INI, PR_LVL_INF, "FS initialization finish.\n");

  initialize_process();

  printk(PR_SS_INI, PR_LVL_INF, "Process initialization finish.\n");

  rest_init();
  //  schedule();

  printk(PR_SS_INI, PR_LVL_INF, "Will enable IRQ.\n");

  init_IRQ();
  init_timer();
  init_uart();

  ring_buffer_init();

  exception_enable();
  printk(PR_SS_INI, PR_LVL_INF, "IRQ initialization finish.\n");

  printk(PR_SS_INI, PR_LVL_INF, "Kernel is running ...\n");

  /*
  while(1) {
	//	int i = 0;
	//	for (i = 0; i < 65535 ; i++){}
	printk(PR_SS_INI, PR_LVL_INF, "%s : Kernel is running ...\n", __func__);
	//schedule();
  }
  */

  cpu_idle();
}
