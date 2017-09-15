//===-- Diva/DivaOptions.h --------------------------------------*- C++ -*-===//
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
/// Command line option parsing for DIVA.
///
//===----------------------------------------------------------------------===//

#ifndef DIVAOPTIONS_H_
#define DIVAOPTIONS_H_

#include "ViewSpecification.h"

#include <iostream>
#include <set>
#include <string>
#include <vector>

enum class OutputFormat { TEXT, YAML };

enum class SortingKey { LINE, OFFSET, NAME };

/// \brief Class that parses command line arguments into DIVA's options (using
/// ArgumentParser).
///
/// Generates control structs that can be used by the rest of the Diva program.
class DivaOptions {
public:
  /// \brief Creates the options from a set of command line arguments.
  ///
  /// Note the first argument should not be the executable name (argv[0]).
  DivaOptions(const std::vector<std::string> &CMDArgs, std::ostream &HelpOut,
              std::ostream &VersionOut, std::ostream &ErrOut);

  /// \brief Get the view specifications.
  std::vector<LibScopeView::ViewSpecification> convertToViewSpecs() const;

  /// \brief Get the command line options.
  LibScopeView::CmdOptions convertToCmdOptions() const;

  std::vector<std::string> InputFiles;

  bool QuietMode;
  bool ShowSummary;

  bool SplitOutput;
  std::string OutputDirectory;
  std::set<OutputFormat> OutputFormats;

  SortingKey SortKey;

  std::vector<std::string> Filters;
  std::vector<std::string> FilterAnys;
  std::vector<std::string> WithChildrenFilters;
  std::vector<std::string> WithChildrenFilterAnys;

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

  bool ShowCodeline;
  bool ShowCodelineAttributes;
  bool ShowCombined;
  bool ShowDWARFOffset;
  bool ShowDWARFParent;
  bool ShowDWARFTag;
  bool ShowGenerated;
  bool ShowIsGlobal;
  bool ShowIndent;
  bool ShowLevel;
  bool ShowOnlyGlobals;
  bool ShowOnlyLocals;
  bool ShowQualified;
  bool ShowUnderlying;
  bool ShowVoid;
  bool ShowZeroLine;

  bool ShowPerformanceTime;
  bool ShowPerformanceMemory;
  bool ShowScopeAllocation;
  bool ShowStringPoolInfo;
  bool DumpStringPool;

private:
  void parseArgs(const std::vector<std::string> &CMDArgs, std::ostream &HelpOut,
                 std::ostream &VersionOut);

  void showNone() { setMoreShowOptions(false); }
  void showBrief();
  void showAll() { setMoreShowOptions(true); }
  // Set the show options from "More object options" to true/false.
  void setMoreShowOptions(bool SetTo);

  // Some options need to be translated from input strings to enum values.
  std::set<std::string> OutputFormatStrings;
  std::string SortKeyString;
};

#endif // DIVAOPTIONS_H_
