#include <stdio.h>
#include <stdlib.h>

void main() {
  char *param1 = (char *)malloc(16);
  char *param2 = (char *)malloc(16);
  char *param3 = (char *)malloc(16);
  char *param4 = (char *)malloc(16);
  int i = 0, r;

  param1[0] = '\0';
  param2[0] = '\0';
  param3[0] = '\0';
  param4[0] = '\0';
  printf("999999999999999999999999999999999999999999\n");
  params(param1, param2, param3, param4);
  printf("param1 = %s\n", param1);
  printf("param2 = %s\n", param2);
  printf("param3 = %s\n", param3);
  printf("param4 = %s\n", param4);

  for(i = 0; i < 5; i++) {
	r = random();
	printf("random() = %d\n", r);
  }

}
