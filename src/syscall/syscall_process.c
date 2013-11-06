#include <type.h>
#include <linkage.h>
#include <printk.h>
#include <process.h>
#include <string.h>

extern struct task_struct *current_task;
extern unsigned long jiffies;

long sys_reset(void) {
  printk(PR_SS_IRQ, PR_LVL_DBG6, "%s\n", __func__);
  while(1);
  return 0;
}

long sys_sleep(int msec) {
  printk(PR_SS_IRQ, PR_LVL_DBG6, "%s: msec = %x\n", __func__, msec);
  current_task->sched_en.state = PROCESS_STATE_WAITING;
  current_task->sched_en.waiting_type = PROCESS_WAITING_TYPE_TIME;
  current_task->sched_en.wake_up_jiffy = jiffies + msec; /* simplity: I use jiffies as msec */
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

long sys_params(char *param1, char *param2, char *param3, char *param4) {
  printk(PR_SS_IRQ, PR_LVL_DBG6, "%s: param1 = %x, param2 = %x, param3 = %x, param4 = %x\n", __func__, param1, param2, param3, param4);
  if ((current_task->parameters[0]) && (param1))
	memcpy(param1, current_task->parameters[0], strlen(current_task->parameters[0]) + 1);
  if ((current_task->parameters[1]) && (param2))
	memcpy(param2, current_task->parameters[1], strlen(current_task->parameters[1]) + 1);
  if ((current_task->parameters[2]) && (param3))
	memcpy(param3, current_task->parameters[2], strlen(current_task->parameters[2]) + 1);
  if ((current_task->parameters[3]) && (param4))
	memcpy(param4, current_task->parameters[3], strlen(current_task->parameters[3]) + 1);

  return 0;
}

unsigned long noise = 0; /* random seed */
unsigned long sys_random() {

  if (0 == noise)
	noise = jiffies;

  noise *= jiffies;

  noise %= 32768;
  
  return noise;
}

