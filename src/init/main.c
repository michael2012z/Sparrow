#include <linkage.h>
#include <printk.h>
#include <type.h>
#include <mm.h>
#include <exception.h>
#include <timer.h>
#include <process.h>
#include <vfs.h>
#include <irq.h>
#include <head.h>

#define UFCON0	((volatile unsigned int *)(0x7f005020))

void __init helloworld(void){
	const char *p="helloworld\n";
	while(*p){
		*(volatile unsigned int *)0xef005020=*p++;
	};
}

extern unsigned int _kernel_end;
void __init store_registers() {

  asm("mov %0, r7\n"
	  : "=r"(_kernel_end)
	  : );
}

extern struct file_system_type listfs_file_system_type;

static int __init kernel_init(void *unused) {
	/*
	if (!run_init_process("/bin/init"))
	  return 0;
	else {
	  printk(PR_SS_INI, PR_LVL_ERR, "Init process can't be created, kernel is killed.\n");
	  while(1);
	}
	*/
	return 0;
}

static int __init kernel_demo(void *unused) {
  process_test();
  return 0;
}

static void __init rest_init(void) {
  create_kernel_thread(kernel_init);
  create_kernel_thread(kernel_demo);
  cpu_idle();
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
  printk(PR_SS_INI, PR_LVL_INF, "Enter registers stored.\n");

  exception_disable();
  printk(PR_SS_INI, PR_LVL_INF, "Enter irq diabled.\n");

  health_check();
  printk(PR_SS_INI, PR_LVL_INF, "Passed health check.\n");

  mm_init();

  printk(PR_SS_INI, PR_LVL_INF, "MM initialization finish.\n");

  exception_init();
  init_IRQ();
  init_timer();

  exception_enable();

  printk(PR_SS_INI, PR_LVL_INF, "IRQ initialization finish.\n");

  register_file_system(&listfs_file_system_type);

  map_fs_to_ram();

  mount_file_system("ListFS");

  printk(PR_SS_INI, PR_LVL_INF, "FS initialization finish.\n");

  initialize_process();

  printk(PR_SS_INI, PR_LVL_INF, "Process initialization finish.\n");

  rest_init();

  printk(PR_SS_INI, PR_LVL_INF, "Kernel is running ...\n");
  while(1);
}
