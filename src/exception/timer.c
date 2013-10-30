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

extern int g_debug_flag;
/* Get called on periodic tick.
 * Update current task ticks. Re-schedule if the min time slices has been used up.
 */
void on_timer() {

  jiffies++;

  /*
  if ((1 == g_debug_flag) && (2 == current_task->pid)) {
	print_memory_byte(STACK_TOP-64, STACK_TOP);
  }
  */

  if (is_scheduler_ready()){
	/* Update current on-going task ticks. */
	update_task_on_tick();
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
