/* example_09.cpp */
typedef char CHAR;

class A {
  int a;
};

A a;

CHAR foo(char *p)
{
  CHAR c = *p;
  return c;
}
