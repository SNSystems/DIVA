// Note: GCC leave both m_unspecified and m_public unspecified, CLang makes them
// both DW_ACCESS_public

struct A {
    int m_unspecified = 1;
private:
    int m_private = 1;
public:
    int m_public = 1;
protected:
    int m_protected = 1;
};

int func() {
    A a;
    return a.m_public;
}
