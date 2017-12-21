//===-- LibScopeView/Line.cpp -----------------------------------*- C++ -*-===//
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
/// Implementation for the Line class.
///
//===----------------------------------------------------------------------===//

#include "Line.h"
#include "PrintSettings.h"
#include "Utilities.h"

#include <sstream>

using namespace LibScopeView;

Line::Line() : Element(SV_Line), Discriminator(0) {}

uint32_t Line::LinesAllocated = 0;

std::string Line::getAsText(const PrintSettings &Settings) const {
  std::stringstream Result;
  Result << '{' << getKindAsString() << '}';
  if (Settings.ShowCodelineAttributes) {
    if (getIsNewStatement()) {
      Result << '\n' << formatAttributeText("NewStatement");
    }
    if (getIsPrologueEnd()) {
      Result << '\n' << formatAttributeText("PrologueEnd");
    }
    if (getIsLineEndSequence()) {
      Result << '\n' << formatAttributeText("EndSequence");
    }
    if (getIsNewBasicBlock()) {
      Result << '\n' << formatAttributeText("BasicBlock");
    }
    if (getHasDiscriminator()) {
      Result << '\n' << formatAttributeText("Discriminator");
    }
    if (getIsEpilogueBegin()) {
      Result << '\n' << formatAttributeText("EpilogueBegin");
    }
  }
  return Result.str();
}

std::string Line::getAsYAML() const {
  std::stringstream YAML;
  std::stringstream Attrs;
  const std::string YAMLTrue(": true");
  const std::string YAMLFalse(": false");
  Attrs << "\n  NewStatement"
        << (getIsNewStatement() ? YAMLTrue : YAMLFalse);
  Attrs << "\n  PrologueEnd"
        << (getIsPrologueEnd() ? YAMLTrue : YAMLFalse);
  Attrs << "\n  EndSequence"
        << (getIsLineEndSequence() ? YAMLTrue : YAMLFalse);
  Attrs << "\n  BasicBlock"
        << (getIsNewBasicBlock() ? YAMLTrue : YAMLFalse);
  Attrs << "\n  Discriminator"
        << (getHasDiscriminator() ? YAMLTrue : YAMLFalse);
  Attrs << "\n  EpilogueBegin"
        << (getIsEpilogueBegin() ? YAMLTrue : YAMLFalse);

  if (Attrs.str().empty())
    Attrs << " {}";

  YAML << getCommonYAML() << "\nattributes:" << Attrs.str();
  return YAML.str();
}
