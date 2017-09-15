//===-- Diva/DivaOptions.cpp ------------------------------------*- C++ -*-===//
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

#include "DivaOptions.h"
#include "ArgumentParser.h"
#include "Error.h"
#include "Platform.h"

namespace {

const static std::string DIVA_VERSION_NUMBER(RC_VERSION_STR);
const static std::string COPYRIGHT_YEAR(RC_COPYYEAR_STR);
const static std::string COMPANY_NAME(RC_COMPANYNAME_STR);

[[noreturn]] void earlyExitSuccess() { std::exit(0); }
[[noreturn]] void earlyExitFailure() { std::exit(1); }

void printVersionDetails(std::ostream &VersionOut) {
#ifndef NDEBUG
  const char *Config = " (Debug)";
#else
  const char *Config = "";
#endif // NDEBUG

#if defined(PLATFORM_WIN)
  const char *Platform = "Win";
#elif defined(PLATFORM_LINUX)
  const char *Platform = "Linux";
#else
  const char *Platform = "Unknown";
#endif // PLATFORM_WIN

#if defined(PLATFORM_32BIT)
  const char *WordSize = "32";
#elif defined(PLATFORM_64BIT)
  const char *WordSize = "64";
#else
  const char *WordSize = "";
#endif // PLATFORM_32BIT

  VersionOut << "DIVA - Debug Information Visual Analyzer - version "
             << DIVA_VERSION_NUMBER << " - " << Platform << WordSize << Config
             << "\n"
             << "Copyright (C) " << COPYRIGHT_YEAR << " " << COMPANY_NAME
             << " All Rights Reserved."
             << "\n";
}

} // end anonymous namespace.

DivaOptions::DivaOptions(const std::vector<std::string> &CMDArgs,
                         std::ostream &HelpOut, std::ostream &VersionOut,
                         std::ostream &) {
  // Set some initial defaults.
  QuietMode = false;
  ShowSummary = false;
  SplitOutput = false;
  SortKey = SortingKey::LINE;

  showBrief();

  ShowCodeline = false;
  ShowCodelineAttributes = false;
  ShowCombined = false;
  ShowDWARFOffset = false;
  ShowDWARFParent = false;
  ShowDWARFTag = false;
  ShowGenerated = false;
  ShowIsGlobal = false;
  ShowIndent = true;
  ShowLevel = false;
  ShowOnlyGlobals = false;
  ShowOnlyLocals = false;
  ShowQualified = false;
  ShowUnderlying = false;
  ShowVoid = true;
  ShowZeroLine = false;

  ShowPerformanceTime = false;
  ShowPerformanceMemory = false;
  ShowScopeAllocation = false;
  ShowStringPoolInfo = false;
  DumpStringPool = false;

  // Parsing.
  try {
    parseArgs(CMDArgs, HelpOut, VersionOut);
  } catch (ArgumentParser::UnrecognisedArg &Err) {
    LibScopeError::fatalError(LibScopeError::ErrorCode::ERR_CMD_UNKNOWN_ARG,
                              Err.Arg.c_str());
  } catch (ArgumentParser::ArgumentValueRequired &Err) {
    LibScopeError::fatalError(LibScopeError::ErrorCode::ERR_CMD_MISSING_VALUE,
                              Err.Arg.c_str());
  } catch (ArgumentParser::UnexpectedArgumentValue &Err) {
    LibScopeError::fatalError(
        LibScopeError::ErrorCode::ERR_CMD_UNEXPECTED_VALUE, Err.Arg.c_str(),
        Err.ArgVal.c_str());
  } catch (ArgumentParser::UnexpectedNegative &Err) {
    LibScopeError::fatalError(LibScopeError::ErrorCode::ERR_CMD_UNKNOWN_ARG,
                              Err.Arg.c_str());
  } catch (ArgumentParser::ShortcutWithArgumentValue &Err) {
    LibScopeError::fatalError(
        LibScopeError::ErrorCode::ERR_CMD_SHORTCUT_WITH_VALUE, Err.Arg.c_str());
  } catch (ArgumentParser::InvalidChoice &Err) {
    LibScopeError::fatalError(LibScopeError::ErrorCode::ERR_CMD_INVALID_VALUE,
                              Err.Arg.c_str(), Err.ArgVal.c_str());
  }

  // Set output formats.
  if (OutputFormatStrings.empty() || OutputFormatStrings.count("text"))
    OutputFormats.emplace(OutputFormat::TEXT);
  if (OutputFormatStrings.count("yaml"))
    OutputFormats.emplace(OutputFormat::YAML);

  // Set sort key.
  if (SortKeyString == "line")
    SortKey = SortingKey::LINE;
  if (SortKeyString == "offset")
    SortKey = SortingKey::OFFSET;
  if (SortKeyString == "name")
    SortKey = SortingKey::NAME;
}

void DivaOptions::parseArgs(const std::vector<std::string> &CMDArgs,
                            std::ostream &HelpOut, std::ostream &VersionOut) {
  using namespace ArgumentParser;

  static const char NSC = Argument::NoShortcut;
  static const std::string Usage("Diva [options] input_file [input_file...]");

  static const Argument::HelpLevel GeneralHelp = 1;
  static const Argument::HelpLevel BasicHelp = 2;
  static const Argument::HelpLevel MoreHelp = 3;
  static const Argument::HelpLevel AdvancedHelp = 4;
  static const Argument::HelpLevel DeveloperHelp = 5;

  static const Argument::HelpLevels ShowGeneralHelp({GeneralHelp});
  static const Argument::HelpLevels ShowGeneralAndBasicHelp(
      {GeneralHelp, BasicHelp});
  static const Argument::HelpLevels ShowMoreHelp({MoreHelp});
  static const Argument::HelpLevels ShowAdvancedHelp({AdvancedHelp});

  bool HelpOrVersionPrinted = false;

  // clang-format off
  Parser DivaArgParser({
    ArgumentGroup("General options", {
      Argument::helpArg('h', "help", "Display basic help information",
                        GeneralHelp, Usage, ShowGeneralAndBasicHelp,
                        HelpOrVersionPrinted, HelpOut),
      Argument::helpArg(NSC, "help-more", "Display more option information",
                        GeneralHelp, Usage, ShowMoreHelp, HelpOrVersionPrinted,
                        HelpOut),
      Argument::helpArg(NSC, "help-advanced",
                        "Display advanced option information", GeneralHelp,
                        Usage, ShowAdvancedHelp, HelpOrVersionPrinted, HelpOut),
      Argument('v', "version", "Display the version information", GeneralHelp,
               [&](const Parser &) {
                 printVersionDetails(VersionOut);
                 HelpOrVersionPrinted = true;
               }),
      Argument::switchArg('q', "quiet", "Suppress output to stdout",
                          GeneralHelp, QuietMode)
    }),

    ArgumentGroup("Output options", {
      Argument('a', "show-all",
               "Print all (expect advanced) objects and attributes", BasicHelp,
               [&](const Parser &) { showAll(); }),
      Argument('b', "show-brief",
               "Print all common objects and attributes (default)", BasicHelp,
               [&](const Parser &) { showBrief(); }),
      Argument::switchArg('t', "show-summary", "Print the summary table",
                          BasicHelp, ShowSummary),
      Argument('d', "output-dir", "[=<dir>]",
               "Print the output into a directory with each compile unit's "
               "output in a separate file. If no dir is given, then diva will "
               "use the input_file string to create an output directory.",
               BasicHelp,
               // This argument behaves like a switch that can also set a value.
               [&](const Parser &) { SplitOutput = true; },
               [&](const Parser &, const std::string &Opt) {
                 SplitOutput = true;
                 OutputDirectory = Opt;
               },
               [&](const Parser &) { SplitOutput = false; }),
      Argument::multiChoiceArg(
          NSC, "output",
          "A comma separated list of output formats.", BasicHelp,
          {"text", "yaml"}, OutputFormatStrings)
    }),

    ArgumentGroup("Sort options", {
      Argument::choiceArg(
          NSC, "sort",
          "Key used when ordering the output objects within the same block. "
          "By default the key is \"line\".", BasicHelp,
          {"line", "offset", "name"}, SortKeyString)
    }),

    ArgumentGroup("Filter options", {
      Argument::multiStringArg(
          NSC, "filter", "text",
          "Only show objects with <text> as its instance name. The output will "
          "only include exact matches, Regex is accepted. Multiple filters can "
          "be given to show objects that match either.",
          BasicHelp, Filters),
      Argument::multiStringArg(
          NSC, "filter-any", "text",
          "Only show objects with <text> in their instance name.",
          BasicHelp, FilterAnys),
      Argument::multiStringArg(
          NSC, "tree", "text",
          "Same as --filter, except the whole subtree of any matching object "
          "will printed.",
          BasicHelp, WithChildrenFilters),
      Argument::multiStringArg(
          NSC, "tree-any", "text",
          "Same as --filter-any with the whole subtree.", BasicHelp,
          WithChildrenFilterAnys),
    }),

    ArgumentGroup("More object options", {
      Argument(
          NSC, "show-none",
          "Print no objects, use this to hide all default objects and then "
          "individual objects can be added to the output using the options "
          "below.",
          MoreHelp, [&](const Parser &) { showNone(); }),
      Argument::rawHelpText(
          "\n"
          "The following options can be used to show/hide specific objects and attributes.\n"
          "If two or more options conflict, precedence is given to the latter of the\n"
          "conflicting options.\n"
          "\n"
          "To hide a specific object, insert 'no-' after the '--'.\n"
          "Example: for \"--show-block --no-show-block\", DIVA will not print any block\n"
          "         objects.\n"
          "         for \"--show-all --no-show-block\", DIVA will show all common objects\n"
          "         except blocks.\n"
          "         for \"--show-none --show-block\", DIVA will only show blocks.\n"
          "\n"
          "The options labeled with a star (*) are enabled by default or when\n"
          "\"--show-brief\" is given and all the following options are enabled when\n"
          "\"--show-all\" is given.\n",
          MoreHelp),
      Argument::switchArg(NSC, "show-alias", "Print alias*", MoreHelp,
                          ShowAlias),
      Argument::switchArg(NSC, "show-block", "Print blocks*", MoreHelp,
                          ShowBlock),
      Argument::switchArg(NSC, "show-block-attributes",
                          "Print block attributes (e.g. try,catch)", MoreHelp,
                          ShowBlockAttributes),
      Argument::switchArg(NSC, "show-class", "Print classes*", MoreHelp,
                          ShowClass),
      Argument::switchArg(NSC, "show-enum", "Print enums*", MoreHelp,
                          ShowEnum),
      Argument::switchArg(NSC, "show-function", "Print functions*", MoreHelp,
                          ShowFunction),
      Argument::switchArg(NSC, "show-member", "Print class members*", MoreHelp,
                          ShowMember),
      Argument::switchArg(NSC, "show-namespace", "Print namespaces*", MoreHelp,
                          ShowNamespace),
      Argument::switchArg(NSC, "show-parameter", "Print parameters*", MoreHelp,
                          ShowParameter),
      Argument::switchArg(NSC, "show-primitivetype", "Print primitives",
                          MoreHelp, ShowPrimitivetype),
      Argument::switchArg(NSC, "show-struct", "Print structures*", MoreHelp,
                          ShowStruct),
      Argument::switchArg(NSC, "show-template", "Print templates*", MoreHelp,
                          ShowTemplate),
      Argument::switchArg(NSC, "show-union", "Print unions*", MoreHelp,
                          ShowUnion),
      Argument::switchArg(NSC, "show-using", "Print using instances*", MoreHelp,
                          ShowUsing),
      Argument::switchArg(NSC, "show-variable", "Print variables*", MoreHelp,
                          ShowVariable)
    }),

    ArgumentGroup("Advanced object options", {
      Argument::rawHelpText(
          "\n"
          "The following options can be used to add additional (advanced) attributes and\n"
          "flags to the output. These options are only considered useful for toolchain-\n"
          "developers who are working at the DWARF level, and they are not enabled by\n"
          "\"--show-brief\" or \"--show-all\".\n"
          "\n"
          "To disable an option, insert 'no-' after the '--'.\n",
          AdvancedHelp),
      Argument::switchArg(NSC, "show-codeline", "Print code lines",
                          AdvancedHelp, ShowCodeline),
      Argument::switchArg(
          NSC, "show-codeline-attributes",
          "Print code line attributes (NewStatement, PrologueEnd)",
          AdvancedHelp, ShowCodelineAttributes),
      Argument::switchArg(NSC, "show-combined",
                          "Print combined scope attributes", AdvancedHelp,
                          ShowCombined),
      Argument::switchArg(NSC, "show-DWARF-offset", "Print DWARF offsets",
                          AdvancedHelp, ShowDWARFOffset),
      Argument::switchArg(NSC, "show-DWARF-parent",
                          "Print parent object DWARF offsets", AdvancedHelp,
                          ShowDWARFParent),
      Argument::switchArg(NSC, "show-DWARF-tag", "Print DWARF tag attribute",
                          AdvancedHelp, ShowDWARFTag),
      Argument::switchArg(NSC, "show-generated",
                          "Print compiler generated attributes", AdvancedHelp,
                          ShowGenerated),
      Argument::switchArg(NSC, "show-global", "Print \"global\" attributes",
                          AdvancedHelp, ShowIsGlobal),
      Argument::switchArg(NSC, "show-indent",
                          "Print indentations to reflect context (default on)",
                          AdvancedHelp, ShowIndent),
      Argument::switchArg(NSC, "show-level", "Print lexical block levels",
                          AdvancedHelp, ShowLevel),
      Argument::switchArg(NSC, "show-only-globals",
                          "Print only \"global\" objects", AdvancedHelp,
                          ShowOnlyGlobals),
      Argument::switchArg(NSC, "show-only-locals",
                          "Print only \"local\" objects", AdvancedHelp,
                          ShowOnlyLocals),
      Argument::switchArg(NSC, "show-qualified",
                          "Print \"qualified name\" attributes*", AdvancedHelp,
                          ShowQualified),
      Argument::switchArg(NSC, "show-underlying",
                          "Print underlying type attributes", AdvancedHelp,
                          ShowUnderlying),
      Argument::switchArg(NSC, "show-void",
                          "Print \"void\" type attributes (default on)",
                          AdvancedHelp, ShowVoid),
      Argument::switchArg(NSC, "show-zero", "Print zero line number attributes",
                          AdvancedHelp, ShowZeroLine),

    }),

    ArgumentGroup("Developer options", {
      Argument::switchArg(NSC, "performance-time",
                          "Print time taken to run diva",
                          DeveloperHelp, ShowPerformanceTime),
      Argument::switchArg(NSC, "performance-memory", "Print peak memory usage",
                          DeveloperHelp, ShowPerformanceMemory),
      Argument::switchArg(NSC, "scope-allocation", "Print scope allocations",
                          DeveloperHelp, ShowScopeAllocation),
      Argument::switchArg(NSC, "info-string-pool", "Print string pool info",
                          DeveloperHelp, ShowStringPoolInfo),
      Argument::switchArg(NSC, "dump-string-pool",
                          "Print the entire string pool", DeveloperHelp,
                          DumpStringPool),
    })
  });
  // clang-format on

  // If no arguments are given then print a subset of help and exit.
  if (CMDArgs.empty()) {
    DivaArgParser.outputHelp(Usage, ShowGeneralHelp, HelpOut);
    earlyExitFailure();
  }

  // Parse all the arguments using the parser created above.
  InputFiles = DivaArgParser.parseCommandLineArgs(CMDArgs);
  if (HelpOrVersionPrinted)
    earlyExitSuccess();
}

void DivaOptions::showBrief() {
  ShowAlias = true;
  ShowBlock = true;
  ShowBlockAttributes = false;
  ShowClass = true;
  ShowEnum = true;
  ShowFunction = true;
  ShowMember = true;
  ShowNamespace = true;
  ShowParameter = true;
  ShowPrimitivetype = false;
  ShowStruct = true;
  ShowTemplate = true;
  ShowUnion = true;
  ShowUsing = true;
  ShowVariable = true;
}

void DivaOptions::setMoreShowOptions(bool SetTo) {
  ShowAlias = SetTo;
  ShowBlock = SetTo;
  ShowBlockAttributes = SetTo;
  ShowClass = SetTo;
  ShowEnum = SetTo;
  ShowFunction = SetTo;
  ShowMember = SetTo;
  ShowNamespace = SetTo;
  ShowParameter = SetTo;
  ShowPrimitivetype = SetTo;
  ShowStruct = SetTo;
  ShowTemplate = SetTo;
  ShowUnion = SetTo;
  ShowUsing = SetTo;
  ShowVariable = SetTo;
}

LibScopeView::CmdOptions DivaOptions::convertToCmdOptions() const {
  LibScopeView::CmdOptions Result;

  Result.setViewSort();
  Result.setPrintScopes();
  Result.setPrintSymbols();
  Result.setPrintLines();
  Result.setFormatFileName();
  Result.setFormatQualifiedName();

  if (QuietMode)
    Result.setTraceQuiet();
  if (ShowSummary)
    Result.setPrintSummary();

  if (SplitOutput)
    Result.setViewSplit();

  Result.setPrintNone();
  if (ShowAlias)
    Result.setPrintAlias();
  if (ShowBlock)
    Result.setPrintBlock();
  if (ShowBlockAttributes)
    Result.setPrintBlockAttributes();
  if (ShowClass)
    Result.setPrintClass();
  if (ShowEnum)
    Result.setPrintEnum();
  if (ShowFunction)
    Result.setPrintFunction();
  if (ShowMember)
    Result.setPrintMember();
  if (ShowNamespace)
    Result.setPrintNamespace();
  if (ShowParameter)
    Result.setPrintParameter();
  if (ShowPrimitivetype)
    Result.setPrintPrimitivetype();
  if (ShowStruct)
    Result.setPrintStruct();
  if (ShowTemplate)
    Result.setPrintTemplate();
  if (ShowUnion)
    Result.setPrintUnion();
  if (ShowUsing)
    Result.setPrintUsing();
  if (ShowVariable)
    Result.setPrintVariable();

  if (ShowCodeline)
    Result.setPrintCodeline();
  if (ShowCodelineAttributes)
    Result.setPrintCodelineAttributes();
  if (ShowCombined)
    Result.setFormatCombineScopes();
  if (ShowDWARFOffset)
    Result.setAttributeOffset();
  if (ShowDWARFParent)
    Result.setAttributeParent();
  if (ShowDWARFTag)
    Result.setAttributeTag();
  if (ShowGenerated)
    Result.setFormatGenerated();
  if (ShowIsGlobal)
    Result.setAttributeGlobal();
  if (ShowIndent)
    Result.setFormatIndentation();
  if (ShowLevel)
    Result.setAttributeLevel();
  if (ShowOnlyGlobals)
    Result.setFormatOnlyGlobals();
  if (ShowOnlyLocals)
    Result.setFormatOnlyLocals();
  if (ShowQualified)
    Result.setFormatQualifiedName();
  if (ShowUnderlying)
    Result.setFormatUnderlyingType();
  if (ShowVoid)
    Result.setFormatVoidType();
  if (ShowZeroLine)
    Result.setFormatZeroLineNumbers();

  if (ShowStringPoolInfo)
    Result.setUsageStringPoolInfo();
  if (DumpStringPool)
    Result.setUsageStringPoolTable();

  return Result;
}

std::vector<LibScopeView::ViewSpecification>
DivaOptions::convertToViewSpecs() const {
  std::vector<LibScopeView::ViewSpecification> Result;
  auto CMDOptions = convertToCmdOptions();

  unsigned int SpecID = 0;
  for (const auto &InputFile : InputFiles) {
    Result.emplace_back(CMDOptions);
    LibScopeView::ViewSpecification &Spec = Result.back();
    Spec.setID(std::to_string(++SpecID));
    Spec.setInputFile(InputFile);

    if (SplitOutput) {
      if (OutputDirectory.empty())
        Spec.setPrintSplitDir(Spec.getInputFile() + "_CUs");
      else
        Spec.setPrintSplitDir(OutputDirectory);
    }

    switch (SortKey) {
    case SortingKey::LINE:
      Spec.setSortMode(LibScopeView::SortMode::sr_line);
      break;
    case SortingKey::NAME:
      Spec.setSortMode(LibScopeView::SortMode::sr_name);
      break;
    case SortingKey::OFFSET:
      Spec.setSortMode(LibScopeView::SortMode::sr_offset);
      break;
    }

    for (const auto &Filter : Filters) {
      LibScopeView::Match FilterMatcher;
      FilterMatcher.Pattern = Filter;
      FilterMatcher.Mode = LibScopeView::MatchMode::mm_regex;
      try {
        FilterMatcher.RE = std::regex(Filter);
      } catch (std::regex_error &) {
        fatalError(LibScopeError::ErrorCode::ERR_CMD_INVALID_REGEX,
                   Filter.c_str());
      }
      Spec.addFilterPattern(FilterMatcher);
    }
    for (const auto &Filter : FilterAnys) {
      LibScopeView::Match FilterMatcher;
      FilterMatcher.Pattern = Filter;
      FilterMatcher.Mode = LibScopeView::MatchMode::mm_any;
      Spec.addFilterPattern(FilterMatcher);
    }
    for (const auto &Filter : WithChildrenFilters) {
      LibScopeView::Match FilterMatcher;
      FilterMatcher.Pattern = Filter;
      FilterMatcher.Mode = LibScopeView::MatchMode::mm_regex;
      try {
        FilterMatcher.RE = std::regex(Filter);
      } catch (std::regex_error &) {
        fatalError(LibScopeError::ErrorCode::ERR_CMD_INVALID_REGEX,
                   Filter.c_str());
      }
      Spec.addTreePattern(FilterMatcher);
    }
    for (const auto &Filter : WithChildrenFilterAnys) {
      LibScopeView::Match FilterMatcher;
      FilterMatcher.Pattern = Filter;
      FilterMatcher.Mode = LibScopeView::MatchMode::mm_any;
      Spec.addTreePattern(FilterMatcher);
    }
  }

  return Result;
}
