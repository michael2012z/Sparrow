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

extern unsigned int _kernel_end;
void __init store_registers() {

  asm("mov %0, r8\n"
	  : "=r"(_kernel_end)
	  : );
}

extern struct file_system_type listfs_file_system_type;

static void __init cpu_init() {
  arm_cpu_init();
}

static void __init rest_init(void) {
  create_kernel_thread(process_cleaner);
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

  store_registers();

  printu("\n  Sparrow OS, by Michael Z\n");

  /* print logo */
  printu("\n");
  for (i = 0; i < sizeof(logo_l)/sizeof(char *); i++)
    printu("%s", logo_l[i]);
  printu("\n");
  
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
