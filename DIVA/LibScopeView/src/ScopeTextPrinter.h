//===-- LibScopeView/ScopeTextPrinter.h -------------------------*- C++ -*-===//
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
/// This file contains the declaration of the ScopeTextPrinter class.
///
//===----------------------------------------------------------------------===//

#ifndef SCOPEVIEW_SCOPETEXTPRINTER_H
#define SCOPEVIEW_SCOPETEXTPRINTER_H

#include "ScopePrinter.h"

#include <unordered_set>

namespace LibScopeView {

/// \brief A Scope printer that outputs in the text format.
class ScopeTextPrinter : public ScopePrinter {
public:
  ScopeTextPrinter(const PrintSettings &PrintingSettings, std::string InputFile,
                   uint8_t IndentSize = 2);

private:
  void initBeforePrint(const Object *Obj) override;

  const std::string &getFileExtension() override;
  const std::string &getHeader() override;

  void printImpl(const Object *Obj, std::ostream &OutputStream) override;
  void printObjectText(const Object *Obj, std::ostream &OutputStream);
  void printIndentedChildren(const Object *Obj);

  std::string HeaderText;
  const uint8_t IndentSize;
  size_t CurrentLevel = 0;
  size_t IndentLevel = 1;
  size_t CurrentFileIndex = 0;

  // Indent sizes calculated from the tree being printed.
  size_t LineNumberIndentSize = 0;
  size_t TagIndentSize = 0;
  size_t LevelNumberIndentSize = 0;
  size_t AttributesIndentSize = 0;
  size_t FollowingLineExtraIndent = 0;

  // Objects where the children match a tree filter.
  std::unordered_set<const Object *> ObjectsWithTreeFilteredChildren;
  // Set to true when the parent matched a tree filter.
  bool IgnoreFilters = false;
};

} // end namespace LibScopeView

#endif // SCOPEVIEW_SCOPETEXTPRINTER_H
