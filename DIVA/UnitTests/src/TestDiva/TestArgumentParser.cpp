//===-- UnitTests/TestDiva/TestArgumentParser.cpp ---------------*- C++ -*-===//
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
/// Tests for the ArgumentParser.
///
//===----------------------------------------------------------------------===//

#include "ArgumentParser.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include <stdexcept>

using namespace ArgumentParser;

using ::testing::InSequence;
using ::testing::Ref;

namespace {

// Useful for testing argument processing functions where the parser doesn't
// matter.
Parser EmptyParser({});

// Mock out argument, with the process functions as methds. Call getArg() to get
// an Argument that will call the corresponding mock methods.
class MockArgument {
public:
  explicit MockArgument(char Shortcut, const std::string &Name,
                        const std::string &NameHelpSuffix,
                        const std::string &Help,
                        const Argument::HelpLevel ArgHelpLevel,
                        bool HelpAlreadyFormatted = false)
      : Shortcut(Shortcut), Name(Name), NameHelpSuffix(NameHelpSuffix),
        Help(Help), ArgHelpLevel(ArgHelpLevel),
        HelpAlreadyFormatted(HelpAlreadyFormatted) {}

  Argument getArg() {
    return Argument(Shortcut, Name, NameHelpSuffix, Help, ArgHelpLevel,
                    [&](const Parser &P) { processArg(P); },
                    [&](const Parser &P, const std::string &Opt) {
                      processArgOption(P, Opt);
                    },
                    [&](const Parser &P) { processNegativeArg(P); },
                    HelpAlreadyFormatted);
  }

  const char Shortcut;
  const std::string Name;
  const std::string NameHelpSuffix;
  const std::string Help;
  const Argument::HelpLevel ArgHelpLevel;
  const bool HelpAlreadyFormatted;

  MOCK_METHOD1(processArg, void(const Parser &));
  MOCK_METHOD2(processArgOption, void(const Parser &, const std::string &Opt));
  MOCK_METHOD1(processNegativeArg, void(const Parser &));
};

typedef testing::StrictMock<MockArgument> StrictMockArgument;
typedef testing::NiceMock<MockArgument> NiceMockArgument;

} // end anonymous namespace.

// Extension of the EXPECT_THROW macro that also tests the thrown exceptions
// Arg value.
#define EXPECT_THROW_WITH_ARG(STATEMENT, EXCEPTION_TYPE, ARG_NAME)             \
  EXPECT_THROW(                                                                \
      {                                                                        \
        try {                                                                  \
          STATEMENT                                                            \
        } catch (const EXCEPTION_TYPE &e) {                                    \
          EXPECT_EQ(e.Arg, ARG_NAME)                                           \
              << "Exception thrown with wrong argument string";                \
          throw;                                                               \
        }                                                                      \
      },                                                                       \
      EXCEPTION_TYPE);

// Extension of the EXPECT_THROW macro that also tests the thrown exceptions
// Arg and Option values.
#define EXPECT_THROW_WITH_ARG_AND_OPT(STATEMENT, EXCEPTION_TYPE, ARG_NAME,     \
                                      ARG_VALUE)                               \
  EXPECT_THROW(                                                                \
      {                                                                        \
        try {                                                                  \
          STATEMENT                                                            \
        } catch (const EXCEPTION_TYPE &e) {                                    \
          EXPECT_EQ(e.Arg, ARG_NAME)                                           \
              << "Exception thrown with wrong argument string";                \
          EXPECT_EQ(e.ArgVal, ARG_VALUE)                                       \
              << "Exception thrown with wrong option string";                  \
          throw;                                                               \
        }                                                                      \
      },                                                                       \
      EXCEPTION_TYPE);

TEST(ArgumentParser, UsageExample) {
  // Should be kept up to date with the usage example at the top of
  // ArgumentParser.h

  bool HelpPrinted = false;
  bool AVal = false;
  bool BVal = true;
  std::string OutFile;
  std::stringstream HelpOut;

  const static Argument::HelpLevel HelpLev = 1;

  Parser P({
    ArgumentGroup("General arguments", {
      Argument::helpArg('h', "help", "print help", HelpLev,
                        "USAGE: a.exe [args]", {HelpLev}, HelpPrinted, HelpOut),
      Argument::switchArg('a', "do-a", "does a", HelpLev, AVal),
      Argument::switchArg('b', "do-b", "does b", HelpLev, BVal),
      Argument::stringArg(Argument::NoShortcut, "ofile", "PATH", "output file",
                          HelpLev, OutFile)
    })
  });

  std::vector<std::string> Positional;
  try {
    Positional = P.parseCommandLineArgs(
        {"-a", "--no-do-b", "--ofile=foo.txt", "bar.txt"});
  } catch (ArgumentParser::UnrecognisedArg &Err) {
    FAIL() << "Unknown arg: " << Err.Arg << "\n";
  }

  EXPECT_FALSE(HelpPrinted);
  EXPECT_TRUE(AVal);
  EXPECT_FALSE(BVal);
  EXPECT_EQ(OutFile, "foo.txt");
  EXPECT_EQ(Positional, std::vector<std::string>({"bar.txt"}));
}

TEST(ArgumentParser, Argument) {
  {
    std::string ProcessOut;
    Argument Arg(
        'a', "arg", "=sfx", "help", 2,
        [&](const Parser &) { ProcessOut = "P"; },
        [&](const Parser &, const std::string &Opt) { ProcessOut = Opt; },
        [&](const Parser &) { ProcessOut = "N"; }, true);

    EXPECT_EQ(Arg.Shortcut, 'a');
    EXPECT_EQ(Arg.Name, "arg");
    EXPECT_EQ(Arg.NameHelpSuffix, "=sfx");
    EXPECT_EQ(Arg.Help, "help");
    EXPECT_EQ(Arg.ArgHelpLevel, 2);
    EXPECT_EQ(Arg.HelpAlreadyFormatted, true);

    EXPECT_TRUE(ProcessOut.empty());
    Arg.ProcessArg(EmptyParser);
    EXPECT_EQ(ProcessOut, "P");
    Arg.ProcessArgWithValue(EmptyParser, "O");
    EXPECT_EQ(ProcessOut, "O");
    Arg.ProcessNegativeArg(EmptyParser);
    EXPECT_EQ(ProcessOut, "N");
  }

  // Argument that only allows ProcessArg.
  {
    std::string ProcessOut;
    Argument Arg('a', "arg", "help", 3,
                 [&](const Parser &) { ProcessOut = "P"; });

    EXPECT_EQ(Arg.Shortcut, 'a');
    EXPECT_EQ(Arg.Name, "arg");
    EXPECT_EQ(Arg.NameHelpSuffix, "");
    EXPECT_EQ(Arg.Help, "help");
    EXPECT_EQ(Arg.ArgHelpLevel, 3);
    EXPECT_EQ(Arg.HelpAlreadyFormatted, false);

    EXPECT_TRUE(ProcessOut.empty());
    Arg.ProcessArg(EmptyParser);
    EXPECT_EQ(ProcessOut, "P");

    EXPECT_THROW_WITH_ARG_AND_OPT(
        { Arg.ProcessArgWithValue(EmptyParser, "O"); }, UnexpectedArgumentValue,
        "--arg", "O");
    EXPECT_THROW_WITH_ARG({ Arg.ProcessNegativeArg(EmptyParser); },
                          UnexpectedNegative, "--no-arg");
  }

  // Argument that only allows ProcessArg and ProcessNegativeArg.
  {
    std::string ProcessOut;
    Argument Arg('a', "arg", "", "help", 1,
                 [&](const Parser &) { ProcessOut = "P"; }, nullptr,
                 [&](const Parser &) { ProcessOut = "N"; });

    EXPECT_EQ(Arg.Shortcut, 'a');
    EXPECT_EQ(Arg.Name, "arg");
    EXPECT_EQ(Arg.NameHelpSuffix, "");
    EXPECT_EQ(Arg.Help, "help");
    EXPECT_EQ(Arg.ArgHelpLevel, 1);
    EXPECT_EQ(Arg.HelpAlreadyFormatted, false);

    EXPECT_TRUE(ProcessOut.empty());
    Arg.ProcessArg(EmptyParser);
    EXPECT_EQ(ProcessOut, "P");
    Arg.ProcessNegativeArg(EmptyParser);
    EXPECT_EQ(ProcessOut, "N");

    EXPECT_THROW_WITH_ARG_AND_OPT(
        { Arg.ProcessArgWithValue(EmptyParser, "O"); }, UnexpectedArgumentValue,
        "--arg", "O");
  }

  // Argument that only allows ProcessArgWithOption.
  {
    std::string ProcessOut;
    Argument Arg(
        'a', "arg", "", "help", 1, nullptr,
        [&](const Parser &, const std::string &Opt) { ProcessOut = Opt; },
        nullptr);

    EXPECT_EQ(Arg.Shortcut, 'a');
    EXPECT_EQ(Arg.Name, "arg");
    EXPECT_EQ(Arg.NameHelpSuffix, "");
    EXPECT_EQ(Arg.Help, "help");
    EXPECT_EQ(Arg.ArgHelpLevel, 1);
    EXPECT_EQ(Arg.HelpAlreadyFormatted, false);

    EXPECT_TRUE(ProcessOut.empty());
    Arg.ProcessArgWithValue(EmptyParser, "O");
    EXPECT_EQ(ProcessOut, "O");

    EXPECT_THROW_WITH_ARG({ Arg.ProcessArg(EmptyParser); },
                          ArgumentValueRequired, "--arg");
    EXPECT_THROW_WITH_ARG({ Arg.ProcessNegativeArg(EmptyParser); },
                          UnexpectedNegative, "--no-arg");
  }
}

TEST(ArgumentParser, ArgumentGroup) {
  ArgumentGroup Group("Args", {
    Argument('A', "A", "", "Help", false, nullptr, nullptr, nullptr),
    Argument('B', "B", "", "Help", false, nullptr, nullptr, nullptr),
    Argument('C', "C", "", "Help", false, nullptr, nullptr, nullptr),
  });

  EXPECT_EQ(Group.Title, "Args");
  ASSERT_EQ(Group.Args.size(), 3U);
  EXPECT_EQ(Group.Args[0].Shortcut, 'A');
  EXPECT_EQ(Group.Args[1].Shortcut, 'B');
  EXPECT_EQ(Group.Args[2].Shortcut, 'C');
}

TEST(ArgumentParser, Parser) {
  // Test ArgumentParser with valid arguments.
  // This also tests the Argument base class at the same time.

  StrictMockArgument Arg1('a', "arg-1", "", "", 0);
  StrictMockArgument Arg2('b', "arg-2", "", "", 0);
  StrictMockArgument Arg3(Argument::NoShortcut, "arg-3", "", "", 0);
  StrictMockArgument Arg4(Argument::NoShortcut, "arg-4", "", "", 0);

  Parser ArgParser({
    ArgumentGroup("Args", {
      Arg1.getArg(),
      Arg2.getArg(),
      Arg3.getArg(),
      Arg4.getArg()
    })
  });

  // Test positional args.
  {
    std::vector<std::string> CMD = {"positional", "file/path.txt", "test"};
    auto Positional = ArgParser.parseCommandLineArgs(CMD);
    EXPECT_EQ(Positional, CMD);
  }

  // Test short args.
  {
    InSequence OrderedCalls;
    std::vector<std::string> CMD = {"-a", "-b", "-a", "-aabb"};
    EXPECT_CALL(Arg1, processArg(Ref(ArgParser)));
    EXPECT_CALL(Arg2, processArg(Ref(ArgParser)));
    EXPECT_CALL(Arg1, processArg(Ref(ArgParser)));
    EXPECT_CALL(Arg1, processArg(Ref(ArgParser)));
    EXPECT_CALL(Arg1, processArg(Ref(ArgParser)));
    EXPECT_CALL(Arg2, processArg(Ref(ArgParser)));
    EXPECT_CALL(Arg2, processArg(Ref(ArgParser)));
    auto Positional = ArgParser.parseCommandLineArgs(CMD);
    EXPECT_EQ(Positional, std::vector<std::string>());
  }

  // Test long args.
  {
    InSequence OrderedCalls;
    std::vector<std::string> CMD = {"--arg-1", "--arg-2", "--arg-3",
                                    "--arg-4", "--arg-2", "--arg-1"};
    EXPECT_CALL(Arg1, processArg(Ref(ArgParser)));
    EXPECT_CALL(Arg2, processArg(Ref(ArgParser)));
    EXPECT_CALL(Arg3, processArg(Ref(ArgParser)));
    EXPECT_CALL(Arg4, processArg(Ref(ArgParser)));
    EXPECT_CALL(Arg2, processArg(Ref(ArgParser)));
    EXPECT_CALL(Arg1, processArg(Ref(ArgParser)));
    auto Positional = ArgParser.parseCommandLineArgs(CMD);
    EXPECT_EQ(Positional, std::vector<std::string>());
  }

  // Test negative args.
  {
    InSequence OrderedCalls;
    std::vector<std::string> CMD = {"--no-arg-1", "--no-arg-2", "--no-arg-3",
                                    "--no-arg-4", "--no-arg-2", "--no-arg-1"};
    EXPECT_CALL(Arg1, processNegativeArg(Ref(ArgParser)));
    EXPECT_CALL(Arg2, processNegativeArg(Ref(ArgParser)));
    EXPECT_CALL(Arg3, processNegativeArg(Ref(ArgParser)));
    EXPECT_CALL(Arg4, processNegativeArg(Ref(ArgParser)));
    EXPECT_CALL(Arg2, processNegativeArg(Ref(ArgParser)));
    EXPECT_CALL(Arg1, processNegativeArg(Ref(ArgParser)));
    auto Positional = ArgParser.parseCommandLineArgs(CMD);
    EXPECT_EQ(Positional, std::vector<std::string>());
  }

  // Test args with options.
  {
    InSequence OrderedCalls;
    std::vector<std::string> CMD = {"--arg-1=x", "--arg-2=foo",
                                    "--arg-1=", "--arg-1=foo=bar"};
    EXPECT_CALL(Arg1, processArgOption(Ref(ArgParser), "x"));
    EXPECT_CALL(Arg2, processArgOption(Ref(ArgParser), "foo"));
    EXPECT_CALL(Arg1, processArgOption(Ref(ArgParser), ""));
    EXPECT_CALL(Arg1, processArgOption(Ref(ArgParser), "foo=bar"));
    auto Positional = ArgParser.parseCommandLineArgs(CMD);
    EXPECT_EQ(Positional, std::vector<std::string>());
  }

  // Test args with multiple options.
  {
    InSequence OrderedCalls;
    std::vector<std::string> CMD = {"--arg-1=x,y", "--arg-1=,,x,",
                                    "--arg-1=foo=bar,bar=foo"};
    EXPECT_CALL(Arg1, processArgOption(Ref(ArgParser), "x"));
    EXPECT_CALL(Arg1, processArgOption(Ref(ArgParser), "y"));
    EXPECT_CALL(Arg1, processArgOption(Ref(ArgParser), ""));
    EXPECT_CALL(Arg1, processArgOption(Ref(ArgParser), ""));
    EXPECT_CALL(Arg1, processArgOption(Ref(ArgParser), "x"));
    EXPECT_CALL(Arg1, processArgOption(Ref(ArgParser), ""));
    EXPECT_CALL(Arg1, processArgOption(Ref(ArgParser), "foo=bar"));
    EXPECT_CALL(Arg1, processArgOption(Ref(ArgParser), "bar=foo"));
    auto Positional = ArgParser.parseCommandLineArgs(CMD);
    EXPECT_EQ(Positional, std::vector<std::string>());
  }

  // Test mixed.
  {
    InSequence OrderedCalls;
    std::vector<std::string> CMD = {
        "--arg-3=x", "pos1", "-ab", "--arg-1", "pos2", "--arg-2", "--no-arg-2"};
    EXPECT_CALL(Arg3, processArgOption(Ref(ArgParser), "x"));
    EXPECT_CALL(Arg1, processArg(Ref(ArgParser)));
    EXPECT_CALL(Arg2, processArg(Ref(ArgParser)));
    EXPECT_CALL(Arg1, processArg(Ref(ArgParser)));
    EXPECT_CALL(Arg2, processArg(Ref(ArgParser)));
    EXPECT_CALL(Arg2, processNegativeArg(Ref(ArgParser)));
    auto Positional = ArgParser.parseCommandLineArgs(CMD);
    EXPECT_EQ(Positional, std::vector<std::string>({"pos1", "pos2"}));
  }
}

TEST(ArgumentParser, UnrecognisedArguments) {
  NiceMockArgument Arg('a', "arg", "", "", 0);

  Parser ArgParser({ArgumentGroup("G", {Arg.getArg()})});

  std::vector<std::string> CMD = {"-a", "--bad-arg", "--arg"};
  EXPECT_THROW_WITH_ARG({ ArgParser.parseCommandLineArgs(CMD); },
                        UnrecognisedArg, "--bad-arg");

  CMD = {"-a", "--bad-arg=bad", "--arg"};
  EXPECT_THROW_WITH_ARG({ ArgParser.parseCommandLineArgs(CMD); },
                        UnrecognisedArg, "--bad-arg=bad");

  CMD = {"-a", "-arg"};
  EXPECT_THROW_WITH_ARG({ ArgParser.parseCommandLineArgs(CMD); },
                        UnrecognisedArg, "-arg");

  CMD = {"-a", "-b"};
  EXPECT_THROW_WITH_ARG({ ArgParser.parseCommandLineArgs(CMD); },
                        UnrecognisedArg, "-b");
}

TEST(ArgumentParser, ShortArgOption) {
  NiceMockArgument Arg1('a', "arg-a", "", "", 0);
  NiceMockArgument Arg2('b', "arg-b", "", "", 0);

  Parser ArgParser({ArgumentGroup("G", {Arg1.getArg(), Arg2.getArg()})});

  std::vector<std::string> CMD = {"-a=opt"};
  EXPECT_THROW_WITH_ARG({ ArgParser.parseCommandLineArgs(CMD); },
                        ShortcutWithArgumentValue, "-a=opt");

  CMD = {"-aba=opt"};
  EXPECT_THROW_WITH_ARG({ ArgParser.parseCommandLineArgs(CMD); },
                        ShortcutWithArgumentValue, "-aba=opt");
}

TEST(ArgumentParser, NegativeArgOption) {
  NiceMockArgument Arg1('a', "arg-a", "", "", 0);
  NiceMockArgument Arg2('b', "arg-b", "", "", 0);

  Parser ArgParser({ArgumentGroup("G", {Arg1.getArg(), Arg2.getArg()})});

  std::vector<std::string> CMD = {"--no-arg-a=opt"};
  EXPECT_THROW_WITH_ARG_AND_OPT({ ArgParser.parseCommandLineArgs(CMD); },
                                UnexpectedArgumentValue, "--no-arg-a", "opt");
}

TEST(ArgumentParser, RegisterDuplicateArgument) {
  StrictMockArgument Arg('a', "arg-name", "", "", 0);
  StrictMockArgument DuplicateArg('b', "arg-name", "", "", 0);
  StrictMockArgument DuplicateShortcutArg('a', "other-name", "", "", 0);

  EXPECT_THROW({
    Parser ArgParser({ArgumentGroup("G1", {
      Arg.getArg(),
      DuplicateArg.getArg()
    })});
  }, std::logic_error);

  EXPECT_THROW({
    Parser ArgParser({ArgumentGroup("G1",{
      Arg.getArg(),
      DuplicateShortcutArg.getArg()
    })});
  }, std::logic_error);
}

TEST(ArgumentParser, OutputHelp) {
  std::stringstream HelpOut;

  Argument::HelpLevel HelpArgLevel = 1;
  Argument::HelpLevel SimpleArgLevel = 3;
  Argument::HelpLevel OtherSimpleArgLevel = 4;
  Argument::HelpLevel LongArgLevel = 5;

  MockArgument Help('h', "help", "", "print help", HelpArgLevel);

  const char NSC = Argument::NoShortcut;
  MockArgument Arg1('a', "Arg1", "", "Arg1 description", SimpleArgLevel);
  MockArgument Arg2(NSC, "Arg2", "=<A>", "Arg2 description", SimpleArgLevel);
  MockArgument Arg3(NSC, "Arg3", "=<B>", "Arg3 description",
                    OtherSimpleArgLevel);

  MockArgument LongArg(NSC, "VeryLongArgumentName123", "", "description",
                       LongArgLevel);
  MockArgument LongerArg(NSC, "VeryLongArgumentName123456", "", "description",
                         LongArgLevel);
  std::string LongDescStr("This is a long description string for testing the "
                          "help output wraps over multiple lines by the spaces "
                          "between words and indents properly");
  MockArgument LongDescription(NSC, "LongDesc", "", LongDescStr, LongArgLevel);
  MockArgument LongArgAndDescription(NSC, "VeryLongArgumentNameAndDesc", "",
                                     LongDescStr, LongArgLevel);

  Parser ArgParser({
    ArgumentGroup("Help arguments", { Help.getArg() }),
    ArgumentGroup("Some other arguments", {
      Arg1.getArg(),
      Arg2.getArg(),
      Arg3.getArg(),
      // Also test the rawHelpText 'Argument'
      Argument::rawHelpText(
          "\n  This is some pre-formatted...\n  Raw help text.", SimpleArgLevel)
    }),
    ArgumentGroup("Length test arguments", {
      LongArg.getArg(),
      LongerArg.getArg(),
      LongDescription.getArg(),
      LongArgAndDescription.getArg()
    })
  });

  ArgParser.outputHelp("Program [options] input", 0, HelpOut);
  EXPECT_EQ(HelpOut.str(), "Usage: Program [options] input\n");
  HelpOut.str("");

  ArgParser.outputHelp(
      "Program [options] input",
      {HelpArgLevel, 2, SimpleArgLevel, OtherSimpleArgLevel, LongArgLevel},
      HelpOut);

  EXPECT_EQ(HelpOut.str(),
    "Usage: Program [options] input\n\n"
    "Help arguments\n"
    "  -h  --help                   print help\n"
    "\n"
    "Some other arguments\n"
    "  -a  --Arg1                   Arg1 description\n"
    "      --Arg2=<A>               Arg2 description\n"
    "      --Arg3=<B>               Arg3 description\n"
    "\n"
    "  This is some pre-formatted...\n"
    "  Raw help text.\n"
    "\n"
    "Length test arguments\n"
    "      --VeryLongArgumentName123\n"
    "                               description\n"
    "      --VeryLongArgumentName123456\n"
    "                               description\n"
    "      --LongDesc               This is a long description string for testing the\n"
    "                               help output wraps over multiple lines by the\n"
    "                               spaces between words and indents properly\n"
    "      --VeryLongArgumentNameAndDesc\n"
    "                               This is a long description string for testing the\n"
    "                               help output wraps over multiple lines by the\n"
    "                               spaces between words and indents properly\n"
  );

  HelpOut.str("");
  ArgParser.outputHelp("Program [options] input", SimpleArgLevel, HelpOut);
  EXPECT_EQ(HelpOut.str(),
    "Usage: Program [options] input\n\n"
    "Some other arguments\n"
    "  -a  --Arg1                   Arg1 description\n"
    "      --Arg2=<A>               Arg2 description\n"
    "\n"
    "  This is some pre-formatted...\n"
    "  Raw help text.\n"
  );
}

TEST(ArgumentParser, HelpArgument) {
  Parser ArgParser({
    ArgumentGroup("G1", {
      Argument('a', "aa", "", "help a", 1, nullptr, nullptr, nullptr, false)
    }),
    ArgumentGroup("G2", {
      Argument('b', "bb", "", "help b", 2, nullptr, nullptr, nullptr, false)
    })
  });

  bool HelpPrinted = true;

  std::stringstream Help1Out;
  std::stringstream Help2Out;
  std::stringstream Help3Out;
  Argument Help1(Argument::helpArg('h', "help1", "show help 1", 0, "example1",
                                   {1}, HelpPrinted, Help1Out));
  Argument Help2(Argument::helpArg('q', "help2", "show help 2", 0, "example2",
                                   {1, 2}, HelpPrinted, Help2Out));

  EXPECT_FALSE(HelpPrinted)
      << "Help args should set the printed bool to false on creation";

  Help1.ProcessArg(ArgParser);
  EXPECT_EQ(Help1Out.str(),
    "Usage: example1\n\n"
    "G1\n"
    "  -a  --aa                     help a\n"
  );
  EXPECT_TRUE(HelpPrinted);
  EXPECT_TRUE(Help2Out.str().empty());
  EXPECT_TRUE(Help3Out.str().empty());

  HelpPrinted = false;
  Help2.ProcessArg(ArgParser);
  EXPECT_EQ(Help2Out.str(),
    "Usage: example2\n\n"
    "G1\n"
    "  -a  --aa                     help a\n"
    "\nG2\n"
    "  -b  --bb                     help b\n"
  );
  EXPECT_TRUE(HelpPrinted);
  Help1Out.str("");
  Help2Out.str("");

  EXPECT_THROW_WITH_ARG_AND_OPT({ Help1.ProcessArgWithValue(ArgParser, "O"); },
                                UnexpectedArgumentValue, "--help1", "O");
  EXPECT_THROW_WITH_ARG({ Help1.ProcessNegativeArg(ArgParser); },
                        UnexpectedNegative, "--no-help1");
}

TEST(ArgumentParser, SwitchArgument) {
  bool ArgVal = false;
  Argument Arg(Argument::switchArg('s', "switch", "", 0, ArgVal));
  EXPECT_FALSE(ArgVal);

  Arg.ProcessArg(EmptyParser);
  EXPECT_TRUE(ArgVal);
  Arg.ProcessNegativeArg(EmptyParser);
  EXPECT_FALSE(ArgVal);

  EXPECT_THROW_WITH_ARG_AND_OPT({ Arg.ProcessArgWithValue(EmptyParser, "O"); },
                                UnexpectedArgumentValue, "--switch", "O");
}

TEST(ArgumentParser, StringArgument) {
  std::string ArgVal("default");
  Argument Arg(Argument::stringArg(Argument::NoShortcut, "str-arg", "VAL", "",
                                   0, ArgVal));
  EXPECT_EQ(ArgVal, "default");
  EXPECT_EQ(Arg.NameHelpSuffix, "=<VAL>");

  Arg.ProcessArgWithValue(EmptyParser, "abc");
  EXPECT_EQ(ArgVal, "abc");

  Arg.ProcessArgWithValue(EmptyParser, "xyz");
  EXPECT_EQ(ArgVal, "xyz");

  EXPECT_THROW_WITH_ARG({ Arg.ProcessArg(EmptyParser); }, ArgumentValueRequired,
                        "--str-arg");

  EXPECT_THROW_WITH_ARG({ Arg.ProcessNegativeArg(EmptyParser); },
                        UnexpectedNegative, "--no-str-arg");
}

TEST(ArgumentParser, MultipleStringArgument) {
  std::vector<std::string> ArgValues;
  Argument Arg(
      Argument::multiStringArg('m', "mstr-arg", "VAL", "", 0, ArgValues));
  EXPECT_EQ(ArgValues, std::vector<std::string>());
  EXPECT_EQ(Arg.NameHelpSuffix, "=<VAL>");

  Arg.ProcessArgWithValue(EmptyParser, "one");
  EXPECT_EQ(ArgValues, std::vector<std::string>({"one"}));

  Arg.ProcessArgWithValue(EmptyParser, "two");
  Arg.ProcessArgWithValue(EmptyParser, "three");
  Arg.ProcessArgWithValue(EmptyParser, "one");
  Arg.ProcessArgWithValue(EmptyParser, "four");
  EXPECT_EQ(ArgValues,
            std::vector<std::string>({"one", "two", "three", "one", "four"}));

  Arg.ProcessArgWithValue(EmptyParser, "five");
  EXPECT_EQ(ArgValues, std::vector<std::string>(
                           {"one", "two", "three", "one", "four", "five"}));

  EXPECT_THROW_WITH_ARG({ Arg.ProcessArg(EmptyParser); }, ArgumentValueRequired,
                        "--mstr-arg");
  EXPECT_THROW_WITH_ARG({ Arg.ProcessNegativeArg(EmptyParser); },
                        UnexpectedNegative, "--no-mstr-arg");
}

TEST(ArgumentParser, StringChoiceArgument) {
  std::string ArgVal("none");
  Argument Arg(Argument::choiceArg('c', "colour", "", 0,
                                   {"blue", "yellow", "green"}, ArgVal));
  EXPECT_EQ(ArgVal, "none");
  EXPECT_EQ(Arg.NameHelpSuffix, "=<blue|green|yellow>");

  Arg.ProcessArgWithValue(EmptyParser, "blue");
  EXPECT_EQ(ArgVal, "blue");

  Arg.ProcessArgWithValue(EmptyParser, "yellow");
  EXPECT_EQ(ArgVal, "yellow");

  // Test exception for invalid choice.
  EXPECT_THROW_WITH_ARG_AND_OPT(
      { Arg.ProcessArgWithValue(EmptyParser, "greenish"); }, InvalidChoice,
      "--colour", "greenish");

  EXPECT_THROW_WITH_ARG({ Arg.ProcessArg(EmptyParser); }, ArgumentValueRequired,
                        "--colour");
  EXPECT_THROW_WITH_ARG({ Arg.ProcessNegativeArg(EmptyParser); },
                        UnexpectedNegative, "--no-colour");
}

TEST(ArgumentParser, MultipleStringChoiceArgument) {
  std::set<std::string> ArgValues;
  Argument Arg(Argument::multiChoiceArg(
      'm', "colours", "", 0,
      {"blue", "yellow", "green", "red", "orange", "purple"}, ArgValues));
  EXPECT_TRUE(ArgValues.empty());
  EXPECT_EQ(Arg.NameHelpSuffix, "=<blue|green|orange|purple|red|yellow>");

  Arg.ProcessArgWithValue(EmptyParser, "blue");
  EXPECT_EQ(ArgValues, std::set<std::string>({"blue"}));

  Arg.ProcessArgWithValue(EmptyParser, "yellow");
  Arg.ProcessArgWithValue(EmptyParser, "green");
  Arg.ProcessArgWithValue(EmptyParser, "blue");
  Arg.ProcessArgWithValue(EmptyParser, "red");
  EXPECT_EQ(ArgValues,
            std::set<std::string>({"blue", "yellow", "green", "red"}));

  Arg.ProcessArgWithValue(EmptyParser, "orange");
  EXPECT_EQ(ArgValues, std::set<std::string>(
                           {"blue", "yellow", "green", "red", "orange"}));

  // Test exception for invalid choice.
  EXPECT_THROW_WITH_ARG_AND_OPT(
      { Arg.ProcessArgWithValue(EmptyParser, "grey"); }, InvalidChoice,
      "--colours", "grey");

  EXPECT_THROW_WITH_ARG({ Arg.ProcessArg(EmptyParser); }, ArgumentValueRequired,
                        "--colours");
  EXPECT_THROW_WITH_ARG({ Arg.ProcessNegativeArg(EmptyParser); },
                        UnexpectedNegative, "--no-colours");
}
