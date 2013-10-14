void printf(const char *fmt, ...);

int main() {
  int i,j;
  i = 0x11223344;
  j = 0xaabbccdd;

  printf("%d +++++++++++++++++++++++++++++++++++++++++++++++\n", i);

  i = 0xaaaaaaaa;
  j = 0xcccccccc;

  while(1);
  return 0;
}
