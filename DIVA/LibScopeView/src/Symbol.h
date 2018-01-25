//===-- LibScopeView/Symbol.h -----------------------------------*- C++ -*-===//
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
/// Definitions of the Symbol class and its subclasses.
///
//===----------------------------------------------------------------------===//

#ifndef SCOPEVIEWSYMBOL_H
#define SCOPEVIEWSYMBOL_H

#include "Object.h"

namespace LibScopeView {

/// \brief Class to represent a DWARF Symbol object.
class Symbol : public Element {
public:
  Symbol();

  /// \brief Return true if Obj is an instance of Symbol.
  static bool classof(const Object *Obj) {
    return Obj->getKind() == SV_Symbol;
  }

private:
  // Flags specifying various properties of the Symbol.
  enum SymbolAttributes {
    IsMember,
    IsParameter,
    IsUnspecifiedParameter,
    IsVariable,
    IsStatic,
    SymbolAttributesSize
  };
  std::bitset<SymbolAttributesSize> SymbolAttributesFlags;

public:
  bool getIsMember() const { return SymbolAttributesFlags[IsMember]; }
  void setIsMember() { SymbolAttributesFlags.set(IsMember); }

  bool getIsParameter() const { return SymbolAttributesFlags[IsParameter]; }
  void setIsParameter() { SymbolAttributesFlags.set(IsParameter); }

  bool getIsUnspecifiedParameter() const {
    return SymbolAttributesFlags[IsUnspecifiedParameter];
  }
  void setIsUnspecifiedParameter() {
    SymbolAttributesFlags.set(IsUnspecifiedParameter);
  }

  bool getIsVariable() const { return SymbolAttributesFlags[IsVariable]; }
  void setIsVariable() { SymbolAttributesFlags.set(IsVariable); }

  /// \brief Access specifier, only valid for members.
  AccessSpecifier getAccessSpecifier() const;
  void setAccessSpecifier(AccessSpecifier Access);

  bool getIsStatic() const { return SymbolAttributesFlags[IsStatic]; }
  void setIsStatic() { SymbolAttributesFlags.set(IsStatic); }

private:
  AccessSpecifier TheAccessSpecifier;

public:
  /// \brief Returns a text representation of this DIVA Object.
  std::string getAsText(const PrintSettings &Settings) const override;
  /// \brief Returns a YAML representation of this DIVA Object.
  std::string getAsYAML() const override;
};

} // namespace LibScopeView

#endif // SCOPEVIEWSYMBOL_H
