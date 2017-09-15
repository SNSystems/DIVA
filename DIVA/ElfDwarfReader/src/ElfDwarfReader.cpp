//===-- ElfDwarfReader/ElfDwarfReader.cpp -----------------------*- C++ -*-===//
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
/// This file implements the DwarfReader::createScopes method to create a scope
/// tree from an object file in the ELF format with DWARF debug information.
///
//===----------------------------------------------------------------------===//

#include "ElfDwarfReader.h"
#include "Error.h"
#include "FileUtilities.h"
#include "LibDwarfHelpers.h"
#include "Symbol.h"
#include "Type.h"
#include "Line.h"

#include <iomanip>
#include <iostream>
#include <sstream>

using namespace ElfDwarfReader;

namespace {

// Create a mapping from DWARF file IDs to the file paths.
std::vector<std::string> getSourceFileMapping(const DwarfDebugData &DebugData,
                                              const DwarfDie &CUDie) {
  std::vector<std::string> Mapping;
  char **SourceFiles = nullptr;
  Dwarf_Signed SourceFilesCount;

  auto Ret = dwarf_srcfiles(*CUDie, &SourceFiles, &SourceFilesCount, nullptr);
  if (Ret != DW_DLV_OK)
    return Mapping;

  // A file ID of 0 always means no file, so set [0] to empty string.
  Mapping.emplace_back("");

  for (Dwarf_Signed i = 0; i < SourceFilesCount; ++i) {
    Mapping.emplace_back(SourceFiles[i]);
    dwarf_dealloc(*DebugData, SourceFiles[i], DW_DLA_STRING);
  }
  dwarf_dealloc(*DebugData, SourceFiles, DW_DLA_LIST);

  return Mapping;
}

// Get the access specifier (Public, Private, etc.).
LibScopeView::AccessSpecifier getAccessSpecifier(const DwarfDie &Die) {
  if (auto Access = Die.getAttrAsSigned(DW_AT_accessibility)) {
    switch (*Access) {
    case DW_ACCESS_private:
      return LibScopeView::AccessSpecifier::Private;
    case DW_ACCESS_protected:
      return LibScopeView::AccessSpecifier::Protected;
    case DW_ACCESS_public:
      return LibScopeView::AccessSpecifier::Public;
    }
  }
  return LibScopeView::AccessSpecifier::Unspecified;
}

// Set the source file of an Object from a DWARF file ID.
static void setSourceFile(LibScopeView::Object &Obj, const std::vector<std::string> &Mapping,
                          Dwarf_Unsigned ID) {
  // size_t is smaller than Dwarf_Unsigned on 32 bit, but this shouldn't be a
  // problem unless there are 2^32 file names in the ELF.
  assert(ID < std::numeric_limits<size_t>::max());
  if (ID < Mapping.size()) {
    Obj.setFileName(Mapping[static_cast<size_t>(ID)].c_str());
  } else {
    Obj.setFileNameIndex(static_cast<size_t>(ID));
    Obj.setInvalidFileName();
  }
}

// Set one Object to reference another, handling any type specifics.
void addObjectReference(LibScopeView::Object *Obj,
                        LibScopeView::Object *Reference) {
  if (auto Scp = dynamic_cast<LibScopeView::Scope *>(Obj)) {
    // Scope to Scope.
    if (auto RefScp = dynamic_cast<LibScopeView::Scope *>(Reference))
      Scp->setReference(RefScp);
  } else if (auto Sym = dynamic_cast<LibScopeView::Symbol *>(Obj)) {
    // Symbol to Symbol.
    if (auto RefSym = dynamic_cast<LibScopeView::Symbol *>(Reference))
      Sym->setReference(RefSym);
  }
}

} // end anonymous namespace

bool DwarfReader::createScopes() {
  auto *Root = new LibScopeView::ScopeRoot(0U);
  Root->setIsRoot();
  Root->setName(getInputFile().c_str());
  Scopes = Root;

  try {
    LibScopeView::FileDescriptor FD(getInputFile());
    const DwarfDebugData DebugData(FD.get());
    createCompileUnits(DebugData, *Root);
  } catch (LibDwarfError &Err) {
#ifndef NDEBUG
    std::cerr << Err.getErrorMessage();
#else
    static_cast<void>(Err);
#endif
    LibScopeError::fatalError(LibScopeError::ErrorCode::ERR_INVALID_DWARF,
                              getInputFile());
  }

  return true;
}

void DwarfReader::createCompileUnits(const DwarfDebugData &DebugData,
                                     LibScopeView::ScopeRoot &Root) {
  for (const auto &CU : DebugData.getCompileUnits()) {
    CurrentCURange = std::make_pair(CU.HeaderOffset, CU.NextHeaderOffset);
    SourceFileMapping = getSourceFileMapping(DebugData, CU.CUDie);

    // Recursively create the tree of Objects from the CU and down.
    createObject(DebugData, CU.CUDie, Root, 0U);
  }

  assert(TypesToBeSet.empty() &&
         "Some objects had a type that was not created");

  assert(ReferencesToBeSet.empty() &&
         "Some objects had a reference that was not created");
}

void DwarfReader::createObject(const DwarfDebugData &DebugData,
                               const DwarfDie &Die,
                               LibScopeView::Object &ParentObj,
                               LibScopeView::LevelType Level) {
  // For now do nothing if the parent is not a scope.
  if (!ParentObj.getIsScope())
    return;
  auto &ParentScope = dynamic_cast<LibScopeView::Scope &>(ParentObj);

  auto ObjOffset = Die.getGlobalOffset();
  auto ObjTag = Die.getTag();

  // Create the object from the DWARF tag.
  LibScopeView::Object *Obj = createObjectByTag(ObjTag, Level);
  if (!Obj)
    return;

  // Add to the parent.
  if (auto Scp = dynamic_cast<LibScopeView::Scope *>(Obj))
    ParentScope.addObject(Scp);
  else if (auto Ty = dynamic_cast<LibScopeView::Type *>(Obj))
    ParentScope.addObject(Ty);
  else if (auto Sym = dynamic_cast<LibScopeView::Symbol *>(Obj))
    ParentScope.addObject(Sym);
  else {
    assert(false && "Obj is not a Scope, Type or Symbol");
    delete Obj;
    return;
  }

  // Check this object hasn't been created before.
  assert(CreatedObjects.count(ObjOffset) == 0U && "DWARF offset seen twice");

  // Record the Object by offset for lookup when creating other objects.
  CreatedObjects[ObjOffset] = Obj;

  // Set attributes.
  initObjectFromAttrs(*Obj, Die, ObjOffset, ObjTag);

  // Set any references.
  initObjectReferences(*Obj, Die);

  // Update any references to this object.
  updateReferencesToObject(*Obj, ObjOffset);

  // Recurse on the DIE children.
  for (auto IT = Die.childrenBegin(), End = Die.childrenEnd(); IT != End; ++IT)
    createObject(DebugData, *IT, *Obj, Level + 1);
}

LibScopeView::Object *
DwarfReader::createObjectByTag(Dwarf_Half Tag,
                               LibScopeView::LevelType Level) {
  switch (Tag) {
  // Types.
  case DW_TAG_base_type: {
    auto Obj = new LibScopeView::Type(Level);
    Obj->setIsBaseType();
    return Obj;
  }
  case DW_TAG_const_type: {
    auto Obj = new LibScopeView::Type(Level);
    Obj->setIsConstType();
    return Obj;
  }
  case DW_TAG_enumerator: {
    auto Obj = new LibScopeView::TypeEnumerator(Level);
    Obj->setIsEnumerator();
    return Obj;
  }
  case DW_TAG_imported_declaration: {
    auto Obj = new LibScopeView::TypeImport(Level);
    Obj->setIsImportedDeclaration();
    return Obj;
  }
  case DW_TAG_imported_module: {
    auto Obj = new LibScopeView::TypeImport(Level);
    Obj->setIsImportedModule();
    return Obj;
  }
  case DW_TAG_inheritance: {
    auto Obj = new LibScopeView::TypeImport(Level);
    Obj->setIsInheritance();
    return Obj;
  }
  case DW_TAG_pointer_type: {
    auto Obj = new LibScopeView::Type(Level);
    Obj->setIsPointerType();
    return Obj;
  }
  case DW_TAG_ptr_to_member_type: {
    auto Obj = new LibScopeView::Type(Level);
    Obj->setIsPointerMemberType();
    return Obj;
  }
  case DW_TAG_reference_type: {
    auto Obj = new LibScopeView::Type(Level);
    Obj->setIsReferenceType();
    return Obj;
  }
  case DW_TAG_restrict_type: {
    auto Obj = new LibScopeView::Type(Level);
    Obj->setIsRestrictType();
    return Obj;
  }
  case DW_TAG_rvalue_reference_type: {
    auto Obj = new LibScopeView::Type(Level);
    Obj->setIsRvalueReferenceType();
    return Obj;
  }
  case DW_TAG_subrange_type: {
    auto Obj = new LibScopeView::TypeSubrange(Level);
    Obj->setIsSubrangeType();
    return Obj;
  }
  case DW_TAG_template_value_parameter: {
    auto Obj = new LibScopeView::TypeParam(Level);
    Obj->setIsTemplateValue();
    return Obj;
  }
  case DW_TAG_template_type_parameter: {
    auto Obj = new LibScopeView::TypeParam(Level);
    Obj->setIsTemplateType();
    return Obj;
  }
  case DW_TAG_GNU_template_template_parameter: {
    auto Obj = new LibScopeView::TypeParam(Level);
    Obj->setIsTemplateTemplate();
    return Obj;
  }
  case DW_TAG_typedef: {
    auto Obj = new LibScopeView::TypeDefinition(Level);
    Obj->setIsTypedef();
    return Obj;
  }
  case DW_TAG_unspecified_type: {
    auto Obj = new LibScopeView::Type(Level);
    Obj->setIsUnspecifiedType();
    return Obj;
  }
  case DW_TAG_volatile_type: {
    auto Obj = new LibScopeView::Type(Level);
    Obj->setIsVolatileType();
    return Obj;
  }
  // Symbols.
  case DW_TAG_formal_parameter: {
    auto Obj = new LibScopeView::Symbol(Level);
    Obj->setIsParameter();
    return Obj;
  }
  case DW_TAG_unspecified_parameters: {
    auto Obj = new LibScopeView::Symbol(Level);
    Obj->setIsUnspecifiedParameter();
    return Obj;
  }
  case DW_TAG_member: {
    auto Obj = new LibScopeView::Symbol(Level);
    Obj->setIsMember();
    return Obj;
  }
  case DW_TAG_variable: {
    auto Obj = new LibScopeView::Symbol(Level);
    Obj->setIsVariable();
    return Obj;
  }
  // Scopes.
  case DW_TAG_catch_block: {
    auto Obj = new LibScopeView::Scope(Level);
    Obj->setIsCatchBlock();
    return Obj;
  }
  case DW_TAG_lexical_block: {
    auto Obj = new LibScopeView::Scope(Level);
    Obj->setIsLexicalBlock();
    return Obj;
  }
  case DW_TAG_try_block: {
    auto Obj = new LibScopeView::Scope(Level);
    Obj->setIsTryBlock();
    return Obj;
  }
  case DW_TAG_compile_unit: {
    auto Obj = new LibScopeView::ScopeCompileUnit(0);
    Obj->setIsCompileUnit();
    return Obj;
  }
  case DW_TAG_inlined_subroutine: {
    auto Obj = new LibScopeView::ScopeFunctionInlined(Level);
    Obj->setIsInlinedSubroutine();
    return Obj;
  }
  case DW_TAG_namespace: {
    auto Obj = new LibScopeView::ScopeNamespace(Level);
    Obj->setIsNamespace();
    return Obj;
  }
  case DW_TAG_template_alias: {
    auto Obj = new LibScopeView::ScopeAlias(Level);
    Obj->setIsTemplateAlias();
    Obj->setIsTemplate();
    return Obj;
  }
  case DW_TAG_array_type: {
    auto Obj = new LibScopeView::ScopeArray(Level);
    Obj->setIsArrayType();
    return Obj;
  }
  case DW_TAG_entry_point: {
    auto Obj = new LibScopeView::ScopeFunction(Level);
    Obj->setIsEntryPoint();
    return Obj;
  }
  case DW_TAG_subprogram: {
    auto Obj = new LibScopeView::ScopeFunction(Level);
    Obj->setIsSubprogram();
    return Obj;
  }
  case DW_TAG_subroutine_type: {
    auto Obj = new LibScopeView::ScopeFunction(Level);
    Obj->setIsSubroutineType();
    return Obj;
  }
  case DW_TAG_label: {
    auto Obj = new LibScopeView::ScopeFunction(Level);
    Obj->setIsLabel();
    return Obj;
  }
  case DW_TAG_class_type: {
    auto Obj = new LibScopeView::ScopeAggregate(Level);
    Obj->setIsClassType();
    return Obj;
  }
  case DW_TAG_structure_type: {
    auto Obj = new LibScopeView::ScopeAggregate(Level);
    Obj->setIsStructType();
    return Obj;
  }
  case DW_TAG_union_type: {
    auto Obj = new LibScopeView::ScopeAggregate(Level);
    Obj->setIsUnionType();
    return Obj;
  }
  case DW_TAG_enumeration_type: {
    auto Obj = new LibScopeView::ScopeEnumeration(Level);
    Obj->setIsEnumerationType();
    return Obj;
  }
  case DW_TAG_GNU_template_parameter_pack: {
    auto Obj = new LibScopeView::ScopeTemplatePack(Level);
    Obj->setIsTemplatePack();
    return Obj;
  }
  default:
    if (!UnknownDWTags.count(Tag)) {
      UnknownDWTags.insert(Tag);
      std::stringstream Msg;
      Msg << "Ignoring unknown/unsupported DWARF tag 0x" << std::setw(4)
          << std::setfill('0') << std::hex << Tag << ".";
      LibScopeError::warning(Msg.str());
    }
    return nullptr;
  }
}

void DwarfReader::initObjectFromAttrs(LibScopeView::Object &Obj,
                                      const DwarfDie &Die, Dwarf_Off ObjOffset,
                                      Dwarf_Half ObjTag) {
  Obj.setDieOffset(ObjOffset);
  Obj.setDieTag(ObjTag);
  Obj.setName(Die.getName().c_str());
  Obj.setLineNumber(Die.getAttrAsUnsigned(DW_AT_decl_line, 0U));

  auto DeclFileID = Die.getAttrAsUnsigned(DW_AT_decl_file);
  if (DeclFileID)
    setSourceFile(Obj, SourceFileMapping, *DeclFileID);

  if (auto Scp = dynamic_cast<LibScopeView::Scope *>(&Obj))
    initScopeFromAttrs(*Scp, Die);
  else if (auto Ty = dynamic_cast<LibScopeView::Type *>(&Obj))
    initTypeFromAttrs(*Ty, Die);
  else if (auto Sym = dynamic_cast<LibScopeView::Symbol *>(&Obj))
    initSymbolFromAttrs(*Sym, Die);
}

void DwarfReader::initScopeFromAttrs(LibScopeView::Scope &Scp,
                                     const DwarfDie &Die) {
  Scp.resolveQualifiedName();

  // Parents of template packs are templates.
  if (Scp.getIsTemplatePack())
    if (auto ScpParent = dynamic_cast<LibScopeView::Scope *>(Scp.getParent()))
      ScpParent->setIsTemplate();

  // CU lines.
  if (auto CU = dynamic_cast<LibScopeView::ScopeCompileUnit *>(&Scp))
    createLines(Die, *CU);
  // Enum class.
  else if (auto ScpEnum =
               dynamic_cast<LibScopeView::ScopeEnumeration *>(&Scp)) {
    if (Die.getAttrAsFlag(DW_AT_enum_class))
      ScpEnum->setIsClass();
  }
  // Functions.
  else if (auto Func = dynamic_cast<LibScopeView::ScopeFunction *>(&Scp)) {
    if (Die.getAttrAsFlag(DW_AT_declaration))
      Func->setIsDeclaration();

    // A function is static if it is missing DW_AT_external and its declaration
    // (if it exists) is missing DW_AT_external.
    if (!Die.hasAttr(DW_AT_specification) && !Die.getAttrAsFlag(DW_AT_external))
      Func->setIsStatic();
    // The references aren't set up yet, so addObjectReference checks if the
    // declaration is static.

    if (auto Inline = Die.getAttrAsUnsigned(DW_AT_inline))
      if (*Inline == DW_INL_declared_inlined ||
          *Inline == DW_INL_declared_not_inlined)
        Func->setIsDeclaredInline();
  }
}

void DwarfReader::initTypeFromAttrs(LibScopeView::Type &Ty,
                                    const DwarfDie &Die) const {
  Ty.resolveQualifiedName();

  // Parents of template parameters are templates.
  if (Ty.getIsTemplateParam() && Ty.getParent()->getIsScope())
    if (auto ScpParent = dynamic_cast<LibScopeView::Scope *>(Ty.getParent()))
      ScpParent->setIsTemplate();

  // PrimitiveType byte size.
  if (Ty.getIsBaseType()) {
    Dwarf_Unsigned ByteSize = Die.getAttrAsUnsigned(DW_AT_byte_size, 0U);
    assert(ByteSize < std::numeric_limits<unsigned>::max());
    Ty.setByteSize(static_cast<unsigned>(ByteSize));
  }
  // Enum values and template values.
  else if (Ty.getIsEnumerator() || Ty.getIsTemplateValue()) {
    if (auto Val = Die.getAttrAsSignedOrUnsigned(DW_AT_const_value)) {
      if (Val->IsSigned)
        Ty.setValue(std::to_string(Val->SignedValue).c_str());
      else
        Ty.setValue(std::to_string(Val->UnsignedValue).c_str());
    }
  }
  // Template template value.
  else if (Ty.getIsTemplateTemplate()) {
    if (auto TemplateName = Die.getAttrAsString(DW_AT_GNU_template_name))
      Ty.setValue(TemplateName.getValue().c_str());
  }
  // Subranges.
  else if (Ty.getIsSubrangeType()) {
    std::stringstream SubrangeName;
    SubrangeName << "[";

    // Default lower bound for C++ is 0.
    Dwarf_Unsigned Lower = Die.getAttrAsUnsigned(DW_AT_lower_bound, 0U);
    try {
      if (auto Count = Die.getAttrAsUnsigned(DW_AT_count))
        SubrangeName << (Lower + *Count);
      else if (auto Upper = Die.getAttrAsUnsigned(DW_AT_upper_bound)) {
        if (Lower != 0)
          SubrangeName << Lower << ".." << *Upper;
        else
          SubrangeName << (*Upper + 1);
      } else
        // Invalid subrange (no count or upper).
        SubrangeName << "?";
    } catch (LibDwarfError &Err) {
      if (Err.getErrorNumber() != DW_DLE_ATTR_FORM_BAD)
        throw;
      // TODO: Handle DW_AT_upper_bounds that are references properly.
      SubrangeName << "?";
    }

    SubrangeName << "]";
    Ty.setName(SubrangeName.str().c_str());
  }
  // Inheritance.
  else if (Ty.getIsInheritance()) {
    auto &Inheritance = dynamic_cast<LibScopeView::TypeImport &>(Ty);
    Inheritance.setInheritanceAccess(getAccessSpecifier(Die));
  }
}

void DwarfReader::initSymbolFromAttrs(LibScopeView::Symbol &Sym,
                                      const DwarfDie &Die) {
  if (Sym.getIsMember())
    Sym.setAccessSpecifier(getAccessSpecifier(Die));
}

void DwarfReader::createLines(const DwarfDie &CUDie,
                              LibScopeView::ScopeCompileUnit &CUObj) {
  auto LineTable = CUDie.getLineTable();

  for (size_t LineIndex = 0; LineIndex < LineTable.size(); ++LineIndex) {
    auto DwarfLine = LineTable[LineIndex];
    auto *Ln = new LibScopeView::Line(1U);

    Ln->setIsLineRecord();
    CUObj.addObject(Ln);
    Ln->setLineNumber(DwarfLine.LineNo);
    Ln->setAddress(DwarfLine.LineAddr);
    Ln->setDieOffset(static_cast<Dwarf_Off>(DwarfLine.LineAddr));

    setSourceFile(*Ln, SourceFileMapping, DwarfLine.SrcFileID);

    // set DWARF qualifiers.
    Ln->setDiscriminator(static_cast<Dwarf_Half>(DwarfLine.Discriminator));
    if (DwarfLine.IsBeginStatement)
      Ln->setIsNewStatement();
    if (DwarfLine.IsBeginBlock)
      Ln->setIsNewBasicBlock();
    if (DwarfLine.IsEndSequence)
      Ln->setIsLineEndSequence();
    if (DwarfLine.IsEpilogueBegin)
      Ln->setIsEpilogueBegin();
    if (DwarfLine.IsPrologEnd)
      Ln->setIsPrologueEnd();
  }
}

void DwarfReader::initObjectReferences(LibScopeView::Object &Obj,
                                       const DwarfDie &Die) {
  // Set type or add to missing list to be resolved later.
  auto TypeOffset = Die.getAttrAsRef(DW_AT_type);
  // DW_AT_import is treated as a type by LibScopeView.
  if (!TypeOffset)
    TypeOffset = Die.getAttrAsRef(DW_AT_import);

  if (TypeOffset) {
    auto IT = CreatedObjects.find(*TypeOffset);
    if (IT != CreatedObjects.end()) {
      Obj.setType(IT->second);
      // If the type is in another CU mark it as global.
      if (*TypeOffset < CurrentCURange.first ||
          *TypeOffset > CurrentCURange.second)
        IT->second->setIsGlobalReference();
    } else
      // Set the type for this Object when we encounter TypeOffset.
      TypesToBeSet.emplace(*TypeOffset, &Obj);
  }

  // Set reference from a DW_AT_specification / DW_AT_abstract_origin /
  // DW_AT_extension or add to list to be resolved later.
  auto ReferenceOffset = Die.getAttrAsRef(DW_AT_specification);
  if (!ReferenceOffset)
    ReferenceOffset = Die.getAttrAsRef(DW_AT_abstract_origin);
  if (!ReferenceOffset)
    ReferenceOffset = Die.getAttrAsRef(DW_AT_extension);

  if (ReferenceOffset) {
    auto IT = CreatedObjects.find(*ReferenceOffset);
    // If the referenced function hasn't been created yet, add to
    // ReferencesToBeSet for later.
    if (IT == CreatedObjects.end())
      ReferencesToBeSet.emplace(*ReferenceOffset, &Obj);
    else {
      addObjectReference(&Obj, IT->second);
      // If the reference is in another CU mark it as global.
      if (*ReferenceOffset < CurrentCURange.first ||
          *ReferenceOffset > CurrentCURange.second)
        IT->second->setIsGlobalReference();
    }
  }
}

void DwarfReader::updateReferencesToObject(LibScopeView::Object &Obj,
                                           Dwarf_Off ObjOffset) {
  // If there are other Objects that have this Object as their type, then update
  // them.
  auto TyFoundRange = TypesToBeSet.equal_range(ObjOffset);
  for (auto IT = TyFoundRange.first; IT != TyFoundRange.second; ++IT) {
    IT->second->setType(&Obj);
    // If the other Object is in another CU mark this Object as global.
    if (IT->second->getDieOffset() < CurrentCURange.first ||
        IT->second->getDieOffset() > CurrentCURange.second)
      Obj.setIsGlobalReference();
  }
  TypesToBeSet.erase(TyFoundRange.first, TyFoundRange.second);

  // If there are other Objects that have this Object as a reference then update
  // them.
  auto RefFoundRange = ReferencesToBeSet.equal_range(ObjOffset);
  for (auto IT = RefFoundRange.first; IT != RefFoundRange.second; ++IT) {
    addObjectReference(IT->second, &Obj);
    // If the other Object is in another CU mark this Object as global.
    if (IT->second->getDieOffset() < CurrentCURange.first ||
        IT->second->getDieOffset() > CurrentCURange.second)
      IT->second->setIsGlobalReference();
  }
  ReferencesToBeSet.erase(RefFoundRange.first, RefFoundRange.second);
}
