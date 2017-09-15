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

  EXPECT_EQ(Output.str(), "");

  EXPECT_TRUE(DOpt.InputFiles.empty());

  EXPECT_FALSE(DOptForQuietDefault.QuietMode);
  EXPECT_FALSE(DOpt.ShowSummary);
  EXPECT_FALSE(DOpt.SplitOutput);
  EXPECT_TRUE(DOpt.OutputDirectory.empty());
  EXPECT_EQ(DOpt.OutputFormats, std::set<OutputFormat>({OutputFormat::TEXT}));

  EXPECT_EQ(DOpt.SortKey, SortingKey::LINE);

  EXPECT_TRUE(DOpt.Filters.empty());
  EXPECT_TRUE(DOpt.FilterAnys.empty());
  EXPECT_TRUE(DOpt.WithChildrenFilters.empty());
  EXPECT_TRUE(DOpt.WithChildrenFilterAnys.empty());

  EXPECT_TRUE(DOpt.ShowAlias);
  EXPECT_TRUE(DOpt.ShowBlock);
  EXPECT_FALSE(DOpt.ShowBlockAttributes);
  EXPECT_TRUE(DOpt.ShowClass);
  EXPECT_TRUE(DOpt.ShowEnum);
  EXPECT_TRUE(DOpt.ShowFunction);
  EXPECT_TRUE(DOpt.ShowMember);
  EXPECT_TRUE(DOpt.ShowNamespace);
  EXPECT_TRUE(DOpt.ShowParameter);
  EXPECT_FALSE(DOpt.ShowPrimitivetype);
  EXPECT_TRUE(DOpt.ShowStruct);
  EXPECT_TRUE(DOpt.ShowTemplate);
  EXPECT_TRUE(DOpt.ShowUnion);
  EXPECT_TRUE(DOpt.ShowUsing);
  EXPECT_TRUE(DOpt.ShowVariable);

  EXPECT_FALSE(DOpt.ShowCodeline);
  EXPECT_FALSE(DOpt.ShowCodelineAttributes);
  EXPECT_FALSE(DOpt.ShowCombined);
  EXPECT_FALSE(DOpt.ShowDWARFOffset);
  EXPECT_FALSE(DOpt.ShowDWARFParent);
  EXPECT_FALSE(DOpt.ShowDWARFTag);
  EXPECT_FALSE(DOpt.ShowGenerated);
  EXPECT_FALSE(DOpt.ShowIsGlobal);
  EXPECT_TRUE(DOpt.ShowIndent);
  EXPECT_FALSE(DOpt.ShowLevel);
  EXPECT_FALSE(DOpt.ShowOnlyGlobals);
  EXPECT_FALSE(DOpt.ShowOnlyLocals);
  EXPECT_FALSE(DOpt.ShowQualified);
  EXPECT_FALSE(DOpt.ShowUnderlying);
  EXPECT_TRUE(DOpt.ShowVoid);
  EXPECT_FALSE(DOpt.ShowZeroLine);

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
    EXPECT_TRUE(DOpt.SplitOutput);
    EXPECT_EQ(DOpt.OutputDirectory, "");
  }
  {
    DivaOptions DOpt({"--output-dir"}, Output, Output, Output);
    EXPECT_EQ(Output.str(), "");
    EXPECT_TRUE(DOpt.SplitOutput);
    EXPECT_EQ(DOpt.OutputDirectory, "");
  }
  {
    DivaOptions DOpt({"--output-dir=test/dir/"}, Output, Output, Output);
    EXPECT_EQ(Output.str(), "");
    EXPECT_TRUE(DOpt.SplitOutput);
    EXPECT_EQ(DOpt.OutputDirectory, "test/dir/");
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
    EXPECT_EQ(DOpt.SortKey, SortingKey::LINE);
  }
  {
    DivaOptions DOpt({"--sort=line", "--sort=name"}, Output, Output, Output);
    EXPECT_EQ(Output.str(), "");
    EXPECT_EQ(DOpt.SortKey, SortingKey::NAME);
  }
  {
    DivaOptions DOpt({"--sort=line", "--sort=offset"}, Output, Output, Output);
    EXPECT_EQ(Output.str(), "");
    EXPECT_EQ(DOpt.SortKey, SortingKey::OFFSET);
  }
}

TEST(DivaOptions, Filters) {
  std::stringstream Output;
  DivaOptions DOpt({"--filter=f1", "--filter=f2,f3", "--filter-any=fa1",
                    "--filter-any=fa2,fa3", "--tree=t1", "--tree=t2,t3",
                    "--tree-any=ta1", "--tree-any=ta2,ta3"},
                   Output, Output, Output);
  EXPECT_EQ(Output.str(), "");
  EXPECT_EQ(DOpt.Filters, std::vector<std::string>({"f1", "f2", "f3"}));
  EXPECT_EQ(DOpt.FilterAnys, std::vector<std::string>({"fa1", "fa2", "fa3"}));
  EXPECT_EQ(DOpt.WithChildrenFilters,
            std::vector<std::string>({"t1", "t2", "t3"}));
  EXPECT_EQ(DOpt.WithChildrenFilterAnys,
            std::vector<std::string>({"ta1", "ta2", "ta3"}));
}

TEST(DivaOptions, ShowNone) {
  std::stringstream Output;
  DivaOptions DOpt({"--show-all", "--show-none"}, Output, Output, Output);
  EXPECT_EQ(Output.str(), "");
  EXPECT_FALSE(DOpt.ShowAlias);
  EXPECT_FALSE(DOpt.ShowBlock);
  EXPECT_FALSE(DOpt.ShowBlockAttributes);
  EXPECT_FALSE(DOpt.ShowClass);
  EXPECT_FALSE(DOpt.ShowEnum);
  EXPECT_FALSE(DOpt.ShowFunction);
  EXPECT_FALSE(DOpt.ShowMember);
  EXPECT_FALSE(DOpt.ShowNamespace);
  EXPECT_FALSE(DOpt.ShowParameter);
  EXPECT_FALSE(DOpt.ShowPrimitivetype);
  EXPECT_FALSE(DOpt.ShowStruct);
  EXPECT_FALSE(DOpt.ShowTemplate);
  EXPECT_FALSE(DOpt.ShowUnion);
  EXPECT_FALSE(DOpt.ShowUsing);
  EXPECT_FALSE(DOpt.ShowVariable);
}

TEST(DivaOptions, ShowBrief) {
  std::stringstream Output;
  DivaOptions DOpt({"--show-none", "--show-brief"}, Output, Output, Output);
  EXPECT_EQ(Output.str(), "");
  EXPECT_TRUE(DOpt.ShowAlias);
  EXPECT_TRUE(DOpt.ShowBlock);
  EXPECT_FALSE(DOpt.ShowBlockAttributes);
  EXPECT_TRUE(DOpt.ShowClass);
  EXPECT_TRUE(DOpt.ShowEnum);
  EXPECT_TRUE(DOpt.ShowFunction);
  EXPECT_TRUE(DOpt.ShowMember);
  EXPECT_TRUE(DOpt.ShowNamespace);
  EXPECT_TRUE(DOpt.ShowParameter);
  EXPECT_FALSE(DOpt.ShowPrimitivetype);
  EXPECT_TRUE(DOpt.ShowStruct);
  EXPECT_TRUE(DOpt.ShowTemplate);
  EXPECT_TRUE(DOpt.ShowUnion);
  EXPECT_TRUE(DOpt.ShowUsing);
  EXPECT_TRUE(DOpt.ShowVariable);
}

TEST(DivaOptions, ShowAll) {
  std::stringstream Output;
  DivaOptions DOpt({"--show-none", "--show-all"}, Output, Output, Output);
  EXPECT_EQ(Output.str(), "");
  EXPECT_TRUE(DOpt.ShowAlias);
  EXPECT_TRUE(DOpt.ShowBlock);
  EXPECT_TRUE(DOpt.ShowBlockAttributes);
  EXPECT_TRUE(DOpt.ShowClass);
  EXPECT_TRUE(DOpt.ShowEnum);
  EXPECT_TRUE(DOpt.ShowFunction);
  EXPECT_TRUE(DOpt.ShowMember);
  EXPECT_TRUE(DOpt.ShowNamespace);
  EXPECT_TRUE(DOpt.ShowParameter);
  EXPECT_TRUE(DOpt.ShowPrimitivetype);
  EXPECT_TRUE(DOpt.ShowStruct);
  EXPECT_TRUE(DOpt.ShowTemplate);
  EXPECT_TRUE(DOpt.ShowUnion);
  EXPECT_TRUE(DOpt.ShowUsing);
  EXPECT_TRUE(DOpt.ShowVariable);
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

  CHECK_FLAG("quiet", QuietMode);
  CHECK_FLAG("show-summary", ShowSummary);

  CHECK_FLAG("show-alias", ShowAlias);
  CHECK_FLAG("show-block", ShowBlock);
  CHECK_FLAG("show-block-attributes", ShowBlockAttributes);
  CHECK_FLAG("show-class", ShowClass);
  CHECK_FLAG("show-enum", ShowEnum);
  CHECK_FLAG("show-function", ShowFunction);
  CHECK_FLAG("show-member", ShowMember);
  CHECK_FLAG("show-namespace", ShowNamespace);
  CHECK_FLAG("show-parameter", ShowParameter);
  CHECK_FLAG("show-primitivetype", ShowPrimitivetype);
  CHECK_FLAG("show-struct", ShowStruct);
  CHECK_FLAG("show-template", ShowTemplate);
  CHECK_FLAG("show-union", ShowUnion);
  CHECK_FLAG("show-using", ShowUsing);
  CHECK_FLAG("show-variable", ShowVariable);

  CHECK_FLAG("show-codeline", ShowCodeline);
  CHECK_FLAG("show-codeline-attributes", ShowCodelineAttributes);
  CHECK_FLAG("show-combined", ShowCombined);
  CHECK_FLAG("show-DWARF-offset", ShowDWARFOffset);
  CHECK_FLAG("show-DWARF-parent", ShowDWARFParent);
  CHECK_FLAG("show-DWARF-tag", ShowDWARFTag);
  CHECK_FLAG("show-generated", ShowGenerated);
  CHECK_FLAG("show-global", ShowIsGlobal);
  CHECK_FLAG("show-indent", ShowIndent);
  CHECK_FLAG("show-level", ShowLevel);
  CHECK_FLAG("show-only-globals", ShowOnlyGlobals);
  CHECK_FLAG("show-only-locals", ShowOnlyLocals);
  CHECK_FLAG("show-qualified", ShowQualified);
  CHECK_FLAG("show-underlying", ShowUnderlying);
  CHECK_FLAG("show-void", ShowVoid);
  CHECK_FLAG("show-zero", ShowZeroLine);

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
