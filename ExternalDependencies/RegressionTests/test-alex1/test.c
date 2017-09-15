#include <libdwarf.h>
#include <dwarf.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/*#define WORKING 1 */

void print_type_name(Dwarf_Debug dbg, Dwarf_Off offset) {
  Dwarf_Error err;
  Dwarf_Die new_die;
  Dwarf_Bool res;

  if (dwarf_offdie(dbg, offset, &new_die, &err) != DW_DLV_OK) {
    printf("Failed\n");
  }
  if (dwarf_hasattr(new_die, DW_AT_name, &res, &err) == DW_DLV_OK && res) {
    printf("0x%llx die has DW_AT_name!\n", offset);
  } else {
    printf("0x%llx die has NO DW_AT_name\n", offset);
  }
}

int main() {
  int fd;
  Dwarf_Debug dbg;
  Dwarf_Error err;
  Dwarf_Die new_die;
  Dwarf_Unsigned next_cu_hdr;
  Dwarf_Unsigned cu_hdr_len;
  Dwarf_Half version_stamp, address_size;
  Dwarf_Off abbrev_offset;
  int c_res;
  Dwarf_Error dwarf_err;
  
  fd = open("orig.a.out", O_RDONLY);
  if (dwarf_init(fd, DW_DLC_READ, NULL, NULL, &dbg, &err) != DW_DLV_OK) {
    printf("Error\n");
    return 1;
  }
#if WORKING
  print_type_name(dbg,0x5e);
  print_type_name(dbg,0xb5);
  print_type_name(dbg,0x14b);
  print_type_name(dbg,0x1b1);
  print_type_name(dbg,0x219);
#else
  if (dwarf_offdie(dbg, 0x5e, &new_die, &err) != DW_DLV_OK) {
    printf("Failed\n");
  }
  c_res = dwarf_next_cu_header(dbg, &cu_hdr_len, &version_stamp, &abbrev_offset,
			       &address_size, &next_cu_hdr, &dwarf_err);
  c_res = dwarf_next_cu_header(dbg, &cu_hdr_len, &version_stamp, &abbrev_offset,
			       &address_size, &next_cu_hdr, &dwarf_err);
  c_res = dwarf_next_cu_header(dbg, &cu_hdr_len, &version_stamp, &abbrev_offset,
			       &address_size, &next_cu_hdr, &dwarf_err);
  if (dwarf_offdie(dbg, 0x219, &new_die, &err) != DW_DLV_OK) {
    printf("Failed\n");
  }

  print_type_name(dbg,0x5e);
  print_type_name(dbg,0xb5);
  print_type_name(dbg,0x14b);
  print_type_name(dbg,0x1b1);
  print_type_name(dbg,0x219);
#endif

  return 0;
}

