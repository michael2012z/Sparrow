
char inputc() {
  register int res __asm__ ("r0");
  /* system call */
  asm("mov r0, #0  @ parameter 1\n"
	  "mov r1, #0  @ parameter 2\n"
	  "mov r2, #0  @ parameter 3\n"
	  "mov r3, #0  @ parameter 4\n"
	  "mov r7, #4  @ scno\n"
	  "swi #0 \n");
  return (char)res;
}
