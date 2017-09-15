
template<typename = void>
int sum() { return 0; }

template<typename... Targs>
int sum(int x, Targs... args) {
    return x + sum(args...);
}

void test() {
    sum(1, 2, 3);
}
