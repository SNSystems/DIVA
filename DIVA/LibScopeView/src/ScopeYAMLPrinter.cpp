//===-- LibScopeView/ScopeYAMLPrinter.cpp ------------------------- C++ -*-===//
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
/// This file contains the definitions of ScopeYAMLPrinter's methods.
///
//===----------------------------------------------------------------------===//

#include "ScopeYAMLPrinter.h"
#include "Scope.h"

#include <algorithm>
#include <assert.h>
#include <sstream>

using namespace LibScopeView;

namespace {

// Adds a '\' in front of any '\' characters in input and returns the result.
std::string escapeBackslashes(const std::string &input) {
  std::string Result;
  Result.reserve(input.size()+16);
  for (char c : input) {
    Result += c;
    if (c == '\\')
      Result += '\\';
  }
  return Result;
}

} // namespace

ScopeYAMLPrinter::ScopeYAMLPrinter(const PrintSettings &Settings,
                                   const std::string &InputFile,
                                   const std::string &Version,
                                   uint8_t SizeOfIndent)
    : ScopePrinter(Settings), IndentSize(SizeOfIndent), IndentLevel(1) {
  YAMLHeader.append("input_file: \"")
      .append(escapeBackslashes(InputFile))
      .append("\"\noutput_version: \"")
      .append(Version)
      .append("\"\nobjects:\n");
}

const std::string &ScopeYAMLPrinter::getFileExtension() {
  static std::string YAMLExtension = "yaml";
  return YAMLExtension;
}

const std::string &ScopeYAMLPrinter::getHeader() { return YAMLHeader; }

void ScopeYAMLPrinter::printImpl(const Object *Obj,
                                 std::ostream &OutputStream) {
  // Don't print anything for the scope root, but do visit the children.
  if (Obj->getIsScope() && static_cast<const Scope *>(Obj)->getIsRoot()) {
    printChildren(Obj);
    return;
  }

  // Skip objects that shouldn't be printed as an object
  if (!Obj->getIsPrintedAsObject())
    return;

  std::stringstream ObjYAML(Obj->getAsYAML());
  assert(!ObjYAML.str().empty());

  // Add indentation.
  // We need to indent the first level of objects once so they are under the
  // header, then all subsequent layers need to be indented once for the
  // children list itself and the once more for the child.
  // We then need to indent by " -" for the first line to show it is an item in
  // the list, and then by "  " on the other lines.
  std::string Indent(((IndentLevel * 2) - 1) * IndentSize, ' ');
  std::string Line;
  std::getline(ObjYAML, Line);
  OutputStream << Indent << "- " << Line << '\n';
  while (std::getline(ObjYAML, Line)) {
    OutputStream << Indent << "  " << Line << '\n';
  }

  // Print children.
  OutputStream << Indent << "  "
               << "children:";
  if (Obj->getIsScope()) {
    auto const *Scp = static_cast<const Scope *>(Obj);
    const auto &Children = Scp->getChildren();
    if (std::none_of(Children.cbegin(), Children.cend(), [](Object *Child) {
          return Child->getIsPrintedAsObject();
        })) {
      OutputStream << " []\n";
      return;
    } else
      OutputStream << "\n";
    IndentLevel += 1;
    printChildren(Obj);
    IndentLevel -= 1;
  } else
    OutputStream << " []\n";
}
