
extern int g ();
struct point
{
  int x;
  int y;
};
struct point a;

static void
add_point (struct point *ap, const struct point *bp)
{
  ap->x += bp->x;
  ap->y += bp->y;
}

static void
f (int *m)
{

  *m = g ();
  struct point b;
  a.x = 1;
  a.y = 2;
  b.x = 3;
  b.y = 4;
  add_point (&a, &b);
}

int
h (void)
{
  int mv = 27;
  f (&mv);
  return mv;
}

