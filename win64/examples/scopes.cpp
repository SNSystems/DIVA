/* scopes.cpp */
#ifdef _A
typedef int INT;
#endif /* _A */

void foo()
{
#ifndef _A
  typedef int INT;
#endif /* _A */
  INT a = 1;
}
