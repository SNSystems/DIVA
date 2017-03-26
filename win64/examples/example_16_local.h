/* example_16_local.h */
class Local {
  public:
    int foo(int l);
  private:
    int m_l;
};

int do_local(int l);
int do_global(int g);
