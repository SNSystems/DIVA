

struct x {
	int a;
	int b;
};

struct y {
  int f;
  char *g;
};

struct x xs;
struct y ys;

int goo(struct x *);
int hoo(struct y *);
int foo() {
    int a = 0;

    xs.a = 1;
    xs.b = 2;
    a = goo(&xs);
    a += hoo(&ys);

    return a;
}
