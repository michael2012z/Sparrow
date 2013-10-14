
void func1();

int main() {
  int i,j;
  i = 0x11223344;
  j = 0xaabbccdd;

  i = i % j; 

  func1();

  while(1);

  return 0;
}

void func3() {
  int i, j;
  i = 0xf3f3f3f3;
  j = 0xeeeeeeee;
  return;
}

void func2() {
  int i, j;
  i = 0xf2f2f2f2;
  j = 0xeeeeeeee;
  func3();
  return;
}

void func1() {
  int i, j;
  i = 0xf1f1f1f1;
  j = 0xeeeeeeee;
  func2();
  return;
}

