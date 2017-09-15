int var;

struct S {
    int mem;
};

void test(int param) {
    S s;
    s.mem = 0;
}
void var_param(...) {}
