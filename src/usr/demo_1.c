
void main() {
  char *str = "Hello, I'm demo 1.\n";
  int i = 0, j = 0;
  char uart;
  char *uart_p = &uart;
  char buf[16];

  for (i = 0; i < 19; i++)
	*uart_p = str[i];

  while(1) {
	for (i = 0; i < 16; i++) {
	  for (j = 0; j < 16; j++)
		buf[j] = i;
	  for (j = 0; j < 64; j++)
		j = j; /* just waste time */
	}
  }
}
