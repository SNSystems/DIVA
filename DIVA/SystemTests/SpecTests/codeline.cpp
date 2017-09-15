// g++ -std=c++11 -g -c codeline.cpp -o codeline.o

void test() {
    int x;
    int y;
    int z;

    x = 0;
    y = 1;
    z = 2;

    x = y; y = z; z = x;
}
