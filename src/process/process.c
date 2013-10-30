#include <type.h>
#include <list.h>
#include "pid.h"
#include <process.h>
#include <string.h>
#include <printk.h>
#include "elf.h"

extern unsigned long kernel_pgd;
extern struct list_head *task_list;
extern struct task_struct *current_task;
extern struct sched_class *scheduler;

void process_test() {
  struct file demo_1, demo_2;
  int pid_1, pid_2;

  demo_1.buf = (void *)0xc4040000;
  demo_1.size = 33807;
  demo_2.buf = (void *)0xc4080000;
  demo_2.size = 33807;

  pid_1 = create_process(&demo_1);
  printk(PR_SS_PROC, PR_LVL_INF, "process %d created\n", pid_1);

  pid_2 = create_process(&demo_2);
  printk(PR_SS_PROC, PR_LVL_INF, "process %d created\n", pid_2);
}

struct task_struct *init_kernel_task = NULL;

static struct task_struct *create_launch_kernel_task() {
  int pid;
  struct task_struct *task = NULL;
  pid = allocate_pid();
  if (pid < 0)
	return NULL;
  
  task = (struct task_struct *)kmalloc(sizeof(struct task_struct));

  if (NULL == task)
	return NULL;

  task->sched_en.state = PROCESS_STATE_RUNNING;
  task->sched_en.blocking_pid = -1;
  task->stack = (void *)&init_thread_union;

  task->pid = pid;

  printk(PR_SS_PROC, PR_LVL_DBG3, "%s process %d stack = %x\n", __func__, pid, task->stack);

  task->mm.pgd = kernel_pgd;

  enqueue_task(task, sched_enqueue_flag_new);

  return task;
}

void initialize_process() {
  initialize_pid();
  schedule_initialize();
  create_launch_kernel_task();
  init_kernel_task = scheduler->pick_next_task();
  current_task = init_kernel_task;
}


void arm_start_kernel_thread(void) __asm__("arm_start_kernel_thread");

int create_kernel_thread(int (*fn)(void *)) {
  int pid;
  struct task_struct *task = NULL;
  struct pt_regs *regs = NULL;
  struct thread_info *thread = NULL;

  pid = allocate_pid();
  if (pid < 0)
	return pid;
  
  /* make task_struct */
  task = (struct task_struct *)kmalloc(sizeof(struct task_struct));

  if (NULL == task)
	return -1;

  task->sched_en.state = PROCESS_STATE_READY;
  task->sched_en.blocking_pid = -1;
  task->stack = (void *)kmalloc(PAGE_SIZE*2);

  task->pid = pid;

  printk(PR_SS_PROC, PR_LVL_DBG3, "%s process %d stack = %x\n", __func__, pid, task->stack);

  INIT_LIST_HEAD(&(task->mm.mmap.list));
  task->mm.pgd = (unsigned long)kmalloc(PAGE_SIZE * 4);
  memcpy((void *)task->mm.pgd, (void *)kernel_pgd, PAGE_SIZE * 4);
  printk(PR_SS_MM, PR_LVL_DBG7, "%s: pid = %x, mm.pgd = %x\n", __func__, pid, task->mm.pgd);
  printk(PR_SS_MM, PR_LVL_DBG7, "%s: *mm.pgd = %x\n", __func__, *((unsigned long *)task->mm.pgd));

  INIT_LIST_HEAD(&(task->sched_en.queue_entry));

  /* populate initial content of stack */
  regs = task_pt_regs(task);
  arm_create_kernel_thread(fn, NULL, regs);
  regs->ARM_r0 = 0;
  regs->ARM_sp = 0; /* this is user-space sp, don't need to set it */
	
  thread = task_thread_info(task);
  thread->task = task;
  thread->cpu_domain = arm_calc_kernel_domain();
  thread->tp_value = 0;
  thread->cpu_context.sp = (unsigned long)regs;
  thread->cpu_context.pc = (unsigned long)arm_start_kernel_thread;

  enqueue_task(task, sched_enqueue_flag_new);

  return pid;
}

int create_user_thread(int (*fn)(char *), char *elf_file_name, char **parameters) {
  int pid;
  struct task_struct *task = NULL;
  struct pt_regs *regs = NULL;
  struct thread_info *thread = NULL;
  int i = 0;

  pid = allocate_pid();
  if (pid < 0)
	return pid;
  
  /* make task_struct */
  task = (struct task_struct *)kmalloc(sizeof(struct task_struct));

  if (NULL == task)
	return -1;

  task->sched_en.state = PROCESS_STATE_READY;
  task->sched_en.blocking_pid = -1;
  task->stack = (void *)kmalloc(PAGE_SIZE*2);

  task->pid = pid;

  printk(PR_SS_PROC, PR_LVL_DBG3, "%s process %d stack = %x\n", __func__, pid, task->stack);

  INIT_LIST_HEAD(&(task->mm.mmap.list));
  task->mm.pgd = (unsigned long)kmalloc(PAGE_SIZE * 4);
  memcpy((void *)task->mm.pgd, (void *)kernel_pgd, PAGE_SIZE * 4);
  printk(PR_SS_MM, PR_LVL_DBG7, "%s: pid = %x, mm.pgd = %x\n", __func__, pid, task->mm.pgd);
  printk(PR_SS_MM, PR_LVL_DBG7, "%s: *mm.pgd = %x\n", __func__, *((unsigned long *)task->mm.pgd));

  task->elf_file_name = (char *)kmalloc(strlen(elf_file_name) + 1);
  memcpy(task->elf_file_name, elf_file_name, strlen(elf_file_name) + 1);
  for(i = 0; i < 4; i++)
	if (parameters[i]) {
	  task->parameters[i] = (char *)kmalloc(strlen(parameters[i]) + 1);
	  memcpy(task->parameters[i], parameters[i], strlen(parameters[i]) + 1);	  
	}
	  

  INIT_LIST_HEAD(&(task->sched_en.queue_entry));

  /* populate initial content of stack */
  regs = task_pt_regs(task);
  arm_create_user_thread(fn, task->elf_file_name, regs);
  regs->ARM_r0 = 0;
  regs->ARM_sp = 0; /* this is user-space sp, don't need to set it */
	
  thread = task_thread_info(task);
  thread->task = task;
  thread->cpu_domain = arm_calc_kernel_domain();
  thread->tp_value = 0;
  thread->cpu_context.sp = (unsigned long)regs;
  thread->cpu_context.pc = (unsigned long)arm_start_kernel_thread;

  enqueue_task(task, sched_enqueue_flag_new);

  return pid;
}

int create_process(struct file *filep) {
  int pid;
  struct task_struct *task = NULL;
  pid = allocate_pid();
  if (pid < 0)
	return pid;
  
  task = (struct task_struct *)kmalloc(sizeof(struct task_struct));

  if (NULL == task)
	return -1;

  task->sched_en.state = PROCESS_STATE_READY;
  task->sched_en.blocking_pid = -1;
  task->stack = (void *)kmalloc(PAGE_SIZE*2);

  task->pid = pid;

  printk(PR_SS_PROC, PR_LVL_DBG3, "%s process %d stack = %x\n", __func__, pid, task->stack);

  INIT_LIST_HEAD(&(task->mm.mmap.list));
  task->mm.pgd = (unsigned long)kmalloc(PAGE_SIZE * 4);
  memcpy((void *)task->mm.pgd, (void *)kernel_pgd, PAGE_SIZE * 4);

  INIT_LIST_HEAD(&(task->sched_en.queue_entry));

  load_elf_binary(filep, &task->regs, &task->mm);

  enqueue_task(task, sched_enqueue_flag_new);

  return pid;
}

int execute_binary(struct task_struct *task, struct file *filep) {
  printk(PR_SS_PROC, PR_LVL_DBG3, "%s: current process: %d\n", __func__, task->pid);
  load_elf_binary(filep, &(task->regs), &(task->mm));
  return 0; // meaningless return value
}

void destroy_process(struct task_struct *task) {
  free_pid(task->pid);
  kfree(task);
}


void run_kernel_process(char *init_filename)
{
  arm_kernel_execve(init_filename, NULL, NULL);
}

