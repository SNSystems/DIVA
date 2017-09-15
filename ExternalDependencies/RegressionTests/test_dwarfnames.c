
/* Test code to verify table and switch code work
   in libdwarf.
*/


#include <stdio.h>
#include <dwarf.h>
#include <libdwarf.h>

typedef int (*tfunc)(unsigned int, const char **out);


static void
run_test(tfunc callme, const char *name)
{
    int i;
    for ( i = 0; i < 50; ++i) {
          const char *out = 0;
          int res = callme(i,&out);
          if(res == DW_DLV_OK) {
             printf(" %s %d %s\n",name,i,out);
          } else {
             printf(" %s %d not valid code \n",name,i);
          }
    }
}

int main()
{
    run_test(dwarf_get_TAG_name,"dwarf_get_TAG_name");
    run_test(dwarf_get_children_name,"dwarf_get_children_name");
    run_test(dwarf_get_FORM_name,"dwarf_get_FORM_name");
    run_test(dwarf_get_AT_name,"dwarf_get_AT_name");
    run_test(dwarf_get_OP_name,"dwarf_get_OP_name");
    run_test(dwarf_get_ATE_name,"dwarf_get_ATE_name");
    run_test(dwarf_get_DS_name,"dwarf_get_DS_name");
    run_test(dwarf_get_END_name,"dwarf_get_END_name");
    run_test(dwarf_get_ATCF_name,"dwarf_get_ATCF_name");
    run_test(dwarf_get_ACCESS_name,"dwarf_get_ACCESS_name");
    run_test(dwarf_get_VIS_name,"dwarf_get_VIS_name");
    run_test(dwarf_get_VIRTUALITY_name,"dwarf_get_VIRTUALITY_name");
    run_test(dwarf_get_LANG_name,"dwarf_get_LANG_name");
    run_test(dwarf_get_ID_name,"dwarf_get_ID_name");
    run_test(dwarf_get_CC_name,"dwarf_get_CC_name");
    run_test(dwarf_get_INL_name,"dwarf_get_INL_name");
    run_test(dwarf_get_ORD_name,"dwarf_get_ORD_name");
    run_test(dwarf_get_DSC_name,"dwarf_get_DSC_name");
    run_test(dwarf_get_LNS_name,"dwarf_get_LNS_name");
    run_test(dwarf_get_LNE_name,"dwarf_get_LNE_name");
    run_test(dwarf_get_MACINFO_name,"dwarf_get_MACINFO_name");
    run_test(dwarf_get_CFA_name,"dwarf_get_CFA_name");
    run_test(dwarf_get_EH_name,"dwarf_get_EH_name");
    run_test(dwarf_get_FRAME_name,"dwarf_get_FRAME_name");
    run_test(dwarf_get_CHILDREN_name,"dwarf_get_CHILDREN_name");
    run_test(dwarf_get_ADDR_name,"dwarf_get_ADDR_name");
    return 0;
}
