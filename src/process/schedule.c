#include <type.h>
#include <process.h>

struct task_struct idle_task;
struct list_head *task_list = &idle_task.list;
struct task_struct *current_task = &idle_task;

void schedule_initialize() {
  INIT_LIST_HEAD(task_list);
  current_task = &idle_task;
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
	struct mm_struct *mm, *oldmm;

	switch_pgd(next->pgd, next->pid);
	switch_to(prev, next);
}

void schedule() {
  /*
  if (list_empty(task_list)) {
	context_switch(NULL, &idle_task);
  } else {
	current_task->past ++;
	if (current_task->past >= current_task->prio) {  // time is out
	  struct task_struct *old_task = current_task;
	  struct list_head *next = current_task->list.next; // get the next task
	  if (next == task_list) // if this is the idle task, skip it
		next = next->next;
	  current_task = list_entry(next, struct task_struct, list);
	  current_task->past = 0;
	  context_switch(old_task, current_task);
	}
  }
  */
}
