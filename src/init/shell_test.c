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
  static int i = 0;

  if (0 == i) {
    static char *ch_p = "jiffies\n";
    if (0 != *ch_p)
      return *(ch_p++);
    else {
      i++;
    }
  } 
  if (1 == i) {
    static char *ch_p = "vruntime\n";
    if (0 != *ch_p)
      return *(ch_p++);
    else {
      i++;
    }
  }
  if (2 == i) {
    static char *ch_p = "ls\n";
    if (0 != *ch_p)
      return *(ch_p++);
    else {
      i++;
    }
  }
  if (3 == i) {
    static char *ch_p = "ls /ListFS\n";
    if (0 != *ch_p)
      return *(ch_p++);
    else {
      i++;
    }
  }
  if (4 == i) {
    static char *ch_p = "help\n";
    if (0 != *ch_p)
      return *(ch_p++);
    else {
      i++;
    }
  }
  if (5 == i) {
    static char *ch_p = "help ls\n";
    if (0 != *ch_p)
      return *(ch_p++);
    else {
      i++;
    }
  }
  if (6 == i) {
    static char *ch_p = "elf /bin/demo_4\n";
    if (0 != *ch_p)
      return *(ch_p++);
    else {
      i++;
    }
  }
  if (7 == i) {
    static char *ch_p = "elfs /bin/demo_7\n";
    if (0 != *ch_p)
      return *(ch_p++);
    else {
      i++;
    }
  }
  if (8 == i) {
    static char *ch_p = "elfa /demo_5\n";
    if (0 != *ch_p)
      return *(ch_p++);
    else {
      i++;
    }
  }
  if (9 == i) {
    static char *ch_p = " asdffds \n";
    if (0 != *ch_p)
      return *(ch_p++);
    else {
      i++;
    }
  }
  
  printk(PR_SS_INI, PR_LVL_DBG5, "%s: command test finished, jump into dead-loop\n", __func__);
  while(1);
  return 0;
}
