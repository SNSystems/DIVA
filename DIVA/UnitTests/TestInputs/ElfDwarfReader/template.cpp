template<int VAL, typename TY> void t_func() {}

void test() {
    t_func<1, int>();
    t_func<-1, int>();
}
