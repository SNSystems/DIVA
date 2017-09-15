include(CMakeParseArguments)

# Allow grouping projects into folders in VisualStudio
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Platform/Architecture checks
if(WIN32)
    set(platform_name "win")
else()
    set(platform_name "linux")
endif()
if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
    set(is_64_bit "TRUE")
    set(architecture_name "x64")
else()
    set(is_64_bit "FALSE")
    set(architecture_name "x86")
endif()

# Wrapper for cmake_parse_arguments that throws an error for extra unexpected
# arguments
macro(parse_all_arguments prefix opt one_val multi_val)
    cmake_parse_arguments("${prefix}" "${opt}" "${one_val}" "${multi_val}" ${ARGN})
    if("${${prefix}_UNPARSED_ARGUMENTS}")
        list(GET "${prefix}_UNPARSED_ARGUMENTS" 0 unparsed)
        message(FATAL_ERROR "Unexpected argument: ${unparsed}")
    endif()
endmacro()

# create_target([LIB | EXE] <name>
#               [OUTPUT_NAME <output_name>]
#               [SOURCE [<source1> ...]]
#               [HEADERS [<header1> ...]]
#               [INCLUDE [<include1> ...]]
#               [DEFINE [<definition1> ...]]
#               [LINK [<library1> ...]]
#               [DEPENDS [<dependency1> ...]]
# )
function(create_target project_type project_name)
    # Parse Args
    set(one_val_args "OUTPUT_NAME")
    set(multi_val_args "SOURCE" "HEADERS" "INCLUDE" "DEFINE" "LINK" "DEPENDS")
    parse_all_arguments("ARG" "" "${one_val_args}" "${multi_val_args}" ${ARGN})

    # Create target
    if(${project_type} STREQUAL "LIB")
        add_library(${project_name} STATIC ${ARG_SOURCE} ${ARG_HEADERS})
    elseif(${project_type} STREQUAL "EXE")
        add_executable(${project_name} ${ARG_SOURCE} ${ARG_HEADERS})
    else()
        message(FATAL_ERROR "The first argument to create_target must be EXE or LIB")
    endif()
    if(ARG_OUTPUT_NAME)
        set_target_properties(${project_name} PROPERTIES OUTPUT_NAME
                              ${ARG_OUTPUT_NAME})
    endif()
    target_include_directories(${project_name} PRIVATE ${ARG_INCLUDE})
    target_compile_definitions(${project_name} PRIVATE ${ARG_DEFINE})
    target_link_libraries(${project_name} ${ARG_LINK})
    foreach(dep ${ARG_DEPENDS})
        add_dependencies(${project_name} ${dep})
    endforeach()
endfunction()

# copy_files_post_build(<target> <from_dir> <to_dir> <file1> [<file2> ...])
function(copy_files_post_build target from_dir to_dir)
    # Create commands
    set(commands "")
    foreach(file_to_copy ${ARGN})
        list(APPEND commands COMMAND ${CMAKE_COMMAND} -E copy_if_different
             "${from_dir}/${file_to_copy}"
             "${to_dir}")
    endforeach()
    # Add post build step
    add_custom_command(TARGET "${target}" POST_BUILD ${commands})
endfunction()

# copy_dir_post_build_config_aware <target> <config> <from_dir> <to_dir>
# Only does the copy if the current configuration matches config
function(copy_dir_post_build_config_aware target config from_dir to_dir)
    add_custom_command(
        TARGET ${target}
        POST_BUILD
        COMMAND if $(ConfigurationName) == ${config} "${CMAKE_COMMAND}" -E copy_directory "${from_dir}" "${to_dir}")
endfunction()
