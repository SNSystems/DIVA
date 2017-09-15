# LibRegex
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
    set(LibRegexDef "regex_debug.def")
else()
    set(LibRegexDef "regex.def")
endif()

set(version "${major_version},${minor_version},0,0")
set(version_str "\"${major_version}.${minor_version}.0.0\"")

create_target(LIB LibRegex
    SOURCE
        ${LibRegexDef}
        ${resource_file}
        "Src/Distribution/src/regex.c"
    HEADERS
        "${CMAKE_CURRENT_BINARY_DIR}/Src/config.h"
        "Src/Distribution/src/private.h"
        "Src/Distribution/src/re_comp.h"
        "Src/Distribution/src/regex.h"
        "Src/Distribution/src/regex_internal.h"
        "../DwarfDump/Src/stdafx.h"
    INCLUDE
        "${CMAKE_CURRENT_BINARY_DIR}/Src"
        "Src/Distribution"
        "Src/Distribution/src"
        "../DwarfDump/Src"
    DEFINE
        "-DHAVE_CONFIG_H"
        "-DREGEX_STATIC"
        "-DRC_COMPANYNAME_STR=\"${company_name}\""
        "-DRC_COPYYEAR_STR=\"${copyright_year}\""
        "-DRC_VERSION=${version}"
        "-DRC_VERSION_STR=${version_str}"
)

# add _debug postfix for debug builds.
set_target_properties(LibRegex PROPERTIES DEBUG_POSTFIX "_debug")
