//===-- LibScopeView/PrintContext.cpp ---------------------------*- C++ -*-===//
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
/// Implementation for the PrintContext class.
///
//===----------------------------------------------------------------------===//

#include "PrintContext.h"
#include "FileUtilities.h"
#include "Platform.h"

#include <cstdarg>

using namespace LibScopeView;

std::unique_ptr<PrintContext> LibScopeView::GlobalPrintContext;

PrintContext::PrintContext()
    : File(nullptr), FileSave(nullptr), TheLocation(""), LocationDone(false) {}

PrintContext::PrintContext(FILE *context)
    : File(context), FileSave(nullptr), TheLocation(""), LocationDone(false) {}

PrintContext::~PrintContext() {}

void PrintContext::create(FILE *Context) {
  GlobalPrintContext = std::make_unique<PrintContext>(Context);
}

bool PrintContext::createLocation(const std::string &Location) {

  // The 'location' will represent the root directory for the output created
  // by the context. The best sample, is the different CUs files, that will
  // be extracted from a single ELF.
  TheLocation = unifyFilePath(Location);
  if (TheLocation.back() != '/') {
    TheLocation.append("/");
  }

  LocationDone = recursiveMakeDir(TheLocation);
  return LocationDone;
}

bool PrintContext::open(const std::string &FilePath) {
#ifdef PLATFORM_WIN
  FILE *OpenedFile;
  auto err = fopen_s(&OpenedFile, nativeFilePath(FilePath).c_str(), "w");
  if (err != 0)
    OpenedFile = nullptr;
#else
  FILE *OpenedFile = fopen(FilePath.c_str(), "w");
#endif

  if (OpenedFile) {
    // Preserve the current printing context.
    FileSave = File;
    File = OpenedFile;
  }
  return OpenedFile != nullptr;
}

void PrintContext::close() {
  if (File) {
    fclose(File);

    // Restore the printing context.
    File = FileSave;
  }
}

int PrintContext::print(const char *Fmt, ...) {
  va_list ap;
  va_start(ap, Fmt);

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-nonliteral"
#endif
  int result = vfprintf(File, Fmt, ap);
#ifdef __clang__
#pragma clang diagnostic pop
#endif

  va_end(ap);
  return result;
}
