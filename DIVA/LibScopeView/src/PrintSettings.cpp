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
  ShowPrimitiveType = false;
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
  ShowPrimitiveType = SetTo;
  ShowStruct = SetTo;
  ShowTemplate = SetTo;
  ShowUnion = SetTo;
  ShowUsing = SetTo;
  ShowVariable = SetTo;
}

bool PrintSettings::printObject(const Object &Obj) const {
  if ((ShowOnlyGlobals && !Obj.getIsGlobalReference()) ||
      (ShowOnlyLocals && Obj.getIsGlobalReference()))
    return false;

  if (auto Scp = dyn_cast<Scope>(&Obj)) {
    if (Scp->getIsTemplate())
      return ShowTemplate;
  }

  switch (Obj.getKind()) {
  case Object::SV_Line:
    return ShowCodeline;
  case Object::SV_Scope:
    if (cast<Scope>(Obj).getIsBlock())
      return ShowBlock;
    break;
  case Object::SV_ScopeAggregate: {
    auto &Agg = cast<ScopeAggregate>(Obj);
    if (Agg.getIsClassType())
      return ShowClass;
    if (Agg.getIsStructType())
      return ShowStruct;
    if (Agg.getIsUnionType())
      return ShowUnion;
  } break;
  case Object::SV_ScopeAlias:
    return ShowAlias;
  case Object::SV_ScopeEnumeration:
    return ShowEnum;
  case Object::SV_ScopeFunction:
  case Object::SV_ScopeFunctionInlined:
    return ShowFunction;
  case Object::SV_ScopeNamespace:
    return ShowNamespace;
  case Object::SV_ScopeTemplatePack:
    return ShowTemplate;
  case Object::SV_ScopeCompileUnit:
  case Object::SV_ScopeRoot:
    return true;
  case Object::SV_Symbol: {
    auto &Sym = cast<Symbol>(Obj);
    if (Sym.getIsMember())
      return ShowMember;
    else if (Sym.getIsParameter())
      return ShowParameter;
    else if (Sym.getIsVariable())
      return ShowVariable;
    return true;
  }
  case Object::SV_Type: {
    auto &Ty = cast<Type>(Obj);
    if (Ty.getIsBaseType())
      return ShowPrimitiveType;
    return false;
  }
  case Object::SV_TypeDefinition:
    return ShowAlias;
  case Object::SV_TypeEnumerator:
    return ShowEnum;
  case Object::SV_TypeImport: {
    auto &Import = cast<TypeImport>(Obj);
    if (Import.getIsInheritance())
      return ShowClass || ShowStruct;
    return ShowUsing;
  }
  case Object::SV_TypeParam:
    return ShowTemplate;
  case Object::SV_ScopeArray:
  case Object::SV_TypeSubrange:
    return false;
  }

  assert(false && "Unreachable");
  return false;
}

namespace {
bool matchPattern(const std::string &Name,
                  const std::vector<std::regex> &RegexFilters,
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

bool PrintSettings::matchesTreeFilterPattern(const std::string &Name) const {
  return matchPattern(Name, TreeFilters, TreeFilterAnys);
}

bool PrintSettings::hasFilters() const {
  return !(Filters.empty() && FilterAnys.empty() && TreeFilters.empty() &&
           TreeFilterAnys.empty());
}
