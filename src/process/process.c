#include <type.h>
#include <list.h>
#include "pid.h"
#include <process.h>
#include <string.h>
#include <printk.h>

extern unsigned long mm_pgd;
extern struct list_head *task_list;

void process_test() {
  struct file demo_1, demo_2;
  int pid_1, pid_2;
  int i;
  char *magic;

  demo_1.buf = (void *)0xc4040000;
  demo_1.size = 33807;
  demo_2.buf = (void *)0xc4080000;
  demo_2.size = 33807;

  pid_1 = create_process(&demo_1);
  printk(PR_SS_PROC, PR_LVL_INF, "process %d created\n", pid_1);

  pid_2 = create_process(&demo_2);
  printk(PR_SS_PROC, PR_LVL_INF, "process %d created\n", pid_2);
}


void initialize_process() {
  initialize_pid();
  schedule_initialize();
}

static int create_process_demo(struct file *filep) {
  int pid;
  struct task_struct *task = NULL;
  // test 
  static struct task_struct task1, task2;  

  pid = allocate_pid();

  if (pid < 0)
	return pid;

  if (filep->buf == (void *)0xc4040000) // demo 1
	task = &task1;

  if (filep->buf == (void *)0xc4080000) // demo 1
	task = &task2;

  // for test
  //  task = (struct task_struct *)kmalloc(sizeof(struct task_struct));
  // test ~

  if (NULL == task)
	return -1;

  task->pid = pid;

  INIT_LIST_HEAD(&(task->mm.mmap.list));
  //  task->mm.pgd = (unsigned long)kmalloc(PAGE_SIZE * 4);
  //  memcpy((void *)task->mm.pgd, (void *)mm_pgd, PAGE_SIZE * 4);

  //  list_add_tail(&task->list, task_list);

  load_elf_binary(filep, NULL, &task->mm);

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

  task->pid = pid;

  INIT_LIST_HEAD(&(task->mm.mmap.list));
  task->mm.pgd = (unsigned long)kmalloc(PAGE_SIZE * 4);
  memcpy((void *)task->mm.pgd, (void *)mm_pgd, PAGE_SIZE * 4);

  load_elf_binary(filep, NULL, &task->mm);

  enqueue_task(task, sched_enqueue_flag_new);

  return pid;
}

void destroy_process(struct task_struct *task) {
  free_pid(task->pid);
  kfree(task);
}
