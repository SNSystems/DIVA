//===-- Diva/ArgumentParser.h -----------------------------------*- C++ -*-===//
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
/// This file contains the class definitions for an argument parser, a set of
/// argument types and the exceptions that can be thrown during parsing.
///
/// Typical usage:
/// \code
///
///   bool HelpPrinted = false;
///   bool AVal = false;
///   bool BVal = true;
///   std::string OutFile;
///
///   const static Argument::HelpLevel HelpLev = 1;
///
///   Parser P({
///     ArgumentGroup("General arguments", {
///       Argument::helpArg('h', "help", "print help", HelpLev,
///                         "USAGE: a.exe [args]", {HelpLev}, HelpPrinted,
///                         std::cout),
///       Argument::switchArg('a', "do-a", "does a", HelpLev, AVal),
///       Argument::switchArg('b', "do-b", "does b", HelpLev, BVal),
///       Argument::stringArg(Argument::NoShortcut, "ofile", "PATH",
///                           "output file", HelpLev, OutFile)
///     })
///   });
///
///   std::vector<std::string> Positional;
///   try {
///      Positional = P.parseCommandLineArgs(
///          {"-a", "--no-b", "--ofile=foo.txt", "bar.txt"});
///   } catch (ArgumentParser::UnrecognisedArg &Err) {
///     std::cerr << "Unknown arg: " << Err.Arg << "\n";
///   }
///
///   assert(!HelpPrinted && AVal && !BVal && OutFile == "foo.txt" &&
///          Positional[0] == "bar.txt");
/// \endcode
///
//===----------------------------------------------------------------------===//

// NOTE: Please update `TEST(ArgumentParser, UsageExample)` in
//       UnitTests/TestArgumentParser.cpp if you modify the above example.

#ifndef ARGUMENT_PARSER_H
#define ARGUMENT_PARSER_H

#include <exception>
#include <functional>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

namespace ArgumentParser {

class ArgumentException : public std::exception {
public:
  ArgumentException(const std::string &ArgName) : Arg(ArgName) {}
  ArgumentException(const std::string &ArgName, const std::string &Value)
      : Arg(ArgName), ArgVal(Value) {}

  const std::string Arg;
  const std::string ArgVal;

  virtual const char *what() const noexcept override;

private:
  virtual std::string getExceptionName() const = 0;
  mutable std::string Msg;
};

class UnrecognisedArg : public ArgumentException {
public:
  UnrecognisedArg(const std::string &Arg) : ArgumentException(Arg) {}

private:
  std::string getExceptionName() const override;
};

class ArgumentValueRequired : public ArgumentException {
public:
  ArgumentValueRequired(const std::string &Arg) : ArgumentException(Arg) {}

private:
  std::string getExceptionName() const override;
};

class UnexpectedArgumentValue : public ArgumentException {
public:
  UnexpectedArgumentValue(const std::string &Arg, const std::string &ArgVal)
      : ArgumentException(Arg, ArgVal) {}

private:
  std::string getExceptionName() const override;
};

class UnexpectedNegative : public ArgumentException {
public:
  UnexpectedNegative(const std::string &Arg) : ArgumentException(Arg) {}

private:
  std::string getExceptionName() const override;
};

class ShortcutWithArgumentValue : public ArgumentException {
public:
  ShortcutWithArgumentValue(const std::string &Arg) : ArgumentException(Arg) {}

private:
  std::string getExceptionName() const override;
};

class InvalidChoice : public ArgumentException {
public:
  InvalidChoice(const std::string &Arg, const std::string &ArgVal)
      : ArgumentException(Arg, ArgVal) {}

private:
  std::string getExceptionName() const override;
};

class Parser;

/// \brief A command line argument with stored functions for custom behaviour.
class Argument {
public:
  /// \brief Function that processes a simple argument.
  ///
  /// e.g. "--arg"
  typedef std::function<void(const Parser &)> ProcessArgFunc;

  /// \brief Function that processes an argument with a value.
  ///
  /// e.g. "--arg=example"
  typedef std::function<void(const Parser &, const std::string &)>
      ProcessArgValFunc;

  /// \brief Function that processes a negative argument.
  ///
  /// e.g. "--no-arg"
  typedef std::function<void(const Parser &)> ProcessNegArgFunc;

  /// \brief Level at which help for an argument is printed.
  typedef uint8_t HelpLevel;
  /// \brief Levels at which help for an argument is printed.
  typedef std::set<HelpLevel> HelpLevels;

  /// \brief Create an argument with custom argument processing behaviour.
  ///
  /// If any of ProcessArg, ProcessArgWithValue, ProcessNegativeArg are nullptr
  /// then they will be set to throw an appropriate ParserException,
  /// e.g. if ProcessArgWithValue is nullptr and the argument is called with a
  /// value, then UnexpectedArgumentValue will be thrown.
  explicit Argument(const char Shortcut, const std::string &Name,
                    const std::string &NameHelpSuffix, const std::string &Help,
                    const HelpLevel ArgHelpLevel,
                    const ProcessArgFunc ProcessArg,
                    const ProcessArgValFunc ProcessArgWithValue,
                    const ProcessNegArgFunc ProcessNegativeArg,
                    const bool HelpAlreadyFormatted = false);

  /// \brief Create an argument that can NOT be called with a value or as a
  /// negative.
  explicit Argument(const char Shortcut, const std::string &Name,
                    const std::string &Help, const HelpLevel ArgHelpLevel,
                    const ProcessArgFunc ProcessArg);

  // Static functions creating specific argument patterns.

  /// \brief Create an argument that prints the help for argument groups.
  static const Argument helpArg(const char Shortcut, const std::string &Name,
                                const std::string &Help,
                                const HelpLevel ArgHelpLevel,
                                const std::string &Usage,
                                const HelpLevels &HelpLevelsToPrint,
                                bool &Printed, std::ostream &OutStream);

  /// \brief Create a raw block of text to be printed in the help output.
  static const Argument rawHelpText(const std::string &Text,
                                    const HelpLevel ArgHelpLevel);

  /// \brief Create an Argument referencing a boolean value that can be set to
  /// true or false from the command line.
  ///
  /// Giving the argument sets the value to true (e.g. --do-thing). Giving the
  /// argument with a '-no' prefix sets the value to false (e.g. --no-do-thing).
  static const Argument switchArg(const char Shortcut, const std::string &Name,
                                  const std::string &Help,
                                  const HelpLevel ArgHelpLevel, bool &Value);

  /// \brief Create an Argument that sets a string from the command line.
  ///
  /// e.g. --file=foo.txt
  ///
  /// \param VarNameForHelp help name for the string variable
  /// e.g. 'FILENAME' in "--file=<FILENAME>    Set the filename"
  static const Argument stringArg(const char Shortcut, const std::string &Name,
                                  const std::string &VarNameForHelp,
                                  const std::string &Help,
                                  const HelpLevel ArgHelpLevel,
                                  std::string &Value);

  /// \brief Create an Argument that sets a vector of strings set from the
  /// command line.
  ///
  /// e.g. --files=foo.txt,bar.txt --files=baz.txt
  ///
  /// \param VarNameForHelp help name for the string variable
  /// e.g. 'FILENAME' in "--files=<FILENAME>    Set the filenames"
  static const Argument multiStringArg(const char Shortcut,
                                       const std::string &Name,
                                       const std::string &VarNameForHelp,
                                       const std::string &Help,
                                       const HelpLevel ArgHelpLevel,
                                       std::vector<std::string> &Values);

  /// \brief Create an Argument that sets a string value to one of a set of
  /// accepted values.
  ///
  /// Passing an unexcepted value results in an InvalidChoice exception.
  ///
  /// e.g. --detail=high --detail=low
  static const Argument choiceArg(const char Shortcut, const std::string &Name,
                                  const std::string &Help,
                                  const HelpLevel ArgHelpLevel,
                                  const std::set<std::string> &AcceptedValues,
                                  std::string &Value);

  /// \brief Create an Argument that records which values were given on the
  /// command line from a set of accepted values.
  ///
  /// Passing an unexcepted value results in an InvalidChoice exception.
  ///
  /// e.g. --output=text,yaml --output=json
  static const Argument
  multiChoiceArg(const char Shortcut, const std::string &Name,
                 const std::string &Help, const HelpLevel ArgHelpLevel,
                 const std::set<std::string> &AcceptedValues,
                 std::set<std::string> &Values);

  const static char NoShortcut;

  const char Shortcut;
  const std::string Name;
  const std::string NameHelpSuffix;

  /// \brief The level at which the help for this argument should be printed.
  const HelpLevel ArgHelpLevel;

  const std::string Help;
  const bool HelpAlreadyFormatted;
  const ProcessArgFunc ProcessArg;
  const ProcessArgValFunc ProcessArgWithValue;
  const ProcessNegArgFunc ProcessNegativeArg;
};

/// \brief Collection of Arguments with a help section title.
class ArgumentGroup {
public:
  explicit ArgumentGroup(const std::string &GroupTitle,
                         std::vector<Argument> &&Arguments)
      : Title(GroupTitle), Args(std::move(Arguments)) {}

  bool containsArgWithHelpLevel(const Argument::HelpLevels &Levels) const;

  const std::string Title;
  const std::vector<Argument> Args;
};

/// \brief Parser for command line arguments.
class Parser {
public:
  /// \brief Create an argument parser with all its arguments.
  ///
  /// This constructor will throw a std::logic_error if there are duplicate
  /// arguments.
  explicit Parser(std::vector<ArgumentGroup> &&Groups);

  /// \brief Process all added arguments and return any positional args.
  ///
  /// Note the first argument should not be the executable (argv[0]).
  std::vector<std::string>
  parseCommandLineArgs(const std::vector<std::string> &CMDArgs);

  /// \brief Display the help text for the given help levels.
  void outputHelp(const std::string &Usage, const Argument::HelpLevel Level,
                  std::ostream &OutStream) const;
  void outputHelp(const std::string &Usage, const Argument::HelpLevels &Levels,
                  std::ostream &OutStream) const;

private:
  void parseCMDArg(const char Shortcut, const std::string &FullArgCMD);
  void parseCMDArg(const std::string &LongArg, const std::string &FullArgCMD,
                   const bool IsNegative);

  void splitArgumentValuesAndProcess(const Argument &Arg,
                                     const std::string &ArgValue);

  std::string getOutputArgName(const Argument &Arg) const;
  std::string getOutputArgDescription(const size_t ArgNameSize,
                                      const Argument &Arg) const;

  const std::vector<ArgumentGroup> ArgGroups;
  std::unordered_map<char, const Argument *> ShortArguments;
  std::unordered_map<std::string, const Argument *> LongArguments;
};

} // end namespace ArgumentParser.

#endif // ARGUMENT_PARSER_H
