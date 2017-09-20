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

// DwarfDebugData methods.

DwarfDebugData::DwarfDebugData(int FileDescriptor) : Dbg(nullptr) {
  // Errors in dwarf_init occur before the handler is setup, so use the error
  // pointer interface here, and then throw the exception 'manually'.
  Dwarf_Error Err;
  int ret = dwarf_init(FileDescriptor, DW_DLC_READ, dwarfErrorHandler,
                       /*errarg*/ &Dbg, &Dbg, &Err);
  if (ret != DW_DLV_OK) {
    LibDwarfError LibErr(Err, Dbg); // Create the error before freeing Dbg.
    freeDbg(); // If Dbg was set we need to free it.
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

#define GET_ATTR(TYPE, GETTER_FUNC, ATTR)                                      \
  getAttr<TYPE, decltype(GETTER_FUNC)>(ATTR, GETTER_FUNC)

bool DwarfDie::getAttrAsFlag(Dwarf_Half Attr) const {
  auto Ret = GET_ATTR(Dwarf_Bool, dwarf_formflag, Attr);
  return Ret.hasValue() && Ret.getValue() != 0;
}
const DwarfDie::OptionalAttrValue<Dwarf_Addr>
DwarfDie::getAttrAsAddr(Dwarf_Half Attr) const {
  return GET_ATTR(Dwarf_Addr, dwarf_formaddr, Attr);
}
const DwarfDie::OptionalAttrValue<Dwarf_Off>
DwarfDie::getAttrAsRef(Dwarf_Half Attr) const {
  return GET_ATTR(Dwarf_Off, dwarf_global_formref, Attr);
}
const DwarfDie::OptionalAttrValue<Dwarf_Unsigned>

DwarfDie::getAttrAsUnsigned(Dwarf_Half Attr) const {
  return GET_ATTR(Dwarf_Unsigned, dwarf_formudata, Attr);
}
const DwarfDie::OptionalAttrValue<Dwarf_Signed>
DwarfDie::getAttrAsSigned(Dwarf_Half Attr) const {
  return GET_ATTR(Dwarf_Signed, dwarf_formsdata, Attr);
}
const DwarfDie::OptionalAttrValue<std::string>
DwarfDie::getAttrAsString(Dwarf_Half Attr) const {
  if (auto Ret = GET_ATTR(char *, dwarf_formstring, Attr))
    return OptionalAttrValue<std::string>(
        DebugData.copyAndFreeDwarfString(*Ret));
  return OptionalAttrValue<std::string>();
}

#undef GET_ATTR

const DwarfDie::OptionalAttrValue<DwarfDie::SignedUnsigned>
DwarfDie::getAttrAsSignedOrUnsigned(Dwarf_Half Attr) const {
  // Get attr.
  Dwarf_Attribute Attribute;
  int ret = dwarf_attr(Die, Attr, &Attribute, nullptr);
  if (ret != DW_DLV_OK)
    return OptionalAttrValue<SignedUnsigned>();
  SignedUnsigned Result;
  // Check sign.
  Dwarf_Bool IsSigned;
  dwarf_hasform(Attribute, DW_FORM_sdata, &IsSigned, nullptr);
  Result.IsSigned = (IsSigned != 0);
  // Get value.
  if (Result.IsSigned)
    dwarf_formsdata(Attribute, &Result.SignedValue, nullptr);
  else
    dwarf_formudata(Attribute, &Result.UnsignedValue, nullptr);
  dwarf_dealloc(*DebugData, Attribute, DW_DLA_ATTR);
  return OptionalAttrValue<SignedUnsigned>(Result);
}

Dwarf_Addr DwarfDie::getAttrAsAddr(Dwarf_Half Attr, Dwarf_Addr Default) const {
  if (auto Ret = getAttrAsAddr(Attr))
    return *Ret;
  return Default;
}
Dwarf_Off DwarfDie::getAttrAsRef(Dwarf_Half Attr, Dwarf_Off Default) const {
  if (auto Ret = getAttrAsRef(Attr))
    return *Ret;
  return Default;
}
Dwarf_Unsigned DwarfDie::getAttrAsUnsigned(Dwarf_Half Attr,
                                           Dwarf_Unsigned Default) const {
  if (auto Ret = getAttrAsUnsigned(Attr))
    return *Ret;
  return Default;
}
Dwarf_Signed DwarfDie::getAttrAsSigned(Dwarf_Half Attr,
                                       Dwarf_Signed Default) const {
  if (auto Ret = getAttrAsSigned(Attr))
    return *Ret;
  return Default;
}
std::string DwarfDie::getAttrAsString(Dwarf_Half Attr,
                                      const std::string &Default) const {
  if (auto Ret = getAttrAsString(Attr))
    return *Ret;
  return Default;
}

// Common functionality for attribute getters.
//
// Template Params
//   ValTy: Type of the attribute value being retrieved
//   getterFunc: int(*)(Dwarf_Attribute, T*, Dwarf_Error *)
//
// This template function:
//   - gets the attribute with tag Attr
//   - If the attribute was not found returns an empty optional
//   - gets the value of the attribute (type T) with the getter
//   - Frees the attribute memory
//   - Returns an optional containing the value
template <typename ValTy, typename getterFunc>
DwarfDie::OptionalAttrValue<ValTy> DwarfDie::getAttr(Dwarf_Half Attr,
                                                     getterFunc getter) const {
  Dwarf_Attribute Attribute;
  int Ret = dwarf_attr(Die, Attr, &Attribute, nullptr);
  if (Ret != DW_DLV_OK)
    return OptionalAttrValue<ValTy>();
  ValTy Result;
  getter(Attribute, &Result, nullptr);
  dwarf_dealloc(*DebugData, Attribute, DW_DLA_ATTR);
  return OptionalAttrValue<ValTy>(Result);
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

DwarfAttrValue::~DwarfAttrValue() {
  switch (Kind) {
  case ValueKind::Empty:
  case ValueKind::UnknownForm:
  case ValueKind::Offset:
  case ValueKind::Address:
  case ValueKind::Boolean:
  case ValueKind::Unsigned:
  case ValueKind::Signed:
    break;
  case ValueKind::Bytes:
    Value.Bytes.~vector();
    break;
  case ValueKind::String:
    Value.String.~basic_string();
    break;
  }
}

Dwarf_Half DwarfAttrValue::getUnknownForm() const {
  assert(Kind == ValueKind::UnknownForm);
  return Value.UnknownForm;
}

Dwarf_Off DwarfAttrValue::getOffset() const {
  assert(Kind == ValueKind::Offset);
  return Value.Offset;
}

Dwarf_Addr DwarfAttrValue::getAddress() const {
  assert(Kind == ValueKind::Offset);
  return Value.Address;
}

Dwarf_Bool DwarfAttrValue::getBool() const {
  assert(Kind == ValueKind::Offset);
  return Value.Boolean;
}

Dwarf_Unsigned DwarfAttrValue::getUnsigned() const {
  assert(Kind == ValueKind::Offset);
  return Value.Unsigned;
}

Dwarf_Signed DwarfAttrValue::getSigned() const {
  assert(Kind == ValueKind::Offset);
  return Value.Signed;
}

const std::vector<uint8_t> &DwarfAttrValue::getBytes() const {
  assert(Kind == ValueKind::Offset);
  return Value.Bytes;
}

const std::string &DwarfAttrValue::getString() const {
  assert(Kind == ValueKind::Offset);
  return Value.String;
}

DwarfAttrValue::DwarfAttrValue() : Kind(ValueKind::Empty) {}

DwarfAttrValue::DwarfAttrValue(Dwarf_Half Val) : Kind(ValueKind::UnknownForm) {
  Value.UnknownForm = Val;
}

DwarfAttrValue::DwarfAttrValue(Dwarf_Bool Val) : Kind(ValueKind::Boolean) {
  Value.Boolean = Val;
}

DwarfAttrValue::DwarfAttrValue(Dwarf_Signed Val) : Kind(ValueKind::Signed) {
  Value.Signed = Val;
}

DwarfAttrValue::DwarfAttrValue(std::vector<uint8_t> &&Val)
    : Kind(ValueKind::Bytes) {
  new (&Value.Bytes) std::vector<uint8_t>(std::move(Val));
}

DwarfAttrValue::DwarfAttrValue(std::string &&Val) : Kind(ValueKind::String) {
  new (&Value.String) std::string(std::move(Val));
}

DwarfAttrValue::DwarfAttrValue(Dwarf_Unsigned Val, ValueKind ValKind)
    : Kind(ValKind) {
  switch (Kind) {
  case ValueKind::Offset:
    Value.Offset = Val;
    break;
  case ValueKind::Address:
    Value.Address = Val;
    break;
  case ValueKind::Unsigned:
    Value.Unsigned = Val;
    break;
  case ValueKind::Empty:
  case ValueKind::UnknownForm:
  case ValueKind::Boolean:
  case ValueKind::Signed:
  case ValueKind::Bytes:
  case ValueKind::String:
    assert(false && "Bad ValueKind");
  }
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
