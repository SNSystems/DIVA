//===-- UnitTests/TestDiva/TestDivaOptions.cpp ------------------*- C++ -*-===//
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
/// Tests for the Diva option parsing.
///
//===----------------------------------------------------------------------===//

#include "DivaOptions.h"

#include "gtest/gtest.h"

using ::testing::ExitedWithCode;

TEST(DivaOptions, Defaults) {
  // Passing no arguments results in an early exit, so pass --quiet to check
  // all the other defaults and  pass --summary to test the default for
  // QuietMode.
  std::stringstream Output;
  DivaOptions DOpt({"--quiet"}, Output, Output, Output);
  DivaOptions DOptForQuietDefault({"--show-summary"}, Output, Output, Output);
  LibScopeView::PrintSettings &PSet = DOpt.PrintingSettings;

  EXPECT_EQ(Output.str(), "");

  EXPECT_TRUE(DOpt.InputFiles.empty());

  EXPECT_FALSE(DOptForQuietDefault.PrintingSettings.QuietMode);
  EXPECT_FALSE(DOpt.ShowSummary);
  EXPECT_FALSE(PSet.SplitOutput);
  EXPECT_TRUE(PSet.OutputDirectory.empty());
  EXPECT_EQ(DOpt.OutputFormats, std::set<OutputFormat>({OutputFormat::TEXT}));

  EXPECT_EQ(PSet.SortKey, LibScopeView::SortingKey::LINE);

  EXPECT_TRUE(PSet.Filters.empty());
  EXPECT_TRUE(PSet.FilterAnys.empty());
  EXPECT_TRUE(PSet.WithChildrenFilters.empty());
  EXPECT_TRUE(PSet.WithChildrenFilterAnys.empty());

  EXPECT_TRUE(PSet.ShowAlias);
  EXPECT_TRUE(PSet.ShowBlock);
  EXPECT_FALSE(PSet.ShowBlockAttributes);
  EXPECT_TRUE(PSet.ShowClass);
  EXPECT_TRUE(PSet.ShowEnum);
  EXPECT_TRUE(PSet.ShowFunction);
  EXPECT_TRUE(PSet.ShowMember);
  EXPECT_TRUE(PSet.ShowNamespace);
  EXPECT_TRUE(PSet.ShowParameter);
  EXPECT_FALSE(PSet.ShowPrimitiveType);
  EXPECT_TRUE(PSet.ShowStruct);
  EXPECT_TRUE(PSet.ShowTemplate);
  EXPECT_TRUE(PSet.ShowUnion);
  EXPECT_TRUE(PSet.ShowUsing);
  EXPECT_TRUE(PSet.ShowVariable);

  EXPECT_FALSE(PSet.ShowCodeline);
  EXPECT_FALSE(PSet.ShowCodelineAttributes);
  EXPECT_FALSE(PSet.ShowCombined);
  EXPECT_FALSE(PSet.ShowDWARFOffset);
  EXPECT_FALSE(PSet.ShowDWARFParent);
  EXPECT_FALSE(PSet.ShowDWARFTag);
  EXPECT_FALSE(PSet.ShowGenerated);
  EXPECT_FALSE(PSet.ShowIsGlobal);
  EXPECT_TRUE(PSet.ShowIndent);
  EXPECT_FALSE(PSet.ShowLevel);
  EXPECT_FALSE(PSet.ShowOnlyGlobals);
  EXPECT_FALSE(PSet.ShowOnlyLocals);
  EXPECT_FALSE(PSet.ShowQualified);
  EXPECT_FALSE(PSet.ShowUnderlying);
  EXPECT_TRUE(PSet.ShowVoid);
  EXPECT_FALSE(PSet.ShowZeroLine);

  EXPECT_FALSE(DOpt.ShowPerformanceTime);
  EXPECT_FALSE(DOpt.ShowPerformanceMemory);
  EXPECT_FALSE(DOpt.ShowScopeAllocation);
  EXPECT_FALSE(DOpt.ShowStringPoolInfo);
  EXPECT_FALSE(DOpt.DumpStringPool);
}

TEST(DivaOptions, InputFiles) {
  std::stringstream Output;
  DivaOptions DOpt({"input1.o", "--quiet", "input2.elf", "-d", "input3.o"},
                   Output, Output, Output);

  EXPECT_EQ(Output.str(), "");
  EXPECT_EQ(DOpt.InputFiles,
            std::vector<std::string>({"input1.o", "input2.elf", "input3.o"}));
}

TEST(DivaOptions, OutputDir) {
  std::stringstream Output;

  {
    DivaOptions DOpt({"-d"}, Output, Output, Output);
    EXPECT_EQ(Output.str(), "");
    EXPECT_TRUE(DOpt.PrintingSettings.SplitOutput);
    EXPECT_EQ(DOpt.PrintingSettings.OutputDirectory, "");
  }
  {
    DivaOptions DOpt({"--output-dir"}, Output, Output, Output);
    EXPECT_EQ(Output.str(), "");
    EXPECT_TRUE(DOpt.PrintingSettings.SplitOutput);
    EXPECT_EQ(DOpt.PrintingSettings.OutputDirectory, "");
  }
  {
    DivaOptions DOpt({"--output-dir=test/dir/"}, Output, Output, Output);
    EXPECT_EQ(Output.str(), "");
    EXPECT_TRUE(DOpt.PrintingSettings.SplitOutput);
    EXPECT_EQ(DOpt.PrintingSettings.OutputDirectory, "test/dir/");
  }
}

TEST(DivaOptions, EarlyExitArgs) {
  std::stringstream Output;
  // Version.
  EXPECT_EXIT({ DivaOptions({"--version"}, Output, Output, Output); },
              ExitedWithCode(0), "");
  EXPECT_EXIT({ DivaOptions({"-v"}, Output, Output, Output); },
              ExitedWithCode(0), "");

  // No Args.
  EXPECT_EXIT({ DivaOptions({}, Output, Output, Output); }, ExitedWithCode(1),
              "");

  // Help.
  EXPECT_EXIT({ DivaOptions({"--help"}, Output, Output, Output); },
              ExitedWithCode(0), "");
  EXPECT_EXIT({ DivaOptions({"-h"}, Output, Output, Output); },
              ExitedWithCode(0), "");

  // Help more.
  EXPECT_EXIT({ DivaOptions({"--help-more"}, Output, Output, Output); },
              ExitedWithCode(0), "");

  // Help advanced.
  EXPECT_EXIT({ DivaOptions({"--help-advanced"}, Output, Output, Output); },
              ExitedWithCode(0), "");
}

TEST(DivaOptions, OutputFormat) {
  std::stringstream Output;

  {
    DivaOptions DOpt({"--output=text"}, Output, Output, Output);
    EXPECT_EQ(Output.str(), "");
    EXPECT_EQ(DOpt.OutputFormats, std::set<OutputFormat>({OutputFormat::TEXT}));
  }

  {
    DivaOptions DOpt({"--output=yaml"}, Output, Output, Output);
    EXPECT_EQ(Output.str(), "");
    EXPECT_EQ(DOpt.OutputFormats, std::set<OutputFormat>({OutputFormat::YAML}));
  }

  {
    DivaOptions DOpt({"--output=text", "--output=yaml"}, Output, Output,
                     Output);
    EXPECT_EQ(Output.str(), "");
    EXPECT_EQ(DOpt.OutputFormats,
              std::set<OutputFormat>({OutputFormat::TEXT, OutputFormat::YAML}));
  }
}

TEST(DivaOptions, Sorting) {
  std::stringstream Output;

  {
    DivaOptions DOpt({"--sort=name", "--sort=line"}, Output, Output, Output);
    EXPECT_EQ(Output.str(), "");
    EXPECT_EQ(DOpt.PrintingSettings.SortKey, LibScopeView::SortingKey::LINE);
  }
  {
    DivaOptions DOpt({"--sort=line", "--sort=name"}, Output, Output, Output);
    EXPECT_EQ(Output.str(), "");
    EXPECT_EQ(DOpt.PrintingSettings.SortKey, LibScopeView::SortingKey::NAME);
  }
  {
    DivaOptions DOpt({"--sort=line", "--sort=offset"}, Output, Output, Output);
    EXPECT_EQ(Output.str(), "");
    EXPECT_EQ(DOpt.PrintingSettings.SortKey, LibScopeView::SortingKey::OFFSET);
  }
}

TEST(DivaOptions, Filters) {
  std::stringstream Output;
  DivaOptions DOpt({"--filter=f1", "--filter=f2,f3", "--filter-any=fa1",
                    "--filter-any=fa2,fa3", "--tree=t1", "--tree=t2,t3",
                    "--tree-any=ta1", "--tree-any=ta2,ta3"},
                   Output, Output, Output);
  EXPECT_EQ(Output.str(), "");
  EXPECT_EQ(DOpt.PrintingSettings.FilterAnys,
            std::vector<std::string>({"fa1", "fa2", "fa3"}));
  EXPECT_EQ(DOpt.PrintingSettings.WithChildrenFilterAnys,
            std::vector<std::string>({"ta1", "ta2", "ta3"}));

  ASSERT_EQ(DOpt.PrintingSettings.Filters.size(), 3U);
  EXPECT_TRUE(std::regex_match("f1", DOpt.PrintingSettings.Filters[0]));
  EXPECT_TRUE(std::regex_match("f2", DOpt.PrintingSettings.Filters[1]));
  EXPECT_TRUE(std::regex_match("f3", DOpt.PrintingSettings.Filters[2]));

  ASSERT_EQ(DOpt.PrintingSettings.WithChildrenFilters.size(), 3U);
  EXPECT_TRUE(std::regex_match(
      "t1", DOpt.PrintingSettings.WithChildrenFilters[0]));
  EXPECT_TRUE(std::regex_match(
      "t2", DOpt.PrintingSettings.WithChildrenFilters[1]));
  EXPECT_TRUE(std::regex_match(
      "t3", DOpt.PrintingSettings.WithChildrenFilters[2]));
}

TEST(DivaOptions, ShowNone) {
  std::stringstream Output;
  DivaOptions DOpt({"--show-all", "--show-none"}, Output, Output, Output);
  LibScopeView::PrintSettings &PSet = DOpt.PrintingSettings;

  EXPECT_EQ(Output.str(), "");
  EXPECT_FALSE(PSet.ShowAlias);
  EXPECT_FALSE(PSet.ShowBlock);
  EXPECT_FALSE(PSet.ShowBlockAttributes);
  EXPECT_FALSE(PSet.ShowClass);
  EXPECT_FALSE(PSet.ShowEnum);
  EXPECT_FALSE(PSet.ShowFunction);
  EXPECT_FALSE(PSet.ShowMember);
  EXPECT_FALSE(PSet.ShowNamespace);
  EXPECT_FALSE(PSet.ShowParameter);
  EXPECT_FALSE(PSet.ShowPrimitiveType);
  EXPECT_FALSE(PSet.ShowStruct);
  EXPECT_FALSE(PSet.ShowTemplate);
  EXPECT_FALSE(PSet.ShowUnion);
  EXPECT_FALSE(PSet.ShowUsing);
  EXPECT_FALSE(PSet.ShowVariable);
}

TEST(DivaOptions, ShowBrief) {
  std::stringstream Output;
  DivaOptions DOpt({"--show-none", "--show-brief"}, Output, Output, Output);
  LibScopeView::PrintSettings &PSet = DOpt.PrintingSettings;

  EXPECT_EQ(Output.str(), "");
  EXPECT_TRUE(PSet.ShowAlias);
  EXPECT_TRUE(PSet.ShowBlock);
  EXPECT_FALSE(PSet.ShowBlockAttributes);
  EXPECT_TRUE(PSet.ShowClass);
  EXPECT_TRUE(PSet.ShowEnum);
  EXPECT_TRUE(PSet.ShowFunction);
  EXPECT_TRUE(PSet.ShowMember);
  EXPECT_TRUE(PSet.ShowNamespace);
  EXPECT_TRUE(PSet.ShowParameter);
  EXPECT_FALSE(PSet.ShowPrimitiveType);
  EXPECT_TRUE(PSet.ShowStruct);
  EXPECT_TRUE(PSet.ShowTemplate);
  EXPECT_TRUE(PSet.ShowUnion);
  EXPECT_TRUE(PSet.ShowUsing);
  EXPECT_TRUE(PSet.ShowVariable);
}

TEST(DivaOptions, ShowAll) {
  std::stringstream Output;
  DivaOptions DOpt({"--show-none", "--show-all"}, Output, Output, Output);
  LibScopeView::PrintSettings &PSet = DOpt.PrintingSettings;

  EXPECT_EQ(Output.str(), "");
  EXPECT_TRUE(PSet.ShowAlias);
  EXPECT_TRUE(PSet.ShowBlock);
  EXPECT_TRUE(PSet.ShowBlockAttributes);
  EXPECT_TRUE(PSet.ShowClass);
  EXPECT_TRUE(PSet.ShowEnum);
  EXPECT_TRUE(PSet.ShowFunction);
  EXPECT_TRUE(PSet.ShowMember);
  EXPECT_TRUE(PSet.ShowNamespace);
  EXPECT_TRUE(PSet.ShowParameter);
  EXPECT_TRUE(PSet.ShowPrimitiveType);
  EXPECT_TRUE(PSet.ShowStruct);
  EXPECT_TRUE(PSet.ShowTemplate);
  EXPECT_TRUE(PSet.ShowUnion);
  EXPECT_TRUE(PSet.ShowUsing);
  EXPECT_TRUE(PSet.ShowVariable);
}

TEST(DivaOptions, FlagArguments) {
  std::stringstream Output;

// Test each flag argument by setting it on and then off.
#define CHECK_FLAG(ARG_NAME, VAL)                                              \
  do {                                                                         \
    DivaOptions DOpt1({"--" ARG_NAME}, Output, Output, Output);                \
    EXPECT_TRUE(DOpt1.VAL);                                                    \
    DivaOptions DOpt2({"--no-" ARG_NAME}, Output, Output, Output);             \
    EXPECT_FALSE(DOpt2.VAL);                                                   \
  } while (0)

  CHECK_FLAG("quiet", PrintingSettings.QuietMode);
  CHECK_FLAG("show-summary", ShowSummary);

  CHECK_FLAG("show-alias", PrintingSettings.ShowAlias);
  CHECK_FLAG("show-block", PrintingSettings.ShowBlock);
  CHECK_FLAG("show-block-attributes", PrintingSettings.ShowBlockAttributes);
  CHECK_FLAG("show-class", PrintingSettings.ShowClass);
  CHECK_FLAG("show-enum", PrintingSettings.ShowEnum);
  CHECK_FLAG("show-function", PrintingSettings.ShowFunction);
  CHECK_FLAG("show-member", PrintingSettings.ShowMember);
  CHECK_FLAG("show-namespace", PrintingSettings.ShowNamespace);
  CHECK_FLAG("show-parameter", PrintingSettings.ShowParameter);
  CHECK_FLAG("show-primitivetype", PrintingSettings.ShowPrimitiveType);
  CHECK_FLAG("show-struct", PrintingSettings.ShowStruct);
  CHECK_FLAG("show-template", PrintingSettings.ShowTemplate);
  CHECK_FLAG("show-union", PrintingSettings.ShowUnion);
  CHECK_FLAG("show-using", PrintingSettings.ShowUsing);
  CHECK_FLAG("show-variable", PrintingSettings.ShowVariable);

  CHECK_FLAG("show-codeline", PrintingSettings.ShowCodeline);
  CHECK_FLAG("show-codeline-attributes",
             PrintingSettings.ShowCodelineAttributes);
  CHECK_FLAG("show-combined", PrintingSettings.ShowCombined);
  CHECK_FLAG("show-DWARF-offset", PrintingSettings.ShowDWARFOffset);
  CHECK_FLAG("show-DWARF-parent", PrintingSettings.ShowDWARFParent);
  CHECK_FLAG("show-DWARF-tag", PrintingSettings.ShowDWARFTag);
  CHECK_FLAG("show-generated", PrintingSettings.ShowGenerated);
  CHECK_FLAG("show-global", PrintingSettings.ShowIsGlobal);
  CHECK_FLAG("show-indent", PrintingSettings.ShowIndent);
  CHECK_FLAG("show-level", PrintingSettings.ShowLevel);
  CHECK_FLAG("show-only-globals", PrintingSettings.ShowOnlyGlobals);
  CHECK_FLAG("show-only-locals", PrintingSettings.ShowOnlyLocals);
  CHECK_FLAG("show-qualified", PrintingSettings.ShowQualified);
  CHECK_FLAG("show-underlying", PrintingSettings.ShowUnderlying);
  CHECK_FLAG("show-void", PrintingSettings.ShowVoid);
  CHECK_FLAG("show-zero", PrintingSettings.ShowZeroLine);

  CHECK_FLAG("performance-time", ShowPerformanceTime);
  CHECK_FLAG("performance-memory", ShowPerformanceMemory);
  CHECK_FLAG("scope-allocation", ShowScopeAllocation);
  CHECK_FLAG("info-string-pool", ShowStringPoolInfo);
  CHECK_FLAG("dump-string-pool", DumpStringPool);

  EXPECT_EQ(Output.str(), "");
}

TEST(DivaOptions, Errors) {
  std::stringstream Output;

  // UnrecognisedArg.
  EXPECT_EXIT(
      { DivaOptions DOpt1({"--not-an-arg"}, Output, Output, std::cerr); },
      ExitedWithCode(1),
      "ERR_CMD_UNKNOWN_ARG: Unknown argument '--not-an-arg'.");

  // ArgumentValueRequired.
  EXPECT_EXIT(
      { DivaOptions DOpt1({"--output"}, Output, Output, std::cerr); },
      ExitedWithCode(1),
      "ERR_CMD_MISSING_VALUE: Argument '--output' requires a value.");

  // UnexpectedArgumentValue.
  EXPECT_EXIT(
      { DivaOptions DOpt1({"--quiet=foo"}, Output, Output, std::cerr); },
      ExitedWithCode(1),
      "ERR_CMD_UNEXPECTED_VALUE: Argument '--quiet' can not be given a value "
      "\\('foo'\\).");

  // UnexpectedNegative.
  EXPECT_EXIT(
      { DivaOptions DOpt1({"--no-show-brief"}, Output, Output, std::cerr); },
      ExitedWithCode(1),
      "ERR_CMD_UNKNOWN_ARG: Unknown argument '--no-show-brief'.");

  // ShortcutWithOption.
  EXPECT_EXIT({ DivaOptions DOpt1({"-d=foo"}, Output, Output, std::cerr); },
              ExitedWithCode(1),
              "ERR_CMD_SHORTCUT_WITH_VALUE: Shortcut arguments can not be "
              "given values '-d=foo'.");

  // InvalidChoice.
  EXPECT_EXIT(
      { DivaOptions DOpt1({"--output=bad"}, Output, Output, std::cerr); },
      ExitedWithCode(1),
      "ERR_CMD_INVALID_VALUE: Argument '--output' was given the invalid value "
      "'bad'.");
}
