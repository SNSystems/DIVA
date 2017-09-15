#if 0
cc -c -g -O2 "$0" && readelf -wi "${0%.c}.o"
exit 0
#endif
static void f(int *m) { *m = g(); }
void h(void) { int m; f(&m); }
