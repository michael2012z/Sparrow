#include <type.h>
#include <process.h>
#include <sched.h>
#include <printk.h>
#include "scheduler_cfs_queue.h"

static struct list_head* queue=NULL;

void cfs_queue_init () {
  // initialize the queue
  queue = kmalloc(sizeof(struct list_head));
  INIT_LIST_HEAD(queue);
}

void cfs_queue_enqueue (struct sched_entity *en) {
  struct list_head *pos = NULL, *head = queue;
  struct sched_entity *current;

  /* check task state:
   * if task is ready: add to first;
   * if task is waiting/sleeping: add to tail;
   * else: insert by vruntime.
   **/
  if (PROCESS_STATE_READY == en->state) {
	list_add(&en->queue_entry, queue);
  } else if (PROCESS_STATE_RUNNING == en->state) {
	printk(PR_SS_PROC, PR_LVL_DBG7, "%s\n", __func__);
	list_for_each(pos, head) {
	  printk(PR_SS_PROC, PR_LVL_DBG7, "%s 1\n", __func__);
	  current = list_entry(pos, struct sched_entity, queue_entry);
	  if (PROCESS_STATE_READY == current->state)
		continue;
	  else if (PROCESS_STATE_WAITING == current->state)
		break;
	  else if (en->vruntime < current->vruntime) {
		printk(PR_SS_PROC, PR_LVL_DBG7, "%s 2\n", __func__);
		break;
	  }
	}
	list_add_tail(&en->queue_entry, pos);
  } else if (PROCESS_STATE_WAITING == en->state)
    list_add_tail(&en->queue_entry, queue);
  else
	while(1)
	  printk(PR_SS_PROC, PR_LVL_ERROR, "%s: invalid process state: %d, kernel panic\n", __func__, en->state);	
  
  return;
}

void cfs_queue_dequeue (struct sched_entity* en) {
  list_del(&en->queue_entry);
}

struct sched_entity* cfs_queue_find_first () {
  if (list_empty(queue))
	return NULL;
  else {
	struct list_head *first = NULL;
	first = queue->next;
	return container_of(first, struct sched_entity, queue_entry);
  }
}

struct sched_entity* cfs_queue_dequeue_first () {
  struct sched_entity* first_en = cfs_queue_find_first();
  if (NULL != first_en)
	list_del(&first_en->queue_entry);
  return first_en;
}

int cfs_queue_size () {
  struct list_head *pos = NULL, *head = queue;
  int count = 0;
  list_for_each(pos, head) {
	if (pos == head)
	  break;
	else 
	  count++;
  }
  return count;
}

struct sched_entity* cfs_queue_get_nth_entity(int index) {
  struct list_head *pos = NULL, *head = queue;
  struct sched_entity *current;

  list_for_each(pos, head) {
	current = list_entry(pos, struct sched_entity, queue_entry);
	if (index == 0)
	  return current;
	else 
	  index--;
  }
  return NULL;
}

void cfs_queue_dump() {
  struct list_head *pos = NULL, *head = queue;
  struct sched_entity *current;
  struct task_struct* task = NULL;

  printk(PR_SS_PROC, PR_LVL_DBG7, "%s\n", __func__);
  printk(PR_SS_PROC, PR_LVL_DBG7, "%s: queue size: %d\n", __func__, cfs_queue_size());
  list_for_each(pos, head) {
	current = list_entry(pos, struct sched_entity, queue_entry);
	task = 	container_of(current, struct task_struct, sched_en);
	printk(PR_SS_PROC, PR_LVL_DBG7, "%s: pid: %d, state: %d, vruntime: %d \n", __func__, task->pid, current->state, current->vruntime);
  }
  return;
}
