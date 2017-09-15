#include <stdio.h>

class foo {
public:
  int myVal;

  foo() { myVal = 0; }

  int bar(int bar_i);
};

__attribute__((noinline))
int foo::bar(int bar_i) {
  int sum = 0;

  sum += printf("this->myVal = %d\n", this->myVal);
  this->myVal = bar_i;
  sum += printf("bar_i = %d %d %d %d %d %d %d %d %d %d\n", bar_i, bar_i, bar_i, bar_i, bar_i, bar_i, bar_i, bar_i, bar_i, bar_i);
  bar_i++;
  sum += printf("bar_i = %d %d %d %d %d %d %d %d %d %d\n", bar_i, bar_i, bar_i, bar_i, bar_i, bar_i, bar_i, bar_i, bar_i, bar_i);
  {
    int retval = sum + 1;
    retval += printf("bar_i = %d %d %d %d %d %d %d %d %d %d\n", bar_i, bar_i, bar_i, bar_i, bar_i, bar_i, bar_i, bar_i, bar_i, bar_i);
    sum += printf("retval = %d\n", retval);
    retval += printf("sum = %d\n", sum);

    return retval;
  }
}

int main() {
  foo v01;
  v01.bar(20);
  printf ("ALL DONE\x1A\n");
  return 0;
}