#include <timer.h>
#include <printk.h>

void init_timer() {
  arm_init_timer();
}

/* Get called on periodic tick.
 * Update current task ticks. Re-schedule if the min time slices has been used up.
 */
void on_timer() {
  if (is_scheduler_ready()){
	/* Update current on-going task ticks. */
	update_task_on_tick();
	/* Check left time slices,
	 * reschedule if time slices is up.
	 */
	if (check_should_schedule()) {
	  printk(PR_SS_IRQ, PR_LVL_INF, "%s, current task times up, will reschedule\n", __func__);
	  schedule();
	} else
	  printk(PR_SS_IRQ, PR_LVL_INF, "%s, current task should continue, not kicked out\n", __func__);
  }
}
