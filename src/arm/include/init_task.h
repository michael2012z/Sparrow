#ifndef _INIT_TASK_H_
#define _INIT_TASK_H_

/* Attach to the init_task data structure for proper alignment */
#define __init_task_data __attribute__((__section__(".data..init_task")))


#endif
