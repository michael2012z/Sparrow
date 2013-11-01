#include <type.h>
#include <process.h>
#include <printk.h>

struct task_struct *current_task = NULL;
struct sched_class *scheduler = NULL;

extern struct task_struct *init_kernel_task;
extern struct sched_class sched_class_cfs;
extern bool need_reschedule;

bool is_scheduler_ready() {
  return (scheduler != NULL);
}

void schedule_initialize() {
  struct sched_class *tmp_scheduler = &sched_class_cfs;
  tmp_scheduler->init();
  scheduler = tmp_scheduler;
}

static void switch_pgd(unsigned long pgd, int pid) {
  /* The second parameter is context id, we don't support it. */
  //  printk(PR_SS_MM, PR_LVL_DBG9, "%s: pid = %d: dump page table level 1:\n", __func__, pid);  
  //print_memory_byte(pgd, pgd + 4*PAGE_SIZE);
  //while(1);
  cpu_v6_switch_mm(__virt_to_phys(pgd), 0);
}

static void switch_to(struct task_struct *prev, struct task_struct *next) {
  struct thread_info *prev_thread = task_thread_info(prev);
  struct thread_info *next_thread = task_thread_info(next);  
  printk(PR_SS_PROC, PR_LVL_DBG5, "%s, prev_thread = %x, next_thread = %x\n", __func__, prev_thread, next_thread);  
  __switch_to(prev,task_thread_info(prev), task_thread_info(next));
}

static void
context_switch(struct task_struct *prev,
	       struct task_struct *next)
{
  register unsigned long sp asm ("sp");
  printk(PR_SS_PROC, PR_LVL_DBG5, "%s, sp = %x\n", __func__, sp);
  printk(PR_SS_PROC, PR_LVL_DBG5, "%s, prev = %x, next = %x\n", __func__, prev, next);  
  printk(PR_SS_PROC, PR_LVL_DBG5, "%s, prev.pgd = %x, next.pgd = %x\n", __func__, prev->mm.pgd, next->mm.pgd);  
  switch_pgd(next->mm.pgd, next->pid);
  printk(PR_SS_PROC, PR_LVL_DBG5, "%s, sp = %x\n", __func__, sp);
  switch_to(prev, next);
}

void enqueue_task(struct task_struct *task, enum sched_enqueue_flag flag) {
  scheduler->enqueue_task(task, flag);
}

void dequeue_task(struct task_struct *task) {
  scheduler->dequeue_task(task);
}

void update_task_on_tick(unsigned long jiffy) {
  if (NULL != current_task)
	scheduler->task_tick(current_task);
  scheduler->wake_up_sleeping(jiffy);
}

void check_and_schedule() {
  printk(PR_SS_PROC, PR_LVL_DBG5, "%s\n", __func__);
  if (check_should_schedule())
	schedule();
  printk(PR_SS_PROC, PR_LVL_DBG5, "%s: return\n", __func__);
}

bool check_should_schedule() {
  printk(PR_SS_PROC, PR_LVL_DBG5, "%s: current_task = %x\n", __func__, current_task);
  if (NULL != current_task)  
	return scheduler->need_to_reschedule(current_task);
  else 
	return true;
}

struct task_struct * find_task_by_pid(int pid) {
  return scheduler->find_task_by_pid(pid);
}


void schedule() {
  struct task_struct *next_task = NULL;
  struct task_struct *prev_task = current_task;

  register unsigned long sp asm ("sp");
  printk(PR_SS_PROC, PR_LVL_DBG5, "%s, sp = %x\n", __func__, sp);

  need_reschedule = false;

  if (NULL == current_task) while(1);

  if (PROCESS_STATE_DEAD == current_task->sched_en.state) {
	int waiting_pid = current_task->sched_en.blocked_pid;
	if (-1 != waiting_pid) {
	  struct task_struct *waiting_task = find_task_by_pid(waiting_pid);
	  if (waiting_task) {
		dequeue_task(waiting_task);
		waiting_task->sched_en.state = PROCESS_STATE_READY;
		waiting_task->sched_en.blocking_pid = -1;
		enqueue_task(waiting_task, sched_enqueue_flag_timeout);
	  }
	}
	destroy_user_thread(current_task);
	current_task = NULL;
  } else
	scheduler->enqueue_task(current_task, sched_enqueue_flag_timeout);

  scheduler->dump();

  next_task = scheduler->pick_next_task();

  if (current_task)
	printk(PR_SS_PROC, PR_LVL_DBG5, "%s, current_task->pid = %d\n", __func__, current_task->pid);
  if (next_task)
	printk(PR_SS_PROC, PR_LVL_DBG5, "%s, next_task->pid = %d\n", __func__, next_task->pid);

  if (current_task == next_task) {
	printk(PR_SS_PROC, PR_LVL_DBG5, "%s, current_task == next_task\n", __func__);
	return;
  } else if (NULL == next_task) {
	printk(PR_SS_PROC, PR_LVL_DBG5, "%s, NULL == next_task\n", __func__);
	return;
  } else if (NULL == current_task) {
	printk(PR_SS_PROC, PR_LVL_DBG5, "%s, NULL == current_task\n", __func__);
	current_task = next_task;
  } else {
	printk(PR_SS_PROC, PR_LVL_DBG5, "%s, current_task != next_task\n", __func__);
	current_task = next_task;
  }

  printk(PR_SS_PROC, PR_LVL_DBG5, "%s, context_switch %d <--> %d start\n", __func__, prev_task->pid, next_task->pid);  
  if (1) context_switch(prev_task, next_task);
  printk(PR_SS_PROC, PR_LVL_DBG5, "%s, context_switch %d <--> %d finish\n", __func__, prev_task->pid, next_task->pid);
}
