struct A {
protected:
    int m;
};

struct B : public A {
    using A::m;
};

namespace NS {
    int x;
}

using namespace NS;

void test() {
    x = 0;
    B b;
    b.m = 0;
}
