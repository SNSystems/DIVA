template<class T>
T GetMax(T a,T b) {
  int ba;
  const T   *d = &a;
  const int *e = &ba;

  T c = a > b ? a : b;
  return c;
}

template<class T,class U>
T GetMin(T a,U b) {
  T c = a > b ? a : b;
  return c;
}

template<class T,class U>
class MyPair {
  T a;
  U b;
  public:
    MyPair(T first,U second) { a = first; b = second; }
    T GetMax();
    template<class V>
    void BB(V p) {}
};

template<class T,class U> 
T MyPair<T,U>::GetMax()
{
  T c = a > b ? a : b;
  return c;
}

int main() {
  GetMax       (505,620);
  GetMax<float>(505,620);
  GetMax       (14.2,15.5);

  GetMin<int,long>(505,620);

  MyPair<int,long> myobject(505,620);
  char pr = 'L';
  myobject.BB(pr);

  return 0;
}
