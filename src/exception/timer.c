#include <type.h>
#include <timer.h>
#include <printk.h>
#include <process.h>

void init_timer() {
  arm_init_timer();
}

bool need_reschedule = false;
unsigned long jiffies = 0;

extern struct task_struct *current_task;

/* Get called on periodic tick.
 * Update current task ticks. Re-schedule if the min time slices has been used up.
 */
void on_timer() {

  jiffies++;

  if (is_scheduler_ready()){
	/* Update current on-going task ticks. */
	update_task_on_tick(jiffies);
	/* Check left time slices,
	 * reschedule if time slices is up.
	 */
	if (check_should_schedule()) {
	  printk(PR_SS_IRQ, PR_LVL_DBG1, "%s, current task (%d) times up, will reschedule\n", __func__, current_task->pid);
	  need_reschedule = true;
	  //	  schedule();
	} else {
	  printk(PR_SS_IRQ, PR_LVL_DBG1, "%s, current task (%d) should continue, not kicked out\n", __func__, current_task->pid);
	  need_reschedule = false;
	}
  }
}
