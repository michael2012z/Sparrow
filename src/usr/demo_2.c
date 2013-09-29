
void main() {
  char buf[16];
  int i, j, count;
  count = 0;
  while(1) {
	for (i = 0; i < 16; i++) {
	  for (j = 0; j < 16; j++)
		buf[j] = i;
	  for (j = 0; j < 1024; j++);
	  count ++;
	}
  }
}
