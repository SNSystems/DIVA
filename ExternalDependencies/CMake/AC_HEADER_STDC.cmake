# Adapted from https://github.com/ufz/tiff/blob/master/cmake/Modules/AutoconfHelper.cmake
# Replaces Autoconf's AC_HEADER_STDC macro
include(CheckCSourceCompiles)

function(check_stdc_headers variable)
    #TODO figure out how to stop this printing messages every time it runs
    message(STATUS "Looking for ANSI-C headers")
    set(code "
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <float.h>
int main(int argc, char **argv)
{
  void *ptr;
  free((void*)1);
  ptr = memchr((void*)1, 0, 0);
  return (int)ptr;
}
    ")
    # FIXME Check the ctype.h high bit
    CHECK_C_SOURCE_COMPILES("${code}" ${variable})
    if(${variable})
        set(${variable} 1 PARENT_SCOPE)
        message(STATUS "Looking for ANSI-C headers - found")
    else()
        message(STATUS "Looking for ANSI-C headers - not found")
    endif()
endfunction()
