#include <type.h>
#include <linkage.h>
#include <uart.h>
#include <printk.h>
#include <process.h>

int uart_registered_pid = -1;
char *uart_registerd_ch = NULL;
extern bool need_reschedule;
extern struct task_struct *current_task;

void init_uart() {
  arm_init_uart();
}

int register_uart_input_c (int pid, char *ch_p) {
  if (uart_registered_pid == -1) {
	uart_registered_pid = pid;
	uart_registerd_ch = ch_p;
	return 0;
  } else {
	printk(PR_SS_IRQ, PR_LVL_DBG6, "%s: input device (uart0) has been occupied by pid = %x\n", __func__, uart_registered_pid);
	return -1;
  }
}

void uart_input_char(char ch) {
  if (uart_registered_pid == -1) {
	printk(PR_SS_IRQ, PR_LVL_DBG6, "%s: no process is listening uart0\n", __func__);
  } else {
	struct task_struct *listener = find_task_by_pid(uart_registered_pid);
	*uart_registerd_ch = ch;
	uart_registered_pid = -1;
	if (NULL == listener) {
	  printk(PR_SS_IRQ, PR_LVL_DBG6, "%s: no uart0 listener found\n", __func__);
	  return;
	}
	else {
	  printk(PR_SS_IRQ, PR_LVL_DBG6, "%s: pid = %d is listening uart0\n", __func__, listener->pid);
	  dequeue_task(listener);
	  listener->sched_en.state = PROCESS_STATE_READY;
	  enqueue_task(listener, sched_enqueue_flag_new);
	  need_reschedule = true;
	}
	uart_registerd_ch = NULL;
  }
}

char inputc() {
  char ch;
  if (register_uart_input_c(current_task->pid, &ch) < 0)
	return 0;
  else {
	/* change state as waiting */
	/* schedule */
	current_task->sched_en.state = PROCESS_STATE_WAITING;
	schedule();
  }
  return ch;
}
