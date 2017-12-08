//===-- LibScopeView/ScopePrinter.cpp ----------------------------- C++ -*-===//
///
/// Copyright (c) 2017 by Sony Interactive Entertainment Inc.
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
/// This file contains the definitions of ScopePrinter's methods.
///
//===----------------------------------------------------------------------===//

#include "ScopePrinter.h"
#include "Error.h"
#include "FileUtilities.h"
#include "Scope.h"

#include <assert.h>
#include <fstream>

using namespace LibScopeView;

namespace {
// Default return for getHeader and getFooter.
std::string EmptyString;
} // namespace

void ScopePrinter::print(const Object *Obj, std::ostream &Output) {
  initBeforePrint(Obj);
  printSingleOutput(Obj, Output);
}

void ScopePrinter::print(const ScopeRoot *Root, const std::string &OutputDir) {
  if (Root->getChildren().size() == 0)
    return;

  // Add a trailing seperator to OutputDir.
  std::string SplitOutputDir(unifyFilePath(OutputDir));
  if (SplitOutputDir.back() != '/') {
    SplitOutputDir += '/';
  }
  // Create the directory.
  if (!recursiveMakeDir(SplitOutputDir)) {
    fatalError(LibScopeError::ErrorCode::ERR_FILEIO_MAKE_DIR_FAILURE,
               SplitOutputDir);
  }
  // Print each compile unit
  initBeforePrint(Root);
  for (const auto *CU : Root->getChildren()) {
    if (CU->getIsCompileUnit()) {
      // Open an output file for each CU.
      std::string OutputPath(SplitOutputDir);
      OutputPath += flattenFilePath(CU->getName());
      OutputPath += ".";
      OutputPath += getFileExtension();

      std::ofstream SplitOutputFile(nativeFilePath(OutputPath));
      if (SplitOutputFile.fail())
        fatalError(LibScopeError::ErrorCode::ERR_SPLIT_UNABLE_TO_OPEN_FILE,
                   OutputPath);
      printSingleOutput(CU, SplitOutputFile);
    }
  }
}

const std::string &ScopePrinter::getHeader() { return EmptyString; }

const std::string &ScopePrinter::getFooter() { return EmptyString; }

void ScopePrinter::printSingleOutput(const Object *Obj, std::ostream &Output) {
  OutputStream = &Output;
  *OutputStream << getHeader();
  visit(Obj);
  *OutputStream << getFooter();
}

void ScopePrinter::visitImpl(const Object *Obj) {
  assert(OutputStream && "ScopePrinter methods calling ScopePrinter::visit "
                         "should set OutputStream first");
  printImpl(Obj, *OutputStream);
}
