#include "lto_cross_cu.h"
A::G foo() { return A::G(); }
A::G bar();

int main() {
    foo();
    bar();
    return 0;
}
