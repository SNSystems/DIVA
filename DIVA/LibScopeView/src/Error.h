//===-- LibScopeView/Error.h ------------------------------------*- C++ -*-===//
///
/// Copyright (c) Sony Interactive Entertainment Inc.
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

#include <string>

#ifndef ERROR_H
#define ERROR_H

namespace LibScopeError {

/// \brief Error Codes.
enum class ErrorCode {
  // Parsing Options.
  ERR_CMD_UNKNOWN_ARG = 0,
  ERR_CMD_MISSING_VALUE,
  ERR_CMD_UNEXPECTED_VALUE,
  ERR_CMD_INVALID_VALUE,
  ERR_CMD_SHORTCUT_WITH_VALUE,
  ERR_CMD_INVALID_REGEX,

  // Reading.
  ERR_READ_FAILED,

  // ElfDwarfReader.
  ERR_INVALID_DWARF,

  // FileIO Error.
  ERR_FILEIO_GET_CWD,
  ERR_FILEIO_ABS_PATH,
  ERR_FILEIO_OPEN_FAILURE,
  ERR_FILEIO_MAKE_DIR_FAILURE,

  // Internal Error.
  ERR_SPLIT_UNABLE_TO_OPEN_FILE,

  // Start up Error.
  ERR_FILE_NOT_FOUND,
  ERR_INVALID_FILE,

  // Last Error.
  ERR_LAST_CODE
};

/// \brief Display a warning message.
void warning(const std::string &Msg);

/// \brief Display a fatal error and exit.
[[noreturn]] void fatalError(const ErrorCode Code);
[[noreturn]] void fatalError(const ErrorCode Code, const std::string &Detail1);
[[noreturn]] void fatalError(const ErrorCode Code, const std::string &Detail1,
                             const std::string &Detail2);

} // namespace LibScopeError

#endif // ERROR_H_H
