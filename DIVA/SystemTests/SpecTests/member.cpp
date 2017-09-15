// g++ -std=c++11 -g -c member.cpp -o member.o

struct A {
private:
    int m_private = 1;
public:
    int m_public = 1;
protected:
    int m_protected = 1;
};

void test() {
    A a;
}
