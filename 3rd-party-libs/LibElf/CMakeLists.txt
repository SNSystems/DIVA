# LibElf
include(GenerateConfig)

generate_config("Src/config.h.in" "${CMAKE_CURRENT_BINARY_DIR}/Src/config.h")
generate_config("Src/sys_elf.h.in" "${CMAKE_CURRENT_BINARY_DIR}/Src/sys_elf.h")

# determine which .def file to use depending on release or debug build.
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(LibElfDef "libelf_debug.def")
else()
    set(LibElfDef "libelf.def")
endif()

create_target(LIB LibElf
    SOURCE
        ${LibElfDef}
        "Src/Distribution/lib/32.fsize.c"
        "Src/Distribution/lib/32.getehdr.c"
        "Src/Distribution/lib/32.getphdr.c"
        "Src/Distribution/lib/32.getshdr.c"
        "Src/Distribution/lib/32.newehdr.c"
        "Src/Distribution/lib/32.newphdr.c"
        "Src/Distribution/lib/32.xlatetof.c"
        "Src/Distribution/lib/64.xlatetof.c"
        "Src/Distribution/lib/assert.c"
        "Src/Distribution/lib/begin.c"
        "Src/Distribution/lib/checksum.c"
        "Src/Distribution/lib/cntl.c"
        "Src/Distribution/lib/cook.c"
        "Src/Distribution/lib/data.c"
        "Src/Distribution/lib/end.c"
        "Src/Distribution/lib/errmsg.c"
        "Src/Distribution/lib/errno.c"
        "Src/Distribution/lib/fill.c"
        "Src/Distribution/lib/flag.c"
        "Src/Distribution/lib/gelfehdr.c"
        "Src/Distribution/lib/gelfphdr.c"
        "Src/Distribution/lib/gelfshdr.c"
        "Src/Distribution/lib/gelftrans.c"
        "Src/Distribution/lib/getarhdr.c"
        "Src/Distribution/lib/getaroff.c"
        "Src/Distribution/lib/getarsym.c"
        "Src/Distribution/lib/getbase.c"
        "Src/Distribution/lib/getdata.c"
        "Src/Distribution/lib/getident.c"
        "Src/Distribution/lib/getscn.c"
        "Src/Distribution/lib/hash.c"
        "Src/Distribution/lib/input.c"
        "Src/Distribution/lib/kind.c"
        "Src/Distribution/lib/memset.c"
        "Src/Distribution/lib/ndxscn.c"
        "Src/Distribution/lib/newdata.c"
        "Src/Distribution/lib/newscn.c"
        "Src/Distribution/lib/next.c"
        "Src/Distribution/lib/nextscn.c"
        "Src/Distribution/lib/nlist.c"
        "Src/Distribution/lib/opt.delscn.c"
        "Src/Distribution/lib/rand.c"
        "Src/Distribution/lib/rawdata.c"
        "Src/Distribution/lib/rawfile.c"
        "Src/Distribution/lib/strptr.c"
        "Src/Distribution/lib/swap64.c"
        "Src/Distribution/lib/update.c"
        "Src/Distribution/lib/verdef_32_tof.c"
        "Src/Distribution/lib/verdef_32_tom.c"
        "Src/Distribution/lib/verdef_64_tof.c"
        "Src/Distribution/lib/verdef_64_tom.c"
        "Src/Distribution/lib/version.c"
        "Src/Distribution/lib/x.elfext.c"
        "Src/Distribution/lib/x.movscn.c"
        "Src/Distribution/lib/x.remscn.c"
    HEADERS
        "${CMAKE_CURRENT_BINARY_DIR}/Src/config.h"
        "${CMAKE_CURRENT_BINARY_DIR}/Src/sys_elf.h"
        "Src/Distribution/lib/byteswap.h"
        "Src/Distribution/lib/elf_repl.h"
        "Src/Distribution/lib/errors.h"
        "Src/Distribution/lib/ext_types.h"
        "Src/Distribution/lib/gelf.h"
        "Src/Distribution/lib/libelf.h"
        "Src/Distribution/lib/nlist.h"
        "Src/Distribution/lib/private.h"
        "Src/Distribution/lib/verdef.h"
        "Src/Distribution/lib/verneed.h"
        "../DwarfDump/Src/stdafx.h"
    INCLUDE
        "${CMAKE_CURRENT_BINARY_DIR}/Src"
        "Src/Distribution/lib"
        "../DwarfDump/Src"
    DEFINE
        "-DHAVE_CONFIG_H"
)

# add _debug postfix for debug builds.
set_target_properties(LibElf PROPERTIES DEBUG_POSTFIX "_debug")

# Deploy
install(
    TARGETS LibElf
    RUNTIME DESTINATION "${deploy_dir}/Binaries"
    COMPONENT "dwarf_dump_deploy"
    LIBRARY DESTINATION "${deploy_dir}/Libraries"
    COMPONENT "dwarf_dump_deploy"
    ARCHIVE DESTINATION "${deploy_dir}/Libraries"
    COMPONENT "dwarf_dump_deploy"
)
install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/Src/config.h"
        "${CMAKE_CURRENT_BINARY_DIR}/Src/sys_elf.h"
        "Src/Distribution/lib/elf_repl.h"
        "Src/Distribution/lib/gelf.h"
        "Src/Distribution/lib/libelf.h"
    DESTINATION "${deploy_dir}/Includes/LibElf"
    COMPONENT "dwarf_dump_deploy"
)
