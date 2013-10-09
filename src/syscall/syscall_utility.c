#include <linkage.h>
#include <printk.h>

long sys_print(char *string, int length) {
  printk(PR_SS_IRQ, PR_LVL_DBG6, "%s: string = %x, length = %x\n", __func__, string, length);
  //  while(1);
  return 0;
}
