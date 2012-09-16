#ifndef _PROCESS_H_
#define _PROCESS_H_

#include <mm.h>
#include <mmap.h>
#include <time.h>
#include <type.h>


struct task_struct {
  volatile long state;	/* -1 unrunnable, 0 runnable, >0 stopped */
  void *stack;
  unsigned int flags;	/* per process flags, defined below */

  unsigned long pgd;

  bool kernel_thread;

  int prio, past;

  struct list_head list;

  struct mm_struct mm;

  int pid;

  struct timespec start_time; 		/* monotonic time */
};

struct cpu_context_save {
	__u32	r4;
	__u32	r5;
	__u32	r6;
	__u32	r7;
	__u32	r8;
	__u32	r9;
	__u32	sl;
	__u32	fp;
	__u32	sp;
	__u32	pc;
	__u32	extra[2];		/* Xscale 'acc' register, etc */
};

struct thread_info {
	struct task_struct	*task;		/* main task structure */
	__u32			cpu_domain;	/* cpu domain */
	unsigned long		tp_value;
	struct cpu_context_save	cpu_context;	/* cpu context */
};


#define task_thread_info(task)	((struct thread_info *)(task)->stack)


void process_test();
void initialize_process();
int create_process(struct file *filep);
void destroy_process(struct task_struct *task);

#endif /* _PROCESS_H_ */
