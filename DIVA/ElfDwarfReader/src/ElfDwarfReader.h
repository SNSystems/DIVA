//===-- ElfDwarfReader/ElfDwarfReader.h -------------------------*- C++ -*-===//
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
/// This file contains a derivative of LibScopeView::Reader that builds a
/// scope tree from an object file in the ELF format with DWARF debug
/// information.
///
//===----------------------------------------------------------------------===//

#ifndef ELF_DWARF_READER_H
#define ELF_DWARF_READER_H

#include "Reader.h"

#include <set>
#include <unordered_map>
#include <unordered_set>

namespace ElfDwarfReader {

class DwarfDebugData;
class DwarfDie;
class DwarfAttrValue;
enum class DwarfAttrValueKind;

class DwarfReader : public LibScopeView::Reader {
public:
  DwarfReader() : LibScopeView::Reader() {}
  ~DwarfReader() override {}

  DwarfReader(const DwarfReader &) = delete;
  DwarfReader &operator=(const DwarfReader &) = delete;

private:
  /// Create the full scope tree.
  bool createScopes() override;

  /// Create each compile unit.
  void createCompileUnits(const DwarfDebugData &DebugData,
                          LibScopeView::ScopeRoot &Root);

  /// Create a LibScopeView::Object from a Die and then recursivly create its
  /// children.
  void createObject(const DwarfDebugData &DebugData, const DwarfDie &Die,
                    LibScopeView::Object &ParentObj);

  /// Create the appropriate subclass of LibScopeView::Object for the given
  /// DWARF tag.
  LibScopeView::Object *createObjectByTag(Dwarf_Half Tag);

  /// setup the objects state from attributes on the DWARF Die.
  void initObjectFromAttrs(LibScopeView::Object &Obj, const DwarfDie &Die,
                           Dwarf_Off ObjOffset, Dwarf_Half ObjTag);

  void initScopeFromAttrs(LibScopeView::Scope &Scp, const DwarfDie &Die);
  void initTypeFromAttrs(LibScopeView::Type &Ty, const DwarfDie &Die);
  void initSymbolFromAttrs(LibScopeView::Symbol &Sym, const DwarfDie &Die);

  /// Create all the lines in a compile unit.
  void createLines(const DwarfDie &CUDie,
                   LibScopeView::ScopeCompileUnit &CUObj);

  /// Setup any references from this object to other objects.
  ///
  /// If the other object doesn't exist yet, then record that this reference
  /// needs to be updated when the other object is created.
  void initObjectReferences(LibScopeView::Object &Obj, const DwarfDie &Die);

  /// Set any references from other objects to this object now that it exists.
  void updateReferencesToObject(LibScopeView::Object &Obj, Dwarf_Off ObjOffset);

  /// Get an attribute, but produce a warning an return an empty DwarfAttrValue
  /// if the value is not the ExpectedKind or ValueKind::Empty.
  DwarfAttrValue getAttrExpectingKind(const DwarfDie &Die,
                                      const Dwarf_Half Attr,
                                      const DwarfAttrValueKind ExpectedKind);

  /// Get an attribute, but produce a warning an return an empty DwarfAttrValue
  /// if the value is not in the ExpectedKinds or ValueKind::Empty.
  DwarfAttrValue
  getAttrExpectingKinds(const DwarfDie &Die, const Dwarf_Half Attr,
                        const std::set<DwarfAttrValueKind> &ExpectedKinds);

  /// Return true if Die has Attr and the value is a flag set to true.
  bool attrIsTrueFlag(const DwarfDie &Die, const Dwarf_Half Attr);

  /// Get the access specifier (Public, Private, etc.) of a Die.
  LibScopeView::AccessSpecifier getAccessSpecifier(const DwarfDie &Die);

  // Offset range of the current CU.
  std::pair<Dwarf_Off, Dwarf_Off> CurrentCURange;

  // Mapping from DWARF file IDs to the file paths in the current CU.
  std::vector<std::string> SourceFileMapping;

  // Mapping from DWARF offsets to already created Objects.
  std::unordered_map<Dwarf_Off, LibScopeView::Object *> CreatedObjects;

  // Map of DWARF offsets to multiple Objects, where the offset is of Die that
  // hasn't been read yet, and each of the mapped objects needs to have its
  // type set to the Object that will be created from that Die.
  std::unordered_multimap<Dwarf_Off, LibScopeView::Object *> TypesToBeSet;

  // Map of DWARF offsets to multiple Objects, where the offset is of Die that
  // hasn't been read yet, and each of the mapped objects needs to have its
  // reference set to the Object that will be created from that Die.
  std::unordered_multimap<Dwarf_Off, LibScopeView::Object *> ReferencesToBeSet;

  // Unknown DWARF tags that have already been seen (avoids duplicate warnings).
  std::set<Dwarf_Half> UnknownDWTags;
  // Unrecognised Attr-Form combinations that have already been seen.
  std::set<std::pair<Dwarf_Half, Dwarf_Half>> UnknownAttrFormPairs;
};

} // end namespace ElfDwarfReader

#endif // ELF_DWARF_READER_H
