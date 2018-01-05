//===-- LibScopeView/Type.cpp -----------------------------------*- C++ -*-===//
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
/// Implementations for Type and its subclasses.
///
//===----------------------------------------------------------------------===//

#include "Type.h"
#include "FileUtilities.h"
#include "PrintSettings.h"
#include "Scope.h"
#include "StringPool.h"
#include "Symbol.h"

#include <assert.h>
#include <cstring>
#include <sstream>

using namespace LibScopeView;

Type::Type(ObjectKind K) : Element(K), ByteSize(0) {}

uint32_t Type::TypesAllocated = 0;

bool Type::setFullName(const PrintSettings &Settings) {
  if (isa<TypeTemplateParam>(*this))
    return true;

  const char *BaseText = nullptr;
  Type *BaseType = nullptr;
  Scope *BaseScope = nullptr;

  BaseText = getName().c_str();
  // setFullName adds extra whitespace if you pass "" instead of nullptr.
  if (strlen(BaseText) == 0)
    BaseText = nullptr;

  if (getType()) {
    BaseType = dyn_cast<Type>(getType());
    if (!BaseType)
      BaseScope = dyn_cast<Scope>(getType());
  }

  return setFullName(Settings, BaseType, BaseScope, nullptr, BaseText);
}

namespace {
  std::string EmptyString;
}

const std::string &Type::getValue() const {
  return EmptyString;
}

bool Type::getIsPrintedAsObject() const { return getIsBaseType(); }

std::string Type::getAsText(const PrintSettings &) const {
  std::string Result;
  Result += "{";
  Result += getKindAsString();
  Result += "} -> \"";
  Result += getName();
  Result += "\"";
  unsigned byte_size = getByteSize();
  if (byte_size) {
    Result += '\n';
    Result += formatAttributeText(std::to_string(byte_size));
    Result += " bytes";
  }
  return Result;
}

std::string Type::getAsYAML() const {
  assert(getIsBaseType());

  // We can't use getCommonYAML here as the name is printed under 'type:'.
  std::stringstream YAML;
  YAML << "object: \"" << getKindAsString() << "\"\nname: null\ntype: \""
       << getName() << "\"\nsource:\n  line: null\n  file: null\n"
       << "dwarf:\n  offset: 0x" << std::hex << getDieOffset() << "\n";

  const char *TagName = "";
  if (getDieTag())
    dwarf_get_TAG_name(getDieTag(), &TagName);
  YAML << "  tag: \"" << TagName << "\"\n";

  YAML << "attributes:\n  size: " << std::dec << getByteSize();

  return YAML.str();
}

unsigned Type::getByteSize() const { return ByteSize; }

void Type::setByteSize(unsigned Size) { ByteSize = Size; }

std::string TypeDefinition::getAsText(const PrintSettings &Settings) const {
  std::string Result;
  Result += "{";
  Result += getKindAsString();
  Result += "} \"";
  Result += getName();
  Result += "\" -> ";
  Result += getTypeDieOffsetAsString(Settings);
  Result += "\"";
  if (getType() != nullptr)
    Result += getType()->getName();
  Result += "\"";
  return Result;
}

std::string TypeDefinition::getAsYAML() const {
  return getCommonYAML() + std::string("\nattributes: {}");
}

const std::string &TypeEnumerator::getValue() const {
  return ValueRef ? *ValueRef : EmptyString;
}

void TypeEnumerator::setValue(const std::string &Value) {
  ValueRef = getGlobalStringPool().get(Value);
}

std::string TypeEnumerator::getAsText(const PrintSettings &Settings) const {
  std::string ObjectAsText;
  ObjectAsText.append("\"").append(getName()).append("\" = ").append(
      getValue());

  std::string DieOffset(getTypeDieOffsetAsString(Settings));
  if (!DieOffset.empty())
    ObjectAsText.append(" ").append(DieOffset);

  return formatAttributeText(ObjectAsText);
}

std::string TypeEnumerator::getAsYAML() const {
  // Printing enumerators is handled in ScopeEnumeration.
  return "";
}

AccessSpecifier TypeImport::getInheritanceAccess() const {
  assert(getIsInheritance() &&
         "getInheritanceAccess only valid for inheritance");
  return InheritanceAccess;
}
void TypeImport::setInheritanceAccess(AccessSpecifier access) {
  assert(getIsInheritance() &&
         "setInheritanceAccess only valid for inheritance");
  InheritanceAccess = access;
}

bool TypeImport::getIsPrintedAsObject() const { return !getIsInheritance(); }

std::string TypeImport::getAsText(const PrintSettings &Settings) const {
  if (getIsInheritance())
    return getInheritanceAsText(Settings);
  else
    return getUsingAsText(Settings);
}

std::string
TypeImport::getInheritanceAsText(const PrintSettings &Settings) const {
  std::stringstream Result;
  switch (getInheritanceAccess()) {
  case AccessSpecifier::Private:
    Result << "private";
    break;
  case AccessSpecifier::Protected:
    Result << "protected";
    break;
  case AccessSpecifier::Public:
    Result << "public";
    break;
  case AccessSpecifier::Unspecified:
    assert(getParent());
    if (getParent() && getParent()->getIsClassType())
      Result << "private";
    else
      Result << "public";
    break;
  }
  Result << " \"" << getTypeAsString(Settings) << '"';
  return formatAttributeText(Result.str());
}

std::string TypeImport::getUsingAsText(const PrintSettings &Settings) const {
  std::stringstream Result;
  Result << "{" << getKindAsString() << "}";
  Result << getTypeDieOffsetAsString(Settings);
  Object *ObjType = getType();
  if (ObjType) {
    Scope *Parent = ObjType->getParent();
    if (getIsImportedModule())
      Result << " namespace";
    else if (getIsImportedDeclaration()) {
      if (isa<Type>(*ObjType) || isa<ScopeAggregate>(*ObjType))
        Result << " type";
      else if (isa<ScopeFunction>(*ObjType))
        Result << " function";
      else if (Symbol *Sym = dyn_cast<Symbol>(ObjType))
        if (Sym->getIsVariable() || Sym->getIsMember())
          Result << " variable";
    }

    Result << " \"";
    std::string ParentName;
    if (Parent != nullptr && !isa<ScopeCompileUnit>(*Parent))
      Parent->getQualifiedName(ParentName);
    if (!ParentName.empty())
      Result << ParentName << "::";
    Result << ObjType->getName() << "\"";
  }
  return Result.str();
}

std::string TypeImport::getAsYAML() const {
  // If type import is inheritance, then this object is treated as an attribute
  // and is already printed.
  if (!getIsPrintedAsObject())
    return getInheritanceAsYAML();

  return getUsingAsYAML();
}

std::string
TypeImport::getInheritanceAsYAML() const {
  std::stringstream Result;
  if (!getIsInheritance())
    return Result.str();

  Result << "    - parent: \"" << (getType() ? getType()->getName() : "")
         << "\"\n      access_specifier: ";

  switch (getInheritanceAccess()) {
  case AccessSpecifier::Private:
    Result << "\"private\"";
    break;
  case AccessSpecifier::Protected:
    Result << "\"protected\"";
    break;
  case AccessSpecifier::Public:
    Result << "\"public\"";
    break;
  case AccessSpecifier::Unspecified:
    assert(getParent());
    if (getParent() && getParent()->getIsClassType())
      Result << "\"private\"";
    else
      Result << "\"public\"";
  }

  return Result.str();
}

std::string TypeImport::getUsingAsYAML() const {
  std::stringstream Result;

  // Determine the UsingType and name for the Using object.
  std::string UsingType;
  std::string Name;
  Object *ObjType = getType();
  if (ObjType) {
    Scope *Parent = ObjType->getParent();
    if (getIsImportedModule())
      UsingType = " \"namespace\"";
    else if (getIsImportedDeclaration()) {
      if (isa<Type>(*ObjType) || isa<ScopeAggregate>(*ObjType))
        UsingType = " \"type\"";
      else if (isa<ScopeFunction>(*ObjType))
        UsingType = " \"function\"";
      else if (Symbol *Sym = dyn_cast<Symbol>(ObjType))
        if (Sym->getIsVariable() || Sym->getIsMember())
          UsingType = " \"variable\"";
    }

    if (Parent != nullptr && !isa<ScopeCompileUnit>(*Parent))
      Parent->getQualifiedName(Name);
    if (!Name.empty())
      Name.append("::");
    Name.append(ObjType->getName());
  }

  // We can't use getCommonYAML here as it returns the name of the Using as its
  // type.
  Result << "object: \"" << getKindAsString() << "\""
         << "\nname: \"" << Name << "\""
         << "\ntype: null"
         << "\nsource:"
         << "\n  line: " << getLineNumber() << "\n  file: \""
         << getFileName(getFilePath()) << "\""
         << "\ndwarf:"
         << "\n  offset: 0x" << std::hex << getDieOffset();
  const char *TagName;
  assert(getDieTag());
  dwarf_get_TAG_name(getDieTag(), &TagName);
  Result << "\n  tag: \"" << TagName << "\""
         << "\nattributes:"
         << "\n  using_type:" << UsingType;

  return Result.str();
}

const std::string &TypeTemplateParam::getValue() const {
  return ValueRef ? *ValueRef : EmptyString;
}

void TypeTemplateParam::setValue(const std::string &Value) {
  ValueRef = getGlobalStringPool().get(Value);
}

bool TypeTemplateParam::getIsPrintedAsObject() const {
  // Template parameters within template packs are printed by the pack.
  return !(getParent() && isa<ScopeTemplatePack>(*getParent()));
}

std::string TypeTemplateParam::getAsText(const PrintSettings &Settings) const {
  std::string Result;
  // Template packs print differently.
  const Scope *Parent = getParent();
  bool IsPack = Parent && isa<ScopeTemplatePack>(*Parent);
  if (!IsPack) {
    Result += "{";
    Result += getKindAsString();
    Result += "} \"";
    Result += getQualifiedName();
    Result += getName();
    Result += "\" ";
  }
  Result += "<- ";
  Result += getTypeDieOffsetAsString(Settings);

  if (getIsTemplateType()) {
    Result += "\"";
    Result += getTypeQualifiedName();
    Result += getTypeAsString(Settings);
    Result += "\"";
  } else if (getIsTemplateValue()) {
    Result += getValue();
  } else if (getIsTemplateTemplate()) {
    Result += "\"";
    Result += getValue();
    Result += "\"";
  }
  return Result;
}

std::string TypeTemplateParam::getAsYAML() const {
  std::stringstream YAML;

  // Template parameters within template packs are printed by the pack.
  if (!(getParent() && isa<ScopeTemplatePack>(*getParent())))
    YAML << getCommonYAML() << "\nattributes:\n  types:\n    - ";

  if (getIsTemplateType())
    YAML << "\"" << getTypeQualifiedName()
         << (getType() ? getType()->getName() : "") << "\"";
  else if (getIsTemplateValue())
    YAML << getValue();
  else {
    assert(getIsTemplateTemplate());
    YAML << "\"" << getValue() << "\"";
  }

  return YAML.str();
}

TypeSubrange::~TypeSubrange() {}
