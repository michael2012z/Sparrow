#include <type.h>
#include <linkage.h>
#include <printk.h>
#include <process.h>

extern struct task_struct *current_task;

long sys_reset(void) {
  printk(PR_SS_IRQ, PR_LVL_DBG6, "%s\n", __func__);
  while(1);
  return 0;
}

long sys_sleep(int msec) {
  printk(PR_SS_IRQ, PR_LVL_DBG6, "%s: msec = %x\n", __func__, msec);
  while(1);
  return 0;
}

long sys_exit(int code) {
  printk(PR_SS_IRQ, PR_LVL_DBG6, "%s: code = %x\n", __func__, code);
  /* set current state to dead
   * find waiting process if exist
   * set waiting process to ready, dequeue and enqueue it
   */
  current_task->sched_en.state = PROCESS_STATE_DEAD;

  return 0;
}
