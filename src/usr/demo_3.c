
void main() {
  int i,j;
  char *string = "Hello Sparrow!\n";
  i = 0x11223344;
  j = 0xaabbccdd;

  /* system call */
  asm("mov r0, %0  @ parameter 1\n"
	  "mov r1, #15 @ parameter 2\n"
	  "mov r2, #0  @ parameter 3\n"
	  "mov r3, #0  @ parameter 4\n"
	  "mov r7, #1  @ scno\n"
	  "swi #0 \n"
	  :
	  : "r"(string)
	  : "r0");

  i = 0x11111111;
  j = 0xaaaaaaaa;

  while(1);
}
