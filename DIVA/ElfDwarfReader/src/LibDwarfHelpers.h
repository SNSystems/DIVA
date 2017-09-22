//===-- ElfDwarfReader/LibDwarfHelpers.h ------------------------*- C++ -*-===//
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
/// This file contains the declaration of several classes that wrap libdwarf
/// functionality in a more c++ style (RAII).
///
//===----------------------------------------------------------------------===//

#ifndef LIB_DWARF_HELPERS_H
#define LIB_DWARF_HELPERS_H

// Disable some clang warnings for dwarf.h and libdwarf.h.
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#pragma clang diagnostic ignored "-Wundef"
#endif

#include "dwarf.h"
#include "libdwarf.h"

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#include <assert.h>
#include <exception>
#include <memory>
#include <string>
#include <vector>

namespace ElfDwarfReader {

std::string getDwarfTagAsString(Dwarf_Half Tag);
std::string getDwarfAttrAsString(Dwarf_Half Attr);
std::string getDwarfFormAsString(Dwarf_Half Form);

struct DwarfCompileUnit;
class DwarfDie;
class DwarfDieChildIterator;
class DwarfAttrValue;
class DwarfLineTable;

/// \brief Exception wrapping a LibDwarf error code.
class LibDwarfError : public std::exception {
public:
  LibDwarfError(Dwarf_Error Err, Dwarf_Debug Dbg);

  Dwarf_Unsigned getErrorNumber() const { return ErrorNumber; }
  const std::string &getErrorMessage() const { return ErrorMessage; }

private:
  Dwarf_Unsigned ErrorNumber;
  std::string ErrorMessage;

  const char *what() const noexcept override;
};

/// \brief Wrapper around a Dwarf_Debug with resource management.
class DwarfDebugData {
public:
  DwarfDebugData() : Dbg(nullptr) {}
  explicit DwarfDebugData(int FileDescriptor);
  explicit DwarfDebugData(DwarfDebugData &&Other);
  ~DwarfDebugData() { freeDbg(); }

  DwarfDebugData &operator=(DwarfDebugData &&Other);

  DwarfDebugData(const DwarfDebugData &) = delete;
  DwarfDebugData &operator=(const DwarfDebugData &) = delete;

  /// \brief Return true if there is no debug data.
  bool empty() const { return Dbg == nullptr; }

  /// \brief Get the wrapped Dwarf_Debug instance.
  Dwarf_Debug get() const { return Dbg; }
  /// \brief Get the wrapped Dwarf_Debug instance.
  Dwarf_Debug operator*() const { return Dbg; }

  /// \brief Get all the compile units in the debug data.
  std::vector<DwarfCompileUnit> getCompileUnits() const;

  /// \brief Return a copy of a libdwarf c string and then free the libdwarf
  /// memory.
  std::string copyAndFreeDwarfString(char *DwarfStr) const;

private:
  // Free Dbg and set it to nullptr.
  void freeDbg();

  Dwarf_Debug Dbg;
};

/// \brief Wrapper around a Dwarf_Die with resource management.
class DwarfDie {
public:
  friend class DwarfDieChildIterator;

  explicit DwarfDie(const DwarfDebugData &DbgData, Dwarf_Die RawDie)
      : DebugData(DbgData), Die(RawDie) {}
  DwarfDie(DwarfDie &&Other);
  ~DwarfDie() { freeDie(); }

  DwarfDie &operator=(DwarfDie &&Other);

  DwarfDie(const DwarfDie &) = delete;
  DwarfDie &operator=(const DwarfDie &) = delete;

  /// \brief get the wrapped Dwarf_Die instance.
  Dwarf_Die get() const { return Die; }
  /// \brief get the wrapped Dwarf_Die instance.
  Dwarf_Die operator*() const { return Die; }

  DwarfDieChildIterator childrenBegin() const;
  static DwarfDieChildIterator childrenEnd();

  Dwarf_Off getGlobalOffset() const;
  std::string getName() const;
  Dwarf_Half getTag() const;
  std::string getTagName() const;

  // Attribute getters.
  bool hasAttr(Dwarf_Half Attr) const;
  DwarfAttrValue getAttr(Dwarf_Half Attr) const;

  /// \brief get the line table. Only valid for compile units.
  DwarfLineTable getLineTable() const;

private:
  // Free Die and set it to nullptr.
  void freeDie();

  const DwarfDebugData &DebugData;
  Dwarf_Die Die;
};

/// \brief Container for the CU Die and its metadata.
struct DwarfCompileUnit {
  DwarfCompileUnit(DwarfDie &&CompileUnitDie)
      : CUDie(std::move(CompileUnitDie)), HeaderOffset(0), NextHeaderOffset(0) {
  }
  DwarfDie CUDie;
  Dwarf_Off HeaderOffset;
  Dwarf_Off NextHeaderOffset;
};

/// \brief Access all a DIE's children in sequence.
///
/// Typical usage:
/// \code
///   for (auto IT = Die.childrenBegin(), End = Die.childrenEnd();
///        IT != End; ++IT) {
///     auto Tag = IT->getTag();
///     // ...
///   }
/// \endcode
class DwarfDieChildIterator {
public:
  DwarfDieChildIterator() = default;
  DwarfDieChildIterator(const DwarfDie &Parent);

  const DwarfDie &operator*() const { return *Child; }
  const DwarfDie *operator->() const { return Child.get(); }

  friend bool operator==(const DwarfDieChildIterator &a,
                         const DwarfDieChildIterator &b) {
    return a.Child == b.Child;
  }
  friend bool operator!=(const DwarfDieChildIterator &a,
                         const DwarfDieChildIterator &b) {
    return !(a == b);
  }

  DwarfDieChildIterator &operator++();
  void operator++(int) { ++(*this); }

  bool atEnd() const { return Child == nullptr; }

private:
  std::shared_ptr<DwarfDie> Child;
};

/// \brief the kind of an attribute value.
enum class DwarfAttrValueKind {
  Empty,
  UnknownForm,
  Reference,
  Address,
  Boolean,
  Unsigned,
  Signed,
  Bytes,
  Exprloc,
  String,
};

/// \brief Discriminated union for the values of DWARF attributes.
///
/// Can be 'Empty' signifying there was no attribute, or 'UnknownForm' where
/// the DWARF form for the attribtue is not supported.
class DwarfAttrValue {
public:
  DwarfAttrValue(); // Empty.
  ~DwarfAttrValue() { destroyValue(); }

  DwarfAttrValue(const DwarfAttrValue &Other);
  DwarfAttrValue(DwarfAttrValue &&Other);

  DwarfAttrValue &operator=(const DwarfAttrValue &Other);
  DwarfAttrValue &operator=(DwarfAttrValue &&Other);

  bool empty() const { return Kind == DwarfAttrValueKind::Empty; }
  DwarfAttrValueKind getKind() const { return Kind; }
  Dwarf_Half getForm() const {
    assert(!empty());
    return Form;
  }

  Dwarf_Off getReference() const;
  Dwarf_Addr getAddress() const;
  Dwarf_Bool getBool() const;
  Dwarf_Unsigned getUnsigned() const;
  Dwarf_Signed getSigned() const;
  const std::vector<uint8_t> &getBytes() const;
  const std::vector<uint8_t> &getExprloc() const;
  const std::string &getString() const;

private:
  friend class DwarfDie;

  explicit DwarfAttrValue(Dwarf_Half Form); // Unknown Form.
  explicit DwarfAttrValue(Dwarf_Bool Val, Dwarf_Half Form);
  explicit DwarfAttrValue(Dwarf_Signed Val, Dwarf_Half Form);
  explicit DwarfAttrValue(std::string &&Val, Dwarf_Half Form);

  // Reference, Address and Unsigned have the same underlying type.
  explicit DwarfAttrValue(Dwarf_Unsigned Val, DwarfAttrValueKind ValKind,
                          Dwarf_Half Form);

  // Bytes and Exprloc have the same underlying type.
  explicit DwarfAttrValue(std::vector<uint8_t> &&Val,
                          DwarfAttrValueKind ValKind, Dwarf_Half Form);

  void destroyValue();

  union ValueUnion {
    Dwarf_Off Reference;
    Dwarf_Addr Address;
    Dwarf_Bool Boolean;
    Dwarf_Unsigned Unsigned;
    Dwarf_Signed Signed;
    std::vector<uint8_t> Bytes;
    std::vector<uint8_t> Exprloc;
    std::string String;

    ValueUnion() {}
    ~ValueUnion() {}
  };

  DwarfAttrValueKind Kind;
  Dwarf_Half Form;
  ValueUnion Value;
};

struct DwarfLineEntry {
  Dwarf_Unsigned LineNo;
  Dwarf_Unsigned SrcFileID;
  Dwarf_Addr LineAddr;

  Dwarf_Bool IsBeginStatement;
  Dwarf_Bool IsEndSequence;
  Dwarf_Bool IsBeginBlock;
  Dwarf_Bool IsPrologEnd;
  Dwarf_Bool IsEpilogueBegin;
  Dwarf_Unsigned ISA;
  Dwarf_Unsigned Discriminator;
};

/// Wrapper around a line table with memory management.
class DwarfLineTable {
public:
  explicit DwarfLineTable(const DwarfDie &CU);
  DwarfLineTable(DwarfLineTable &&Other);
  ~DwarfLineTable() { freeLines(); }

  DwarfLineTable(const DwarfLineTable &) = delete;
  DwarfLineTable &operator=(const DwarfLineTable &) = delete;

  DwarfLineTable &operator=(DwarfLineTable &&Other);

  friend void swap(DwarfLineTable &A, DwarfLineTable &B) {
    std::swap(A.Context, B.Context);
    std::swap(A.Lines, B.Lines);
    std::swap(A.LineCount, B.LineCount);
  }

  bool empty() const { return LineCount == 0; }
  size_t size() const { return LineCount; }

  DwarfLineEntry getLine(size_t LineIndex) const;
  DwarfLineEntry operator[](size_t LineIndex) const {
    return getLine(LineIndex);
  }

private:
  // Free lines and reset values to null/zero.
  void freeLines();

  Dwarf_Line_Context Context;
  Dwarf_Line *Lines;
  size_t LineCount;
};

} // end namespace ElfDwarfReader

#endif // LIB_DWARF_HELPERS_H
