// g++ -std=c++11 -g -c struct.cpp -o struct.o

struct A {};

struct B : public A {};

struct C : private A{};

struct D : public B, protected C {};

void test() {
    A a;
    B b;
    C c;
    D d;
}
