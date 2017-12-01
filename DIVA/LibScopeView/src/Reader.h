//===-- LibScopeView/Reader.h -----------------------------------*- C++ -*-===//
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
/// Definition of the Reader base class.
///
//===----------------------------------------------------------------------===//

#ifndef READER_H
#define READER_H

#include "PrintSettings.h"
#include "Scope.h"

#include <memory>

namespace LibScopeView {

class Scope;

/// \brief Representation of a generic reader.
class Reader {
public:
  Reader() = default;
  virtual ~Reader();

  Reader(const Reader &) = delete;
  Reader &operator=(const Reader &) = delete;

  /// \brief Load a ScopeView from the file.
  std::unique_ptr<ScopeRoot> loadFile(const std::string &FileName,
                                      const PrintSettings &Settings);

private:
  /// \brief Implements the creation of the tree from a file.
  virtual std::unique_ptr<ScopeRoot>
  createScopes(const std::string &FileName) = 0;

  /// \brief Do general post creation setup on the tree.
  void postCreationActions(ScopeRoot *Root, const PrintSettings &Settings);
};

} // namespace LibScopeView

#endif // READER_H
