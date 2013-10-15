#include <stdlib.h>

int main() {
  int i,j;
  char *p1, *p2;
  i = 0x11223344;
  j = 0xaabbccdd;

  p1 = (char *)malloc(16);
  p2 = (char *)malloc(32);
  p1[5] = 'a';
  p2[12] = 'b';
  free(p1);
  free(p2);

  i = 0xaaaaaaaa;
  j = 0xcccccccc;

  while(1);
  return 0;
}
