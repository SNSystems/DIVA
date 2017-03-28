# LibTsearch
include(GenerateConfig)

generate_config("Src/config.h.in" "${CMAKE_CURRENT_BINARY_DIR}/Src/config.h")

# determine which .def file to use depending on release or debug build.
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(LibTsearchDef "LibTsearch_debug.def")
else()
    set(LibTsearchDef "LibTsearch.def")
endif()

create_target(LIB LibTsearch
    SOURCE
        ${LibTsearchDef}
        "Src/Distribution/main_tsearch.c"
    HEADERS
        "${CMAKE_CURRENT_BINARY_DIR}/Src/config.h"
        "Src/Distribution/dwarf_incl.h"
        "Src/Distribution/dwarf_tsearch.h"
        "../DwarfDump/Src/stdafx.h"
    INCLUDE
        "${CMAKE_CURRENT_BINARY_DIR}/Src"
        "Src/Distribution"
        "../DwarfDump/Src"
    DEFINE
        "-DHAVE_CONFIG_H"
)

# add _debug postfix for debug builds.
set_target_properties(LibTsearch PROPERTIES DEBUG_POSTFIX "_debug")

# Deploy
install(
    TARGETS LibTsearch
    RUNTIME DESTINATION "${deploy_dir}/Binaries"
    COMPONENT "dwarf_dump_deploy"
    LIBRARY DESTINATION "${deploy_dir}/Libraries"
    COMPONENT "dwarf_dump_deploy"
    ARCHIVE DESTINATION "${deploy_dir}/Libraries"
    COMPONENT "dwarf_dump_deploy"
)
install(
    FILES "Src/Distribution/dwarf_tsearch.h"
    DESTINATION "${deploy_dir}/Includes/LibTsearch"
    COMPONENT "dwarf_dump_deploy"
)
