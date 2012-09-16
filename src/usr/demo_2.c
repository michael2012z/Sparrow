
void main() {
  char *str = "Hello, I'm demo 2.\n";
  int i = 0;
  char uart;
  char *uart_p = &uart;

  for (i = 0; i < 19; i++)
	*uart_p = str[i];

  while(1);
}
