#include <type.h>
#include <process.h>
#include <printk.h>

struct task_struct *current_task = NULL;
struct sched_class *scheduler = NULL;

extern struct sched_class sched_class_cfs;

bool is_scheduler_ready() {
  return (scheduler != NULL);
}

void schedule_initialize() {
  struct sched_class *tmp_scheduler = &sched_class_cfs;
  tmp_scheduler->init();
  scheduler = tmp_scheduler;
}

static void switch_pgd(unsigned long pgd, int pid) {
  cpu_v6_switch_mm(__virt_to_phys(pgd), pid);
}

static void switch_to(struct task_struct *prev, struct task_struct *next) {
	__switch_to(prev,task_thread_info(prev), task_thread_info(next));
}

static void
context_switch(struct task_struct *prev,
	       struct task_struct *next)
{
	switch_pgd(next->pgd, next->pid);
	switch_to(prev, next);
}

void enqueue_task(struct task_struct *task, enum sched_enqueue_flag flag) {
  scheduler->enqueue_task(task, flag);
}

void update_task_on_tick() {
  if (NULL != current_task)
	scheduler->task_tick(current_task);
}

bool check_should_schedule() {
  if (NULL != current_task)  
	return scheduler->need_to_reschedule(current_task);
  else 
	return true;
}

void schedule() {
  struct task_struct *next_task = NULL;
  if (current_task)
	scheduler->enqueue_task(current_task, sched_enqueue_flag_timeout);
  next_task = scheduler->pick_next_task();

  if (current_task)
	printk(PR_SS_PROC, PR_LVL_DBG3, "%s, current_task->pid = %d\n", __func__, current_task->pid);
  if (next_task)
	printk(PR_SS_PROC, PR_LVL_DBG3, "%s, next_task->pid = %d\n", __func__, next_task->pid);

  if (current_task == next_task) {
	printk(PR_SS_PROC, PR_LVL_DBG3, "%s, current_task == next_task\n", __func__);
	return;
  } else if (NULL == next_task) {
	printk(PR_SS_PROC, PR_LVL_DBG3, "%s, NULL == next_task\n", __func__);
	return;
  } else if (NULL == current_task) {
	printk(PR_SS_PROC, PR_LVL_DBG3, "%s, NULL == current_task\n", __func__);
	current_task = next_task;
  } else {
	printk(PR_SS_PROC, PR_LVL_DBG3, "%s, current_task != next_task\n", __func__);
	current_task = next_task;
  }
  
  if (0)
	context_switch(current_task, next_task);
}
