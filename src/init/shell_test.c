#include <linkage.h>
#include <uart.h>
#include <printk.h>

char* test_cmds[] = {
  "jiffies\n",
  "vruntime\n",
  "help\n",
  "help ls\n",
  "ls\n",
  "elf /bin/demo_4\n",
  "elf /bin/demo_7 abc def\n",
  "    abcdefg  \n",
};

char inputc() {
  static char *ch_p = "jiffies\n";

  if (0 != *ch_p)
	return *(ch_p++);
  else {
	printk(PR_SS_INI, PR_LVL_DBG5, "%s: command test finished, jump into dead-loop\n", __func__);
	while(1);
  }

}
