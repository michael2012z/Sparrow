#include <linkage.h>
#include <printk.h>
#include <type.h>
#include <mm.h>
#include <exception.h>
#include <timer.h>
#include <process.h>
#include <vfs.h>

#define UFCON0	((volatile unsigned int *)(0x7f005020))

void __init helloworld(void){
	const char *p="helloworld\n";
	while(*p){
		*(volatile unsigned int *)0xef005020=*p++;
	};
}

extern unsigned int _kernel_end;
void store_registers() {

  asm("mov %0, r7\n"
	  : "=r"(_kernel_end)
	  : );
}

extern struct file_system_type listfs_file_system_type;

void __init start_kernel(void) {
  store_registers();
  exception_disable();
  mm_init();

  printk(PR_SS_INI, PR_LVL_INF, "MM initialization finish.\n");
  exception_init();

  init_IRQ();

  init_timer();

  exception_enable();
  printk(PR_SS_INI, PR_LVL_INF, "IRQ initialization finish.\n");

  register_file_system(&listfs_file_system_type);
  mount_file_system("ListFS");

  printk(PR_SS_INI, PR_LVL_INF, "FS initialization finish.\n");

  initialize_process();

  process_test();

  printk(PR_SS_INI, PR_LVL_INF, "Process initialization finish.\n");

  printk(PR_SS_INI, PR_LVL_INF, "Kernel is running ...\n");
  while(1);
}
