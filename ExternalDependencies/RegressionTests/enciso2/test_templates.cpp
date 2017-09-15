/************************************************/
template<class XX,int n,class YY,bool b>
class tmpl_01 {
  typedef XX _type;
  public:
    _type t;
    YY y;
};

tmpl_01<int,620,char,true> my_tmpl_1;

/************************************************/
template<class SS>
struct tmpl_02 {
  SS s;
};

template<template<class AA> class UU,class TT>
struct tmpl_03 {
  UU<int> u;
  TT t;
  void ba() {};
};

void FNT_21()
{
  tmpl_03<tmpl_02,short> my_tmpl_3;
  my_tmpl_3.ba();
}

/************************************************/
template<class S,class T>
struct tmpl_04 {
};

template<class S,class T,class U>
struct tmpl_05 {
  tmpl_04<S,T>   f(S s);
  tmpl_05<S,T,U> g(T s);
  T              h(U u,T t);
};
template<class S,class T,class U>
tmpl_04<S,T> tmpl_05<S,T,U>::f(S s)
{
  tmpl_04<S,T> my_tmpl_04;
  return my_tmpl_04;
}
template<class S,class T,class U>
tmpl_05<S,T,U> tmpl_05<S,T,U>::g(T t)
{
  tmpl_05<S,T,U> my_tmpl_05;
  return my_tmpl_05;
}
template<class S,class T,class U>
T tmpl_05<S,T,U>::h(U u,T t)
{
  T my_T = t;
  return my_T;
}

void FNT_03()
{
  tmpl_05<int,int,int> b;
  b.f(505);
  b.g(620);
  b.h(710,800);
}

/************************************************/
template<class S>
struct tmpl_06 {
  S s;
};

template<class T>
struct tmpl_07 {
  T t;
};

template<template<class A> class U,template<class B> class V>
struct tmpl_08 {
  U<int>  u;
  V<char> v;
};

tmpl_08<tmpl_06,tmpl_07> my_tmpl_8;

/************************************************/
template<typename S,typename T>
T FNT_06(T a,S b)
{
  return a;
}

template<class U,class V>
U FNT_01(U a,V b)
{
  typedef U tu;
  typedef tu tv;
  tv c;
  c = a + b;
  return c;
}

void FNT_04()
{
  FNT_06(false,620);
  FNT_01(505,620);
}

/************************************************/
template<typename T,typename U>
T FNT_02(T a,U b)
{
  return a;
}

template<class T,int i>
class tmpl_09 {
  public:
    char buffer[i];
    T testFunc(T *p);
};

template<class T,int i>
T tmpl_09<T,i>::testFunc(T *p)
{
  return *(p++);
};

void FNT_05()
{
  static char *name = "ba";
  char *p = name;
  tmpl_09<char,5> my_tmpl_09;
  FNT_02(false,620);
  my_tmpl_09.testFunc(p);
}

/************************************************/
template<class T,class U,U Func(T)>
class tmpl_12 {
  public:
    tmpl_12(T t) { Func(t); }
};

char FNT_07(int i) { return (char)0; }

void FNT_15()
{
  tmpl_12<int,char,FNT_07> my_tmpl_12(10);
}

/************************************************/
template<class TT,int aa>
TT FNT_08(TT a,int b)
{
  return a;
}

void FNT_09(int bb)
{
  FNT_08<int,620>(620,bb);
}

/************************************************/
template<class _Ty>
class tmpl_25 {
};

class tmpl_15 {
  public:
    typedef char (*CommandCallback)(char Command);
    void Register(CommandCallback Callback);
  private:
    typedef tmpl_25<CommandCallback> CommandMap;
    CommandMap m_CommandMap;
};
void tmpl_15::Register(CommandCallback Callback)
{
}

class Foo {
  public:
    static char Remote(char Command);
};
char Foo::Remote(char Command) { return 'b'; }

void FNT_13(tmpl_15& Bar)
{
  Bar.Register(Foo::Remote);
}

/************************************************/
class tmpl_23 {
};

class tmpl_24 {
  public:
    tmpl_23 *getAnimBlendMatrix(void);
};

template<class T>
const T FNT_11(T &a)
{
  return a;
}

tmpl_23 *tmpl_24::getAnimBlendMatrix(void)
{
  static tmpl_23 my_tmpl_23;
  int step = FNT_11(step);
  return &my_tmpl_23;
}

void FNT_16()
{
  tmpl_24 my_tmpl_24;
  my_tmpl_24.getAnimBlendMatrix();
}

/************************************************/
template<class S>
struct tmpl_10 {
  S s;
};

template<class T>
struct tmpl_11 {
  tmpl_10<T> t;
};

tmpl_11<char> my_tmpl_11;

template<class T>
struct tmpl_13 {
  T t;
};

/************************************************/
template<class S,class T>
struct tmpl_28 {
  tmpl_13<T> f();
  S s;
};
template<class S,class T>
tmpl_13<T> tmpl_28<S,T>::f()
{
  static tmpl_13<T> my_tmpl_13;
  return my_tmpl_13;
}

void FNT_10()
{
  tmpl_28<char,int> my_tmpl_28;
  my_tmpl_28.f();
}

/************************************************/
template<class _Ty>
class tmpl_26 {
  public:
	  template<class _Other>
    struct rebind {	
		  typedef tmpl_26<_Other> other;
	  };
};

template<class _Ty,class _Alloc>
class tmpl_21 {
  typedef typename _Alloc::template rebind<_Ty>::other _Alty;
};

template<class _Elem,class _Ax>
class tmpl_22 : public tmpl_21<_Elem, _Ax> {
public:
  void Set();
};
template<class _Elem,class _Ax>
void tmpl_22<_Elem,_Ax>::Set()
{
}

void FNT_18()
{
  typedef tmpl_22<char,tmpl_26<char> > tdef_tmpl_22;
  tdef_tmpl_22 my_tmpl_22;
  my_tmpl_22.Set();
}

/************************************************/
namespace evo {
  namespace integer {
    typedef unsigned int u32;
  }
  namespace synchronisation {
    template<typename T>
    class AtomicAddress {
    };
    typedef AtomicAddress<evo::integer::u32> au32;
    template<template<class T> class TT, class T>
    static void GenericAtomicIncrement(TT<T> address) {}
    void AtomicIncrement(au32 address) { GenericAtomicIncrement(address); }
  }
}

/************************************************/
template<class T>
struct tmpl_17 {
  T comp;
};
template<class U>
void FNT_12(tmpl_17<U> formal)
{
}
void FNT_17()
{
  tmpl_17<int> my_tmpl_17;
  my_tmpl_17.comp = 620;
  FNT_12(my_tmpl_17);
}

/************************************************/
template<typename Elem>
class tmpl_18 {
  public:
    int Length();
};
template<typename Elem>
int tmpl_18<Elem>::Length()
{
  return 800;
}

template<template<typename> class Container = tmpl_18>
class tmpl_19 {
};

struct tmpl_20 {
  struct AAA {};
  struct BBB {};
  tmpl_18<AAA> nodes;
  tmpl_18<BBB> more_nodes;
  tmpl_19<> _nodeHash;
};

void FNT_19(tmpl_20 &cg)
{
	int numNodes = cg.nodes.Length();
	numNodes = cg.more_nodes.Length();
}

void FNT_20()
{
  tmpl_20 my_tmpl_20;
  FNT_19(my_tmpl_20);
}

/************************************************/
template<long long T>
class tmpl_27 {
public:
  long long x(void) { return T; }
};
void FNT_14()
{
  tmpl_27<0x123456789L> my_tmpl_27;
  long long ba = my_tmpl_27.x();
}

/************************************************/
namespace std {

  template<class _Elem>
	struct char_traits {
  };

  template<class _Ty>
  class tmpl_26 {
  };

  template<class _Ty1,class _Ty2>
  struct pair	{
  };

  template<class _Elem,class _Traits = char_traits<_Elem>,class _Ax = tmpl_26<_Elem> >
  class tmpl_22 {
  };

  typedef tmpl_22<char,char_traits<char>,tmpl_26<char> > string;

  template<class _Ty,class _Ax = tmpl_26<_Ty> >
  class vector {
  };

  template<class _Ty>
	struct less	{
	};

  template<class _Kty,class _Ty,class _Pr = less<_Kty>,class _Alloc = tmpl_26<pair<const _Kty, _Ty> > >
	class tmpl_25 {
  };
}

class tmpl_14 {
  public:
    typedef std::string (*CommandCallback)(const std::string& Command,std::vector<std::string>& Params);
    void Register(const char *sCmd,CommandCallback Callback);
  private:
    typedef std::tmpl_25<std::string,CommandCallback> CommandMap;
    CommandMap m_CommandMap;
};
void tmpl_14::Register(const char *sCmd,CommandCallback Callback)
{
}

class tmpl_16 {
  public:
    static void Register(tmpl_14 &Bar);
    static std::string Remote(const std::string &Command,std::vector<std::string> &Params);
  private:
    static bool m_bDefVis;
};
std::string tmpl_16::Remote(const std::string &Command,std::vector<std::string> &Params)
{
  std::string name;
  return name;
}

void tmpl_16::Register(tmpl_14& Bar)
{
  Bar.Register("vsp",tmpl_16::Remote);
}
/************************************************/


int main()
{
  tmpl_15 my_tmpl_15;

  FNT_03();
  FNT_04();
  FNT_05();
  FNT_09(620);
  FNT_10();
  FNT_13(my_tmpl_15);
  FNT_14();
  FNT_15();
  FNT_16();
  FNT_17();
  FNT_18();
  FNT_20();
  FNT_21();

  return 0;
}
