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
#include "logo.h"

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
  //  printk(PR_SS_INI, PR_LVL_INF, "kernel_end = %x\n", _kernel_end);
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
  int i;
  printu("\n  Sparrow OS, by Michael Z\n");

  /* print logo */
  printu("\n");
  for (i = 0; i < sizeof(logo_l)/sizeof(char *); i++)
    printu("%s", logo_l[i]);
  printu("\n");
  
  store_registers();

  cpu_init();

  printu("  CPU initialized.\n");

  exception_disable();
  printu("  IRQ diabled.\n");

  health_check();
  printu("  Health check passed.\n");

  mm_init();

  printu("  Memory initialized.\n");

  exception_init();

  exception_disable();

  register_file_system(&listfs_file_system_type);

  map_fs_to_ram();

  mount_file_system("ListFS");

  printu("  File system initialized.\n");

  initialize_process();

  printu("  Process initialized.\n");

  rest_init();
  printu("  Starting daemons.\n");

  init_IRQ();

  init_timer();
  printu("  Timer initialized.\n");

  init_uart();
  printu("  UART initialized.\n");

  ring_buffer_init();
  printu("  Ring-buffer initialized.\n");

  exception_enable();
  printu("  IRQ enabled.\n");

  printu("  Kernel is up.\n");

  cpu_idle();
}
