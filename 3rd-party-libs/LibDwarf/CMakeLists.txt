# Generate LibDwarf config.h so we can use it for GetNames as well
include(GenerateConfig)

generate_config("Src/config.h.in" "${CMAKE_CURRENT_BINARY_DIR}/Src/config.h")
generate_config(
    "../LibElf/Src/sys_elf.h.in" "${CMAKE_CURRENT_BINARY_DIR}/Src/sys_elf.h")

# GetNames
create_target(EXE GetNames
    OUTPUT_NAME
        "getnames"
    SOURCE
        "Src/Distribution/gennames.c"
        "../DwarfDump/Src/Distribution/common.c"
        "Src/Distribution/dwgetopt.c"
    HEADERS
        "${CMAKE_CURRENT_BINARY_DIR}/Src/config.h"
        "../DwarfDump/Src/Distribution/common.h"
        "Src/Distribution/dwgetopt.h"
        "../DwarfDump/Src/getopt/tailor.h"
        "../DwarfDump/Src/stdafx.h"
    INCLUDE
        "${CMAKE_CURRENT_BINARY_DIR}/Src"
        "../DwarfDump/Src/Distribution"
        "Src/Distribution"
        "../LibRegex/Src"
        "../DwarfDump/Src"
)

create_code_generator(GENERATE_NAMES
    GENERATE
        "Src/Distribution/dwarf_names.c"
        "Src/Distribution/dwarf_names.h"
        "Src/Distribution/dwarf_names_enum.h"
        "Src/Distribution/dwarf_names_new.h"
    COMMANDS
        COMMAND GetNames -i Src/Distribution -o Src/Distribution
    DEPENDS
        GetNames
)

# LibDwarf
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(LibDwarfDef "LibDwarf_debug.def")
else()
    set(LibDwarfDef "LibDwarf.def")
endif()

create_target(LIB LibDwarf
    SOURCE
        ${LibDwarfDef}
        "Src/Distribution/dwarf_abbrev.c"
        "Src/Distribution/dwarf_alloc.c"
        "Src/Distribution/dwarf_arange.c"
        "Src/Distribution/dwarf_die_deliv.c"
        "Src/Distribution/dwarf_dsc.c"
        "Src/Distribution/dwarf_elf_access.c"
        "Src/Distribution/dwarf_error.c"
        "Src/Distribution/dwarf_form.c"
        "Src/Distribution/dwarf_frame.c"
        "Src/Distribution/dwarf_frame2.c"
        "Src/Distribution/dwarf_frame3.c"
        "Src/Distribution/dwarf_funcs.c"
        "Src/Distribution/dwarf_gdbindex.c"
        "Src/Distribution/dwarf_global.c"
        "Src/Distribution/dwarf_harmless.c"
        "Src/Distribution/dwarf_init_finish.c"
        "Src/Distribution/dwarf_leb.c"
        "Src/Distribution/dwarf_line.c"
        "Src/Distribution/dwarf_line2.c"
        "Src/Distribution/dwarf_loc.c"
        "Src/Distribution/dwarf_macro.c"
        "Src/Distribution/dwarf_macro5.c"
        "Src/Distribution/dwarf_names.c"
        "Src/Distribution/dwarf_original_elf_init.c"
        "Src/Distribution/dwarf_print_lines.c"
        "Src/Distribution/dwarf_pubtypes.c"
        "Src/Distribution/dwarf_query.c"
        "Src/Distribution/dwarf_ranges.c"
        "Src/Distribution/dwarf_sort_line.c"
        "Src/Distribution/dwarf_string.c"
        "Src/Distribution/dwarf_stubs.c"
        "Src/Distribution/dwarf_tied.c"
        "Src/Distribution/dwarf_types.c"
        "Src/Distribution/dwarf_util.c"
        "Src/Distribution/dwarf_vars.c"
        "Src/Distribution/dwarf_weaks.c"
        "Src/Distribution/dwarf_xu_index.c"
        "Src/Distribution/malloc_check.c"
        "Src/Distribution/pro_alloc.c"
        "Src/Distribution/pro_arange.c"
        "Src/Distribution/pro_die.c"
        "Src/Distribution/pro_encode_nm.c"
        "Src/Distribution/pro_error.c"
        "Src/Distribution/pro_expr.c"
        "Src/Distribution/pro_finish.c"
        "Src/Distribution/pro_forms.c"
        "Src/Distribution/pro_frame.c"
        "Src/Distribution/pro_funcs.c"
        "Src/Distribution/pro_init.c"
        "Src/Distribution/pro_line.c"
        "Src/Distribution/pro_macinfo.c"
        "Src/Distribution/pro_pubnames.c"
        "Src/Distribution/pro_reloc.c"
        "Src/Distribution/pro_reloc_stream.c"
        "Src/Distribution/pro_reloc_symbolic.c"
        "Src/Distribution/pro_section.c"
        "Src/Distribution/pro_types.c"
        "Src/Distribution/pro_vars.c"
        "Src/Distribution/pro_weaks.c"
    HEADERS
        "${CMAKE_CURRENT_BINARY_DIR}/Src/config.h"
        "${CMAKE_CURRENT_BINARY_DIR}/Src/sys_elf.h"
        "Src/Distribution/dwarf.h"
        "Src/Distribution/dwarf_abbrev.h"
        "Src/Distribution/dwarf_alloc.h"
        "Src/Distribution/dwarf_arange.h"
        "Src/Distribution/dwarf_base_types.h"
        "Src/Distribution/dwarf_die_deliv.h"
        "Src/Distribution/dwarf_dsc.h"
        "Src/Distribution/dwarf_elf_access.h"
        "Src/Distribution/dwarf_error.h"
        "Src/Distribution/dwarf_frame.h"
        "Src/Distribution/dwarf_funcs.h"
        "Src/Distribution/dwarf_gdbindex.h"
        "Src/Distribution/dwarf_global.h"
        "Src/Distribution/dwarf_harmless.h"
        "Src/Distribution/dwarf_incl.h"
        "Src/Distribution/dwarf_line.h"
        "Src/Distribution/dwarf_loc.h"
        "Src/Distribution/dwarf_macro.h"
        "Src/Distribution/dwarf_macro5.h"
        "Src/Distribution/dwarf_names.h"
        "Src/Distribution/dwarf_names_enum.h"
        "Src/Distribution/dwarf_names_new.h"
        "Src/Distribution/dwarf_opaque.h"
        "Src/Distribution/dwarf_reloc_arm.h"
        "Src/Distribution/dwarf_reloc_mips.h"
        "Src/Distribution/dwarf_reloc_ppc.h"
        "Src/Distribution/dwarf_reloc_ppc64.h"
        "Src/Distribution/dwarf_reloc_x86_64.h"
        "Src/Distribution/dwarf_types.h"
        "Src/Distribution/dwarf_util.h"
        "Src/Distribution/dwarf_vars.h"
        "Src/Distribution/dwarf_weaks.h"
        "Src/Distribution/dwarf_xu_index.h"
        "Src/Distribution/libdwarf.h"
        "Src/Distribution/libdwarfdefs.h"
        "Src/Distribution/malloc_check.h"
        "Src/Distribution/pro_alloc.h"
        "Src/Distribution/pro_arange.h"
        "Src/Distribution/pro_die.h"
        "Src/Distribution/pro_encode_nm.h"
        "Src/Distribution/pro_error.h"
        "Src/Distribution/pro_expr.h"
        "Src/Distribution/pro_frame.h"
        "Src/Distribution/pro_incl.h"
        "Src/Distribution/pro_line.h"
        "Src/Distribution/pro_macinfo.h"
        "Src/Distribution/pro_opaque.h"
        "Src/Distribution/pro_reloc.h"
        "Src/Distribution/pro_reloc_stream.h"
        "Src/Distribution/pro_reloc_symbolic.h"
        "Src/Distribution/pro_section.h"
        "Src/Distribution/pro_types.h"
        "Src/Distribution/pro_util.h"
    INCLUDE
        "${CMAKE_CURRENT_BINARY_DIR}/Src"
        "Src/Distribution"
        "../LibElf/Src/Distribution/lib"
        "../LibRegex/Src"
        "../LibTsearch/Src/Distribution"
        "../LibZlib/Src/Distribution"
        "../DwarfDump/Src"
    LINK
        "LibElf"
        "LibTsearch"
        "LibZlib"
    DEFINE
        "-DHAVE_CONFIG_H"
        "-DREGEX_STATIC"
    DEPENDS
        GENERATE_NAMES
)

# add _debug postfix for debug builds.
set_target_properties(LibDwarf PROPERTIES DEBUG_POSTFIX "_debug")

# Deploy
install(
    TARGETS LibDwarf
    RUNTIME DESTINATION "${deploy_dir}/Binaries"
    COMPONENT "dwarf_dump_deploy"
    LIBRARY DESTINATION "${deploy_dir}/Libraries"
    COMPONENT "dwarf_dump_deploy"
    ARCHIVE DESTINATION "${deploy_dir}/Libraries"
    COMPONENT "dwarf_dump_deploy"
)

install(
    FILES
        "Src/Distribution/dwarf.h"
        "Src/Distribution/dwarf_names.h"
        "Src/Distribution/libdwarf.h"
    DESTINATION "${deploy_dir}/Includes/LibDwarf"
    COMPONENT "dwarf_dump_deploy"
)
