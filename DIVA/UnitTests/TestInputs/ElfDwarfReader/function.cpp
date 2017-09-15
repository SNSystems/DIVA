int func1(int x) { return x; }

void test(int x) {
    int (*f_ptr)(int) = &func1;
}
