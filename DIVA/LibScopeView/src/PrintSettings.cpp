//===-- Diva/PrintSettings.cpp ----------------------------------*- C++ -*-===//
///
/// Copyright (c) Sony Interactive Entertainment Inc.
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
/// Settings controlling DIVA's output.
///
//===----------------------------------------------------------------------===//

#include "PrintSettings.h"
#include "Object.h"
#include "Scope.h"
#include "Symbol.h"
#include "Type.h"

#include <assert.h>

using namespace LibScopeView;

void PrintSettings::showBrief() {
  ShowAlias = true;
  ShowBlock = true;
  ShowBlockAttributes = false;
  ShowClass = true;
  ShowEnum = true;
  ShowFunction = true;
  ShowMember = true;
  ShowNamespace = true;
  ShowParameter = true;
  ShowPrimitivetype = false;
  ShowStruct = true;
  ShowTemplate = true;
  ShowUnion = true;
  ShowUsing = true;
  ShowVariable = true;
}

void PrintSettings::setMoreShowOptions(bool SetTo) {
  ShowAlias = SetTo;
  ShowBlock = SetTo;
  ShowBlockAttributes = SetTo;
  ShowClass = SetTo;
  ShowEnum = SetTo;
  ShowFunction = SetTo;
  ShowMember = SetTo;
  ShowNamespace = SetTo;
  ShowParameter = SetTo;
  ShowPrimitivetype = SetTo;
  ShowStruct = SetTo;
  ShowTemplate = SetTo;
  ShowUnion = SetTo;
  ShowUsing = SetTo;
  ShowVariable = SetTo;
}

bool PrintSettings::printObject(const Object &Obj) const {
  if (Obj.getIsLine())
    return ShowCodeline;

  if (auto Scp = dynamic_cast<const Scope *>(&Obj)) {
    if (Scp->getIsTemplateAlias())
      return ShowAlias;
    if (Scp->getIsBlock())
      return ShowBlock;
    if (Scp->getIsTemplate() || Scp->getIsTemplatePack())
      return ShowTemplate;
    if (Scp->getIsClassType())
      return ShowClass;
    if (Scp->getIsEnumerationType())
      return ShowEnum;
    if (Scp->getIsFunction())
      return ShowFunction;
    if (Scp->getIsNamespace())
      return ShowNamespace;
    if (Scp->getIsStructType())
      return ShowStruct;
    if (Scp->getIsUnionType())
      return ShowUnion;
    if (Scp->getIsArrayType())
      return false;
    return true;
  }

  if (auto Sym = dynamic_cast<const Symbol *>(&Obj)) {
    if (Sym->getIsMember())
      return ShowMember;
    else if (Sym->getIsParameter())
      return ShowParameter;
    else if (Sym->getIsVariable())
      return ShowVariable;
    return true;
  }

  if (auto Ty = dynamic_cast<const Type *>(&Obj)) {
    if (Ty->getIsSubrangeType())
      return false;
    if (Ty->getIsBaseType())
      return ShowPrimitivetype;
    else if (Ty->getIsTemplateParam())
      return ShowTemplate;
    else if (Ty->getIsTypedef())
      return ShowAlias;
    else if (Ty->getIsInheritance())
      return ShowClass || ShowStruct;
    else if (Ty->getIsImported())
      return ShowUsing;
    else if (Ty->getIsEnumerator())
      return ShowEnum;
    return false;
  }

  assert(false && "Not a Line, Scope, Symbol or Type");
  return false;
}

namespace {
bool matchPattern(
    const std::string &Name, const std::vector<std::regex> &RegexFilters,
    const std::vector<std::string> &StringFilters) {
  for (const std::regex &Filter : RegexFilters) {
    if (std::regex_match(Name, Filter))
      return true;
  }
  for (const std::string &Filter : StringFilters) {
    if (Name.find(Filter) != std::string::npos)
      return true;
  }
  return false;
}
} // namespace

bool PrintSettings::matchesFilterPattern(const std::string &Name) const {
  return matchPattern(Name, Filters, FilterAnys);
}

bool PrintSettings::matchesWithChildrenFilterPattern(
    const std::string &Name) const {
  return matchPattern(Name, WithChildrenFilters, WithChildrenFilterAnys);
}
