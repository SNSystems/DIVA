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
    : Element(SV_Symbol), TheAccessSpecifier(AccessSpecifier::Unspecified),
      IsStatic(false), Reference(nullptr) {}

AccessSpecifier Symbol::getAccessSpecifier() const {
  assert(getIsMember() && "getAccessSpecifier only valid for members");
  return TheAccessSpecifier;
}

void Symbol::setAccessSpecifier(AccessSpecifier Access) {
  assert(getIsMember() && "setAccessSpecifier only valid for members");
  TheAccessSpecifier = Access;
}

std::string Symbol::getAsText(const PrintSettings &Settings) const {
  std::stringstream Result;
  Result << "{" << getKindAsString() << "}";

  // Access specifier.
  if (getIsMember()) {
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

  if (getIsStatic())
    Result << " static";

  if (getIsUnspecifiedParameter()) {
    Result << " \"...\"";
  } else {
    Result << " \"" << getQualifiedName() << getName() << "\"";
    const Scope *Parent = getParent();
    if (Parent && isa<Scope>(*Parent) && Parent->getIsTemplate())
      Result << " <- ";
    else
      Result << " -> ";
    Result << getTypeDieOffsetAsString(Settings) << "\""
           << getTypeQualifiedName() << getTypeAsString(Settings) << "\"";
  }

  return Result.str();
}

std::string Symbol::getAsYAML() const {
  std::stringstream YAML;
  std::stringstream Attrs;

  // Access specifier.
  if (getIsMember()) {
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
