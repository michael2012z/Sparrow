
void main() {
  int i,j;
  i = 0x11223344;
  j = 0xaabbccdd;

  /* system call */
  asm("mov r0, #10 @ parameter 1\n"
	  "mov r1, #11 @ parameter 2\n"
	  "mov r2, #12 @ parameter 3\n"
	  "mov r3, #13 @ parameter 4\n"
	  "mov r7, #1  @ scno\n"
	  "swi #0 \n");

  i = 0x11111111;
  j = 0xaaaaaaaa;

  while(1);
}
