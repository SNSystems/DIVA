# Diva
include(Version.cmake)

set(base_lib_dir "../ExternalDependencies/DwarfDump/Libraries")

if (STATIC_DWARF_LIBS)
    set(static_lib_dir "${base_lib_dir}/${platform_name}_${architecture_name}_static")
    set(static_libs "LibDwarf" "LibElf" "LibTsearch" "LibZlib")
    link_directories("${static_lib_dir}")
else()
    set(debug_lib_dir "${base_lib_dir}/${platform_name}_${architecture_name}_debug")
    set(lib_dir "${base_lib_dir}/${platform_name}_${architecture_name}")
    link_directories("${lib_dir}" "${debug_lib_dir}")
endif()

if(WIN32)
    set(platform_link_args "Psapi")
    set(resource_file "src/resources/windows_version_resource.rc")
else()
    set(platform_link_args
        "-static-libstdc++"
        # Required for C++11 support
        "-pthread"
    )
    set(resource_file "")
endif()

set(diva_version "${diva_major_version},${diva_minor_version},0,0")
set(diva_version_str "\"${diva_major_version}.${diva_minor_version}.0.0\"")


create_target(EXE Diva
    OUTPUT_NAME
        "diva"
    SOURCE
        "src/ArgumentParser.cpp"
        "src/DivaOptions.cpp"
        "src/main.cpp"
    HEADERS
        "src/ArgumentParser.h"
        "src/DivaOptions.h"
        "${resource_file}"
    INCLUDE
        "${CMAKE_CURRENT_BINARY_DIR}/Src"
        "../ElfDwarfReader/src"
        "../LibScopeView/src"
        "../ExternalDependencies/ya_getopt/"
        "../ExternalDependencies/DwarfDump/Includes/LibDwarf"
    LINK
        "ElfDwarfReader"
        "LibScopeView"
        "${static_libs}"
        "${platform_link_args}"
    DEFINE
        "-D__DIVA__"
        "-DRC_VERSION=${diva_version}"
        "-DRC_VERSION_STR=${diva_version_str}"
        "-DRC_COMPANYNAME_STR=\"${company_name}\""
        "-DRC_COPYYEAR_STR=\"${copyright_year}\""
        "-DYAML_OUTPUT_VERSION_STR=\"${yaml_output_version}\""
)

if (NOT STATIC_DWARF_LIBS)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(SUFFIX "_debug")
    elseif()
        set(SUFFIX "")
    endif()

    target_link_libraries(Diva optimized "LibDwarf")
    target_link_libraries(Diva optimized "LibElf")
    target_link_libraries(Diva optimized "LibTsearch")
    target_link_libraries(Diva optimized "LibZlib")
    target_link_libraries(Diva debug "LibDwarf_debug")
    target_link_libraries(Diva debug "LibElf_debug")
    target_link_libraries(Diva debug "LibTsearch_debug")
    target_link_libraries(Diva debug "LibZlib_debug")
endif()

if (UNIX AND NOT STATIC_DWARF_LIBS) # post build step for linux
    # set the runtime path for linux to be the origin of the executable.
    SET_TARGET_PROPERTIES(Diva PROPERTIES INSTALL_RPATH "$ORIGIN/../lib:$ORIGIN/")

    set(SODIR ${PROJECT_SOURCE_DIR}/ExternalDependencies/DwarfDump/Libraries/linux_x64${SUFFIX})

elseif (WIN32 AND NOT STATIC_DWARF_LIBS) # post build copy step for windows
    # determine which set of DwarfDump DLLs to use on window MSVC builds.
    if (is_64_bit STREQUAL "TRUE")
        set(DLLDIR ${PROJECT_SOURCE_DIR}/ExternalDependencies/DwarfDump/Binaries/win_x64)
        set(DLLDIR_DBG ${PROJECT_SOURCE_DIR}/ExternalDependencies/DwarfDump/Binaries/win_x64_debug)
    else()
        set(DLLDIR ${PROJECT_SOURCE_DIR}/ExternalDependencies/DwarfDump/Binaries/win_x86)
        set(DLLDIR_DBG ${PROJECT_SOURCE_DIR}/ExternalDependencies/DwarfDump/Binaries/win_x86_debug)
    endif()

    copy_dir_post_build_config_aware(
        Diva
        Debug
        ${DLLDIR_DBG}
        $(TargetDir)
    )
    copy_dir_post_build_config_aware(
        Diva
        Release
        ${DLLDIR}
        $(TargetDir)
    )
endif()

# Deploy
if(NOT deploy_dir)
    set(deploy_dir "${CMAKE_BINARY_DIR}/Deploy")
endif()
install(
    TARGETS Diva
    RUNTIME DESTINATION "${deploy_dir}/bin"
    COMPONENT "diva_deploy"
)
install(
    FILES "../Documentation/user_guide.md"
    DESTINATION "${deploy_dir}/doc"
    COMPONENT "diva_deploy"
)
file(GLOB example_files "../Examples/*")
install(
    FILES ${example_files}
    DESTINATION "${deploy_dir}/examples"
    COMPONENT "diva_deploy"
)
file(GLOB licenses "../Licenses/*")
install(
    FILES ${licenses}
    DESTINATION "${deploy_dir}/licenses"
    COMPONENT "diva_deploy"
)
install(
    FILES "../LICENSE.txt"
    DESTINATION "${deploy_dir}/"
    COMPONENT "diva_deploy"
)


# Create custom install target for the diva_deploy component
add_custom_target(DEPLOY
    DEPENDS Diva
    COMMAND
        "${CMAKE_COMMAND}" -DCMAKE_INSTALL_COMPONENT=diva_deploy
        "-DBUILD_TYPE=$<CONFIG>" -P "${CMAKE_BINARY_DIR}/cmake_install.cmake"
)


if (UNIX AND NOT STATIC_DWARF_LIBS)
    # Deploy step for linux .so files
    install(
    FILES
        "${SODIR}/libLibDwarf${SUFFIX}.so"
        "${SODIR}/libLibElf${SUFFIX}.so"
        "${SODIR}/libLibZlib${SUFFIX}.so"
        "${SODIR}/libLibTsearch${SUFFIX}.so"
    DESTINATION
        "${deploy_dir}/bin"
    COMPONENT
        "diva_deploy")

elseif (WIN32 AND NOT STATIC_DWARF_LIBS)
    copy_dir_post_build_config_aware(
        DEPLOY
        Debug
        ${DLLDIR_DBG}
        ${deploy_dir}/bin
    )
    copy_dir_post_build_config_aware(
        DEPLOY
        Release
        ${DLLDIR}
        ${deploy_dir}/bin
    )
endif()
