//===-- Diva/ArgumentParser.cpp ---------------------------------*- C++ -*-===//
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
/// This file contains the definitions for argument parsing.
///
//===----------------------------------------------------------------------===//

#include "ArgumentParser.h"

#include <algorithm>
#include <assert.h>
#include <iterator>
#include <sstream>
#include <stdexcept>

using namespace ArgumentParser;

namespace {

// Seperator between arguments and values on the command line, e.g. --arg=opt.
const char ArgumentAndValueSeperator = '=';

static_assert(ArgumentAndValueSeperator != ' ',
              "ArgumentParser::parseCommandLineArgs is not implemented to "
              "handle space (' ') as the ArgumentAndValueSeperator");

// Seperator between multiple argument values on the command line, e.g.
// --arg=opt1,opt2.
const char MultipleValueSeperator = ',';

static_assert(MultipleValueSeperator != ' ',
              "ArgumentParser::parseCommandLineArgs is not implemented to "
              "handle space (' ') as the MultipleValueSeperator");

// NegativeLongArgumentPrefix has priority over LongArgumentPrefix which has
// priority over ShortArgumentPrefix.
const std::string NegativeLongArgumentPrefix("--no-");
const std::string LongArgumentPrefix("--");
const std::string ShortArgumentPrefix("-");

// Return true if s begins with prefix and is not empty after the prefix.
bool strPrefixedWith(const std::string &s, const std::string &prefix) {
  return s.size() > prefix.size() && s.compare(0, prefix.size(), prefix) == 0;
}

// Max column width when printing help text.
const uint8_t HelpMaxWidth = 80;

// Indent for the shortcut column and between the shortcut and long argument
// columns when printing help.
const uint8_t HelpIndentSize = 2;

// Indent from the start of the line for the argument description text when
// printing help.
const uint8_t HelpDescriptionIndentSize = 31;

// Returns a function that makes an Argument throw an exception if it is called
// without a value.
Argument::ProcessArgFunc argumentValueRequired(const std::string &ArgName) {
  return [ArgName](const Parser &) {
    throw ArgumentValueRequired(LongArgumentPrefix + ArgName);
  };
}

// Returns a function that makes an Argument throw an exception if it is called
// with a value.
Argument::ProcessArgValFunc valueForbidden(const std::string &ArgName) {
  return [ArgName](const Parser &, const std::string &Opt) {
    throw UnexpectedArgumentValue(LongArgumentPrefix + ArgName, Opt);
  };
}

// Returns a function that makes an Argument throw an exception if it is called
// as a negative e.g. --no-arg.
Argument::ProcessNegArgFunc negativeForbidden(const std::string &ArgName) {
  return [ArgName](const Parser &) {
    throw UnexpectedNegative(NegativeLongArgumentPrefix + ArgName);
  };
}

std::string buildChoiceArgHelpSuffix(const std::set<std::string> &Choices) {
  if (Choices.empty())
    return "";
  std::string Result("=<");
  for (const auto &Choice : Choices)
    Result.append(Choice).append("|");
  // Replace the extra trailing '|' with the closing '>'.
  Result.back() = '>';
  return Result;
}

} // end anonymous namespace

const char *ArgumentException::what() const noexcept {
  Msg = getExceptionName();
  Msg += std::string(" on argument '") + Arg + std::string("'");
  if (!ArgVal.empty())
    Msg += std::string(" with value '") + ArgVal + std::string("'");
  return Msg.c_str();
}

std::string UnrecognisedArg::getExceptionName() const {
  return "UnrecognisedArg";
}
std::string ArgumentValueRequired::getExceptionName() const {
  return "ArgumentValueRequired";
}
std::string UnexpectedArgumentValue::getExceptionName() const {
  return "UnexpectedArgumentValue";
}
std::string UnexpectedNegative::getExceptionName() const {
  return "UnexpectedNegative";
}
std::string ShortcutWithArgumentValue::getExceptionName() const {
  return "ShortcutWithArgumentValue";
}
std::string InvalidChoice::getExceptionName() const {
  return "InvalidChoice";
}

const char Argument::NoShortcut = '\0';

Argument::Argument(const char ArgShortcut, const std::string &ArgName,
                   const std::string &ArgNameHelpSuffix,
                   const std::string &HelpText, const HelpLevel HelpLevel,
                   const ProcessArgFunc ProcessArgFunc,
                   const ProcessArgValFunc ProcessArgWithValueFunc,
                   const ProcessNegArgFunc ProcessNegativeArgFunc,
                   const bool IsHelpAlreadyFormatted)
    : Shortcut(ArgShortcut), Name(ArgName), NameHelpSuffix(ArgNameHelpSuffix),
      ArgHelpLevel(HelpLevel), Help(HelpText),
      HelpAlreadyFormatted(IsHelpAlreadyFormatted),
      ProcessArg(ProcessArgFunc ? ProcessArgFunc : argumentValueRequired(Name)),
      ProcessArgWithValue(ProcessArgWithValueFunc ? ProcessArgWithValueFunc
                                                  : valueForbidden(Name)),
      ProcessNegativeArg(ProcessNegativeArgFunc ? ProcessNegativeArgFunc
                                                : negativeForbidden(Name)) {}

Argument::Argument(const char ArgShortcut, const std::string &ArgName,
                   const std::string &HelpText, const HelpLevel HelpLevel,
                   const ProcessArgFunc ProcessArgFunc)
    : Argument(ArgShortcut, ArgName, "", HelpText, HelpLevel, ProcessArgFunc,
               nullptr, nullptr) {}

const Argument Argument::helpArg(const char Shortcut, const std::string &Name,
                                 const std::string &Help,
                                 const HelpLevel ArgHelpLevel,
                                 const std::string &Usage,
                                 const HelpLevels &HelpLevelsToPrint,
                                 bool &Printed, std::ostream &OutStream) {
  Printed = false;
  auto ProcessFunc = [&, Usage, HelpLevelsToPrint](const Parser &ArgParser) {
    ArgParser.outputHelp(Usage, HelpLevelsToPrint, OutStream);
    Printed = true;
  };
  return Argument(Shortcut, Name, Help, ArgHelpLevel, ProcessFunc);
}

const Argument Argument::rawHelpText(const std::string &Text,
                                     const HelpLevel ArgHelpLevel) {
  // This 'argument' has no name or shortcut so the process functions should
  // never be called.
  return Argument(Argument::NoShortcut, "", "", Text, ArgHelpLevel, nullptr,
                  nullptr, nullptr, /*HelpAlreadyFormatted*/ true);
}

const Argument Argument::switchArg(const char Shortcut, const std::string &Name,
                                   const std::string &Help,
                                   const HelpLevel ArgHelpLevel, bool &Value) {
  return Argument(Shortcut, Name, "", Help, ArgHelpLevel,
                  [&Value](const Parser &) { Value = true; }, nullptr,
                  [&Value](const Parser &) { Value = false; });
}

const Argument Argument::stringArg(const char Shortcut, const std::string &Name,
                                   const std::string &VarNameForHelp,
                                   const std::string &Help,
                                   const HelpLevel ArgHelpLevel,
                                   std::string &Value) {
  std::string HelpSuffix("=<" + VarNameForHelp + ">");
  return Argument(
      Shortcut, Name, HelpSuffix, Help, ArgHelpLevel, nullptr,
      [&Value](const Parser &, const std::string &Opt) { Value = Opt; },
      nullptr);
}

const Argument Argument::multiStringArg(const char Shortcut,
                                        const std::string &Name,
                                        const std::string &VarNameForHelp,
                                        const std::string &Help,
                                        const HelpLevel ArgHelpLevel,
                                        std::vector<std::string> &Values) {
  std::string HelpSuffix("=<" + VarNameForHelp + ">");
  return Argument(Shortcut, Name, HelpSuffix, Help, ArgHelpLevel, nullptr,
                  [&Values](const Parser &, const std::string &Opt) {
                    Values.emplace_back(Opt);
                  },
                  nullptr);
}

const Argument Argument::choiceArg(const char Shortcut, const std::string &Name,
                                   const std::string &Help,
                                   const HelpLevel ArgHelpLevel,
                                   const std::set<std::string> &AcceptedValues,
                                   std::string &Value) {
  std::string HelpSuffix(buildChoiceArgHelpSuffix(AcceptedValues));
  auto ProcessFunc = [&, Name = LongArgumentPrefix + Name,
                      AcceptedValues ](const Parser &, const std::string &Opt) {
    if (AcceptedValues.count(Opt))
      Value = Opt;
    else
      throw InvalidChoice(Name, Opt);
  };
  return Argument(Shortcut, Name, HelpSuffix, Help, ArgHelpLevel, nullptr,
                  ProcessFunc, nullptr);
}

const Argument
Argument::multiChoiceArg(const char Shortcut, const std::string &Name,
                         const std::string &Help, const HelpLevel ArgHelpLevel,
                         const std::set<std::string> &AcceptedValues,
                         std::set<std::string> &Values) {
  std::string HelpSuffix(buildChoiceArgHelpSuffix(AcceptedValues));
  auto ProcessFunc = [&, Name = LongArgumentPrefix + Name,
                      AcceptedValues ](const Parser &, const std::string &Opt) {
    if (AcceptedValues.count(Opt))
      Values.emplace(Opt);
    else
      throw InvalidChoice(Name, Opt);
  };
  return Argument(Shortcut, Name, HelpSuffix, Help, ArgHelpLevel, nullptr,
                  ProcessFunc, nullptr);
}

bool ArgumentGroup::containsArgWithHelpLevel(
    const Argument::HelpLevels &Levels) const {
  return std::any_of(Args.begin(), Args.end(), [&](const Argument &Arg) {
    return Levels.count(Arg.ArgHelpLevel);
  });
}

Parser::Parser(std::vector<ArgumentGroup> &&Groups)
    : ArgGroups(std::move(Groups)) {
  // Populate argument lookup maps.
  for (const ArgumentGroup &Group : ArgGroups) {
    for (const Argument &Arg : Group.Args) {
      if (Arg.Shortcut != Argument::NoShortcut) {
        bool Inserted = ShortArguments.emplace(Arg.Shortcut, &Arg).second;
        if (!Inserted)
          throw std::logic_error("Duplicate argument shortcut registered");
      }
      if (!Arg.Name.empty()) {
        bool Inserted = LongArguments.emplace(Arg.Name, &Arg).second;
        if (!Inserted)
          throw std::logic_error("Duplicate argument name registered");
      }
    }
  }
}

std::vector<std::string>
Parser::parseCommandLineArgs(const std::vector<std::string> &CMDArgs) {
  std::vector<std::string> PositionalArguments;

  for (const std::string &CMDArg : CMDArgs) {
    if (strPrefixedWith(CMDArg, NegativeLongArgumentPrefix)) {
      // Long negative prefix.
      parseCMDArg(CMDArg.substr(NegativeLongArgumentPrefix.size()), CMDArg,
                  /*IsNegative*/ true);
    } else if (strPrefixedWith(CMDArg, LongArgumentPrefix)) {
      // Long prefix.
      parseCMDArg(CMDArg.substr(LongArgumentPrefix.size()), CMDArg,
                  /*IsNegative*/ false);
    } else if (strPrefixedWith(CMDArg, ShortArgumentPrefix)) {
      // Short prefix.
      // Parse each character after the prefix as a seperate shortcut arg.
      for (auto i = ShortArgumentPrefix.size(); i < CMDArg.size(); ++i) {
        if (CMDArg[i] == ArgumentAndValueSeperator)
          throw ShortcutWithArgumentValue(CMDArg);
        parseCMDArg(CMDArg[i], CMDArg);
      }
    } else {
      // Positional.
      PositionalArguments.push_back(CMDArg);
    }
  }
  return PositionalArguments;
}

void Parser::parseCMDArg(const char Shortcut, const std::string &FullArgCMD) {
  auto ArgEntry = ShortArguments.find(Shortcut);
  if (ArgEntry == ShortArguments.end())
    throw UnrecognisedArg(FullArgCMD);
  ArgEntry->second->ProcessArg(*this);
}

void Parser::parseCMDArg(const std::string &LongArg,
                         const std::string &FullArgCMD,
                         const bool IsNegative) {
  // Split the command by the argument value seperator if it exists.
  auto SeperatorPos = LongArg.find(ArgumentAndValueSeperator);
  bool HasValue = (SeperatorPos != LongArg.npos);
  std::string ArgName(LongArg.substr(0, SeperatorPos));
  std::string ArgValue(HasValue ? LongArg.substr(SeperatorPos + 1) : "");

  if (IsNegative && HasValue)
    throw UnexpectedArgumentValue(NegativeLongArgumentPrefix + ArgName,
                                  ArgValue);

  // Lookup the argument.
  auto ArgEntry = LongArguments.find(ArgName);
  if (ArgEntry == LongArguments.end())
    throw UnrecognisedArg(FullArgCMD);
  const Argument &Arg = *ArgEntry->second;

  if (IsNegative)
    Arg.ProcessNegativeArg(*this);
  else if (HasValue)
    splitArgumentValuesAndProcess(Arg, ArgValue);
  else
    Arg.ProcessArg(*this);
}

void Parser::splitArgumentValuesAndProcess(const Argument &Arg,
                                           const std::string &ArgValue) {
  // Split an argument value into multiple values by the MultipleValueSeperator.
  std::stringstream AllValues(ArgValue);
  std::string SingleValue;
  while (std::getline(AllValues, SingleValue, MultipleValueSeperator))
    Arg.ProcessArgWithValue(*this, SingleValue);
  if (ArgValue.empty() || ArgValue.back() == MultipleValueSeperator)
    Arg.ProcessArgWithValue(*this, "");
}

void Parser::outputHelp(const std::string &Usage,
                        const Argument::HelpLevel Level,
                        std::ostream &OutStream) const {
  outputHelp(Usage, std::set<Argument::HelpLevel>({Level}), OutStream);
}

void Parser::outputHelp(const std::string &Usage,
                        const Argument::HelpLevels &Levels,
                        std::ostream &OutStream) const {
  OutStream << "Usage: " << Usage << '\n';
  for (const ArgumentGroup &Group : ArgGroups) {
    if (!Group.containsArgWithHelpLevel(Levels))
      continue;
    OutStream << '\n' << Group.Title << '\n';
    for (const Argument &Arg : Group.Args) {
      if (!Levels.count(Arg.ArgHelpLevel))
        continue;
      if (Arg.HelpAlreadyFormatted) {
        OutStream << Arg.Help << '\n';
        continue;
      }
      std::string OutputName = getOutputArgName(Arg);
      std::string OutputDesc = getOutputArgDescription(OutputName.size(), Arg);
      OutStream << OutputName << OutputDesc;
    }
  }
}

std::string Parser::getOutputArgName(const Argument &Arg) const {
  // Indent for the shortcut column and between the shortcut and long argument
  // columns.
  static const std::string Indent(HelpIndentSize, ' ');
  // Blank string to fill in the shortcut column for arguments without
  // shortcuts.
  static const std::string NoShortcutStr(ShortArgumentPrefix.size() + 1, ' ');

  std::stringstream Result;
  Result << Indent;
  // Shortcut.
  if (Arg.Shortcut != Argument::NoShortcut)
    Result << ShortArgumentPrefix << Arg.Shortcut;
  else
    Result << NoShortcutStr;
  // Long argument name.
  Result << Indent << LongArgumentPrefix << Arg.Name << Arg.NameHelpSuffix;
  return Result.str();
}

std::string Parser::getOutputArgDescription(const size_t ArgNameSize,
                                            const Argument &Arg) const {
  size_t CursorPos = ArgNameSize;
  std::stringstream Result;
  // If we've gone past the HelpDescriptionIndentSize already then start a new
  // line when we start printing.
  if (CursorPos >= HelpDescriptionIndentSize)
    CursorPos = HelpMaxWidth;
  // Otherwise print spaces upto HelpDescriptionIndentSize.
  else {
    Result << std::string(HelpDescriptionIndentSize - CursorPos - 1, ' ');
    CursorPos = HelpDescriptionIndentSize - 1;
  }

  // Print description text between HelpDescriptionIndentSize and
  // HelpMaxWidth.
  std::stringstream HelpStream(Arg.Help);
  std::string HelpWord;
  while (HelpStream >> HelpWord) {
    // If we've gone over the limit wrap back to the start.
    if (CursorPos + 1 + HelpWord.size() > HelpMaxWidth) {
      Result << '\n' << std::string(HelpDescriptionIndentSize - 1, ' ');
      CursorPos = HelpDescriptionIndentSize - 1;
    }
    Result << ' ' << HelpWord;
    CursorPos += HelpWord.size() + 1;
  }
  Result << '\n';
  return Result.str();
}
