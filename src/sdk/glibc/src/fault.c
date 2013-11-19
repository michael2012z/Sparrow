#include "../inc/stdio.h"

void div0(void) {
  printf("error: division by 0\n");
  while(1);
}
