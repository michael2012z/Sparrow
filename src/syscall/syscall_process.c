#include <linkage.h>
#include <printk.h>

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

