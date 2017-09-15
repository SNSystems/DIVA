// g++ -std=c++11 -g -c using.cpp -o using.o

struct A {
protected:
    int m;
    typedef int INT;
    void foo() {}
};

struct B : public A {
    using A::m;
    using A::INT;
    using A::foo;
};

namespace NS {
    int x;
}

using namespace NS;

void test() {
    x = 0;
    B b;
    b.m = 0;
    B::INT bx;
    b.foo();
}
