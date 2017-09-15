namespace OuterNS {
    namespace InnerNS {
        class C {
            typedef int INT;
            INT member;
            static int s_mem;
        };
    }
}

int OuterNS::InnerNS::C::s_mem;

void test() {
    OuterNS::InnerNS::C c;
}
