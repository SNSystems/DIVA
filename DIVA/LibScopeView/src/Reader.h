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

namespace LibScopeView {

class Scope;

/// \brief Representation for a generic reader.
class Reader {
public:
  Reader() : Scopes(nullptr), PrintedHeader(false) {}

  bool loadFile(const std::string &FileName, const PrintSettings &Settings);
  void print(const PrintSettings &Settings);

  virtual ~Reader() { delete Scopes; }

private:
  /// \brief Implements the creation of the tree from a file.
  virtual bool createScopes() = 0;

  void postCreationActions(const PrintSettings &Settings);

  void destroyScopes() {
    delete Scopes;
    Scopes = nullptr;
  }

protected:
  Scope *Scopes;

  // A header has been printed.
  bool PrintedHeader;

private:
  std::string InputFile;

protected:
  // Scopes that match a pattern.
  typedef std::vector<Scope *> MatchedScopes;
  MatchedScopes ViewMatchedScopes;

  // Objects that match a pattern.
  typedef std::vector<Object *> MatchedObjects;
  MatchedObjects ViewMatchedObjects;

protected:
  virtual void printObjects(const PrintSettings &Settings);
  virtual void printScopes(const PrintSettings &Settings);

public:
  void propagatePatternMatch();
  void resolveTreePatternMatch(Scope *scope, const PrintSettings &Settings);
  void resolveFilterPatternMatch(Object *object, const PrintSettings &Settings);
  void resolveFilterPatternMatch(Line *line, const PrintSettings &Settings);

public:
  std::string getInputFile() const { return InputFile; }

  // Shortcut for any printing.
  static bool getPrintObjects() { return true; }

  // Access to the scopes root.
  Scope *getScopesRoot() const { return Scopes; }
};

} // namespace LibScopeView

#endif // READER_H
