# LibTsearch
include(GenerateConfig)
include(Version.cmake)

generate_config("Src/config.h.in" "${CMAKE_CURRENT_BINARY_DIR}/Src/config.h")

if(UNIX)
    set(resource_file "")
else()
    set(resource_file "src/resources/windows_version_resource.rc")
endif()

# determine which .def file to use depending on release or debug build.
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(LibTsearchDef "LibTsearch_debug.def")
else()
    set(LibTsearchDef "LibTsearch.def")
endif()

set(version "${major_version},${minor_version},0,0")
set(version_str "\"${major_version}.${minor_version}.0.0\"")

create_target(LIB LibTsearch
    SOURCE
        ${LibTsearchDef}
        ${resource_file}
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
        "-DRC_COMPANYNAME_STR=\"${company_name}\""
        "-DRC_COPYYEAR_STR=\"${copyright_year}\""
        "-DRC_VERSION=${version}"
        "-DRC_VERSION_STR=${version_str}"
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
