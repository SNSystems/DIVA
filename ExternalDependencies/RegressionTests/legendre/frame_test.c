#include "dwarf.h"
#include "libdwarf.h"
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>

#define FILENAME "./libmpich.so.1.0"
#define ADDR 0x5c183
#define REG 12


/**
 * With libdwarf-20090510 and latter this produces the output:
 *  Got reg value 0
 * With libdwarf-20090330 and earlier this produces the output:
 *  Got reg value 1036
 *
 * I believe the 1036 value is the correct one, as it leads to a
 * proper stack recipe for the function.
 **/
int main(int argc, char *argv[])
{
   Dwarf_Error err;
   Dwarf_Debug dbg;
   Dwarf_Cie *cie_data;
   Dwarf_Signed cie_count;
   Dwarf_Fde *fde_data, fde;
   Dwarf_Signed fde_count;
   Dwarf_Small value_type;
   Dwarf_Signed offset_relevant, register_num, offset_or_block_len;
   Dwarf_Ptr block_ptr;
   Dwarf_Addr row_pc;
   int result;
   int fd;

   fd = open("./libmpich.so.1.0", O_RDONLY);
   if (fd == -1) {
      perror("Could not open ./libmpich.so.1.0 for test");
      return -1;
   }
   result = dwarf_init(fd, DW_DLC_READ, NULL, NULL, &dbg, &err);
   assert(result == DW_DLV_OK);

   result = dwarf_get_fde_list(dbg, &cie_data, &cie_count, &fde_data, &fde_count,
                               &err);
   assert(result == DW_DLV_OK);


   result = dwarf_get_fde_at_pc(fde_data, ADDR, &fde, NULL, NULL, &err);
   assert (result == DW_DLV_OK);
     
   result = dwarf_get_fde_info_for_reg3(fde, REG, ADDR, &value_type, 
                                        &offset_relevant, &register_num,
                                        &offset_or_block_len,
                                        &block_ptr, &row_pc, &err);
   assert(result == DW_DLV_OK);
   
   
/*
   printf("value type %ld\n", (long int) value_type);
   printf("offset relevant %ld\n", (long int) offset_relevant);
   printf("offset or bl len %ld\n", (long int) offset_or_block_len);
   printf("bl ptr %ld\n", (long int) block_ptr);
   printf("row_pc %ld (0x%lx)\n", (long int) row_pc,(long int)row_pc);
*/

   printf("Register num  %ld\n", (long int) register_num);
#ifdef OLD
   assert(register_num == DW_FRAME_CFA_COL);
#else
   assert(register_num == DW_FRAME_CFA_COL3);
#endif
   /* This just shows we really test something. */
   assert(DW_FRAME_CFA_COL3 != DW_FRAME_CFA_COL);
   
   return 0;
}
