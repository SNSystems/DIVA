struct STR {
  int size;
};

template <typename T>
T ADD(T a)
{
  STR s;
  s.size = 12;

  return a;
}

template <typename T>
T SUB(T a)
{
  STR t;
  t.size = 24;

  return a;
}

void foo()
{
  int add = ADD(505);
  int sub = SUB(525);
}
