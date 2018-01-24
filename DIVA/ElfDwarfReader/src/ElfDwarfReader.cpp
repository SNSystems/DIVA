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
#include "Line.h"
#include "Symbol.h"
#include "Type.h"

#include <iomanip>
#include <iostream>
#include <sstream>

using namespace ElfDwarfReader;

using LibScopeView::isa;
using LibScopeView::cast;
using LibScopeView::dyn_cast;

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
  Mapping.reserve(static_cast<size_t>(SourceFilesCount) + 1);
  Mapping.emplace_back("");

  for (Dwarf_Signed i = 0; i < SourceFilesCount; ++i) {
    Mapping.emplace_back(SourceFiles[i]);
    dwarf_dealloc(*DebugData, SourceFiles[i], DW_DLA_STRING);
  }
  dwarf_dealloc(*DebugData, SourceFiles, DW_DLA_LIST);

  return Mapping;
}

// Set the source file of an Object from a DWARF file ID.
static void setSourceFile(LibScopeView::Object &Obj,
                          const std::vector<std::string> &Mapping,
                          Dwarf_Unsigned ID) {
  if (ID == 0)
    return;

  // size_t is smaller than Dwarf_Unsigned on 32 bit, but this shouldn't be a
  // problem unless there are 2^32 file names in the ELF.
  assert(ID < std::numeric_limits<size_t>::max());
  if (ID < Mapping.size()) {
    Obj.setFilePath(Mapping[static_cast<size_t>(ID)]);
  } else {
    Obj.setInvalidFileName();
  }
}

// Set one Object to reference another, handling any type specifics.
void addObjectReference(LibScopeView::Object *Obj,
                        LibScopeView::Object *Reference) {
  if (auto Scp = dyn_cast<LibScopeView::Scope>(Obj)) {
    // Scope to Scope.
    if (auto RefScp = dyn_cast<LibScopeView::Scope>(Reference))
      Scp->setReference(RefScp);
  } else if (auto Sym = dyn_cast<LibScopeView::Symbol>(Obj)) {
    // Symbol to Symbol.
    if (auto RefSym = dyn_cast<LibScopeView::Symbol>(Reference))
      Sym->setReference(RefSym);
  }
}

// Write the Str to Out, unless it is empty then write Val as hex.
//
// Used when printing DWARF codes.
void writeStringOrHex(std::ostream &Out, const std::string Str,
                      Dwarf_Half Val) {
  if (Str.empty())
    Out << "0x" << std::setw(4) << std::setfill('0') << std::hex << Val;
  else
    Out << Str;
}

} // end anonymous namespace

std::unique_ptr<LibScopeView::ScopeRoot>
DwarfReader::createScopes(const std::string &FileName) {
  auto Root = std::make_unique<LibScopeView::ScopeRoot>();
  Root->setName(FileName.c_str());

  LibScopeView::FileDescriptor FD(FileName);
  try {
    const DwarfDebugData DebugData(FD.get());
    createCompileUnits(DebugData, *Root);
  } catch (LibDwarfError &Err) {
#ifndef NDEBUG
    std::cerr << Err.getErrorMessage();
#else
    static_cast<void>(Err);
#endif
    LibScopeError::fatalError(LibScopeError::ErrorCode::ERR_INVALID_DWARF,
                              FileName);
  }

  if (Root->getChildren().empty())
    LibScopeError::warning("No DWARF debug data found.");

  return Root;
}

void DwarfReader::createCompileUnits(const DwarfDebugData &DebugData,
                                     LibScopeView::ScopeRoot &Root) {
  for (const auto &CU : DebugData.getCompileUnits()) {
    CurrentCURange = std::make_pair(CU.HeaderOffset, CU.NextHeaderOffset);
    SourceFileMapping = getSourceFileMapping(DebugData, CU.CUDie);

    // Recursively create the tree of Objects from the CU and down.
    createObject(DebugData, CU.CUDie, Root);
  }

  // If we didn't skip any Dies (because of unknown tags) then we should have
  // resolved all the types and references.
  assert(!(!TypesToBeSet.empty() && UnknownDWTags.empty()) &&
         "Some objects had a type that was not created");
  assert(!(!ReferencesToBeSet.empty() && UnknownDWTags.empty()) &&
         "Some objects had a reference that was not created");
}

void DwarfReader::createObject(const DwarfDebugData &DebugData,
                               const DwarfDie &Die,
                               LibScopeView::Object &ParentObj) {
  auto &ParentScope = cast<LibScopeView::Scope>(ParentObj);

  auto ObjOffset = Die.getGlobalOffset();
  auto ObjTag = Die.getTag();

  // Create the object from the DWARF tag.
  LibScopeView::Object *Obj = createObjectByTag(ObjTag);
  if (!Obj)
    return;

  // Add to the parent.
  ParentScope.addChild(Obj);

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
    createObject(DebugData, *IT, *Obj);
}

LibScopeView::Object *
DwarfReader::createObjectByTag(Dwarf_Half Tag) {
  switch (Tag) {
  // Types.
  case DW_TAG_base_type: {
    auto Obj = new LibScopeView::Type;
    Obj->setIsBaseType();
    return Obj;
  }
  case DW_TAG_const_type: {
    auto Obj = new LibScopeView::Type;
    Obj->setIsConstType();
    return Obj;
  }
  case DW_TAG_enumerator:
    return new LibScopeView::TypeEnumerator;
  case DW_TAG_imported_declaration: {
    auto Obj = new LibScopeView::TypeImport;
    Obj->setIsImportedDeclaration();
    return Obj;
  }
  case DW_TAG_imported_module: {
    auto Obj = new LibScopeView::TypeImport;
    Obj->setIsImportedModule();
    return Obj;
  }
  case DW_TAG_inheritance: {
    auto Obj = new LibScopeView::TypeImport;
    Obj->setIsInheritance();
    return Obj;
  }
  case DW_TAG_pointer_type: {
    auto Obj = new LibScopeView::Type;
    Obj->setIsPointerType();
    return Obj;
  }
  case DW_TAG_ptr_to_member_type: {
    auto Obj = new LibScopeView::Type;
    Obj->setIsPointerMemberType();
    return Obj;
  }
  case DW_TAG_reference_type: {
    auto Obj = new LibScopeView::Type;
    Obj->setIsReferenceType();
    return Obj;
  }
  case DW_TAG_restrict_type: {
    auto Obj = new LibScopeView::Type;
    Obj->setIsRestrictType();
    return Obj;
  }
  case DW_TAG_rvalue_reference_type: {
    auto Obj = new LibScopeView::Type;
    Obj->setIsRvalueReferenceType();
    return Obj;
  }
  case DW_TAG_subrange_type:
    return new LibScopeView::TypeSubrange;
  case DW_TAG_template_value_parameter: {
    auto Obj = new LibScopeView::TypeTemplateParam;
    Obj->setIsTemplateValue();
    return Obj;
  }
  case DW_TAG_template_type_parameter: {
    auto Obj = new LibScopeView::TypeTemplateParam;
    Obj->setIsTemplateType();
    return Obj;
  }
  case DW_TAG_GNU_template_template_parameter: {
    auto Obj = new LibScopeView::TypeTemplateParam;
    Obj->setIsTemplateTemplate();
    return Obj;
  }
  case DW_TAG_typedef:
    return new LibScopeView::TypeDefinition;
  case DW_TAG_unspecified_type: {
    auto Obj = new LibScopeView::Type;
    Obj->setIsUnspecifiedType();
    return Obj;
  }
  case DW_TAG_volatile_type: {
    auto Obj = new LibScopeView::Type;
    Obj->setIsVolatileType();
    return Obj;
  }
  // Symbols.
  case DW_TAG_formal_parameter: {
    auto Obj = new LibScopeView::Symbol;
    Obj->setIsParameter();
    return Obj;
  }
  case DW_TAG_unspecified_parameters: {
    auto Obj = new LibScopeView::Symbol;
    Obj->setIsUnspecifiedParameter();
    return Obj;
  }
  case DW_TAG_member: {
    auto Obj = new LibScopeView::Symbol;
    Obj->setIsMember();
    return Obj;
  }
  case DW_TAG_variable: {
    auto Obj = new LibScopeView::Symbol;
    Obj->setIsVariable();
    return Obj;
  }
  // Scopes.
  case DW_TAG_catch_block: {
    auto Obj = new LibScopeView::Scope;
    Obj->setIsCatchBlock();
    return Obj;
  }
  case DW_TAG_lexical_block: {
    auto Obj = new LibScopeView::Scope;
    Obj->setIsLexicalBlock();
    return Obj;
  }
  case DW_TAG_try_block: {
    auto Obj = new LibScopeView::Scope;
    Obj->setIsTryBlock();
    return Obj;
  }
  case DW_TAG_compile_unit:
    return new LibScopeView::ScopeCompileUnit;
  case DW_TAG_inlined_subroutine:
    return new LibScopeView::ScopeFunctionInlined;
  case DW_TAG_namespace:
    return new LibScopeView::ScopeNamespace;
  case DW_TAG_template_alias:
    return new LibScopeView::ScopeAlias;
  case DW_TAG_array_type:
    return new LibScopeView::ScopeArray;
  case DW_TAG_entry_point: {
    auto Obj = new LibScopeView::ScopeFunction;
    Obj->setIsEntryPoint();
    return Obj;
  }
  case DW_TAG_subprogram: {
    auto Obj = new LibScopeView::ScopeFunction;
    Obj->setIsSubprogram();
    return Obj;
  }
  case DW_TAG_subroutine_type: {
    auto Obj = new LibScopeView::ScopeFunction;
    Obj->setIsSubroutineType();
    return Obj;
  }
  case DW_TAG_label: {
    auto Obj = new LibScopeView::ScopeFunction;
    Obj->setIsLabel();
    return Obj;
  }
  case DW_TAG_class_type: {
    auto Obj = new LibScopeView::ScopeAggregate;
    Obj->setIsClassType();
    return Obj;
  }
  case DW_TAG_structure_type: {
    auto Obj = new LibScopeView::ScopeAggregate;
    Obj->setIsStructType();
    return Obj;
  }
  case DW_TAG_union_type: {
    auto Obj = new LibScopeView::ScopeAggregate;
    Obj->setIsUnionType();
    return Obj;
  }
  case DW_TAG_enumeration_type:
    return new LibScopeView::ScopeEnumeration;
  case DW_TAG_GNU_template_parameter_pack:
    return new LibScopeView::ScopeTemplatePack;
  default:
    if (!UnknownDWTags.count(Tag)) {
      UnknownDWTags.insert(Tag);
      std::stringstream Msg;
      Msg << "Ignoring unknown/unsupported DWARF tag '";
      writeStringOrHex(Msg, getDwarfTagAsString(Tag), Tag);
      Msg << "'.";
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

  DwarfAttrValue LineNo(
      getAttrExpectingKind(Die, DW_AT_decl_line, DwarfAttrValueKind::Unsigned));
  Obj.setLineNumber(LineNo.empty() ? 0 : LineNo.getUnsigned());

  DwarfAttrValue DeclFileID(
      getAttrExpectingKind(Die, DW_AT_decl_file, DwarfAttrValueKind::Unsigned));
  if (!DeclFileID.empty())
    setSourceFile(Obj, SourceFileMapping, DeclFileID.getUnsigned());

  if (auto Scp = dyn_cast<LibScopeView::Scope>(&Obj))
    initScopeFromAttrs(*Scp, Die);
  else if (auto Ty = dyn_cast<LibScopeView::Type>(&Obj))
    initTypeFromAttrs(*Ty, Die);
  else if (auto Sym = dyn_cast<LibScopeView::Symbol>(&Obj))
    initSymbolFromAttrs(*Sym, Die);
}

void DwarfReader::initScopeFromAttrs(LibScopeView::Scope &Scp,
                                     const DwarfDie &Die) {
  Scp.resolveQualifiedName();

  // Parents of template packs are templates.
  if (isa<LibScopeView::ScopeTemplatePack>(Scp))
    if (auto ScpParent = dyn_cast<LibScopeView::Scope>(Scp.getParent()))
      ScpParent->setIsTemplate();

  // CU lines.
  if (auto CU = dyn_cast<LibScopeView::ScopeCompileUnit>(&Scp))
    createLines(Die, *CU);
  // Enum class.
  else if (auto ScpEnum =
               dyn_cast<LibScopeView::ScopeEnumeration>(&Scp)) {
    if (attrIsTrueFlag(Die, DW_AT_enum_class))
      ScpEnum->setIsClass();
  }
  // Functions.
  else if (auto Func = dyn_cast<LibScopeView::ScopeFunction>(&Scp)) {
    if (attrIsTrueFlag(Die, DW_AT_declaration))
      Func->setIsDeclaration();

    // A function is static if it is missing DW_AT_external and its declaration
    // (if it exists) is missing DW_AT_external.
    if (!Die.hasAttr(DW_AT_specification) &&
        !attrIsTrueFlag(Die, DW_AT_external))
      Func->setIsStatic();
    // The references aren't set up yet, so addObjectReference checks if the
    // declaration is static.

    DwarfAttrValue InlineAttrVal(
        getAttrExpectingKind(Die, DW_AT_inline, DwarfAttrValueKind::Unsigned));
    if (!InlineAttrVal.empty()) {
      auto Inline = InlineAttrVal.getUnsigned();
      if (Inline == DW_INL_declared_inlined ||
          Inline == DW_INL_declared_not_inlined)
        Func->setIsDeclaredInline();
    }
  }
}

void DwarfReader::initTypeFromAttrs(LibScopeView::Type &Ty,
                                    const DwarfDie &Die) {
  Ty.resolveQualifiedName();

  // Parents of template parameters are templates.
  if (isa<LibScopeView::TypeTemplateParam>(Ty) &&
      isa<LibScopeView::Scope>(*Ty.getParent()))
    if (auto ScpParent = dyn_cast<LibScopeView::Scope>(Ty.getParent()))
      ScpParent->setIsTemplate();

  // PrimitiveType byte size.
  if (Ty.getIsBaseType()) {
    DwarfAttrValue ByteSize(getAttrExpectingKind(Die, DW_AT_byte_size,
                                                 DwarfAttrValueKind::Unsigned));
    if (ByteSize.empty())
      Ty.setByteSize(0U);
    else {
      assert(ByteSize.getUnsigned() < std::numeric_limits<unsigned>::max());
      Ty.setByteSize(static_cast<unsigned>(ByteSize.getUnsigned()));
    }
  }
  // Enum values and template values.
  else if (isa<LibScopeView::TypeEnumerator>(Ty) || Ty.getIsTemplateValue()) {
    DwarfAttrValue Val(getAttrExpectingKinds(
        Die, DW_AT_const_value,
        {DwarfAttrValueKind::Unsigned, DwarfAttrValueKind::Signed}));
    if (Val.getKind() == DwarfAttrValueKind::Unsigned)
      Ty.setValue(std::to_string(Val.getUnsigned()).c_str());
    else if (Val.getKind() == DwarfAttrValueKind::Signed)
      Ty.setValue(std::to_string(Val.getSigned()).c_str());
  }
  // Template template value.
  else if (Ty.getIsTemplateTemplate()) {
    DwarfAttrValue TemplateName(getAttrExpectingKind(
        Die, DW_AT_GNU_template_name, DwarfAttrValueKind::String));
    if (!TemplateName.empty())
      Ty.setValue(TemplateName.getString().c_str());
  }
  // Subranges.
  else if (isa<LibScopeView::TypeSubrange>(Ty)) {
    std::stringstream SubrangeName;
    SubrangeName << "[";

    // Default lower bound for C++ is 0.
    Dwarf_Unsigned Lower = 0U;
    DwarfAttrValue LowerBound(getAttrExpectingKind(
        Die, DW_AT_lower_bound, DwarfAttrValueKind::Unsigned));
    if (!LowerBound.empty())
      Lower = LowerBound.getUnsigned();

    DwarfAttrValue Count(
        getAttrExpectingKind(Die, DW_AT_count, DwarfAttrValueKind::Unsigned));
    DwarfAttrValue Upper(getAttrExpectingKinds(
        Die, DW_AT_upper_bound,
        {DwarfAttrValueKind::Unsigned, DwarfAttrValueKind::Exprloc}));
    if (!Count.empty())
      SubrangeName << (Lower + Count.getUnsigned());
    else if (Upper.getKind() == DwarfAttrValueKind::Unsigned) {
      if (Lower != 0)
        SubrangeName << Lower << ".." << Upper.getUnsigned();
      else
        SubrangeName << (Upper.getUnsigned() + 1);
    } else if (Upper.getKind() == DwarfAttrValueKind::Exprloc)
      // TODO: Handle DW_AT_upper_bounds that are DW_FORM_exprloc properly.
      SubrangeName << "?";
    else
      // Invalid subrange (no count or upper).
      SubrangeName << "?";

    SubrangeName << "]";
    Ty.setName(SubrangeName.str().c_str());
  }
  // Inheritance.
  else if (Ty.getIsInheritance()) {
    auto *Inheritance = dyn_cast<LibScopeView::TypeImport>(&Ty);
    Inheritance->setInheritanceAccess(getAccessSpecifier(Die));
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
    auto *Ln = new LibScopeView::Line;

    CUObj.addChild(Ln);
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
  DwarfAttrValue TypeRef(
      getAttrExpectingKind(Die, DW_AT_type, DwarfAttrValueKind::Reference));

  // DW_AT_import is treated as a type by LibScopeView.
  if (TypeRef.empty())
    TypeRef =
        getAttrExpectingKind(Die, DW_AT_import, DwarfAttrValueKind::Reference);

  if (!TypeRef.empty()) {
    auto TypeOffset = TypeRef.getReference();
    auto IT = CreatedObjects.find(TypeOffset);
    if (IT != CreatedObjects.end()) {
      Obj.setType(IT->second);
      // If the type is in another CU mark it as global.
      if (TypeOffset < CurrentCURange.first ||
          TypeOffset > CurrentCURange.second)
        IT->second->setIsGlobalReference();
    } else
      // Set the type for this Object when we encounter TypeOffset.
      TypesToBeSet.emplace(TypeOffset, &Obj);
  }

  // Set reference from a DW_AT_specification / DW_AT_abstract_origin /
  // DW_AT_extension or add to list to be resolved later.
  DwarfAttrValue ReferenceOffset(getAttrExpectingKind(
      Die, DW_AT_specification, DwarfAttrValueKind::Reference));
  if (ReferenceOffset.empty())
    ReferenceOffset = getAttrExpectingKind(Die, DW_AT_abstract_origin,
                                           DwarfAttrValueKind::Reference);
  if (ReferenceOffset.empty())
    ReferenceOffset = getAttrExpectingKind(Die, DW_AT_extension,
                                           DwarfAttrValueKind::Reference);

  if (!ReferenceOffset.empty()) {
    auto RefOffset = ReferenceOffset.getReference();
    auto IT = CreatedObjects.find(RefOffset);
    // If the referenced function hasn't been created yet, add to
    // ReferencesToBeSet for later.
    if (IT == CreatedObjects.end())
      ReferencesToBeSet.emplace(RefOffset, &Obj);
    else {
      addObjectReference(&Obj, IT->second);
      // If the reference is in another CU mark it as global.
      if (RefOffset < CurrentCURange.first || RefOffset > CurrentCURange.second)
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

DwarfAttrValue
DwarfReader::getAttrExpectingKind(const DwarfDie &Die, const Dwarf_Half Attr,
                                  const DwarfAttrValueKind ExpectedKind) {
  return getAttrExpectingKinds(Die, Attr, {ExpectedKind});
}

DwarfAttrValue DwarfReader::getAttrExpectingKinds(
    const DwarfDie &Die, const Dwarf_Half Attr,
    const std::set<DwarfAttrValueKind> &ExpectedKinds) {
  DwarfAttrValue AttrVal(Die.getAttr(Attr));
  if (AttrVal.empty() || ExpectedKinds.count(AttrVal.getKind()))
    return AttrVal;

  auto Form = AttrVal.getForm();
  auto AttrFormPair = std::make_pair(Attr, Form);
  if (!UnknownAttrFormPairs.count(AttrFormPair)) {
    UnknownAttrFormPairs.insert(AttrFormPair);
    std::stringstream Msg;
    Msg << "Ignoring unrecognised DW_AT, DW_FORM combination '";
    writeStringOrHex(Msg, getDwarfAttrAsString(Attr), Attr);
    Msg << "', '";
    writeStringOrHex(Msg, getDwarfFormAsString(Form), Form);
    Msg << "'.";
    LibScopeError::warning(Msg.str());
  }
  return DwarfAttrValue();
}

bool DwarfReader::attrIsTrueFlag(const DwarfDie &Die, const Dwarf_Half Attr) {
  DwarfAttrValue AttrVal(
      getAttrExpectingKind(Die, Attr, DwarfAttrValueKind::Boolean));
  return (!AttrVal.empty() && AttrVal.getBool());
}

LibScopeView::AccessSpecifier
DwarfReader::getAccessSpecifier(const DwarfDie &Die) {
  DwarfAttrValue AttrVal(getAttrExpectingKind(Die, DW_AT_accessibility,
                                              DwarfAttrValueKind::Unsigned));
  if (!AttrVal.empty()) {
    switch (AttrVal.getUnsigned()) {
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
