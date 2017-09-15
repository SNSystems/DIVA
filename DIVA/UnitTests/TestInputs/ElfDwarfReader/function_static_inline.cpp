// Warning: This probably only builds correctly (as far as the unit tests are
// concerned) in GCC
static int func1(int x) { return x; }

inline __attribute__((always_inline)) int func2(int x) { return x; }

static inline __attribute__((always_inline)) int func3(int x) { return x; }

static int func4(int x);

int func4(int x) { return x; }

int other() {
    int ret1 = func2(1);
    int ret2 = func3(3);
}
