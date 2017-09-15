#include "header.h"

extern int foo(void);

struct s str;

int main(void)
{
	return foo() * str.x;
}

