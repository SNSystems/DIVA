namespace nsp {
  struct ee {
    struct ff {
      struct gg {
        int v_integer;
      };
    };
  };
}

int main() {
  nsp::ee::ff::gg v_gg;
  v_gg.v_integer = 620;

  return 0;
}
