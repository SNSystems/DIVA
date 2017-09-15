cmake_minimum_required(VERSION 3.2.2)
get_filename_component(CMAKE_MODULE_PATH "CMake" ABSOLUTE)

project(DwarfValidation)

include(Utilities)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
# set the runtime path for linux to be the origin of the executable.
set(CMAKE_INSTALL_RPATH "$ORIGIN/../Libraries:$ORIGIN/")

if(NOT deploy_dir)
    set(deploy_dir "${CMAKE_BINARY_DIR}/Deploy")
endif()

add_subdirectory(dicheck)
add_subdirectory(LibRegex)
add_subdirectory(LibTsearch)
add_subdirectory(LibZlib)
add_subdirectory(LibElf)
add_subdirectory(LibDwarf)
