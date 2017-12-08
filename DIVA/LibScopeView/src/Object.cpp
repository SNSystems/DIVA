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

#include "FileUtilities.h"
#include "Line.h"
#include "PrintSettings.h"
#include "StringPool.h"
#include "Scope.h"
#include "Symbol.h"
#include "Type.h"
#include "Utilities.h"

// Disable some clang warnings for dwarf.h.
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif

#include "dwarf.h"

#ifdef __clang__
#pragma clang diagnostic pop
#endif

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

Object::Object() {
  LineNumber = 0;
  Parent = nullptr;
  DieOffset = 0;
  DieTag = 0;
}

namespace {

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
  return getHasType() ? (getTypeName()) : (Settings.ShowVoid ? "void" : "");
}

void Object::resolveQualifiedName(const Scope *ExplicitParent) {
  std::string QualifiedName;

  // Get the qualified name, excluding the Compile Unit, Functions, and the
  // scope root.
  const Object *ObjParent = ExplicitParent;
  while (ObjParent && !ObjParent->getIsCompileUnit() &&
         !ExplicitParent->getIsFunction() &&
         !(ObjParent->getIsScope() &&
           static_cast<const Scope *>(ObjParent)->getIsRoot())) {

    if (strlen(ObjParent->getName()) != 0) {
      QualifiedName.insert(0, "::");
      QualifiedName.insert(0, ObjParent->getName());
    }
    ObjParent = ObjParent->getParent();
  }

  if (!QualifiedName.empty()) {
    setQualifiedName(QualifiedName.c_str());
    setHasQualifiedName();
  }
}

bool Object::setFullName(const PrintSettings &Settings, Type *BaseType,
                         Scope *BaseScope, Scope *SpecScope,
                         const char *BaseText) {
  // In the case of scopes that have been updated using the specification
  // or abstract_origin attributes, the name already contain some patterns,
  // such as '()' or 'class'; for that case do not add the pattern.
  const char *ParentTypename = nullptr;
  if (BaseType) {
    ParentTypename = BaseType->getName();
  } else {
    if (BaseScope) {
      ParentTypename = BaseScope->getName();
    }
  }

  const char *BaseParent = nullptr;
  const char *PreText = nullptr;
  const char *PostText = nullptr;
  const char *PostPostText = nullptr;
  bool GetBaseTypename = false;

  bool UseParentTypeName = true;
  bool UseBaseText = true;
  Dwarf_Half tag = getDieTag();
  switch (tag) {
  case DW_TAG_base_type:
  case DW_TAG_compile_unit:
  case DW_TAG_namespace:
  case DW_TAG_class_type:
  case DW_TAG_structure_type:
  case DW_TAG_union_type:
  case DW_TAG_unspecified_type:
  case DW_TAG_enumeration_type:
  case DW_TAG_enumerator:
  case DW_TAG_inheritance:
  case DW_TAG_GNU_template_parameter_pack:
    GetBaseTypename = true;
    break;
  case DW_TAG_array_type:
  case DW_TAG_subrange_type:
  case DW_TAG_imported_module:
  case DW_TAG_imported_declaration:
  case DW_TAG_subprogram:
  case DW_TAG_subroutine_type:
  case DW_TAG_inlined_subroutine:
  case DW_TAG_entry_point:
  case DW_TAG_label:
  case DW_TAG_typedef:
    GetBaseTypename = true;
    UseParentTypeName = false;
    break;
  case DW_TAG_const_type:
    PreText = "const";
    break;
  case DW_TAG_pointer_type:
    PostText = "*";
    // For the following sample code,
    //   void *p;
    // some compilers do not generate a DIE for the 'void' type.
    //   <0x0000002a> DW_TAG_variable
    //                  DW_AT_name p
    //                  DW_AT_type <0x0000003f>
    //   <0x0000003f> DW_TAG_pointer_type
    // For that case, we can emit the 'void' type.
    if (!BaseType && !getType() && Settings.ShowVoid)
      ParentTypename = "void";
    break;
  case DW_TAG_ptr_to_member_type:
    PostText = "*";
    break;
  case DW_TAG_rvalue_reference_type:
    PostText = "&&";
    break;
  case DW_TAG_reference_type:
    PostText = "&";
    break;
  case DW_TAG_restrict_type:
    PreText = "restrict";
    break;
  case DW_TAG_volatile_type:
    PreText = "volatile";
    break;
  case DW_TAG_template_type_parameter:
  case DW_TAG_template_value_parameter:
  case DW_TAG_catch_block:
  case DW_TAG_lexical_block:
  case DW_TAG_try_block:
    UseBaseText = false;
    break;
  case DW_TAG_GNU_template_template_parameter:
    break;
  default:
    return false;
  }

  // Overwrite if no given value.
  if (!BaseText) {
    if (GetBaseTypename) {
      BaseText = getName();
    }
  }

  // Concatenate the elements to get the full type name.
  // Type will be: base_parent + pre + base + parent + post.
  std::string FullName;

  if (BaseParent) {
    FullName.append(BaseParent);
  }
  if (PreText && !SpecScope) {
    FullName.append(PreText);
  }
  if (UseBaseText && BaseText) {
    if (PreText)
      FullName.append(" ");
    FullName.append(BaseText);
  }
  if (UseParentTypeName && ParentTypename) {
    if (UseBaseText && BaseText)
      FullName.append(" ");
    else if (PreText)
      FullName.append(" ");
    FullName.append(ParentTypename);
  }
  if (PostText && !SpecScope) {
    if (UseParentTypeName && ParentTypename)
      FullName.append(" ");
    else if (UseBaseText && BaseText)
      FullName.append(" ");
    else if (PreText)
      FullName.append(" ");
    FullName.append(PostText);
  }
  if (PostPostText) {
    FullName.append(PostPostText);
  }

  // Remove any double spaces.
  size_t StartPos = 0;
  std::string Spaces("  ");
  std::string Single(" ");
  while ((StartPos = FullName.find(Spaces, StartPos)) != std::string::npos) {
    FullName.replace(StartPos, Spaces.length(), Single);
    StartPos += Single.length();
  }

  setName(FullName.c_str());

  return true;
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
  if (getHasQualifiedName())
    Name += getQualifiedName();
  if (getIsSymbol() &&
      static_cast<const Symbol *>(this)->getIsUnspecifiedParameter())
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
  if (getType() &&
      // Template's types are printed in attributes.
      !(getIsType() && static_cast<const Type *>(this)->getIsTemplateParam())) {
    std::string TypeName;
    if (getType()->getHasQualifiedName())
      TypeName += getType()->getQualifiedName();
    TypeName += getType()->getName();
    YAML << "\"" << TypeName << "\"\n";
    // Functions must have types.
  } else if (getIsScope() && static_cast<const Scope *>(this)->getIsFunction())
    YAML << "\"void\"\n";
  else
    YAML << "null\n";

  // Source.
  YAML << "source:\n  line: ";
  if (getLineNumber() != 0)
    YAML << getLineNumber() << '\n';
  else
    YAML << "null\n";

  std::string FileName(getFileName(/*format_options*/ true));
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
Element::Element() : Object() {
  NameIndex = 0;
  QualifiedIndex = 0;
  FilenameIndex = 0;
  TheType = nullptr;
}

void Element::setName(const char *name) {
  NameIndex = StringPool::getStringIndex(name);
}

const char *Element::getName() const {
  return StringPool::getStringValue(NameIndex);
}

void Element::setNameIndex(size_t name_index) { NameIndex = name_index; }

size_t Element::getNameIndex() const { return NameIndex; }

void Element::setQualifiedName(const char *QualName) {
  QualifiedIndex = StringPool::getStringIndex(QualName);
}

const char *Element::getQualifiedName() const {
  return StringPool::getStringValue(QualifiedIndex);
}

const char *Element::getTypeName() const {
  return getType() ? getType()->getName() : "";
}

std::string Element::getFileName(bool NameOnly) const {
  // If 'format_options', then we use the format command line options, to
  // return the full pathname or just the filename. The string stored in
  // the string pool, is the full pathname.
  std::string FName = StringPool::getStringValue(getFileNameIndex());
  if (NameOnly)
    FName = LibScopeView::getFileName(FName);
  return FName;
}

void Element::setFileName(const char *FileName) {
  FilenameIndex = StringPool::getStringIndex(unifyFilePath(FileName));
}

size_t Element::getFileNameIndex() const { return FilenameIndex; }

void Element::setFileNameIndex(size_t FNameIndex) {
  FilenameIndex = FNameIndex;
}

const char *Element::getTypeQualifiedName() const {
  if (!getType())
    return "";
  return getType()->getQualifiedName();
}
