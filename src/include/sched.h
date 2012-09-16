#ifndef _SCHED_H_
#define _SCHED_H_

#include <memory.h>
#include <process.h>

union thread_union {
	struct thread_info thread_info;
	unsigned long stack[THREAD_SIZE/sizeof(long)];
};

extern union thread_union init_thread_union;

#endif
