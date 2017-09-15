//===-- LibScopeView/ViewSpecification.cpp ----------------------*- C++ -*-===//
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
/// Implementations for the ViewSpecification class.
///
//===----------------------------------------------------------------------===//

#include "ViewSpecification.h"
#include "Error.h"
#include "FileUtilities.h"
#include "Scope.h"
#include "Symbol.h"
#include "Type.h"

using namespace LibScopeView;

ViewSpecification::ViewSpecification()
    : ViewReaderType(rt_unknown), ViewSortMode(sr_line) {}

ViewSpecification::ViewSpecification(CmdOptions &options)
    : ViewReaderType(), ViewSortMode() {

  Options = options;
}

bool ViewSpecification::matchPattern(const char *Input,
                                     const MatchInfo &MatchInfo) const {
  std::string Name(Input);
  // Traverse all match specifications.
  for (auto Matcher : MatchInfo) {
    switch (Matcher.Mode) {
    case mm_any:
      if (Name.find(Matcher.Pattern) != std::string::npos)
        return true;
      break;
    case mm_match:
      if (!Name.compare(Matcher.Pattern))
        return true;
      break;
    case mm_regex:
      if (std::regex_match(Name, Matcher.RE))
        return true;
      break;
    case mm_none:
      // No pattern, match nothing.
      break;
    }
  }
  return false;
}

bool ViewSpecification::matchFilterPattern(const char *input) {
  return matchPattern(input, FilterMatchInfo);
}

bool ViewSpecification::matchTreePattern(const char *input) {
  return matchPattern(input, TreeMatchInfo);
}

bool ViewSpecification::printFileName() {
  return (getOptions().getPrintFilenames());
}

bool ViewSpecification::printObject(Line * /*Ln*/) {
  return getOptions().getPrintCodeline();
}

bool ViewSpecification::printObject(Scope *Scp) {
  // A scope will be printed if it matches any speicified patterns and printing
  // of the scope kind has not been disabled by the command line.

  bool DoPrint = false;
  if (FilterMatchInfo.size()) {
    // A pattern was specified in the command line.
    DoPrint = getOptions().getPrintScopes();
  } else {
    DoPrint = getOptions().getPrintScopes() ||
               (getOptions().getPrintSymbols() && Scp->getHasSymbols()) ||
               (getOptions().getPrintLines() && Scp->getHasLines()) ||
               (getOptions().getPrintTypes() && Scp->getHasTypes());
  }

  if (Scp->getIsTemplateAlias())
    DoPrint = getOptions().getPrintAlias();
  else if (Scp->getIsArrayType())
    DoPrint = getOptions().getPrintArray();
  else if (Scp->getIsBlock())
    DoPrint = getOptions().getPrintBlock();
  else if (Scp->getIsTemplate() || Scp->getIsTemplatePack())
    DoPrint = getOptions().getPrintTemplate();
  else if (Scp->getIsClassType())
    DoPrint = getOptions().getPrintClass();
  else if (Scp->getIsEnumerationType())
    DoPrint = getOptions().getPrintEnum();
  else if (Scp->getIsFunction())
    DoPrint = getOptions().getPrintFunction();
  else if (Scp->getIsNamespace())
    DoPrint = getOptions().getPrintNamespace();
  else if (Scp->getIsStructType())
    DoPrint = getOptions().getPrintStruct();
  else if (Scp->getIsUnionType())
    DoPrint = getOptions().getPrintUnion();

  return DoPrint;
}

bool ViewSpecification::printObject(Symbol *Sym) {
  bool DoPrint = getOptions().getPrintSymbols();;

  if (Sym->getIsMember())
    DoPrint = getOptions().getPrintMember();
  else if (Sym->getIsParameter())
    DoPrint = getOptions().getPrintParameter();
  else if (Sym->getIsVariable())
    DoPrint = getOptions().getPrintVariable();

  return DoPrint;
}

bool ViewSpecification::printObject(Type *Ty) {
  bool DoPrint = getOptions().getPrintTypes();;

  // Only print array subranges if the command line option was given.
  if (Ty->getIsSubrangeType() && !getOptions().getFormatArraysEncoded()) {
    DoPrint = false;
  }

  if (Ty->getIsBaseType())
    DoPrint = getOptions().getPrintPrimitivetype();
  else if (Ty->getIsTemplateParam())
    DoPrint = getOptions().getPrintTemplate();
  else if (Ty->getIsTypedef())
    DoPrint = getOptions().getPrintTypedef();
  else if (Ty->getIsInheritance())
    DoPrint = getOptions().getPrintClass() || getOptions().getPrintStruct();
  else if (Ty->getIsImported())
    DoPrint = getOptions().getPrintUsing();
  else if (Ty->getIsEnumerator())
    DoPrint = getOptions().getPrintEnum();
  else if (Ty->getIsReferenceType())
    DoPrint = getOptions().getPrintTypes();

  return DoPrint;
}

ReaderType ViewSpecification::resolveReaderType(const std::string &Arg) {
  if (isFileFormatElf(Arg))
    return rt_libdwarf;
  return rt_unknown;
}

void ViewSpecification::setInputFile(const std::string &Arg) {
  InputFile = Arg;

  // Check that the file exists.
  if (!doesFileExist(InputFile))
    fatalError(LibScopeError::ErrorCode::ERR_VIEW_INVALID_OPEN, InputFile);

  // Resolve the reader type.
  ReaderType RType = resolveReaderType(Arg);
  if (RType == rt_unknown) {
    fatalError(LibScopeError::ErrorCode::ERR_INVALID_FILE, Arg);
  } else {
    ViewReaderType = RType;
  }
}
