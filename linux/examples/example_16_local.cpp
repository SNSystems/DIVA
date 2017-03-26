/* example_16_local.cpp */
#include "example_16_local.h"
#include "example_16_global.h"

int Local::foo(int l)
{
  m_l = l * 505;
  return m_l;
}

__attribute__((optnone))
int do_local(int l)
{
  Local local;
  return local.foo(l);
}

__attribute__((optnone))
int do_global(int g)
{
  Global global;
  return global.foo(g);
}
