/* example_14.cpp */
struct bar {};
typedef char CHAR;

namespace nsp_1 {
  struct bar {};
  typedef char CHAR;
  namespace nsp_2 {
    struct bar {};
    typedef char CHAR;
  }
}

template<class _Ty>
class foo {
  _Ty b;
};

CHAR a;
nsp_1::CHAR b;
nsp_1::nsp_2::CHAR c;

foo<bar> b1;
foo<nsp_1::bar> b2;
foo<nsp_1::nsp_2::bar> b3;
