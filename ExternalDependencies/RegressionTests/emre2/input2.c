
#include <stdio.h>
class MyClass {
 public:
  MyClass(int i) : i_(i) { }
 private:
  int i_;
};

int main() {
  MyClass obj1(1);
  MyClass obj2(2);
  return 0;
}

