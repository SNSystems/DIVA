//===-- LibScopeView/ScopeTextPrinter.cpp ------------------------- C++ -*-===//
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
/// This file contains the definitions of ScopeTextPrinter's methods.
///
//===----------------------------------------------------------------------===//

#include "ScopeTextPrinter.h"
#include "Object.h"
#include "Scope.h"

#include <cassert>
#include <iomanip>
#include <sstream>

using namespace LibScopeView;

namespace {

const size_t DwarfOffsetHexStringLength = 8;

// Visitor that finds the maximum sizes of text output for aligning printing.
class IndentSizeFinder : private ConstScopeVisitor {
public:
  IndentSizeFinder(const Object *Obj)
      : TagNameIndent(0), MaxLine(0), MaxLevel(0) {
    visit(Obj);
  }

  size_t getLineIndent() { return std::to_string(MaxLine).size(); }
  size_t getTagIndent() { return TagNameIndent; }
  size_t getLevelIndent() { return std::to_string(MaxLevel).size(); }

private:
  void visitImpl(const Object *Obj) override {
    MaxLine = std::max(MaxLine, Obj->getLineNumber());
    MaxLevel = std::max(MaxLevel, Obj->getLevel());

    // TODO: Store the tag name string in the Object.
    Dwarf_Half Tag = Obj->getDieTag();
    if (Tag && !SeenDwarfTags.count(Tag)) {
      SeenDwarfTags.emplace(Tag);
      const char *TagName;
      dwarf_get_TAG_name(Tag, &TagName);
      TagNameIndent = std::max(TagNameIndent, strlen(TagName));
    }

    visitChildren(Obj);
  }

  size_t TagNameIndent;
  uint64_t MaxLine;
  LevelType MaxLevel;

  std::set<Dwarf_Half> SeenDwarfTags;
};

// Visitor that finds Objects with children that match a tree filter.
class TreeFilteredParentFinder : private ConstScopeVisitor {
public:
  TreeFilteredParentFinder(
      const Object *Obj, const PrintSettings &PrintingSettings,
      std::unordered_set<const Object *> &FilteredParentsOut)
      : Settings(PrintingSettings), FilteredParents(FilteredParentsOut) {
    visit(Obj);
  }

private:
  void visitImpl(const Object *Obj) override {
    if (Settings.matchesTreeFilterPattern(Obj->getName())) {
      for (const Object *Parent = Obj->getParent(); Parent;
           Parent = Parent->getParent())
        FilteredParents.emplace(Parent);
      return;
    }
    visitChildren(Obj);
  }

  const PrintSettings &Settings;
  std::unordered_set<const Object *> &FilteredParents;
};

// Get any DWARF info for the start of the object line.
// [OFFSET][PARENT OFFSET]LEVEL [TAG]
std::string getDWARFAttributesString(const Object *Obj,
                                     const PrintSettings &Settings,
                                     size_t LevelNumberIndentSize,
                                     size_t TagIndentSize) {
  std::stringstream AttrString;
  AttrString << std::setfill('0');
  // [OFFSET]
  if (Settings.ShowDWARFOffset)
    AttrString << "[0x" << std::setw(DwarfOffsetHexStringLength) << std::hex
               << Obj->getDieOffset() << ']';
  // [PARENT OFFSET]
  if (Settings.ShowDWARFParent) {
    if (Obj->getParent())
      AttrString << "[0x" << std::setw(DwarfOffsetHexStringLength) << std::hex
                 << Obj->getParent()->getDieOffset() << ']';
    else
      AttrString << '[' << std::string(DwarfOffsetHexStringLength + 2, ' ')
                 << ']';
  }
  // LEVEL
  if (Settings.ShowLevel)
    AttrString << std::setw(static_cast<int>(LevelNumberIndentSize)) << std::dec
               << Obj->getLevel() << ' ';
  AttrString << std::setfill(' ');
  // [TAG]
  if (Settings.ShowDWARFTag) {
    const char *TagName = "";
    auto Tag = Obj->getDieTag();
    if (Tag)
      dwarf_get_TAG_name(Tag, &TagName);
    std::string TagNameWithBraces("[");
    TagNameWithBraces.append(TagName).append("]");
    AttrString << std::setw(static_cast<int>(TagIndentSize) + 2) << std::left
               << TagNameWithBraces;
  }

  std::string Result(AttrString.str());
  if (!Result.empty())
    Result.append("  ");
  return Result;
}

std::string getFlagAttributesString(const Object *Obj,
                                    const PrintSettings &Settings) {
  std::string Result;
  if (Settings.ShowIsGlobal)
    Result.append(Obj->getIsGlobalReference() ? "X " : "  ");
  return Result;
}

} // end anonymous namespace.

ScopeTextPrinter::ScopeTextPrinter(const PrintSettings &PrintingSettings,
                                   std::string InputFile, uint8_t Indent)
    : ScopePrinter(PrintingSettings),
      HeaderText(std::string("{InputFile} \"") + InputFile + "\"\n"),
      IndentSize(Indent) {}

void ScopeTextPrinter::initBeforePrint(const Object *Obj) {
  // Set all the indent sizes by examining Obj and its children.
  IndentSizeFinder IndentSizes(Obj);
  LineNumberIndentSize = IndentSizes.getLineIndent();
  TagIndentSize = IndentSizes.getTagIndent();
  LevelNumberIndentSize = IndentSizes.getLevelIndent();

  // Figure out how much indent will be needed on lines without dwarf attributes
  // by getting the length of any dwarf attribute line.
  std::string DAttrs = getDWARFAttributesString(
      Obj, Settings, LevelNumberIndentSize, TagIndentSize);
  // Find out the length of the flag attributes.
  std::string FAttrs = getFlagAttributesString(Obj, Settings);

  AttributesIndentSize = DAttrs.size() + FAttrs.size();
  FollowingLineExtraIndent = AttributesIndentSize + LineNumberIndentSize;

  // If we are tree filtering then find parents that need to be printed.
  ObjectsWithTreeFilteredChildren.clear();
  if (!(Settings.TreeFilters.empty() && Settings.TreeFilterAnys.empty()))
    TreeFilteredParentFinder(Obj, Settings, ObjectsWithTreeFilteredChildren);
}

const std::string &ScopeTextPrinter::getFileExtension() {
  static std::string TextExtension("txt");
  return TextExtension;
}

const std::string &ScopeTextPrinter::getHeader() {
  if (!HeaderText.empty() && HeaderText.front() != ' ' && AttributesIndentSize)
    HeaderText = std::string(AttributesIndentSize, ' ') + HeaderText;
  return HeaderText;
}

void ScopeTextPrinter::printImpl(const Object *Obj,
                                 std::ostream &OutputStream) {
  // Don't print anything for the scope root, but do visit the children.
  if (Obj->getIsScope() && static_cast<const Scope *>(Obj)->getIsRoot()) {
    printChildren(Obj);
    return;
  }

  if (!Obj->getIsPrintedAsObject())
    return;

  if (!Settings.printObject(*Obj)) {
    // --no-show-*, Don't print, but show the children.
    printIndentedChildren(Obj);
    return;
  }

  // Filtering.
  if (!IgnoreFilters && Settings.hasFilters()) {
    if (ObjectsWithTreeFilteredChildren.count(Obj)) {
      // A child matches a tree filter so print this (and its children).
      printObjectText(Obj, OutputStream);
      printIndentedChildren(Obj);
      return;
    } else if (Settings.matchesTreeFilterPattern(Obj->getName())) {
      // Print this and all children regardless of filters.
      printObjectText(Obj, OutputStream);
      IgnoreFilters = true;
      printIndentedChildren(Obj);
      IgnoreFilters = false;
      return;
    } else if (!Settings.matchesFilterPattern(Obj->getName())) {
      // Doesn't match the filters so don't print. It's children might so visit
      // them.
      printIndentedChildren(Obj);
      return;
    }
  }

  printObjectText(Obj, OutputStream);
  printIndentedChildren(Obj);
}

void ScopeTextPrinter::printObjectText(const Object *Obj,
                                       std::ostream &OutputStream) {
  // Print file names.
  size_t FileNameIndex = Obj->getFileNameIndex();
  if (FileNameIndex && CurrentFileIndex != FileNameIndex) {
    CurrentFileIndex = FileNameIndex;
    std::string FileName = Obj->getFileName(/*format_options*/ true);
    FileName = FileName.empty() ? "?" : FileName;
    OutputStream << '\n'
                 << std::string(AttributesIndentSize, ' ') << "{Source} \""
                 << FileName << "\"\n";
  }

  // Preceding attributes.
  OutputStream << getDWARFAttributesString(Obj, Settings, LevelNumberIndentSize,
                                           TagIndentSize);
  OutputStream << getFlagAttributesString(Obj, Settings);

  auto LineNo = Obj->getLineNumber();
  std::string LineNoStr =
      (LineNo == 0 && !Settings.ShowZeroLine) ? " " : std::to_string(LineNo);

  // Create indentation.
  auto TreeIndentAmount = IndentSize * (Settings.ShowIndent ? IndentLevel : 1);
  std::string FirstLineIndent(TreeIndentAmount, ' ');
  std::string FollowingLineIndent(FollowingLineExtraIndent + TreeIndentAmount,
                                  ' ');

  // Print the first line of the text.
  std::stringstream ObjText(Obj->getAsText(Settings));
  assert(!ObjText.str().empty());
  std::string TextOutputLine;
  std::getline(ObjText, TextOutputLine);
  OutputStream << std::setw(static_cast<int>(LineNumberIndentSize)) << LineNoStr
               << FirstLineIndent << TextOutputLine << '\n';

  // Print the other lines of the text with more indent.
  while (std::getline(ObjText, TextOutputLine)) {
    OutputStream << FollowingLineIndent << TextOutputLine << '\n';
  }
}

void ScopeTextPrinter::printIndentedChildren(const Object *Obj) {
  IndentLevel += 1;
  printChildren(Obj);
  IndentLevel -= 1;
}
