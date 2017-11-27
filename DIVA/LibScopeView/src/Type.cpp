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
#include "PrintSettings.h"
#include "StringPool.h"
#include "Scope.h"
#include "Symbol.h"

#include <assert.h>
#include <cstring>
#include <sstream>

using namespace LibScopeView;

Type::Type(LevelType Lvl) : Element(Lvl), ByteSize(0) {
  setIsType();

  Type::setTag();
}

Type::Type() : ByteSize(0) {
  setIsType();

  Type::setTag();
}

Type::~Type() {}

uint32_t Type::TypesAllocated = 0;

void Type::setTag() {
  ++Type::TypesAllocated;
#ifndef NDEBUG
  Tag = Type::TypesAllocated;
#endif
}

uint32_t Type::getTag() const {
#ifndef NDEBUG
  return Tag;
#else
  return 0;
#endif
}

// Type Kind.
const char *Type::KindBase = "PrimitiveType";
const char *Type::KindConst = "Const";
const char *Type::KindEnumerator = "Enumerator";
const char *Type::KindImport = "Using";
const char *Type::KindImportDeclaration = "UsingDeclaration";
const char *Type::KindImportModule = "UsingModule";
const char *Type::KindInherits = "Inherits";
const char *Type::KindPointer = "Pointer";
const char *Type::KindPointerMember = "PointerMember";
const char *Type::KindReference = "Reference";
const char *Type::KindRestrict = "Restrict";
const char *Type::KindRvalueReference = "RvalueReference";
const char *Type::KindSubrange = "Subrange";
const char *Type::KindTemplateTemplate = "TemplateParameter";
const char *Type::KindTemplateType = "TemplateParameter";
const char *Type::KindTemplateValue = "TemplateParameter";
const char *Type::KindTypedef = "Alias";
const char *Type::KindUndefined = "Undefined";
const char *Type::KindUnspecified = "Unspecified";
const char *Type::KindVolatile = "Volatile";

const char *Type::getKindAsString() const {
  const char *kind = KindUndefined;
  if (getIsBaseType())
    kind = KindBase;
  else if (getIsConstType())
    kind = KindConst;
  else if (getIsEnumerator())
    kind = KindEnumerator;
  else if (getIsImported())
    kind = KindImport;
  else if (getIsInheritance())
    kind = KindInherits;
  else if (getIsPointerMemberType())
    kind = KindPointerMember;
  else if (getIsPointerType())
    kind = KindPointer;
  else if (getIsReferenceType())
    kind = KindReference;
  else if (getIsRestrictType())
    kind = KindRestrict;
  else if (getIsRvalueReferenceType())
    kind = KindRvalueReference;
  else if (getIsSubrangeType())
    kind = KindSubrange;
  else if (getIsTemplateType())
    kind = KindTemplateType;
  else if (getIsTemplateValue())
    kind = KindTemplateValue;
  else if (getIsTemplateTemplate())
    kind = KindTemplateTemplate;
  else if (getIsTypedef())
    kind = KindTypedef;
  else if (getIsUnspecifiedType())
    kind = KindUnspecified;
  else if (getIsVolatileType())
    kind = KindVolatile;
  return kind;
}

const char *Type::resolveName() { return getName(); }

bool Type::setFullName(const PrintSettings &Settings) {
  if (getIsTemplateParam())
    return true;

  const char *BaseText = nullptr;
  Type *BaseType = nullptr;
  Scope *BaseScope = nullptr;

  BaseText = getName();
  // setFullName adds extra whitespace if you pass "" instead of nullptr.
  if (strlen(BaseText) == 0)
    BaseText = nullptr;

  if (getType() && getType()->getIsType()) {
    BaseType = static_cast<Type *>(getType());
  } else if (getType() && getType()->getIsScope()) {
    BaseScope = static_cast<Scope *>(getType());
  }

  return setFullName(Settings, BaseType, BaseScope, nullptr, BaseText);
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

/// \brief Class to represent a DWARF typedef object.
TypeDefinition::TypeDefinition(LevelType Lvl) : Type(Lvl) {}

TypeDefinition::TypeDefinition() : Type() {}

TypeDefinition::~TypeDefinition() {}

Object *TypeDefinition::getUnderlyingType() {
  Object *BaseType = nullptr;
  Type *Base = this;
  bool Traverse = true;
  while (Traverse) {
    if (Base->getIsType()) {
      Base = static_cast<Type *>(Base->getType());
      BaseType = Base;
      if (!Base->getIsTypedef() || !Base->getIsType()) {
        Traverse = false;
      }
    } else if (Base->getIsScope()) {
      BaseType = static_cast<Type *>(Base->getType());
      Traverse = false;
    }
  }

  return BaseType;
}

void TypeDefinition::setUnderlyingType(Object *Obj) { setType(Obj); }

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

/// \brief Class to represent a DWARF enumerator (DW_TAG_enumerator).
TypeEnumerator::TypeEnumerator(LevelType Lvl) : Type(Lvl) { ValueIndex = 0; }

TypeEnumerator::TypeEnumerator() : Type() { ValueIndex = 0; }

TypeEnumerator::~TypeEnumerator() {}

const char *TypeEnumerator::getValue() const {
  return StringPool::getStringValue(ValueIndex);
}

void TypeEnumerator::setValue(const char *value) {
  ValueIndex = StringPool::getStringIndex(value);
}

size_t TypeEnumerator::getValueIndex() const { return ValueIndex; }

std::string TypeEnumerator::getAsText(const PrintSettings &Settings) const {
  std::string ObjectAsText;
  ObjectAsText.append("\"").append(getName()).append("\" = ")
              .append(getValue());

  std::string DieOffset(getTypeDieOffsetAsString(Settings));
  if (!DieOffset.empty())
    ObjectAsText.append(" ").append(DieOffset);

  return formatAttributeText(ObjectAsText);
}

std::string TypeEnumerator::getAsYAML() const {
  // Printing enumerators is handled in ScopeEnumeration.
  return "";
}

/// \brief Class to represent a DWARF Import object (Using).
TypeImport::TypeImport(LevelType Lvl)
    : Type(Lvl), InheritanceAccess(AccessSpecifier::Unspecified) {}

TypeImport::TypeImport()
    : Type(), InheritanceAccess(AccessSpecifier::Unspecified) {}

TypeImport::~TypeImport() {}

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
    return getInheritanceAsText();
  else
    return getUsingAsText(Settings);
}

std::string TypeImport::getInheritanceAsText() const {
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
  Result << " \"" << getTypeName() << '"';
  return formatAttributeText(Result.str());
}

std::string TypeImport::getUsingAsText(const PrintSettings &Settings) const {
  std::stringstream Result;
  Result << "{" << getKindAsString() << "}";
  Result << getTypeDieOffsetAsString(Settings);
  Object *objtype = getType();
  if (objtype) {
    Scope *Parent = nullptr;
    if (getIsImportedDeclaration()) {
      if (objtype->getIsType()) {
        Parent = objtype->getParent();
        Result << " type";
      } else if (objtype->getIsScope()) {
        auto scope = static_cast<Scope *>(objtype);
        Parent = scope->getParent();
        if (scope->getIsFunction())
          Result << " function";
        else if (scope->getIsAggregate())
          Result << " type";
      } else if (objtype->getIsSymbol()) {
        auto symbol = static_cast<Symbol *>(objtype);
        if (symbol->getIsVariable() || symbol->getIsMember()) {
          Parent = symbol->getParent();
          Result << " variable";
        }
      }
    } else if (getIsImportedModule())
      Result << " namespace";
    Result << " \"";
    std::string ParentName;
    if (Parent != nullptr && !Parent->getIsCompileUnit())
      Parent->getQualifiedName(ParentName);
    if (!ParentName.empty())
      Result << ParentName << "::";
    Result << objtype->getName() << "\"";
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

std::string TypeImport::getInheritanceAsYAML() const {
  std::stringstream Result;
  if (!getIsInheritance())
    return Result.str();

  Result << "    - parent: \"" << getTypeName() << "\"\n"
         << "      access_specifier: ";

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
    Scope *Parent = nullptr;
    if (getIsImportedDeclaration()) {
      if (ObjType->getIsType()) {
        Parent = ObjType->getParent();
        UsingType = " \"type\"";
      } else if (ObjType->getIsScope()) {
        auto Scp = static_cast<Scope *>(ObjType);
        Parent = Scp->getParent();
        if (Scp->getIsFunction())
          UsingType = " \"function\"";
        else if (Scp->getIsAggregate())
          UsingType = " \"type\"";
      } else if (ObjType->getIsSymbol()) {
        auto Sym = static_cast<Symbol *>(ObjType);
        if (Sym->getIsVariable() || Sym->getIsMember()) {
          Parent = Sym->getParent();
          UsingType = " \"variable\"";
        }
      }
    } else if (getIsImportedModule())
      UsingType = " \"namespace\"";

    if (Parent != nullptr && !Parent->getIsCompileUnit())
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
         << getFileName(true) << "\""
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

TypeParam::TypeParam(LevelType Lvl) : Type(Lvl) { ValueIndex = 0; }

TypeParam::TypeParam() : Type() { ValueIndex = 0; }

TypeParam::~TypeParam() {}

const char *TypeParam::getValue() const {
  return StringPool::getStringValue(ValueIndex);
}

void TypeParam::setValue(const char *value) {
  ValueIndex = StringPool::getStringIndex(value);
}

size_t TypeParam::getValueIndex() const { return ValueIndex; }

bool TypeParam::getIsPrintedAsObject() const {
  // Template parameters within template packs are printed by the pack.
  return !(getParent() && getParent()->getIsTemplatePack());
}

std::string TypeParam::getAsText(const PrintSettings &Settings) const {
  std::string Result;
  // Template packs print differently.
  const Scope *Parent = getParent();
  bool IsPack = Parent && Parent->getIsTemplatePack();
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
    Result += getTypeName();
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

std::string TypeParam::getAsYAML() const {
  std::stringstream YAML;

  // Template parameters within template packs are printed by the pack.
  if (!(getParent() && getParent()->getIsTemplatePack()))
    YAML << getCommonYAML() << "\nattributes:\n  types:\n    - ";

  if (getIsTemplateType())
    YAML << "\"" << getTypeQualifiedName() << getTypeName() << "\"";
  else if (getIsTemplateValue())
    YAML << getValue();
  else {
    assert(getIsTemplateTemplate());
    YAML << "\"" << getValue() << "\"";
  }

  return YAML.str();
}

TypeSubrange::TypeSubrange(LevelType Lvl) : Type(Lvl) {}

TypeSubrange::TypeSubrange() : Type() {}

TypeSubrange::~TypeSubrange() {}
