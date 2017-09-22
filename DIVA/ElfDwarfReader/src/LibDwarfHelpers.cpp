//===-- ElfDwarfReader/LibDwarfHelpers.cpp ----------------------*- C++ -*-===//
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
/// This file contains the method definitions for the libdwarf wrapper classes
/// declared in LibDwarfHelpers.h.
///
//===----------------------------------------------------------------------===//

#include "LibDwarfHelpers.h"

#include <cstdlib>

using namespace ElfDwarfReader;

namespace {

const bool IsInfo = true;

[[noreturn]] void dwarfErrorHandler(Dwarf_Error Error, Dwarf_Ptr PtrToDbg) {
  Dwarf_Debug Dbg = *static_cast<Dwarf_Debug *>(PtrToDbg);
  throw LibDwarfError(Error, Dbg);
}

} // end anonymous namespace.

LibDwarfError::LibDwarfError(Dwarf_Error Err, Dwarf_Debug Dbg)
    : ErrorNumber(dwarf_errno(Err)), ErrorMessage(dwarf_errmsg(Err)) {
  // Dwarf_Errors have to be deallocated, and we need to do so now while Dbg is
  // still in scope.
  if (Dbg != nullptr)
    dwarf_dealloc(Dbg, Err, DW_DLA_ERROR);
  else {
    // From the libdwarf docs:
    // "An Dwarf_Error returned from dwarf_init() or dwarf_elf_init() in case of
    // a failure cannot be freed using dwarf_dealloc().The only way to free the
    // Dwarf_Error from either of those calls is to use free(3) directly.Every
    // Dwarf_Error must be freed by dwarf_dealloc() except those returned by
    // dwarf_init() or dwarf_elf_init()."
    free(Err);
  }
}

const char *LibDwarfError::what() const noexcept {
  return ErrorMessage.c_str();
}

std::string ElfDwarfReader::getDwarfTagAsString(Dwarf_Half Tag) {
  const char *Result;
  if (dwarf_get_TAG_name(Tag, &Result) == DW_DLV_OK)
    return std::string(Result);
  return std::string();
}

std::string ElfDwarfReader::getDwarfAttrAsString(Dwarf_Half Attr) {
  const char *Result;
  if (dwarf_get_AT_name(Attr, &Result) == DW_DLV_OK)
    return std::string(Result);
  return std::string();
}

std::string ElfDwarfReader::getDwarfFormAsString(Dwarf_Half Form) {
  const char *Result;
  if (dwarf_get_FORM_name(Form, &Result) == DW_DLV_OK)
    return std::string(Result);
  return std::string();
}

// DwarfDebugData methods.

DwarfDebugData::DwarfDebugData(int FileDescriptor) : Dbg(nullptr) {
  // Errors in dwarf_init occur before the handler is setup, so use the error
  // pointer interface here, and then throw the exception 'manually'.
  Dwarf_Error Err;
  int ret = dwarf_init(FileDescriptor, DW_DLC_READ, dwarfErrorHandler,
                       /*errarg*/ &Dbg, &Dbg, &Err);
  if (ret != DW_DLV_OK) {
    LibDwarfError LibErr(Err, Dbg); // Create the error before freeing Dbg.
    freeDbg();                      // If Dbg was set we need to free it.
    throw LibErr;
  }
}

DwarfDebugData::DwarfDebugData(DwarfDebugData &&Other) : Dbg(nullptr) {
  std::swap(Dbg, Other.Dbg);
}

DwarfDebugData &DwarfDebugData::operator=(DwarfDebugData &&Other) {
  if (Dbg != Other.Dbg) {
    freeDbg();
    std::swap(Dbg, Other.Dbg);
  }
  return *this;
}

void DwarfDebugData::freeDbg() {
  if (Dbg) {
    Dwarf_Error Err; // To prevent throwing a LibDwarfError.
    dwarf_finish(Dbg, &Err);
    Dbg = nullptr;
  }
}

std::vector<DwarfCompileUnit> DwarfDebugData::getCompileUnits() const {
  std::vector<DwarfCompileUnit> Result;

  Dwarf_Unsigned CurrentHeader = 0U;
  for (;;) {
    Dwarf_Unsigned NextHeader;
    int ret = dwarf_next_cu_header_d(
        Dbg, IsInfo, /*cu_header_length*/ nullptr, /*version_stamp*/ nullptr,
        /*abbrev_offset*/ nullptr, /*address_size*/ nullptr,
        /*offset_size*/ nullptr, /*extension_size*/ nullptr,
        /*signature*/ nullptr, /*typeoffse*/ nullptr, &NextHeader,
        /*header_cu_type*/ nullptr, /*error*/ nullptr);
    if (ret != DW_DLV_OK)
      break;

    // Each CU header should have a CU sibling.
    Dwarf_Die RawCUDie;
    ret = dwarf_siblingof_b(Dbg, /*die*/ nullptr, IsInfo, &RawCUDie, nullptr);
    if (ret != DW_DLV_OK)
      break;

    Result.emplace_back(DwarfDie(*this, RawCUDie));
    Result.back().HeaderOffset = CurrentHeader;
    Result.back().NextHeaderOffset = NextHeader;

    CurrentHeader = NextHeader;
  }

  return Result;
}

std::string DwarfDebugData::copyAndFreeDwarfString(char *DwarfStr) const {
  std::string Result(DwarfStr);
  dwarf_dealloc(Dbg, DwarfStr, DW_DLA_STRING);
  return Result;
}

// DwarfDie methods.

DwarfDie::DwarfDie(DwarfDie &&Other)
    : DebugData(Other.DebugData), Die(nullptr) {
  std::swap(Die, Other.Die);
}

DwarfDie &DwarfDie::operator=(DwarfDie &&Other) {
  assert(DebugData.get() == Other.DebugData.get() &&
         "Assigning DIE to DIE from other debug data.");
  if (Die != Other.Die) {
    freeDie();
    std::swap(Die, Other.Die);
  }
  return *this;
}

DwarfDieChildIterator DwarfDie::childrenBegin() const {
  return DwarfDieChildIterator(*this);
}

DwarfDieChildIterator DwarfDie::childrenEnd() {
  return DwarfDieChildIterator();
}

Dwarf_Off DwarfDie::getGlobalOffset() const {
  Dwarf_Off Offset;
  dwarf_dieoffset(Die, &Offset, nullptr);
  return Offset;
}

std::string DwarfDie::getName() const {
  char *Name;
  int ret = dwarf_diename(Die, &Name, nullptr);
  return (ret == DW_DLV_OK) ? DebugData.copyAndFreeDwarfString(Name) : "";
}

Dwarf_Half DwarfDie::getTag() const {
  Dwarf_Half Result;
  dwarf_tag(Die, &Result, nullptr);
  return Result;
}

std::string DwarfDie::getTagName() const {
  const char *TagName;
  int ret = dwarf_get_TAG_name(getTag(), &TagName);
  return (ret == DW_DLV_OK) ? TagName : "";
}

DwarfLineTable DwarfDie::getLineTable() const { return DwarfLineTable(*this); }

void DwarfDie::freeDie() {
  if (Die) {
    dwarf_dealloc(*DebugData, Die, DW_DLA_DIE);
    Die = nullptr;
  }
}

bool DwarfDie::hasAttr(Dwarf_Half Attr) const {
  Dwarf_Bool Result;
  dwarf_hasattr(Die, Attr, &Result, nullptr);
  return Result != 0;
}

DwarfAttrValue DwarfDie::getAttr(Dwarf_Half Attr) const {
  // Get attr.
  Dwarf_Attribute Attribute;
  int ret = dwarf_attr(Die, Attr, &Attribute, nullptr);
  if (ret != DW_DLV_OK)
    return DwarfAttrValue(); // Empty.

  Dwarf_Half Form;
  dwarf_whatform(Attribute, &Form, nullptr);
  switch (Form) {
  case DW_FORM_ref_addr:
  case DW_FORM_ref1:
  case DW_FORM_ref2:
  case DW_FORM_ref4:
  case DW_FORM_ref8:
  case DW_FORM_ref_udata:
  case DW_FORM_ref_sig8:
  case DW_FORM_sec_offset: {
    Dwarf_Off Reference;
    dwarf_global_formref(Attribute, &Reference, nullptr);
    return DwarfAttrValue(Reference, DwarfAttrValueKind::Reference, Form);
  }
  case DW_FORM_addr:
  case DW_FORM_addrx:
  case DW_FORM_GNU_addr_index: {
    Dwarf_Addr Address;
    dwarf_formaddr(Attribute, &Address, nullptr);
    return DwarfAttrValue(Address, DwarfAttrValueKind::Address, Form);
  }
  case DW_FORM_flag:
  case DW_FORM_flag_present: {
    Dwarf_Bool Boolean;
    dwarf_formflag(Attribute, &Boolean, nullptr);
    return DwarfAttrValue(Boolean, Form);
  }
  case DW_FORM_data1:
  case DW_FORM_data2:
  case DW_FORM_data4:
  case DW_FORM_data8:
  case DW_FORM_udata:
  case DW_FORM_implicit_const: {
    Dwarf_Unsigned Unsigned;
    dwarf_formudata(Attribute, &Unsigned, nullptr);
    return DwarfAttrValue(Unsigned, DwarfAttrValueKind::Unsigned, Form);
  }
  case DW_FORM_sdata: {
    Dwarf_Signed Signed;
    dwarf_formsdata(Attribute, &Signed, nullptr);
    return DwarfAttrValue(Signed, Form);
  }
  case DW_FORM_block:
  case DW_FORM_block1:
  case DW_FORM_block2:
  case DW_FORM_block4: {
    Dwarf_Block *Blocks;
    dwarf_formblock(Attribute, &Blocks, nullptr);
    uint8_t *Data = reinterpret_cast<uint8_t *>(Blocks->bl_data);
    DwarfAttrValue Result(std::vector<uint8_t>(Data, Data + Blocks->bl_len),
                          DwarfAttrValueKind::Bytes, Form);

    dwarf_dealloc(DebugData.get(), Blocks, DW_DLA_BLOCK);
    return Result;
  }
  case DW_FORM_exprloc: {
    Dwarf_Unsigned ExprLen;
    Dwarf_Ptr Blocks;
    dwarf_formexprloc(Attribute, &ExprLen, &Blocks, nullptr);
    uint8_t *Data = reinterpret_cast<uint8_t *>(Blocks);
    return DwarfAttrValue(std::vector<uint8_t>(Data, Data + ExprLen),
                          DwarfAttrValueKind::Exprloc, Form);
  }
  case DW_FORM_string:
  case DW_FORM_strp:
  case DW_FORM_strp_sup:
  case DW_FORM_strx:
  case DW_FORM_GNU_strp_alt:
  case DW_FORM_GNU_str_index: {
    char *Str;
    dwarf_formstring(Attribute, &Str, nullptr);
    return DwarfAttrValue(DebugData.copyAndFreeDwarfString(Str), Form);
  }
  default:
    return DwarfAttrValue(Form); // Unknown Form.
  }
}

// DwarfDieChildIterator methods.

DwarfDieChildIterator::DwarfDieChildIterator(const DwarfDie &Parent) {
  Dwarf_Die RawChildDie;
  int ret = dwarf_child(*Parent, &RawChildDie, nullptr);
  if (ret == DW_DLV_OK)
    Child = std::make_shared<DwarfDie>(Parent.DebugData, RawChildDie);
}

DwarfDieChildIterator &DwarfDieChildIterator::operator++() {
  assert(Child && "Incremented end DwarfDieChildIterator");
  if (Child) {
    Dwarf_Die RawChildDie;
    int ret = dwarf_siblingof_b(Child->DebugData.get(), **Child, IsInfo,
                                &RawChildDie, nullptr);
    if (ret == DW_DLV_OK)
      Child = std::make_shared<DwarfDie>(Child->DebugData, RawChildDie);
    else
      Child = nullptr;
  }
  return *this;
}

// DwarfAttrValue methods.

DwarfAttrValue::DwarfAttrValue() : Kind(DwarfAttrValueKind::Empty) {}

DwarfAttrValue::DwarfAttrValue(const DwarfAttrValue &Other) {
  Kind = Other.Kind;
  Form = Other.Form;
  switch (Kind) {
  case DwarfAttrValueKind::Empty:
  case DwarfAttrValueKind::UnknownForm:
    break;
  case DwarfAttrValueKind::Reference:
    Value.Reference = Other.Value.Reference;
    break;
  case DwarfAttrValueKind::Address:
    Value.Address = Other.Value.Address;
    break;
  case DwarfAttrValueKind::Boolean:
    Value.Boolean = Other.Value.Boolean;
    break;
  case DwarfAttrValueKind::Unsigned:
    Value.Unsigned = Other.Value.Unsigned;
    break;
  case DwarfAttrValueKind::Signed:
    Value.Signed = Other.Value.Signed;
    break;
  case DwarfAttrValueKind::Bytes:
    new (&Value.Bytes) std::vector<uint8_t>(Other.Value.Bytes);
    break;
  case DwarfAttrValueKind::Exprloc:
    new (&Value.Exprloc) std::vector<uint8_t>(Other.Value.Exprloc);
    break;
  case DwarfAttrValueKind::String:
    new (&Value.String) std::string(Other.Value.String);
    break;
  }
}

DwarfAttrValue::DwarfAttrValue(DwarfAttrValue &&Other) {
  Kind = Other.Kind;
  Form = Other.Form;
  switch (Kind) {
  case DwarfAttrValueKind::Empty:
  case DwarfAttrValueKind::UnknownForm:
    break;
  case DwarfAttrValueKind::Reference:
    Value.Reference = Other.Value.Reference;
    break;
  case DwarfAttrValueKind::Address:
    Value.Address = Other.Value.Address;
    break;
  case DwarfAttrValueKind::Boolean:
    Value.Boolean = Other.Value.Boolean;
    break;
  case DwarfAttrValueKind::Unsigned:
    Value.Unsigned = Other.Value.Unsigned;
    break;
  case DwarfAttrValueKind::Signed:
    Value.Signed = Other.Value.Signed;
    break;
  case DwarfAttrValueKind::Bytes:
    new (&Value.Bytes) std::vector<uint8_t>(std::move(Other.Value.Bytes));
    break;
  case DwarfAttrValueKind::Exprloc:
    new (&Value.Exprloc) std::vector<uint8_t>(std::move(Other.Value.Exprloc));
    break;
  case DwarfAttrValueKind::String:
    new (&Value.String) std::string(std::move(Other.Value.String));
    break;
  }
}

DwarfAttrValue &DwarfAttrValue::operator=(const DwarfAttrValue &Other) {
  destroyValue();

  Kind = Other.Kind;
  Form = Other.Form;
  switch (Kind) {
  case DwarfAttrValueKind::Empty:
  case DwarfAttrValueKind::UnknownForm:
    break;
  case DwarfAttrValueKind::Reference:
    Value.Reference = Other.Value.Reference;
    break;
  case DwarfAttrValueKind::Address:
    Value.Address = Other.Value.Address;
    break;
  case DwarfAttrValueKind::Boolean:
    Value.Boolean = Other.Value.Boolean;
    break;
  case DwarfAttrValueKind::Unsigned:
    Value.Unsigned = Other.Value.Unsigned;
    break;
  case DwarfAttrValueKind::Signed:
    Value.Signed = Other.Value.Signed;
    break;
  case DwarfAttrValueKind::Bytes:
    new (&Value.Bytes) std::vector<uint8_t>(Other.Value.Bytes);
    break;
  case DwarfAttrValueKind::Exprloc:
    new (&Value.Exprloc) std::vector<uint8_t>(Other.Value.Exprloc);
    break;
  case DwarfAttrValueKind::String:
    new (&Value.String) std::string(Other.Value.String);
    break;
  }
  return *this;
}

DwarfAttrValue &DwarfAttrValue::operator=(DwarfAttrValue &&Other) {
  destroyValue();

  Kind = Other.Kind;
  Form = Other.Form;
  switch (Kind) {
  case DwarfAttrValueKind::Empty:
  case DwarfAttrValueKind::UnknownForm:
    break;
  case DwarfAttrValueKind::Reference:
    Value.Reference = Other.Value.Reference;
    break;
  case DwarfAttrValueKind::Address:
    Value.Address = Other.Value.Address;
    break;
  case DwarfAttrValueKind::Boolean:
    Value.Boolean = Other.Value.Boolean;
    break;
  case DwarfAttrValueKind::Unsigned:
    Value.Unsigned = Other.Value.Unsigned;
    break;
  case DwarfAttrValueKind::Signed:
    Value.Signed = Other.Value.Signed;
    break;
  case DwarfAttrValueKind::Bytes:
    new (&Value.Bytes) std::vector<uint8_t>(std::move(Other.Value.Bytes));
    break;
  case DwarfAttrValueKind::Exprloc:
    new (&Value.Exprloc) std::vector<uint8_t>(std::move(Other.Value.Exprloc));
    break;
  case DwarfAttrValueKind::String:
    new (&Value.String) std::string(std::move(Other.Value.String));
    break;
  }
  return *this;
}

Dwarf_Off DwarfAttrValue::getReference() const {
  assert(Kind == DwarfAttrValueKind::Reference);
  return Value.Reference;
}

Dwarf_Addr DwarfAttrValue::getAddress() const {
  assert(Kind == DwarfAttrValueKind::Address);
  return Value.Address;
}

Dwarf_Bool DwarfAttrValue::getBool() const {
  assert(Kind == DwarfAttrValueKind::Boolean);
  return Value.Boolean;
}

Dwarf_Unsigned DwarfAttrValue::getUnsigned() const {
  assert(Kind == DwarfAttrValueKind::Unsigned);
  return Value.Unsigned;
}

Dwarf_Signed DwarfAttrValue::getSigned() const {
  assert(Kind == DwarfAttrValueKind::Signed);
  return Value.Signed;
}

const std::vector<uint8_t> &DwarfAttrValue::getBytes() const {
  assert(Kind == DwarfAttrValueKind::Bytes);
  return Value.Bytes;
}

const std::vector<uint8_t> &DwarfAttrValue::getExprloc() const {
  assert(Kind == DwarfAttrValueKind::Exprloc);
  return Value.Exprloc;
}

const std::string &DwarfAttrValue::getString() const {
  assert(Kind == DwarfAttrValueKind::String);
  return Value.String;
}

DwarfAttrValue::DwarfAttrValue(Dwarf_Half ValForm)
    : Kind(DwarfAttrValueKind::UnknownForm), Form(ValForm) {}

DwarfAttrValue::DwarfAttrValue(Dwarf_Bool Val, Dwarf_Half ValForm)
    : Kind(DwarfAttrValueKind::Boolean), Form(ValForm) {
  Value.Boolean = Val;
}

DwarfAttrValue::DwarfAttrValue(Dwarf_Signed Val, Dwarf_Half ValForm)
    : Kind(DwarfAttrValueKind::Signed), Form(ValForm) {
  Value.Signed = Val;
}

DwarfAttrValue::DwarfAttrValue(std::vector<uint8_t> &&Val,
                               DwarfAttrValueKind ValKind, Dwarf_Half ValForm)
    : Kind(ValKind), Form(ValForm) {
  switch (Kind) {
  case DwarfAttrValueKind::Bytes:
    new (&Value.Bytes) std::vector<uint8_t>(std::move(Val));
    break;
  case DwarfAttrValueKind::Exprloc:
    new (&Value.Exprloc) std::vector<uint8_t>(std::move(Val));
    break;
  case DwarfAttrValueKind::UnknownForm:
  case DwarfAttrValueKind::Boolean:
  case DwarfAttrValueKind::Empty:
  case DwarfAttrValueKind::Reference:
  case DwarfAttrValueKind::Address:
  case DwarfAttrValueKind::Unsigned:
  case DwarfAttrValueKind::Signed:
  case DwarfAttrValueKind::String:
    assert(false && "Bad DwarfAttrValueKind");
  }
}

DwarfAttrValue::DwarfAttrValue(std::string &&Val, Dwarf_Half ValForm)
    : Kind(DwarfAttrValueKind::String), Form(ValForm) {
  new (&Value.String) std::string(std::move(Val));
}

DwarfAttrValue::DwarfAttrValue(Dwarf_Unsigned Val, DwarfAttrValueKind ValKind,
                               Dwarf_Half ValForm)
    : Kind(ValKind), Form(ValForm) {
  switch (Kind) {
  case DwarfAttrValueKind::Reference:
    Value.Reference = Val;
    break;
  case DwarfAttrValueKind::Address:
    Value.Address = Val;
    break;
  case DwarfAttrValueKind::Unsigned:
    Value.Unsigned = Val;
    break;
  case DwarfAttrValueKind::Empty:
  case DwarfAttrValueKind::UnknownForm:
  case DwarfAttrValueKind::Boolean:
  case DwarfAttrValueKind::Signed:
  case DwarfAttrValueKind::Bytes:
  case DwarfAttrValueKind::Exprloc:
  case DwarfAttrValueKind::String:
    assert(false && "Bad DwarfAttrValueKind");
  }
}

void DwarfAttrValue::destroyValue() {
  switch (Kind) {
  case DwarfAttrValueKind::Empty:
  case DwarfAttrValueKind::UnknownForm:
  case DwarfAttrValueKind::Reference:
  case DwarfAttrValueKind::Address:
  case DwarfAttrValueKind::Boolean:
  case DwarfAttrValueKind::Unsigned:
  case DwarfAttrValueKind::Signed:
    break;
  case DwarfAttrValueKind::Bytes:
    Value.Bytes.~vector();
    break;
  case DwarfAttrValueKind::Exprloc:
    Value.Exprloc.~vector();
    break;
  case DwarfAttrValueKind::String:
    Value.String.~basic_string();
    break;
  }
  Kind = DwarfAttrValueKind::Empty;
}

// DwarfLineTable methods.

DwarfLineTable::DwarfLineTable(const DwarfDie &CU) {
  assert(CU.getTag() == DW_TAG_compile_unit &&
         "getLineTable is only valid on compile units");

  Dwarf_Unsigned Version;
  Dwarf_Small TableCount;
  auto Ret = dwarf_srclines_b(*CU, &Version, &TableCount, &Context, nullptr);

  assert(TableCount <= 1 && "Multiline table is not supported");
  if (Ret != DW_DLV_OK || TableCount != 1) {
    freeLines();
    return;
  }

  Dwarf_Signed SLineCount;
  Ret = dwarf_srclines_from_linecontext(Context, &Lines, &SLineCount, nullptr);
  if (Ret != DW_DLV_OK) {
    freeLines();
    return;
  }

  LineCount = SLineCount < 0 ? 0U : static_cast<size_t>(SLineCount);
}

DwarfLineTable::DwarfLineTable(DwarfLineTable &&Other)
    : Context(nullptr), Lines(nullptr), LineCount(0U) {
  swap(*this, Other);
}

DwarfLineTable &DwarfLineTable::operator=(DwarfLineTable &&Other) {
  if (this != &Other) {
    freeLines();
    swap(*this, Other);
  }
  return *this;
}

DwarfLineEntry DwarfLineTable::getLine(size_t LineIndex) const {
  assert(LineIndex < size() && "Index out of range");
  Dwarf_Line Line = Lines[LineIndex];

  DwarfLineEntry Result;
  dwarf_lineno(Line, &Result.LineNo, nullptr);
  dwarf_line_srcfileno(Line, &Result.SrcFileID, nullptr);
  dwarf_lineaddr(Line, &Result.LineAddr, nullptr);

  dwarf_linebeginstatement(Line, &Result.IsBeginStatement, nullptr);
  dwarf_lineendsequence(Line, &Result.IsEndSequence, nullptr);
  dwarf_lineblock(Line, &Result.IsBeginBlock, nullptr);
  dwarf_prologue_end_etc(Line, &Result.IsPrologEnd, &Result.IsEpilogueBegin,
                         &Result.ISA, &Result.Discriminator, nullptr);

  return Result;
}

void DwarfLineTable::freeLines() {
  if (Context)
    dwarf_srclines_dealloc_b(Context);
  Context = nullptr;
  Lines = nullptr;
  LineCount = 0U;
}
