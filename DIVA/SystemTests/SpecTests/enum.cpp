// g++ -std=c++11 -g -c enum.cpp -o enum.o

enum E1 {
    A,
    B,
    C,
};

enum E2 : signed short {
    G = -1,
    H = 0,
    I = 1,
};

enum class E3 {
    D = 1,
    E,
    F,
};

void test() {
    E1 e1;
    E2 e2;
    E3 e3;
}
