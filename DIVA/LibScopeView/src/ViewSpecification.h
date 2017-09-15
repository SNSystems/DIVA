//===-- LibScopeView/ViewSpecification.h ------------------------*- C++ -*-===//
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
/// Definition of the ViewSpecification class.
///
//===----------------------------------------------------------------------===//

#ifndef VIEW_SPECIFICATION_H
#define VIEW_SPECIFICATION_H

#include "CmdOptions.h"
#include "Sort.h"

#include <regex>

namespace LibScopeView {

class Line;
class Scope;
class Symbol;
class Type;

/// \brief All supported readers.
enum ReaderType {
  rt_libdwarf, // LibDwarf libraries.
  rt_unknown   // Unknown format.
};

/// \brief Pattern Mode for a Match.
enum MatchMode {
  mm_none = 0, // No given pattern.
  mm_match,    // Perfect match.
  mm_any,      // Partial match.
  mm_regex     // Regular expression.
};

/// \brief Small data structure to keep the search/tree pattern information.
struct Match {
  std::string Pattern; // Normal pattern.
  std::regex RE;       // Regular Expression Pattern.
  MatchMode Mode;      // Match mode.
  Match() : Mode(mm_none) {}
};

typedef std::vector<Match> MatchInfo;

/// \brief Commmon information for all readers.
///
/// Initialised by the option parser.
class ViewSpecification {
public:
  ViewSpecification();
  ViewSpecification(CmdOptions &options);

private:
  CmdOptions Options;        // Local command line options.
  std::string Id;            // View ID.
  ReaderType ViewReaderType; // Reader type.
  SortMode ViewSortMode;     // Object sort mode.

  std::string InputFile;     // Input file name/path.
  std::string PrintSplitDir; // Split directory name.

  MatchInfo FilterMatchInfo; // Match information.
  MatchInfo TreeMatchInfo;   // Match information.

public:
  /// \brief View command line options.
  CmdOptions &getOptions() { return Options; }
  void setOptions(CmdOptions &options) { Options = options; }

public:
  /// \brief View ID.
  std::string getID() const { return Id; }
  void setID(const std::string &value) { Id = value; }

  /// \brief Reader Type.
  ReaderType getReaderType() const { return ViewReaderType; }
  void setReaderType(ReaderType value) { ViewReaderType = value; }

  /// \brief Sort Mode.
  SortMode getSortMode() const { return ViewSortMode; }
  void setSortMode(SortMode value) { ViewSortMode = value; }

  /// \brief Input filename.
  std::string getInputFile() const { return InputFile; }
  void setInputFile(const std::string &value);

  /// \brief Split Directory Name.
  std::string getPrintSplitDir() const { return PrintSplitDir; }
  void setPrintSplitDir(const std::string &value) { PrintSplitDir = value; }

  /// \brief Any --filter pattern.
  bool getAnyFilterPattern() const { return !FilterMatchInfo.empty(); }

  /// \brief Any --tree pattern.
  bool getAnyTreePattern() const { return !TreeMatchInfo.empty(); }

  /// \brief Filter pattern info.
  void addFilterPattern(Match &M) { FilterMatchInfo.push_back(M); }

  /// \brief Tree pattern info.
  void addTreePattern(Match &M) { TreeMatchInfo.push_back(M); }

private:
  static ReaderType resolveReaderType(const std::string &Arg);

private:
  // Match a general pattern.
  bool matchPattern(const char *Input, const MatchInfo &MatchInfo) const;

public:
  /// \brief Check if input matches the --filter or --tree pattern.
  bool matchFilterPattern(const char *Input);
  bool matchTreePattern(const char *Input);

public:
  /// \brief Conditions to print an object.
  bool printFileName();
  bool printObject(Line *Ln);
  bool printObject(Scope *Scp);
  bool printObject(Symbol *Sym);
  bool printObject(Type *Ty);
};

} // namespace LibScopeView

#endif // VIEW_SPECIFICATION_H
