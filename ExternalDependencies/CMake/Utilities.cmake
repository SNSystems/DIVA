include(CMakeParseArguments)

# Allow grouping projects into folders in VisualStudio
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

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

    # TODO: This should be at the top level but that didn't work
    set(CMAKE_CXX_STANDARD "11")
    set(CMAKE_CXX_STANDARD_REQUIRED "ON")

    # Create target
    if(${project_type} STREQUAL "LIB")
        add_library(${project_name} SHARED ${ARG_SOURCE} ${ARG_HEADERS})
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

# create_code_generator(<name>
#                       GENERATE <out1> [<out2> ...]
#                       COMMANDS <command1> [<command2> ...]
#                       [DEPENDS [<dependency1> ...]]
# )
# The commands should follow the rules for add_custom_command, including being
# prefixed by COMMAND
function(create_code_generator generator_name)
    # Parse Args
    set(arg_sections "GENERATE" "COMMANDS" "DEPENDS")
    parse_all_arguments("ARG" "" "" "${arg_sections}" ${ARGN})
    if(NOT ARG_GENERATE)
        message(FATAL_ERROR "No GENERATE files passed to create_code_generator")
    elseif(NOT ARG_COMMANDS)
        message(FATAL_ERROR "No COMMANDS passed to create_code_generator")
    endif()
    # Get full paths
    set(full_generate_paths "")
    foreach(path ${ARG_GENERATE})
        list(APPEND full_generate_paths "${CMAKE_CURRENT_SOURCE_DIR}/${path}")
    endforeach()
    # Add the custom step
    add_custom_command(
        OUTPUT ${full_generate_paths}
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        ${ARG_COMMANDS}
        DEPENDS ${ARG_DEPENDS}
    )
    add_custom_target(
        ${generator_name}
        DEPENDS ${full_generate_paths}
    )
    set_target_properties(${generator_name} PROPERTIES FOLDER Generate)
    # Set the files as generated
    set_source_files_properties(${ARG_GENERATE} PROPERTIES GENERATED TRUE)
endfunction()
