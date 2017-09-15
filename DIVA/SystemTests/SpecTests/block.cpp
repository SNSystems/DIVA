// g++ -std=c++11 -g -c block.cpp -o block.o

void test() {
    try {
        int x = 1;
        throw 1;
    } catch (...) {
        int y = 2;
    }

    {
        int x = 3;
    }
}
