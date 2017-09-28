// https://github.com/SNSystems/DIVA/issues/17
// g++ repro17.cpp -g -c -O2

template < typename _Tp>
class Trans_NS___cxx11_list
    {

typedef _Tp iterator;

typedef _Tp _Node;
  _Node _M_create_node();

public:
  iterator end() ;
  void push_back(_Tp ) { _M_insert(end()); }
  template < typename... _Args > void _M_insert(_Args ... ) {
    _M_create_node();
  }
};
class sem_item_optimizer {
Trans_NS___cxx11_list< int * > worklist;
  void worklist_push();
};
int *worklist_push_cls;
void sem_item_optimizer::worklist_push() {
  worklist.push_back(worklist_push_cls);
}

