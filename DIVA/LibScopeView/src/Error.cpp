//===-- LibScopeView/Error.cpp ----------------------------------*- C++ -*-===//
///
/// Copyright (c) 2017 by SN Systems Ltd., Sony Interactive Entertainment Inc.
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to
/// deal in the Software without restriction, including without limitation the
/// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
/// sell copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in
/// all copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
/// IN THE SOFTWARE.
///
//===----------------------------------------------------------------------===//
///
/// \file
/// Error handling for DIVA.
///
//===----------------------------------------------------------------------===//

#include "Error.h"

#include <assert.h>
#include <sstream>

using namespace LibScopeError;

// Error reporting for the 'LibScopeView' module.

namespace {

struct ErrorEntry {
  const char *Name;
  const char *Format;
};
const ErrorEntry ErrorTable[] = {
    // Parsing Options.
    {"ERR_CMD_UNKNOWN_ARG", "Unknown argument '%s'."},
    {"ERR_CMD_MISSING_VALUE", "Argument '%s' requires a value."},
    {"ERR_CMD_UNEXPECTED_VALUE",
     "Argument '%s' can not be given a value ('%s')."},
    {"ERR_CMD_INVALID_VALUE",
     "Argument '%s' was given the invalid value '%s'."},
    {"ERR_CMD_SHORTCUT_WITH_VALUE",
     "Shortcut arguments can not be given values '%s'."},
    {"ERR_CMD_INVALID_REGEX", "Invalid Regular Expression '%s'."},

    // Reading.
    {"ERR_READ_FAILED", "Failed to read '%s'."},

    // ElfDwarfReader.
    {"ERR_INVALID_DWARF", "Failed to read DWARF from '%s'."},

    // FileIO Error.
    {"ERR_FILEIO_GET_CWD", "Unable to get current working directory."},
    {"ERR_FILEIO_ABS_PATH", "Unable to find file or directory '%s'."},
    {"ERR_FILEIO_OPEN_FAILURE", "Unable to open file '%s'."},
    {"ERR_FILEIO_MAKE_DIR_FAILURE", "Unable to create directory '%s'."},

    // Internal Error.
    {"ERR_SPLIT_UNABLE_TO_OPEN_FILE",
     "Unable to open file '%s' for Logical View Split."},

    // Start up Error.
    {"ERR_FILE_NOT_FOUND", "Unable to open file '%s'."},
    {"ERR_INVALID_FILE",
     "Invalid input file '%s', please provide a file in a supported format."},
};
static_assert(sizeof(ErrorTable) / sizeof(ErrorEntry) ==
                  static_cast<size_t>(ErrorCode::ERR_LAST_CODE),
              "ErrorCode enum and ErrorTable out of sync");

const ErrorEntry &getEntry(const ErrorCode Code) {
  assert(Code != ErrorCode::ERR_LAST_CODE);
  return ErrorTable[static_cast<size_t>(Code)];
}

} // namespace

void LibScopeError::warning(const std::string &Msg) {
  fprintf(stderr, "\nWarning: %s\n", Msg.c_str());
  // Printing to stderr includes a flush on Linux but not Windows
  fflush(stderr);
}

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-nonliteral"
#pragma clang diagnostic ignored "-Wformat-security"
#endif
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-security"
#endif

void LibScopeError::fatalError(const ErrorCode Code) {
  fprintf(stderr, "\n%s: ", getEntry(Code).Name);
  fprintf(stderr, getEntry(Code).Format);
  fprintf(stderr, "\n");
  exit(1);
}
void LibScopeError::fatalError(const ErrorCode Code,
                               const std::string &Detail1) {
  fprintf(stderr, "\n%s: ", getEntry(Code).Name);
  fprintf(stderr, getEntry(Code).Format, Detail1.c_str());
  fprintf(stderr, "\n");
  exit(1);
}
void LibScopeError::fatalError(const ErrorCode Code, const std::string &Detail1,
                               const std::string &Detail2) {
  fprintf(stderr, "\n%s: ", getEntry(Code).Name);
  fprintf(stderr, getEntry(Code).Format, Detail1.c_str(), Detail2.c_str());
  fprintf(stderr, "\n");
  exit(1);
}

#ifdef __clang__
#pragma clang diagnostic pop
#endif
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
