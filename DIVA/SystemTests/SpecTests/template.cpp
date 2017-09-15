// g++ -std=c++11 -g -c template.cpp -o template.o

class A {};

template<typename T>
class C : public A {};

template<typename T>
struct S : protected A {};

template<typename T>
void func() {}

void test() {
    C<int> c;
    S<int> s;
    func<int>();
}
