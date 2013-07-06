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

  printk(PR_SS_PROC, PR_LVL_DBG3, "%s\n", __func__);
  list_for_each(pos, head) {
	printk(PR_SS_PROC, PR_LVL_DBG3, "%s 1\n", __func__);
	current = list_entry(pos, struct sched_entity, queue_entry);
	if (en->vruntime < current->vruntime) {
	  printk(PR_SS_PROC, PR_LVL_DBG3, "%s 2\n", __func__);
	  list_add_tail(&en->queue_entry, pos);
	  return;
	}
  }
  list_add_tail(&en->queue_entry, queue);
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
