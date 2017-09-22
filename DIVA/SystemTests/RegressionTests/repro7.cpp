// gcc version 7.1
// g++ repro.cpp -g -c

template < int b > class c {};
template < typename d > struct e {};
e< struct a > f;
struct a {
  c< 3 > g;
};

