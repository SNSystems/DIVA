/* multiple.cpp */
#include "multiple_a.cpp"
#include "multiple_b.cpp"
#include "multiple_c.cpp"

void foo()
{
#ifdef _A
  F1(505);
#endif /* _A */

#ifdef _B
  F2(620);
#endif /* _B */

#ifdef _C
  F3(800);
#endif /* _C */
}
