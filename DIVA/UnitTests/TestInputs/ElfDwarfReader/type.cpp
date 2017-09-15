typedef int T_INT;

void rv_ref(int &&i_rv_ref) {} // DW_TAG_rvalue_reference_type

void test() {
    T_INT i = 0;            // DW_TAG_base_type, DW_TAG_typedef
    const int i_const = 0;  // DW_TAG_const_type
    int *i_ptr = &i;        // DW_TAG_pointer_type
    int &i_ref = i;         // DW_TAG_reference_type
    int * __restrict i_res; // DW_TAG_restrict_type
    volatile int i_vol;     // DW_TAG_volatile_type


    // Untested:
    //   DW_TAG_unspecified_type
    //   DW_TAG_ptr_to_member_type
}
