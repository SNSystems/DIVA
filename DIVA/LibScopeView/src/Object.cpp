//===-- LibScopeView/Object.cpp ---------------------------------*- C++ -*-===//
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
/// Implementation for the Object and Element classes.
///
//===----------------------------------------------------------------------===//

#include "Object.h"
#include "FileUtilities.h"
#include "Line.h"
#include "PrintSettings.h"
#include "Scope.h"
#include "StringPool.h"
#include "Symbol.h"
#include "Type.h"

#include <assert.h>
#include <cstring>
#include <sstream>

using namespace LibScopeView;

void LibScopeView::printAllocationInfo(std::ostream &Out) {
#ifndef NDEBUG
  // Data structure sizes.
  Out << "\n** Size of data structures: **\n";
  Out << "Scope:  " << sizeof(Scope) << "\n";
  Out << "Symbol: " << sizeof(Symbol) << "\n";
  Out << "Type:   " << sizeof(Type) << "\n";
  Out << "Line:   " << sizeof(Line) << "\n";
#endif // NDEBUG

  // TODO: Count this here rather than whenever objects are created.
  Out << "\n** Allocated Objects: **\n";
  Out << "Scopes:   " << Scope::getInstanceCount() << "\n";
  Out << "Symbols:  " << Symbol::getInstanceCount() << "\n";
  Out << "Types:    " << Type::getInstanceCount() << "\n";
  Out << "Lines:    " << Line::getInstanceCount() << "\n";
}

//===----------------------------------------------------------------------===//
// Class to represent the logical view of an object.
//===----------------------------------------------------------------------===//

Object::~Object() {}

Object::Object(ObjectKind K) : Kind(K) {
  LineNumber = 0;
  Parent = nullptr;
  DieOffset = 0;
  DieTag = 0;
}

const char *Object::getKindAsString() const {
  switch (Kind) {
  case SV_Line:
    return "CodeLine";
  case SV_Scope:
    if (cast<Scope>(this)->getIsBlock())
      return "Block";
    break;
  case SV_ScopeAggregate: {
    auto *Agg = cast<ScopeAggregate>(this);
    if (Agg->getIsClassType())
      return "Class";
    else if (Agg->getIsStructType())
      return "Struct";
    else if (Agg->getIsUnionType())
      return "Union";
  } break;
  case SV_ScopeAlias:
    return "Alias";
  case SV_ScopeArray:
    return "Array";
  case SV_ScopeCompileUnit:
    return "CompileUnit";
  case SV_ScopeEnumeration:
    return "Enum";
  case SV_ScopeFunction:
  case SV_ScopeFunctionInlined:
    return "Function";
  case SV_ScopeNamespace:
    return "Namespace";
  case SV_ScopeTemplatePack:
    return "TemplateParameter";
  case SV_ScopeRoot:
    return "InputFile";
  case SV_Symbol: {
    auto *Sym = cast<Symbol>(this);
    if (Sym->getIsMember())
      return "Member";
    else if (Sym->getIsParameter() || Sym->getIsUnspecifiedParameter())
      return "Parameter";
    else if (Sym->getIsVariable())
      return "Variable";
  } break;
  case SV_Type: {
    auto *Ty = cast<Type>(this);
    if (Ty->getIsBaseType())
      return "PrimitiveType";
    else if (Ty->getIsConstType())
      return "Const";
    else if (Ty->getIsInheritance())
      return "Inherits";
    else if (Ty->getIsPointerMemberType())
      return "PointerMember";
    else if (Ty->getIsPointerType())
      return "Pointer";
    else if (Ty->getIsReferenceType())
      return "Reference";
    else if (Ty->getIsRestrictType())
      return "Restrict";
    else if (Ty->getIsRvalueReferenceType())
      return "RvalueReference";
    else if (Ty->getIsUnspecifiedType())
      return "Unspecified";
    else if (Ty->getIsVolatileType())
      return "Volatile";
  } break;
  case SV_TypeDefinition:
    return "Alias";
  case SV_TypeEnumerator:
    return "Enumerator";
  case SV_TypeImport:
    return "Using";
  case SV_TypeTemplateParam:
    return "TemplateParameter";
  case SV_TypeSubrange:
    return "Subrange";
  }

  assert(false && "Unreachable");
  return "Undefined";
}

namespace {

std::string EmptyString;

const char *OffsetAsString(Dwarf_Off Offset) {
  // [0x00000000]
  const unsigned MaxLineSize = 16;
  static char Buffer[MaxLineSize];
  int Res = snprintf(Buffer, MaxLineSize, "[0x%08" DW_PR_DUx "]", Offset);
  assert((Res >= 0) && (static_cast<unsigned>(Res) < MaxLineSize) &&
         "string overflow");
  (void)Res;
  return Buffer;
}

} // namespace

const char *Object::getDieOffsetAsString(const PrintSettings &Settings) const {
  const char *Str = "";
  if (Settings.ShowDWARFOffset)
    Str = OffsetAsString(getDieOffset());
  return Str;
}

const char *
Object::getTypeDieOffsetAsString(const PrintSettings &Settings) const {
  const char *Str = "";
  if (Settings.ShowDWARFOffset) {
    Str = OffsetAsString(getType() ? getType()->getDieOffset() : 0);
  }
  return Str;
}

const char *Object::getTypeAsString(const PrintSettings &Settings) const {
  return getType() ? (getType()->getName().c_str())
                   : (Settings.ShowVoid ? "void" : "");
}

const std::string &Object::getTypeQualifiedName() const {
  if (!getType())
    return EmptyString;
  return getType()->getQualifiedName();
}

void Object::resolveQualifiedName(const Scope *ExplicitParent) {
  std::string QualifiedName;

  // Get the qualified name, excluding the Compile Unit, Functions, and the
  // scope root.
  const Object *ObjParent = ExplicitParent;
  while (ObjParent && !isa<ScopeCompileUnit>(*ObjParent) &&
         !isa<ScopeFunction>(*ExplicitParent) && !isa<ScopeRoot>(*ObjParent)) {

    const std::string &ParentName = ObjParent->getName();
    if (!ParentName.empty()) {
      QualifiedName.insert(0, "::");
      QualifiedName.insert(0, ParentName);
    }
    ObjParent = ObjParent->getParent();
  }

  if (!QualifiedName.empty()) {
    setQualifiedName(QualifiedName.c_str());
  }
}

std::string Object::formatAttributeText(const std::string &AttributeText) {
  return "    - " + AttributeText;
}

std::string Object::getCommonYAML() const {
  std::stringstream YAML;

  // Kind.
  YAML << "object: \"" << getKindAsString() << "\"\n";

  // Name.
  std::string Name;
  Name += getQualifiedName();
  if (isa<Symbol>(*this) && cast<Symbol>(this)->getIsUnspecifiedParameter())
    Name += "...";
  else
    Name += getName();

  YAML << "name: ";
  if (!Name.empty())
    YAML << "\"" << Name << "\"\n";
  else
    YAML << "null\n";

  // Type.
  YAML << "type: ";

  // Template's types are printed in attributes.
  if (getType() && !(isa<TypeTemplateParam>(*this))) {
    std::string TypeName;
    TypeName += getType()->getQualifiedName();
    TypeName += getType()->getName();
    YAML << "\"" << TypeName << "\"\n";
  }
  // Functions must have types.
  else if (isa<ScopeFunction>(*this))
    YAML << "\"void\"\n";
  else
    YAML << "null\n";

  // Source.
  YAML << "source:\n  line: ";
  if (getLineNumber() != 0)
    YAML << getLineNumber() << '\n';
  else
    YAML << "null\n";

  std::string FileName(getFileName(getFilePath()));
  YAML << "  file: ";
  if (getInvalidFileName())
    YAML << "\"?\"\n";
  else if (!FileName.empty())
    YAML << "\"" << FileName << "\"\n";
  else
    YAML << "null\n";

  // Dwarf.
  YAML << "dwarf:\n  offset: 0x" << std::hex << getDieOffset() << "\n  tag: ";
  if (getDieTag() != 0) {
    const char *TagName;
    dwarf_get_TAG_name(getDieTag(), &TagName);
    YAML << "\"" << TagName << "\"";
  } else
    YAML << "null";

  return YAML.str();
}

//===----------------------------------------------------------------------===//
// Class to represent the basic data for an object.
//===----------------------------------------------------------------------===//

Element::Element(ObjectKind K)
    : Object(K), NameRef(nullptr), QualifiedRef(nullptr), FilePathRef(nullptr),
      TheType(nullptr) {}

const std::string &Element::getName() const {
  return NameRef ? *NameRef : EmptyString;
}

void Element::setName(const std::string &Name) {
  NameRef = getGlobalStringPool().get(Name);
}

const std::string &Element::getQualifiedName() const {
  return QualifiedRef ? *QualifiedRef : EmptyString;
}

void Element::setQualifiedName(const std::string &QualName) {
  QualifiedRef = getGlobalStringPool().get(QualName);
}

const std::string &Element::getFilePath() const {
  return FilePathRef ? *FilePathRef : EmptyString;
}

void Element::setFilePath(const std::string &FilePath) {
  FilePathRef = getGlobalStringPool().get(FilePath);
}
