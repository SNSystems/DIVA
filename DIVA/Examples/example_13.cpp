/* example_13.cpp */
extern int foo(int X);

int bar(int X, int *A) {
  for (int i = 0; i < X; ++i) {
    A[i] = foo(i);
  }

  return A[0];
}
