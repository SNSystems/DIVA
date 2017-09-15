//-----------------------------------------------------------------------------
// test.cc
//-----------------------------------------------------------------------------

class Foo
{
public:

  int m1;
  int m2;
};

typedef struct 
{ 
  unsigned short flag1 : 1;
  unsigned short flag2 : 2;
  unsigned short flag3 : 3; 

} bar_t;


int main(void)
{
  Foo f;

  f.m1 = 4;
  f.m2 = 9;

  bar_t b;

  b.flag1 = 0;
  b.flag2 = 3;
  b.flag3 = 4;

  return 0;
}

