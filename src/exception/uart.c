#include <type.h>
#include <linkage.h>
#include <uart.h>
#include <printk.h>

int uart_registered_pid = -1;
char *uart_registerd_ch = NULL;

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
	*uart_registerd_ch = ch;
	/* unregister the listener immediatelly */
	uart_registered_pid = -1;
	uart_registerd_ch = NULL;
	//TODO: change process state to ready
	//TODO: set need_schedule flag
  }
}
