#include <type.h>
#include <process.h>
#include <sched.h>
#include <printk.h>
#include "scheduler_cfs_queue.h"

#define TASK_LEAST_CONTINUOUS_TICKS_CFS 4

static u64 min_vruntime;

static void update_min_vruntime(u64 vrt) {
  if (vrt > min_vruntime)
	min_vruntime = vrt;
}

static void scheduler_init_cfs () {
  cfs_queue_init();
  min_vruntime = 0;
}

static u64 calculate_delta_vruntime(struct sched_entity *en) {
  struct task_struct* task = 	container_of(en, struct task_struct, sched_en);
  return en->continuous_ticks + task->priority*2;
}

static u64 calculate_vruntime(struct sched_entity *en) {
  return en->vruntime + calculate_delta_vruntime(en);
}

static void enqueue_task_cfs (struct task_struct *p, enum sched_enqueue_flag flag) {
  struct sched_entity* first_en = NULL;
  struct sched_entity* task_en = &p->sched_en;
  // update vruntime here
  if (flag == sched_enqueue_flag_new)
	task_en->vruntime = min_vruntime;
  else {
	task_en->vruntime += calculate_delta_vruntime(task_en);
  }
  printk(PR_SS_PROC, PR_LVL_DBG6, "%s, pid = %d, task_en->vruntime = %d\n", __func__, p->pid, task_en->vruntime);
  cfs_queue_enqueue(task_en);
  first_en = cfs_queue_find_first();
  update_min_vruntime(first_en->vruntime);
  printk(PR_SS_PROC, PR_LVL_DBG6, "%s, min_vruntime = %d\n", __func__, min_vruntime);
}

static void dequeue_task_cfs (struct task_struct *p) {
  struct sched_entity* first = NULL;
  cfs_queue_dequeue(&p->sched_en);
  first = cfs_queue_find_first();
  update_min_vruntime(first->vruntime);
}

static bool need_to_reschedule_cfs (struct task_struct *p) {
  struct sched_entity* task_en = &p->sched_en;
  struct sched_entity* first_en = NULL;
  printk(PR_SS_PROC, PR_LVL_DBG6, "%s, current: pid = %d, continuous_ticks = %d\n", __func__, p->pid, task_en->continuous_ticks);
  printk(PR_SS_PROC, PR_LVL_DBG6, "%s, current: pid = %d, vruntime = %d\n", __func__, p->pid, task_en->vruntime);
  if (task_en->continuous_ticks < TASK_LEAST_CONTINUOUS_TICKS_CFS)
	return false;
  
  first_en = cfs_queue_find_first();
  if (NULL == first_en)
	return true;

  printk(PR_SS_PROC, PR_LVL_DBG6, "%s, first: pid = %d, vruntime = %d\n", __func__, container_of(first_en, struct task_struct, sched_en)->pid, first_en->vruntime);
  return first_en->vruntime < calculate_vruntime(task_en);
}

static void check_preempt_curr_cfs (struct task_struct *p) {
}

static struct task_struct * pick_next_task_cfs () {
  struct sched_entity* task_en = cfs_queue_dequeue_first();

  if (NULL==task_en)
	return NULL;
  else {
	task_en->continuous_ticks = 0;
	return container_of(task_en, struct task_struct, sched_en);
  }
}

static void task_tick_cfs (struct task_struct *p) {
  struct sched_entity* task_en = &p->sched_en;
  task_en->continuous_ticks ++;
  printk(PR_SS_PROC, PR_LVL_DBG6, "%s, pid = %d, continuous_ticks = %d\n", __func__, p->pid, task_en->continuous_ticks);
}

static void dump_cfs () {
  cfs_queue_dump();
}

const struct sched_class sched_class_cfs = {
  .init = scheduler_init_cfs,
  .enqueue_task		= enqueue_task_cfs,
  .dequeue_task		= dequeue_task_cfs,
  .need_to_reschedule = need_to_reschedule_cfs,
  .check_preempt_curr	= check_preempt_curr_cfs,
  .pick_next_task		= pick_next_task_cfs,
  .task_tick		= task_tick_cfs,
  .dump = dump_cfs,
};
