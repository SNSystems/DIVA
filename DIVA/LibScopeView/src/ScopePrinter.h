//===-- LibScopeView/ScopePrinter.h -----------------------------*- C++ -*-===//
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
/// This file contains the declaration of the ScopePrinter class.
///
//===----------------------------------------------------------------------===//

#ifndef SCOPEVIEW_SCOPEPRINTER_H
#define SCOPEVIEW_SCOPEPRINTER_H

#include "ScopeVisitor.h"
#include "PrintSettings.h"

#include <string>

namespace LibScopeView {

class Object;
class ScopeRoot;

/// \brief An abstract base class for a scope printer.
///
/// The printer can output to a stream, or in a 'split' mode. When
/// splitting each compile unit is written to a seperate file in a given
/// directory.
///
/// Typical usage:
/// \code
///   class MyPrinter : public ScopePrinter {
///     void printImpl(const Object *Obj, std::ostream &OutputStream) override {
///       OutputStream << Obj.getName() << ... << '\n';
///     }
///     const std::string &getFileExtension() override {
///       static std::string Ext = "txt";
///       return Ext;
///     }
///   }
///
///   MyPrinter().print(Root, std::cout);
///   MyPrinter().print(Root, "output/dir");
/// \endcode
class ScopePrinter : private ConstScopeVisitor {
public:
  ScopePrinter(const PrintSettings &PrintingSettings) :
      Settings(PrintingSettings), OutputStream(nullptr) {}
  virtual ~ScopePrinter() override {}

  /// \brief Print Obj to Output.
  void print(const Object *Obj, std::ostream &Output);

  /// \brief Print each CU under the ScopeRoot to a file in OutputDir.
  void print(const ScopeRoot *Root, const std::string &OutputDir);

protected:
  void printChildren(const Object *Obj) { visitChildren(Obj); }

  // Print settings.
  const PrintSettings &Settings;

private:
  /// \brief Subclass interface for printing an object.
  virtual void printImpl(const Object *Obj, std::ostream &OutputStream) = 0;

  /// \brief get the file extension to use when splitting output (e.g. "txt").
  virtual const std::string &getFileExtension() = 0;

  /// \brief get header text to be added to the start of the output, or at the
  /// top of each split file.
  virtual const std::string &getHeader();

  /// \brief get footer text to be added to the end of the output, or at the
  /// bottom of each split file.
  virtual const std::string &getFooter();

  // Call printImpl() on the object with the appropriate OutputStream.
  void visitImpl(const Object *Obj) override;

  // Current output stream.
  std::ostream *OutputStream;
};

} // end namespace LibScopeView

#endif // SCOPEVIEW_SCOPEPRINTER_H
