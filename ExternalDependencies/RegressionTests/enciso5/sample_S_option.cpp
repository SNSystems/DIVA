typedef const short * _Ctype_t;

namespace nsp_a {
  struct aa {
    struct bb {
      struct cc {
        _Ctype_t v_ctype;
        long *v_long;
        float v_float;
      };
    };
  };
}

struct dd {
  nsp_a::aa::bb::cc v_cc;
};

struct ee {
  struct ff {
    struct gg {
      int v_integer;
    };

    struct hh {
      int v_integer;
    };
  };
};

long bar() {
  ee::ff::gg v_gg;

  v_gg.v_integer = 4;

  return v_gg.v_integer;
}

int main() {
  dd v_dd;
  v_dd.v_cc.v_float = 15.5;
  int v_integer = bar();

  return 0;
}
