// g++ -std=c++11 -g -c function.cpp -o function.o

int func1(int x) { return x; }

static int func2(int x) { return x; }

inline __attribute__((always_inline)) int func3(int x) { return x; }

static inline __attribute__((always_inline)) int func4(int x) { return x; }

struct S {
    int method(int x, int y);
};

int S::method(int x, int y) {
    return x + y;
}

void test() {
    func2(0);
    func3(0);
}
