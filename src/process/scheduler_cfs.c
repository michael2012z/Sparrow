#include <type.h>
#include <process.h>
#include <sched.h>
#include "scheduler_cfs_queue.h"

static void scheduler_init_cfs () {
  cfs_queue_init();
}

static void enqueue_task_cfs (struct task_struct *p) {
  cfs_queue_enqueue(&p->sched_en);
}

static void dequeue_task_cfs (struct task_struct *p) {
  cfs_queue_dequeue(&p->sched_en);
}

static void check_preempt_curr_cfs (struct task_struct *p) {

}

static struct task_struct * pick_next_task_cfs () {
  struct sched_entity* en = cfs_queue_dequeue_first();
  if (NULL==en)
	return NULL;
  else
	return container_of(en, struct task_struct, sched_en);
}

static void task_tick_cfs (struct task_struct *p) {

}

const struct sched_class sched_class_cfs = {
  .init = scheduler_init_cfs,
  .enqueue_task		= enqueue_task_cfs,
  .dequeue_task		= dequeue_task_cfs,
  .check_preempt_curr	= check_preempt_curr_cfs,
  .pick_next_task		= pick_next_task_cfs,
  .task_tick		= task_tick_cfs,
};
