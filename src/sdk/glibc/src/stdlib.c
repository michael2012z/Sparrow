#include "../inc/stdlib.h"

void sleep(int msec) {
  asm("mov r0, %0  @ parameter 1\n"
	  "mov r1, #0  @ parameter 2\n"
	  "mov r2, #0  @ parameter 3\n"
	  "mov r3, #0  @ parameter 4\n"
	  "mov r7, #3  @ scno\n"
	  "swi #0 \n"
	  :
	  : "r"(msec)
	  : "r0", "r1", "r2", "r3", "r7");
}

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

void params(char* param1, char* param2, char* param3, char* param4) {
  asm("mov r0, %0  @ parameter 1\n"
	  "mov r1, %1  @ parameter 2\n"
	  "mov r2, %2  @ parameter 3\n"
	  "mov r3, %3  @ parameter 4\n"
	  "mov r7, #6  @ scno\n"
	  "swi #0 \n"
	  :
	  : "r"(param1), "r"(param2), "r"(param3), "r"(param4)
	  : "r0", "r1", "r2", "r3", "r7");
}

int random() {
  register int res __asm__ ("r0");
  asm("mov r0, #0  @ parameter 1\n"
	  "mov r1, #0  @ parameter 2\n"
	  "mov r2, #0  @ parameter 3\n"
	  "mov r3, #0  @ parameter 4\n"
	  "mov r7, #7  @ scno\n"
	  "swi #0 \n"
	  :
	  : 
	  : "r0", "r1", "r2", "r3", "r7");
  return res;
}
