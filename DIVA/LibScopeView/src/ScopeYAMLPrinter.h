//===-- LibScopeView/ScopeYAMLPrinter.h -------------------------*- C++ -*-===//
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
/// This file contains the declaration of the ScopeYAMLPrinter class.
///
//===----------------------------------------------------------------------===//

#ifndef SCOPEVIEW_SCOPEYAMLPRINTER_H
#define SCOPEVIEW_SCOPEYAMLPRINTER_H

#include "ScopePrinter.h"

namespace LibScopeView {

/// \brief A Scope printer that outputs in the YAML format.
class ScopeYAMLPrinter : public ScopePrinter {
public:
  explicit ScopeYAMLPrinter(std::string InputFile, std::string Version,
                            uint8_t SizeOfIndent = 2);

private:
  const std::string &getFileExtension() override;
  const std::string &getHeader() override;
  void printImpl(const Object *Obj, std::ostream &OutputStream) override;

  std::string YAMLHeader;
  const uint8_t IndentSize;
  uint32_t IndentLevel;
};

} // end namespace LibScopeView

#endif // SCOPEVIEW_SCOPEYAMLPRINTER_H
