#include "../inc/stdlib.h"

void exit(int code) {
  asm("mov r0, %0  @ parameter 1\n"
	  "mov r1, #0  @ parameter 2\n"
	  "mov r2, #0  @ parameter 3\n"
	  "mov r3, #0  @ parameter 4\n"
	  "mov r7, #5  @ scno\n"
	  "swi #0 \n"
	  :
	  : "r"(code)
	  : "r0", "r1", "r2", "r3", "r7");
}
