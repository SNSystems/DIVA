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
  Symbol(LevelType Lvl);
  virtual ~Symbol() override;

  Symbol &operator=(const Symbol &) = delete;
  Symbol(const Symbol &) = delete;

private:
  // Symbol Kind.
  static const char *KindMember;
  static const char *KindParameter;
  static const char *KindUndefined;
  static const char *KindUnspecified;
  static const char *KindVariable;

private:
  // Flags specifying various properties of the Symbol.
  enum SymbolAttributes {
    IsMember,
    IsParameter,
    IsSpecifiedParameter,
    IsVariable,
    SymbolAttributesSize
  };
  std::bitset<SymbolAttributesSize> SymbolAttributesFlags;

public:
  /// \brief Gets the object kind as a string.
  const char *getKindAsString() const override;

public:
  bool getIsMember() const { return SymbolAttributesFlags[IsMember]; }
  void setIsMember() { SymbolAttributesFlags.set(IsMember); }

  bool getIsParameter() const { return SymbolAttributesFlags[IsParameter]; }
  void setIsParameter() { SymbolAttributesFlags.set(IsParameter); }

  bool getIsUnspecifiedParameter() const {
    return SymbolAttributesFlags[IsSpecifiedParameter];
  }
  void setIsUnspecifiedParameter() {
    SymbolAttributesFlags.set(IsSpecifiedParameter);
  }

  bool getIsVariable() const { return SymbolAttributesFlags[IsVariable]; }
  void setIsVariable() { SymbolAttributesFlags.set(IsVariable); }

  /// \brief Access specifier, only valid for members.
  AccessSpecifier getAccessSpecifier() const;
  void setAccessSpecifier(AccessSpecifier Access);

  bool getIsStatic() const { return IsStatic; }
  void setIsStatic() { IsStatic = true; }

private:
  AccessSpecifier TheAccessSpecifier;
  bool IsStatic;

public:
  /// \brief Follow the chain of references given by DW_AT_abstract_origin
  /// and/or DW_AT_specification and update the symbol name.
  const char *resolveName();

private:
  // Reference to DW_AT_specification, DW_AT_abstract_origin attribute.
  Symbol *Reference;

public:
  /// \brief Access DW_AT_specification, DW_AT_abstract_origin reference.
  Symbol *getReference() const { return Reference; }
  void setReference(Symbol *Sym) {
    Reference = Sym;
    setHasReference();
  }

public:
  void dump(const PrintSettings &Settings) override;
  virtual void dumpExtra(const PrintSettings &Settings);
  virtual bool dump(bool DoHeader, const char *Header,
                    const PrintSettings &Settings);

  /// \brief Returns a text representation of this DIVA Object.
  std::string getAsText(const PrintSettings &Settings) const override;
  /// \brief Returns a YAML representation of this DIVA Object.
  std::string getAsYAML() const override;

private:
  static uint32_t SymbolsAllocated;

public:
  static uint32_t getInstanceCount() { return SymbolsAllocated; }
  uint32_t getTag() const override;
  void setTag() override;
};

} // namespace LibScopeView

#endif // SCOPEVIEWSYMBOL_H
