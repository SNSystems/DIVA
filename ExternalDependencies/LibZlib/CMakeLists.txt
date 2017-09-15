# LibZlib
include(Version.cmake)

if(UNIX)
    set(resource_file "")
else()
    set(resource_file "src/resources/windows_version_resource.rc")
endif()

# determine which .def file to use depending on release or debug build.
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(zLibDef "zlib_debug.def")
else()
    set(zLibDef "zlib.def")
endif()

set(version "${major_version},${minor_version},0,0")
set(version_str "\"${major_version}.${minor_version}.0.0\"")

set(source_files
    ${zLibDef}
    "Src/Distribution/adler32.c"
    "Src/Distribution/compress.c"
    "Src/Distribution/crc32.c"
    "Src/Distribution/deflate.c"
    "Src/Distribution/gzclose.c"
    "Src/Distribution/gzlib.c"
    "Src/Distribution/gzread.c"
    "Src/Distribution/gzwrite.c"
    "Src/Distribution/infback.c"
    "Src/Distribution/inffast.c"
    "Src/Distribution/inflate.c"
    "Src/Distribution/inftrees.c"
    "Src/Distribution/trees.c"
    "Src/Distribution/uncompr.c"
    "Src/Distribution/zutil.c"
    "Src/Distribution/contrib/minizip/ioapi.c"
    "Src/Distribution/contrib/minizip/unzip.c"
    "Src/Distribution/contrib/minizip/zip.c"
)
if(WIN32)
    list(APPEND source_files "Src/Distribution/contrib/minizip/iowin32.c")
endif()

create_target(LIB LibZlib
    SOURCE
        "${source_files}"
        ${resource_file}
    HEADERS
        "Src/Distribution/crc32.h"
        "Src/Distribution/deflate.h"
        "Src/Distribution/gzguts.h"
        "Src/Distribution/inffast.h"
        "Src/Distribution/inffixed.h"
        "Src/Distribution/inflate.h"
        "Src/Distribution/inftrees.h"
        "Src/Distribution/trees.h"
        "Src/Distribution/zconf.h"
        "Src/Distribution/zlib.h"
        "Src/Distribution/zutil.h"
        "../DwarfDump/Src/stdafx.h"
    INCLUDE
        "Src/Distribution"
        "../DwarfDump/Src"
    DEFINE
        "-DRC_COMPANYNAME_STR=\"${company_name}\""
        "-DRC_COPYYEAR_STR=\"${copyright_year}\""
        "-DRC_VERSION=${version}"
        "-DRC_VERSION_STR=${version_str}"
)

# add _debug postfix for debug builds.
set_target_properties(LibZlib PROPERTIES DEBUG_POSTFIX "_debug")

# Deploy
install(
    TARGETS LibZlib
    RUNTIME DESTINATION "${deploy_dir}/Binaries"
    COMPONENT "dwarf_dump_deploy"
    LIBRARY DESTINATION "${deploy_dir}/Libraries"
    COMPONENT "dwarf_dump_deploy"
    ARCHIVE DESTINATION "${deploy_dir}/Libraries"
    COMPONENT "dwarf_dump_deploy"
)
install(
    FILES
        "Src/Distribution/zconf.h"
        "Src/Distribution/zlib.h"
    DESTINATION "${deploy_dir}/Includes/LibZlib"
    COMPONENT "dwarf_dump_deploy"
)
