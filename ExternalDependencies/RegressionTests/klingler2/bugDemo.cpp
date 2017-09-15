// based on simplereader.c from ubuntu libdwarf-dev 20120410-2

#include <sys/types.h> /* For open() */
#include <sys/stat.h>  /* For open() */
#include <fcntl.h>     /* For open() */
#include <stdlib.h>     /* For exit() */
#include <unistd.h>     /* For close() */
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "dwarf.h"
#include "libdwarf.h"

static void showBug(Dwarf_Debug dbg);

int main(int argc, char **argv) {
  Dwarf_Debug dbg = 0;
  int fd = -1;
  const char *filepath = "<stdin>";
  int res = DW_DLV_ERROR;
  Dwarf_Error error;
  Dwarf_Handler errhand = 0;
  Dwarf_Ptr errarg = 0;

  if (argc < 2) {
    fd = 0; /* stdin */
  } else {
    filepath = argv[1];
    fd = open(filepath, O_RDONLY);
  }
  if (fd < 0) {
    printf("Failure attempting to open \"%s\"\n", filepath);
  }
  res = dwarf_init(fd, DW_DLC_READ, errhand, errarg, &dbg, &error);
  if (res != DW_DLV_OK) {
    printf("Giving up, cannot do DWARF processing\n");
    exit(1);
  }

  showBug(dbg);

  res = dwarf_finish(dbg, &error);
  if (res != DW_DLV_OK) {
    printf("dwarf_finish failed!\n");
  }
  close(fd);
  return 0;
}

static void showBug(Dwarf_Debug dbg) {
  Dwarf_Unsigned cu_header_length = 0;
  Dwarf_Half version_stamp = 0;
  Dwarf_Unsigned abbrev_offset = 0;
  Dwarf_Half address_size = 0;
  Dwarf_Unsigned next_cu_header = 0;
  Dwarf_Error error;
  int cu_number = 0;

  Dwarf_Die no_die = 0;
  Dwarf_Die cu_die = 0;
  int res = DW_DLV_ERROR;
  res = dwarf_next_cu_header(dbg, &cu_header_length, &version_stamp, &abbrev_offset, &address_size, &next_cu_header, &error);
  if (res == DW_DLV_ERROR) {
    printf("Error in dwarf_next_cu_header\n");
    exit(1);
  }

  printf("SUCCESS!\n");
  exit(0);
}

