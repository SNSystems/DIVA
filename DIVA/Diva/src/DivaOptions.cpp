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

void compileRegexs(std::vector<std::string> &Patterns,
                   std::vector<std::regex> &RegexsOut) {
  for (const std::string &Pattern : Patterns) {
    try {
      RegexsOut.emplace_back(Pattern);
    } catch (std::regex_error &) {
      fatalError(LibScopeError::ErrorCode::ERR_CMD_INVALID_REGEX, Pattern);
    }
  }
}

} // end anonymous namespace.

DivaOptions::DivaOptions(const std::vector<std::string> &CMDArgs,
                         std::ostream &HelpOut, std::ostream &VersionOut,
                         std::ostream &) {

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
    PrintingSettings.SortKey = LibScopeView::SortingKey::LINE;
  if (SortKeyString == "offset")
    PrintingSettings.SortKey = LibScopeView::SortingKey::OFFSET;
  if (SortKeyString == "name")
    PrintingSettings.SortKey = LibScopeView::SortingKey::NAME;

  // Compile filter regexs.
  compileRegexs(RawFilters, PrintingSettings.Filters);
  compileRegexs(RawTreeFilters, PrintingSettings.TreeFilters);
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
                          GeneralHelp, PrintingSettings.QuietMode)
    }),

    ArgumentGroup("Output options", {
      Argument('a', "show-all",
               "Print all (expect advanced) objects and attributes", BasicHelp,
               [&](const Parser &) { PrintingSettings.showAll(); }),
      Argument('b', "show-brief",
               "Print all common objects and attributes (default)", BasicHelp,
               [&](const Parser &) { PrintingSettings.showBrief(); }),
      Argument::switchArg('t', "show-summary", "Print the summary table",
                          BasicHelp, ShowSummary),
      Argument('d', "output-dir", "[=<dir>]",
               "Print the output into a directory with each compile unit's "
               "output in a separate file. If no dir is given, then diva will "
               "use the input_file string to create an output directory.",
               BasicHelp,
               // This argument behaves like a switch that can also set a value.
               [&](const Parser &) { PrintingSettings.SplitOutput = true; },
               [&](const Parser &, const std::string &Opt) {
                 PrintingSettings.SplitOutput = true;
                 PrintingSettings.OutputDirectory = Opt;
               },
               [&](const Parser &) { PrintingSettings.SplitOutput = false; }),
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
          BasicHelp, RawFilters),
      Argument::multiStringArg(
          NSC, "filter-any", "text",
          "Only show objects with <text> in their instance name.",
          BasicHelp, PrintingSettings.FilterAnys),
      Argument::multiStringArg(
          NSC, "tree", "text",
          "Same as --filter, except the whole subtree of any matching object "
          "will printed.",
          BasicHelp, RawTreeFilters),
      Argument::multiStringArg(
          NSC, "tree-any", "text",
          "Same as --filter-any with the whole subtree.", BasicHelp,
        PrintingSettings.TreeFilterAnys),
    }),

    ArgumentGroup("More object options", {
      Argument(
          NSC, "show-none",
          "Print no objects, use this to hide all default objects and then "
          "individual objects can be added to the output using the options "
          "below.",
          MoreHelp, [&](const Parser &) { PrintingSettings.showNone(); }),
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
                          PrintingSettings.ShowAlias),
      Argument::switchArg(NSC, "show-block", "Print blocks*", MoreHelp,
                          PrintingSettings.ShowBlock),
      Argument::switchArg(NSC, "show-block-attributes",
                          "Print block attributes (e.g. try,catch)", MoreHelp,
                          PrintingSettings.ShowBlockAttributes),
      Argument::switchArg(NSC, "show-class", "Print classes*", MoreHelp,
                          PrintingSettings.ShowClass),
      Argument::switchArg(NSC, "show-enum", "Print enums*", MoreHelp,
                          PrintingSettings.ShowEnum),
      Argument::switchArg(NSC, "show-function", "Print functions*", MoreHelp,
                          PrintingSettings.ShowFunction),
      Argument::switchArg(NSC, "show-member", "Print class members*", MoreHelp,
                          PrintingSettings.ShowMember),
      Argument::switchArg(NSC, "show-namespace", "Print namespaces*", MoreHelp,
                          PrintingSettings.ShowNamespace),
      Argument::switchArg(NSC, "show-parameter", "Print parameters*", MoreHelp,
                          PrintingSettings.ShowParameter),
      Argument::switchArg(NSC, "show-primitivetype", "Print primitives",
                          MoreHelp, PrintingSettings.ShowPrimitiveType),
      Argument::switchArg(NSC, "show-struct", "Print structures*", MoreHelp,
                          PrintingSettings.ShowStruct),
      Argument::switchArg(NSC, "show-template", "Print templates*", MoreHelp,
                          PrintingSettings.ShowTemplate),
      Argument::switchArg(NSC, "show-union", "Print unions*", MoreHelp,
                          PrintingSettings.ShowUnion),
      Argument::switchArg(NSC, "show-using", "Print using instances*", MoreHelp,
                          PrintingSettings.ShowUsing),
      Argument::switchArg(NSC, "show-variable", "Print variables*", MoreHelp,
                          PrintingSettings.ShowVariable)
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
                          AdvancedHelp, PrintingSettings.ShowCodeline),
      Argument::switchArg(
          NSC, "show-codeline-attributes",
          "Print code line attributes (NewStatement, PrologueEnd)",
          AdvancedHelp, PrintingSettings.ShowCodelineAttributes),
      Argument::switchArg(NSC, "show-combined",
                          "Print combined scope attributes", AdvancedHelp,
                          PrintingSettings.ShowCombined),
      Argument::switchArg(NSC, "show-DWARF-offset", "Print DWARF offsets",
                          AdvancedHelp, PrintingSettings.ShowDWARFOffset),
      Argument::switchArg(NSC, "show-DWARF-parent",
                          "Print parent object DWARF offsets", AdvancedHelp,
                          PrintingSettings.ShowDWARFParent),
      Argument::switchArg(NSC, "show-DWARF-tag", "Print DWARF tag attribute",
                          AdvancedHelp, PrintingSettings.ShowDWARFTag),
      Argument::switchArg(NSC, "show-generated",
                          "Print compiler generated attributes", AdvancedHelp,
                          PrintingSettings.ShowGenerated),
      Argument::switchArg(NSC, "show-global", "Print \"global\" attributes",
                          AdvancedHelp, PrintingSettings.ShowIsGlobal),
      Argument::switchArg(NSC, "show-indent",
                          "Print indentations to reflect context (default on)",
                          AdvancedHelp, PrintingSettings.ShowIndent),
      Argument::switchArg(NSC, "show-level", "Print lexical block levels",
                          AdvancedHelp, PrintingSettings.ShowLevel),
      Argument::switchArg(NSC, "show-only-globals",
                          "Print only \"global\" objects", AdvancedHelp,
                          PrintingSettings.ShowOnlyGlobals),
      Argument::switchArg(NSC, "show-only-locals",
                          "Print only \"local\" objects", AdvancedHelp,
                          PrintingSettings.ShowOnlyLocals),
      Argument::switchArg(NSC, "show-qualified",
                          "Print \"qualified name\" attributes*", AdvancedHelp,
                          PrintingSettings.ShowQualified),
      Argument::switchArg(NSC, "show-underlying",
                          "Print underlying type attributes", AdvancedHelp,
                          PrintingSettings.ShowUnderlying),
      Argument::switchArg(NSC, "show-void",
                          "Print \"void\" type attributes (default on)",
                          AdvancedHelp, PrintingSettings.ShowVoid),
      Argument::switchArg(NSC, "show-zero", "Print zero line number attributes",
                          AdvancedHelp, PrintingSettings.ShowZeroLine),

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
