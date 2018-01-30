//===-- LibScopeView/Line.h -------------------------------------*- C++ -*-===//
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
/// Definition of the Line class.
///
//===----------------------------------------------------------------------===//

#ifndef LINE_H
#define LINE_H

#include "Object.h"

namespace LibScopeView {

/// \brief  Class to represent a single line info entry.
///
/// Contains a filename, line number and address.
class Line : public Object {
public:
  Line();

  /// \brief Return true if Obj is an instance of Line.
  static bool classof(const Object *Obj) { return Obj->getKind() == SV_Line; }

private:
  // Flags specifying various properties of the line.
  enum LineAttributes {
    IsLineEndSequence,
    IsNewBasicBlock,
    IsNewStatement,
    IsEpilogueBegin,
    IsPrologueEnd,
    LineAttributesSize
  };
  std::bitset<LineAttributesSize> LineAttributesFlags;

  // Discriminator value (DW_LNE_set_discriminator). The DWARF standard
  // defines the discriminator as an unsigned LEB128 integer. In our case,
  // unless is required, we will use an unsigned half integer.
  Dwarf_Half Discriminator;

  // Address of the line in the executable;
  Dwarf_Addr Address;

public:
  /// \brief Is line end sequence.
  bool getIsLineEndSequence() const {
    return LineAttributesFlags[IsLineEndSequence];
  }
  void setIsLineEndSequence() { LineAttributesFlags.set(IsLineEndSequence); }

  /// \brief Is new basic block.
  bool getIsNewBasicBlock() const {
    return LineAttributesFlags[IsNewBasicBlock];
  }
  void setIsNewBasicBlock() { LineAttributesFlags.set(IsNewBasicBlock); }

  /// \brief Is new statement.
  bool getIsNewStatement() const { return LineAttributesFlags[IsNewStatement]; }
  void setIsNewStatement() { LineAttributesFlags.set(IsNewStatement); }

  /// \brief Is epilogue begin.
  bool getIsEpilogueBegin() const {
    return LineAttributesFlags[IsEpilogueBegin];
  }
  void setIsEpilogueBegin() { LineAttributesFlags.set(IsEpilogueBegin); }

  /// \brief Is prologue end.
  bool getIsPrologueEnd() const { return LineAttributesFlags[IsPrologueEnd]; }
  void setIsPrologueEnd() { LineAttributesFlags.set(IsPrologueEnd); }

  /// \brief Line address.
  Dwarf_Addr getAddress() const { return Address; }
  void setAddress(Dwarf_Addr Addr) { Address = Addr; }

  /// \brief Line discriminator.
  Dwarf_Half getDiscriminator() const { return Discriminator; }
  void setDiscriminator(Dwarf_Half Discrim) {
    Discriminator = Discrim;
  }

  /// \brief Returns a text representation of this DIVA Object.
  std::string getAsText(const PrintSettings &Settings) const override;
  /// \brief Returns a YAML representation of this DIVA Object.
  std::string getAsYAML() const override;
};

} // namespace LibScopeView

#endif // LINE_H
