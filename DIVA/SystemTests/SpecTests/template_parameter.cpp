// clang++ -std=c++11 -g -c template_parameter.cpp -o template_parameter.o

template<typename T>
class vector {};

template<int VAL, typename TY, template<typename> class TEMPLATE>
void t_func() {}

template<typename = void>
int sum() { return 0; }

template<typename... Targs>
int sum(int x, Targs... args) {
    return x + sum(args...);
}

void test() {
    t_func<1, int, vector>();
    sum(1, 2, 3);
}
