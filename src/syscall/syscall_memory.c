#include <linkage.h>
#include <printk.h>

long sys_brk(unsigned long brk) {
  printk(PR_SS_IRQ, PR_LVL_DBG6, "%s: brk = %x\n", __func__, brk);
  while(1);
  return 0;
}
