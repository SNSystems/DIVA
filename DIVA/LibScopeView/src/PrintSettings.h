//===-- Diva/PrintSettings.h ------------------------------------*- C++ -*-===//
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
/// Settings controlling DIVA's output.
///
//===----------------------------------------------------------------------===//

#ifndef SCOPEVIEW_PRINTSETTINGS_H
#define SCOPEVIEW_PRINTSETTINGS_H

#include <regex>
#include <set>
#include <vector>

namespace LibScopeView {

class Object;

enum class SortingKey { LINE, OFFSET, NAME };

class PrintSettings {
public:
  PrintSettings() { showBrief(); }

  void showBrief();
  void showNone() { setMoreShowOptions(false); }
  void showAll() { setMoreShowOptions(true); }

  /// \brief Check if an object should be printed given the current settings.
  bool printObject(const Object &Obj) const;

  /// \brief Check if the name matches a --filter pattern.
  bool matchesFilterPattern(const std::string &Name) const;

  /// \brief Check if the name matches a --tree pattern.
  bool matchesWithChildrenFilterPattern(const std::string &Name) const;

  bool QuietMode = false;
  bool ShowSummary = false;

  bool SplitOutput = false;
  std::string OutputDirectory;

  SortingKey SortKey = SortingKey::LINE;

  std::vector<std::regex> Filters;
  std::vector<std::string> FilterAnys;
  std::vector<std::regex> WithChildrenFilters;
  std::vector<std::string> WithChildrenFilterAnys;

  // The defaults for these values are set by showBrief.
  bool ShowAlias;
  bool ShowBlock;
  bool ShowBlockAttributes;
  bool ShowClass;
  bool ShowEnum;
  bool ShowFunction;
  bool ShowMember;
  bool ShowNamespace;
  bool ShowParameter;
  bool ShowPrimitivetype;
  bool ShowStruct;
  bool ShowTemplate;
  bool ShowUnion;
  bool ShowUsing;
  bool ShowVariable;

  bool ShowCodeline = false;
  bool ShowCodelineAttributes = false;
  bool ShowCombined = false;
  bool ShowDWARFOffset = false;
  bool ShowDWARFParent = false;
  bool ShowDWARFTag = false;
  bool ShowGenerated = false;
  bool ShowIsGlobal = false;
  bool ShowIndent = true;
  bool ShowLevel = false;
  bool ShowOnlyGlobals = false;
  bool ShowOnlyLocals = false;
  bool ShowQualified = false;
  bool ShowUnderlying = false;
  bool ShowVoid = true;
  bool ShowZeroLine = false;

private:
  // Set the show options from "More object options" to true/false.
  void setMoreShowOptions(bool SetTo);
};

} // namespace LibScopeView

#endif // SCOPEVIEW_PRINTSETTINGS_H
