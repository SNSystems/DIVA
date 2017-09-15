struct point { int x; int y;};
extern struct point a; 

int g();
int foo();
void h(void);
int main()
{
  h();
}

int g() 
{
    return 12 + a.x;
}

