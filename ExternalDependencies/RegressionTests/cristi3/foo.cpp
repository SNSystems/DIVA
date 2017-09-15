////////////////////////////////////////////////////////////////
// foo.cpp
#include <iostream>
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define FOOBAR foobar()
#define BARFOO int i=0;
#define RET return i;
#define EMPTY
using namespace std;
int foobar() { return 42; }
int inc(double x) { return x + 1; }
#define INC(a) inc(a)
#define INC1(b) inc(b)
#define INC2 inc

struct Foo
{
	Foo() { }
};
const void* foobar(const Foo& f)
{
	return &f;
}
#define FOOBARX Foo

int main()
{
	BARFOO
	cout << MIN(1, ++i) << endl;
#undef BARFOO
	Foo();
	cout << i << endl;
	cout << INC(inc(.1)) << endl;
	foobar();
	RET
#include "foo.h"
}
