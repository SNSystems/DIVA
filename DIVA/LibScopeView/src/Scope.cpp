//===-- LibScopeView/Scope.cpp ----------------------------------*- C++ -*-===//
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
/// Implementations for the Scope class and its subclasses.
///
//===----------------------------------------------------------------------===//

#include "Scope.h"
#include "Error.h"
#include "FileUtilities.h"
#include "Line.h"
#include "PrintSettings.h"
#include "Symbol.h"
#include "Type.h"

#include <algorithm>
#include <cassert>
#include <sstream>

using namespace LibScopeView;

Scope::Scope(ObjectKind K) : Element(K) {}

Scope::~Scope() {
  for (Object *Child : Children)
    delete Child;
  for (Line *Ln : TheLines)
    delete Ln;
}

void Scope::addChild(Object *Obj) {
  // Do not add the line records to the children, as they represent the
  // logical view for the text section. Preserve the original sequence.
  if (auto *Ln = dyn_cast<Line>(Obj)) {
    TheLines.push_back(Ln);
    Ln->setParent(this);
    return;
  }

  Children.push_back(Obj);
  Obj->setParent(this);
}

void Scope::getQualifiedName(std::string &qualified_name) const {
  if (isa<ScopeRoot>(*this) || isa<ScopeCompileUnit>(*this))
    return;

  Scope *ScpParent = getParent();
  if (ScpParent) {
    ScpParent->getQualifiedName(qualified_name);
  }
  if (!qualified_name.empty()) {
    qualified_name.append("::");
  }
  qualified_name.append(getName());
}

void Scope::sortScopes(const SortingKey &SortKey) {
  // Get the sorting callback function.
  SortFunction SortFunc = getSortFunction(SortKey);
  if (SortFunc) {
    sortScopes(SortFunc);
  }
}

void Scope::sortScopes(SortFunction SortFunc) {
  std::sort(Children.begin(), Children.end(), SortFunc);

  for (Object *Obj : Children)
    if (auto *Scp = dyn_cast<Scope>(Obj))
      Scp->sortScopes(SortFunc);
}

std::string Scope::getAsText(const PrintSettings &Settings) const {
  std::stringstream Result;
  if (getIsBlock()) {
    Result << '{' << getKindAsString() << '}';
    if (Settings.ShowBlockAttributes) {
      if (getIsTryBlock())
        Result << '\n' << formatAttributeText("try");
      else if (getIsCatchBlock())
        Result << '\n' << formatAttributeText("catch");
    }
  }
  return Result.str();
}

std::string Scope::getAsYAML() const {
  if (getIsBlock()) {
    std::stringstream YAML;
    YAML << getCommonYAML() << "\nattributes:"
         << "\n  try: " << (getIsTryBlock() ? "true" : "false")
         << "\n  catch: " << (getIsCatchBlock() ? "true" : "false");
    return YAML.str();
  }
  return "";
}

ScopeAggregate::ScopeAggregate() : Scope(SV_ScopeAggregate) {}

std::string ScopeAggregate::getAsText(const PrintSettings &Settings) const {
  std::string Result;
  Result = "{";
  Result += getKindAsString();
  Result += "} \"";
  Result += getName();
  Result += '"';

  if (getIsTemplate()) {
    Result += '\n';
    Result += formatAttributeText("Template");
  }

  for (const Object *Obj : getChildren())
    if (auto *Ty = dyn_cast<const Type>(Obj))
      if (Ty->getIsInheritance())
        Result.append("\n").append(Ty->getAsText(Settings));

  return Result;
}

std::string ScopeAggregate::getAsYAML() const {
  std::stringstream Result;

  Result << getCommonYAML();
  Result << "\nattributes:\n  is_template: "
         << (getIsTemplate() ? "true" : "false");

  // If we're getting YAML for a Union. then we can't have any inheritance
  // attributes.
  if (getIsUnionType())
    return Result.str();

  Result << "\n  inherits_from:";

  bool hasInheritance = false;
  for (const Object *Obj : getChildren()) {
    if (auto *Ty = dyn_cast<const Type>(Obj)) {
      if (Ty->getIsInheritance()) {
        hasInheritance = true;
        Result << "\n" << Ty->getAsYAML();
      }
    }
  }

  if (!hasInheritance)
    Result << " []";

  return Result.str();
}

std::string ScopeAlias::getAsText(const PrintSettings &Settings) const {
  std::stringstream Result;
  Result << "{" << getKindAsString() << "} \"" << getName() << "\" -> "
         << getTypeDieOffsetAsString(Settings) << '"' << getTypeQualifiedName()
         << getTypeAsString(Settings) << '"';
  return Result.str();
}

std::string ScopeAlias::getAsYAML() const {
  return getCommonYAML() + std::string("\nattributes: {}");
}

std::string ScopeArray::getAsText(const PrintSettings &Settings) const {
  std::stringstream Result;
  Result << "{" << getKindAsString() << "} "
         << getTypeDieOffsetAsString(Settings) << '"' << getName() << '"';
  return Result.str();
}

void ScopeCompileUnit::setName(const std::string &Name) {
  Scope::setName(unifyFilePath(Name));
}

std::string ScopeCompileUnit::getAsText(const PrintSettings &) const {
  std::string ObjectAsText;
  ObjectAsText.append("{").append(getKindAsString()).append("}");
  ObjectAsText.append(" \"").append(getName()).append("\"");

  return ObjectAsText;
}

std::string ScopeCompileUnit::getAsYAML() const {
  return getCommonYAML() + std::string("\nattributes: {}");
}

std::string ScopeEnumeration::getAsText(const PrintSettings &Settings) const {
  std::string ObjectAsText;
  std::string Name = getName();

  ObjectAsText.append("{").append(getKindAsString()).append("}");

  if (getIsClass())
    ObjectAsText.append(" ").append("class");

  ObjectAsText.append(" \"").append(Name).append("\"");

  if (getType() && Name != getType()->getName())
    ObjectAsText.append(" -> \"").append(getType()->getName()).append("\"");

  for (auto *Child : getChildren()) {
    if (!isa<TypeEnumerator>(*Child))
      // TODO: Raise a warning here?
      continue;
    ObjectAsText.append("\n").append(Child->getAsText(Settings));
  }

  return ObjectAsText;
}

std::string ScopeEnumeration::getAsYAML() const {
  std::stringstream YAML;
  YAML << getCommonYAML() << "\nattributes:"
       << "\n  class: " << (getIsClass() ? "true" : "false")
       << "\n  enumerators:";

  std::stringstream Enumerators;
  for (auto *Child : getChildren()) {
    if (!isa<TypeEnumerator>(*Child))
      // TODO: Raise a warning here?
      continue;
    auto *ChildEnumerator = cast<TypeEnumerator>(Child);
    Enumerators << "\n    - enumerator: \"" << ChildEnumerator->getName()
                << "\"\n      value: " << ChildEnumerator->getValue();
  }

  if (Enumerators.str().empty())
    YAML << " []";
  else
    YAML << Enumerators.str();

  return YAML.str();
}

ScopeFunction::ScopeFunction(ObjectKind K)
    : Scope(K), IsStatic(false), DeclaredInline(false), IsDeclaration(false),
      Declaration(nullptr) {}

std::string ScopeFunction::getAsText(const PrintSettings &Settings) const {
  std::string Result = "{";
  Result += getKindAsString();
  Result += "}";

  if (getIsStatic())
    Result += " static";
  if (getIsDeclaredInline())
    Result += " inline";

  std::string Name;
  getQualifiedName(Name);

  Result += " \"";
  Result += Name;
  Result += "\"";
  Result += " -> ";
  Result += getTypeDieOffsetAsString(Settings);
  Result += "\"";
  Result += getTypeQualifiedName();
  Result += getTypeAsString(Settings);
  Result += "\"";

  // Attributes.
  if (getDeclaration()) {
    Result += '\n';
    Result += formatAttributeText("Declaration @ ");
    if (!getDeclaration()->getInvalidFileName())
      Result += getFileName(getDeclaration()->getFilePath());
    else
      Result += '?';
    Result += ',';
    Result += std::to_string(getDeclaration()->getLineNumber());
  } else if (!getIsDeclaration()) {
    Result += '\n';
    Result += formatAttributeText("No declaration");
  }

  if (getIsTemplate()) {
    Result += '\n';
    Result += formatAttributeText("Template");
  }
  if (isa<ScopeFunctionInlined>(*this)) {
    Result += '\n';
    Result += formatAttributeText("Inlined");
  }
  if (getIsDeclaration()) {
    Result += '\n';
    Result += formatAttributeText("Is declaration");
  }

  return Result;
}

std::string ScopeFunction::getAsYAML() const {
  std::stringstream YAML;
  YAML << getCommonYAML() << "\nattributes:\n";

  // Attributes.
  YAML << "  declaration:\n";
  if (getDeclaration()) {
    YAML << "    file: ";
    if (!getDeclaration()->getInvalidFileName())
      YAML << "\"" << getFileName(getDeclaration()->getFilePath()) << "\"";
    else
      YAML << "\"?\"";
    YAML << "\n    line: " << getDeclaration()->getLineNumber() << "\n";
  } else {
    YAML << "    file: null\n    line: null\n";
  }
  YAML << "  is_template: " << (getIsTemplate() ? "true" : "false") << "\n"
       << "  static: " << (getIsStatic() ? "true" : "false") << "\n"
       << "  inline: " << (getIsDeclaredInline() ? "true" : "false") << "\n"
       << "  is_inlined: "
       << (isa<ScopeFunctionInlined>(*this) ? "true" : "false") << "\n"
       << "  is_declaration: " << (getIsDeclaration() ? "true" : "false");

  return YAML.str();
}

ScopeFunctionInlined::~ScopeFunctionInlined() {}

std::string ScopeNamespace::getAsText(const PrintSettings &) const {
  std::stringstream Result;
  Result << '{' << getKindAsString() << '}';
  std::string Name;
  getQualifiedName(Name);
  if (!Name.empty())
    Result << " \"" << Name << '"';
  return Result.str();
}

std::string ScopeNamespace::getAsYAML() const {
  return getCommonYAML() + std::string("\nattributes: {}");
}

std::string ScopeTemplatePack::getAsText(const PrintSettings &Settings) const {
  std::string Result;
  Result += "{";
  Result += getKindAsString();
  Result += "}";
  Result += " \"";
  Result += getName();
  Result += "\"";

  for (const auto *Child : getChildren()) {
    if (isa<TypeTemplateParam>(*Child))
      Result.append("\n    ").append(Child->getAsText(Settings));
  }

  return Result;
}

std::string ScopeTemplatePack::getAsYAML() const {
  std::stringstream YAML;
  YAML << getCommonYAML() << "\nattributes:\n  types:";

  if (getChildren().size() == 0 ||
      std::none_of(
          getChildren().cbegin(), getChildren().cend(),
          [](const Object *Child) { return isa<TypeTemplateParam>(*Child); })) {
    YAML << " []";
    return YAML.str();
  }

  for (const auto *Child : getChildren()) {
    if (isa<TypeTemplateParam>(*Child))
      YAML << "\n    - " << Child->getAsYAML();
  }

  return YAML.str();
}

void ScopeRoot::setName(const std::string &Name) {
  Scope::setName(unifyFilePath(Name));
}

std::string ScopeRoot::getAsText(const PrintSettings &) const {
  std::stringstream Result;
  Result << "{" << getKindAsString() << "} \"" << getName() << '"';
  return Result.str();
}
