struct bar {
  inline __attribute__((always_inline))
  int f01(int a1);
  inline __attribute__((always_inline))
  int f02(int a1,int a2);
  inline __attribute__((always_inline))
  int f03(int a1,int a2,int a3);
  inline __attribute__((always_inline))
  int f04(int a1,int a2,int a3,int a4);
  inline __attribute__((always_inline))
  int f05(int a1,int a2,int a3,int a4,int a5);
};

inline __attribute__((always_inline))
int bar::f01(int a1)
{
  int a;
  return a + a1;
}

inline __attribute__((always_inline))
int bar::f02(int a1,int a2)
{
  int a;
  return a + a1 + a2;
}

inline __attribute__((always_inline))
int bar::f03(int a1,int a2,int a3)
{
  int a;
  return a + a1 + a2 + a3;
}

inline __attribute__((always_inline))
int bar::f04(int a1,int a2,int a3,int a4)
{
  int a;
  return a + a1 + a2 + a3 + a4;
}

inline __attribute__((always_inline))
int bar::f05(int a1,int a2,int a3,int a4,int a5)
{
  int a;
  return a + a1 + a2 + a3 + a4 + a5;
}

void foo()
{
  bar b;
  b.f01(1);
  b.f02(1,2);
  b.f03(1,2,3);
  b.f04(1,2,3,4);
  b.f05(1,2,3,4,5);
}
