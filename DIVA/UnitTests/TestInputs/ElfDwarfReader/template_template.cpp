template<typename T> class vector {
  T* vals;
};

template<typename T, template <typename> class V>
void foo(V<T> vec) {
}

void test() {
    vector<int> v;
    foo(v);
}
