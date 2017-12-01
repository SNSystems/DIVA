//===-- LibScopeView/Symbol.cpp ---------------------------------*- C++ -*-===//
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
/// Implementations for the Symbol class and its subclasses.
///
//===----------------------------------------------------------------------===//

#include "Symbol.h"
#include "PrintSettings.h"
#include "Scope.h"

#include <assert.h>
#include <sstream>

using namespace LibScopeView;

Symbol::Symbol()
    : Element(), TheAccessSpecifier(AccessSpecifier::Unspecified),
      IsStatic(false), Reference(nullptr) {
  setIsSymbol();
}

Symbol::~Symbol() {}

uint32_t Symbol::SymbolsAllocated = 0;

// Symbol Kind.
const char *Symbol::KindMember = "Member";
const char *Symbol::KindParameter = "Parameter";
const char *Symbol::KindUndefined = "Undefined";
const char *Symbol::KindUnspecified = "Parameter";
const char *Symbol::KindVariable = "Variable";

const char *Symbol::getKindAsString() const {
  const char *Kind = KindUndefined;
  if (getIsMember())
    Kind = KindMember;
  else if (getIsParameter())
    Kind = KindParameter;
  else if (getIsUnspecifiedParameter())
    Kind = KindUnspecified;
  else if (getIsVariable())
    Kind = KindVariable;
  return Kind;
}

AccessSpecifier Symbol::getAccessSpecifier() const {
  assert(getIsMember() && "getAccessSpecifier only valid for members");
  return TheAccessSpecifier;
}

void Symbol::setAccessSpecifier(AccessSpecifier Access) {
  assert(getIsMember() && "setAccessSpecifier only valid for members");
  TheAccessSpecifier = Access;
}

const char *Symbol::resolveName() {
  // If the symbol has a DW_AT_specification or DW_AT_abstract_origin,
  // follow the chain to resolve the name from those references.
  if (getHasReference()) {
    Symbol *Specification = getReference();
    if (isUnnamed()) {
      setName(Specification->resolveName());
    }
  }

  return getName();
}

std::string Symbol::getAsText(const PrintSettings &Settings) const {
  std::stringstream Result;
  const Symbol *Sym = getIsInlined() ? Reference : this;
  Result << "{" << Sym->getKindAsString() << "}";

  // Access specifier.
  if (Sym->getIsMember()) {
    switch (getAccessSpecifier()) {
    case AccessSpecifier::Private:
      Result << " private";
      break;
    case AccessSpecifier::Protected:
      Result << " protected";
      break;
    case AccessSpecifier::Public:
      Result << " public";
      break;
    case AccessSpecifier::Unspecified:
      assert(getParent());
      if (getParent() && getParent()->getIsClassType())
        Result << " private";
      else
        Result << " public";
      break;
    }
  }

  if (Sym->getIsStatic())
    Result << " static";

  if (Sym->getIsUnspecifiedParameter()) {
    Result << " \"...\"";
  } else {
    if (Sym->getNameIndex() != 0) {
      Result << " \"";
      if (Sym->getHasQualifiedName())
        Result << Sym->getQualifiedName();
      Result << getName() << "\"";
    }

    const Scope *parent = Sym->getParent();
    if (parent && parent->getIsScope() && parent->getIsTemplate())
      Result << " <- ";
    else
      Result << " -> ";
    Result << Sym->getTypeDieOffsetAsString(Settings) << "\""
           << Sym->getTypeQualifiedName() << Sym->getTypeAsString(Settings)
           << "\"";
  }

  return Result.str();
}

std::string Symbol::getAsYAML() const {
  std::stringstream YAML;
  std::stringstream Attrs;
  const Symbol *Sym = getIsInlined() ? Reference : this;

  // Access specifier.
  if (Sym->getIsMember()) {
    Attrs << "\n  access_specifier: \"";
    switch (getAccessSpecifier()) {
    case AccessSpecifier::Private:
      Attrs << "private";
      break;
    case AccessSpecifier::Protected:
      Attrs << "protected";
      break;
    case AccessSpecifier::Public:
      Attrs << "public";
      break;
    case AccessSpecifier::Unspecified:
      assert(getParent());
      if (getParent() && getParent()->getIsClassType())
        Attrs << "private";
      else
        Attrs << "public";
      break;
    }
    Attrs << '"';
  }

  // TODO: Uncomment and test once static is set by reader.
  // if (getIsMember())
  //   Attrs << "\n  static: " << (Sym->getIsStatic() ? "true" : "false");

  if (Attrs.str().empty())
    Attrs << " {}";

  YAML << getCommonYAML() << "\nattributes:" << Attrs.str();
  return YAML.str();
}
