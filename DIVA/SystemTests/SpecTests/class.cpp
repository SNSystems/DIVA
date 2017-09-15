// g++ -std=c++11 -g -c class.cpp -o class.o

class A {};

class B : public A {};

class C : private A{};

class D : public B, protected C {};

void test() {
    A a;
    B b;
    C c;
    D d;
}
