#ifndef _SCHED_H_
#define _SCHED_H_

#include <memory.h>
#include <process.h>

union thread_union {
	struct thread_info thread_info;
	unsigned long stack[THREAD_SIZE/sizeof(long)];
};

extern union thread_union init_thread_union;

/* scheduler interface */
struct sched_class {
	void (*enqueue_task) (struct task_struct *p);
	void (*dequeue_task) (struct task_struct *p);
	void (*check_preempt_curr) (struct task_struct *p);
	struct task_struct * (*pick_next_task) ();
	void (*task_tick) (struct task_struct *p);
};


#endif
