// stdafx.h : include file for standard system include files
// Use only one stdafx.h to build libelf libdwarf and dwarfdump

#pragma once

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

// Windows Header Files:
//#include <tchar.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <fcntl.h>
#include <direct.h>
#include <sys/types.h>
#include <sys/stat.h>
//#include <windows.h>

/* Visual Studio Versions
MSVC++ 12.0 _MSC_VER == 1800 (Visual Studio 2013)
MSVC++ 11.0 _MSC_VER == 1700 (Visual Studio 2012)
MSVC++ 10.0 _MSC_VER == 1600 (Visual Studio 2010)
MSVC++ 9.0  _MSC_VER == 1500 (Visual Studio 2008)
MSVC++ 8.0  _MSC_VER == 1400 (Visual Studio 2005)
MSVC++ 7.1  _MSC_VER == 1310 (Visual Studio 2003)
MSVC++ 7.0  _MSC_VER == 1300
MSVC++ 6.0  _MSC_VER == 1200
MSVC++ 5.0  _MSC_VER == 1100
*/

#if _MSC_VER >= 1600
#include <stdint.h>
#endif /* _MSC_VER */

/* Visual Studio versions up to 2012, do not support va_copy */
#if _MSC_VER <= 1700
#define va_copy(dest,src) (dest = src)
#endif /* _MSC_VER */

// Reference additional headers your program requires here
#include <malloc.h>

#include <windows.h>

/* SN-Carlos: Disable some warnings */
#pragma warning(disable:4013)
#pragma warning(disable:4018)
#pragma warning(disable:4101)
#pragma warning(disable:4146)
#pragma warning(disable:4244)
#pragma warning(disable:4267)
#pragma warning(disable:4311)
#pragma warning(disable:4996)   /* Warning when migrated to VS2010 */
#pragma warning(disable:4800)   /* DwarfGen Sample and C++*/
#pragma warning(disable:4267)   /* Warning when migrated to VS2010 */

#define snprintf _snprintf
#define snprintf _snprintf
#define vsnprintf _vsnprintf
#define strcasecmp _stricmp
#define strncasecmp _strnicmp

/* SN-Carlos: Windows specific */
#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

#undef FAILED
#define FAILED 1

#ifdef HAVE_REGEX
#include <regex.h>
#endif /* HAVE_REGEX */

/* For C++ do not defined this macro */
#ifdef __cplusplus
/*  class 'std::basic_string<_Elem,_Traits,_Ax>' needs to have
    dll-interface to be used by clients of class '<class>' */
#pragma warning(disable: 4251)
/*  Function call with parameters that may be unsafe - this call relies
    on the caller to check that the passed values are correct. */
/*  Include only algorithms and data structures */
#pragma warning(disable: 4996)
#include <vector>
#include <stack>
#include <map>
#include <set>
#include <list>
#include <algorithm>
#include <bitset>
#include <regex>
#else
typedef int boolean;
#endif /* __cplusplus */

#ifndef MAX_PATH
#define MAX_PATH          260
#endif /* MAX_PATH */

/* SN-Carlos: Enable asserts in the libelf library */
#undef ENABLE_DEBUG
#ifdef _DEBUG
  #define ENABLE_DEBUG 1
#endif /* _DEBUG */

/* SN-Carlos: Enable malloc checks in the libdwarf library */
#undef WANT_LIBBDWARF_MALLOC_CHECK
#ifdef _DEBUG
  //#define WANT_LIBBDWARF_MALLOC_CHECK 1
#endif /* _DEBUG */

#ifndef S_ISREG
#define S_ISREG(A) (1)
#endif /* S_ISREG */

/* When import or exporting symbols */
#define __STDCALL         __stdcall
#define __CDECL           __cdecl
#define __DECLSPEC_EXPORT __declspec(dllexport)
#define __DECLSPEC_IMPORT __declspec(dllimport)

/* Common typedefs */
typedef unsigned char     BYTE;
typedef int               INT;
typedef unsigned int      UINT;

typedef unsigned char     UINT8;
typedef unsigned short    UINT16;
typedef unsigned int      UINT32;
typedef unsigned __int64  UINT64;

typedef signed char       INT8;
typedef signed short      INT16;
typedef signed int        INT32;
typedef signed __int64    INT64;

#if defined(_MSC_VER)
#define strtoll  _strtoi64
#define strtoull _strtoui64
#endif /* _MSC_VER */

#if __MSC__
typedef unsigned char a_bit_field;
#else /* !__MSC__ */
typedef unsigned int a_bit_field;
#endif /* __MSC__ */
