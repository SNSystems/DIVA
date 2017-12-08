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

Scope::Scope() : Element() {
  setIsScope();
}

Scope::~Scope() {
  for (Object *Child : Children)
    delete Child;
  for (Line *Ln : TheLines)
    delete Ln;
}

uint32_t Scope::ScopesAllocated = 0;

// Scope Kind.
const char *Scope::KindAggregate = "Aggregate";
const char *Scope::KindArray = "Array";
const char *Scope::KindBlock = "Block";
const char *Scope::KindCatchBlock = "CatchBlock";
const char *Scope::KindClass = "Class";
const char *Scope::KindCompileUnit = "CompileUnit";
const char *Scope::KindEntryPoint = "EntryPoint";
const char *Scope::KindEnumeration = "Enum";
const char *Scope::KindFile = "InputFile";
const char *Scope::KindFunction = "Function";
const char *Scope::KindFunctionType = "FunctionType";
const char *Scope::KindInlinedFunction = "Function";
const char *Scope::KindLabel = "Label";
const char *Scope::KindLexicalBlock = "LexicalBlock";
const char *Scope::KindNamespace = "Namespace";
const char *Scope::KindStruct = "Struct";
const char *Scope::KindTemplate = "Template";
const char *Scope::KindTemplateAlias = "Alias";
const char *Scope::KindTemplatePack = "TemplateParameter";
const char *Scope::KindTryBlock = "TryBlock";
const char *Scope::KindUndefined = "Undefined";
const char *Scope::KindUnion = "Union";

const char *Scope::getKindAsString() const {
  const char *Kind = KindUndefined;
  if (getIsArrayType())
    Kind = KindArray;
  else if (getIsBlock())
    Kind = KindBlock;
  else if (getIsCompileUnit())
    Kind = KindCompileUnit;
  else if (getIsEnumerationType())
    Kind = KindEnumeration;
  else if (getIsInlinedSubroutine())
    Kind = KindInlinedFunction;
  else if (getIsNamespace())
    Kind = KindNamespace;
  else if (getIsTemplatePack())
    Kind = KindTemplatePack;
  else if (getIsRoot())
    Kind = KindFile;
  else if (getIsTemplateAlias())
    Kind = KindTemplateAlias;
  else if (getIsClassType())
    Kind = KindClass;
  else if (getIsFunction())
    Kind = KindFunction;
  else if (getIsStructType())
    Kind = KindStruct;
  else if (getIsUnionType())
    Kind = KindUnion;
  return Kind;
}

void Scope::addChild(Object *Obj) {
  // Do not add the line records to the children, as they represent the
  // logical view for the text section. Preserve the original sequence.
  if (auto *Ln = dynamic_cast<Line *>(Obj)) {
    assert(getCanHaveLines());
    if (getCanHaveLines()) {
      TheLines.push_back(Ln);
      Ln->setParent(this);
    }
    return;
  }

  Children.push_back(Obj);
  Obj->setParent(this);
}

void Scope::getQualifiedName(std::string &qualified_name) const {
  if (getIsRoot() || getIsCompileUnit()) {
    return;
  }

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
    if (auto *Scp = dynamic_cast<Scope *>(Obj))
      Scp->sortScopes(SortFunc);
}

const char *Scope::resolveName() {
  // If the scope has a DW_AT_specification or DW_AT_abstract_origin,
  // follow the chain to resolve the name from those references.
  if (getHasReference()) {
    Scope *Specification = getReference();
    if (isUnnamed()) {
      setName(Specification->resolveName());
    }
  }
  return getName();
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

ScopeAggregate::ScopeAggregate() : Scope() { Reference = nullptr; }

ScopeAggregate::~ScopeAggregate() {}

std::string ScopeAggregate::getAsText(const PrintSettings &Settings) const {
  std::string Result;
  const char *Name = getName();
  Result = "{";
  Result += getKindAsString();
  Result += "} \"";
  if (Name != nullptr)
    Result += Name;
  Result += '"';

  if (getIsTemplate()) {
    Result += '\n';
    Result += formatAttributeText("Template");
  }

  for (const Object *Obj : Children)
    if (auto *Ty = dynamic_cast<const Type *>(Obj))
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
  for (const Object *Obj : Children) {
    if (auto *Ty = dynamic_cast<const Type *>(Obj)) {
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

ScopeAlias::ScopeAlias() : Scope() {}

ScopeAlias::~ScopeAlias() {}

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

ScopeArray::ScopeArray() : Scope() {}

ScopeArray::~ScopeArray() {}

std::string ScopeArray::getAsText(const PrintSettings &Settings) const {
  std::stringstream Result;
  Result << "{" << getKindAsString() << "} "
         << getTypeDieOffsetAsString(Settings) << '"' << getName() << '"';
  return Result.str();
}

ScopeCompileUnit::ScopeCompileUnit() : Scope() {}

ScopeCompileUnit::~ScopeCompileUnit() {}

void ScopeCompileUnit::setName(const char *Name) {
  std::string Path = unifyFilePath(Name);
  Scope::setName(Path.c_str());
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

ScopeEnumeration::ScopeEnumeration() : Scope(), IsClass(false) {}

ScopeEnumeration::~ScopeEnumeration() {}

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
    if (!(Child->getIsType() &&
          static_cast<const Type *>(Child)->getIsEnumerator()))
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
    if (!(Child->getIsType() &&
          static_cast<const Type *>(Child)->getIsEnumerator()))
      // TODO: Raise a warning here?
      continue;
    auto *ChildEnumerator = static_cast<TypeEnumerator *>(Child);
    Enumerators << "\n    - enumerator: \"" << ChildEnumerator->getName()
                << "\"\n      value: " << ChildEnumerator->getValue();
  }

  if (Enumerators.str().empty())
    YAML << " []";
  else
    YAML << Enumerators.str();

  return YAML.str();
}

ScopeFunction::ScopeFunction()
    : Scope(), IsStatic(false), DeclaredInline(false), IsDeclaration(false) {
  Reference = nullptr;
}

ScopeFunction::~ScopeFunction() {}

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
  Result += getDieOffsetAsString(Settings);
  Result += "\"";
  Result += getTypeQualifiedName();
  Result += getTypeAsString(Settings);
  Result += "\"";

  // Attributes.
  if (Reference && Reference->getIsFunction()) {
    Result += '\n';
    Result += formatAttributeText("Declaration @ ");
    // Cast to element as Scope has a different overload (not override) of
    // getFileName that returns nothing.
    if (!Reference->getInvalidFileName())
      Result += static_cast<LibScopeView::Element *>(Reference)->getFileName(
          /*format_options*/ true);
    else
      Result += '?';
    Result += ',';
    Result += std::to_string(Reference->getLineNumber());
  } else {
    if (!getIsDeclaration()) {
      Result += '\n';
      Result += formatAttributeText("No declaration");
    }
  }

  if (getIsTemplate()) {
    Result += '\n';
    Result += formatAttributeText("Template");
  }
  if (getIsInlined()) {
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
  if (Reference && Reference->getIsFunction()) {
    // Cast to element as Scope has a different overload (not override) of
    // getFileName that returns nothing.
    YAML << "    file: ";
    if (!Reference->getInvalidFileName())
      YAML << "\""
           << static_cast<LibScopeView::Element *>(Reference)->getFileName(
                  /*format_options*/ true)
           << "\"";
    else
      YAML << "\"?\"";
    YAML << "\n    line: " << Reference->getLineNumber() << "\n";
  } else {
    YAML << "    file: null\n    line: null\n";
  }
  YAML << "  is_template: " << (getIsTemplate() ? "true" : "false") << "\n"
       << "  static: " << (getIsStatic() ? "true" : "false") << "\n"
       << "  inline: " << (getIsDeclaredInline() ? "true" : "false") << "\n"
       << "  is_inlined: " << (getIsInlined() ? "true" : "false") << "\n"
       << "  is_declaration: " << (getIsDeclaration() ? "true" : "false");

  return YAML.str();
}

ScopeFunctionInlined::ScopeFunctionInlined()
    : ScopeFunction(), CallLineNumber(0) {
  Discriminator = 0;
}

ScopeFunctionInlined::~ScopeFunctionInlined() {}

ScopeNamespace::ScopeNamespace() : Scope() { Reference = nullptr; }

ScopeNamespace::~ScopeNamespace() {}

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

ScopeTemplatePack::ScopeTemplatePack() : Scope() {}

ScopeTemplatePack::~ScopeTemplatePack() {}

namespace {
bool isTemplate(const Object *Obj) {
  return Obj->getIsType() &&
         static_cast<const Type *>(Obj)->getIsTemplateParam();
};
} // end anonymous namespace.

std::string ScopeTemplatePack::getAsText(const PrintSettings &Settings) const {
  std::string Result;
  Result += "{";
  Result += getKindAsString();
  Result += "}";
  Result += " \"";
  Result += getName();
  Result += "\"";

  for (const auto *Child : getChildren()) {
    if (isTemplate(Child))
      Result.append("\n    ").append(Child->getAsText(Settings));
  }

  return Result;
}

std::string ScopeTemplatePack::getAsYAML() const {
  std::stringstream YAML;
  YAML << getCommonYAML() << "\nattributes:\n  types:";

  if (Children.size() == 0 ||
      std::none_of(getChildren().cbegin(), getChildren().cend(), isTemplate)) {
    YAML << " []";
    return YAML.str();
  }

  for (const auto *Child : getChildren()) {
    if (isTemplate(Child))
      YAML << "\n    - " << Child->getAsYAML();
  }

  return YAML.str();
}

ScopeRoot::ScopeRoot() : Scope() {}

ScopeRoot::~ScopeRoot() {}

void ScopeRoot::setName(const char *Name) {
  std::string Path = unifyFilePath(Name);
  Scope::setName(Path.c_str());
}

std::string ScopeRoot::getAsText(const PrintSettings &) const {
  std::stringstream Result;
  Result << "{" << getKindAsString() << "} \"" << getName() << '"';
  return Result.str();
}
