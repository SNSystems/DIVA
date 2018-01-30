//===-- UnitTests/TestLibScopeView/TestLine.cpp -----------------*- C++ -*-===//
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
/// Tests for LibScopeView::Line.
///
//===----------------------------------------------------------------------===//

#include "Line.h"
#include "PrintSettings.h"

#include "gtest/gtest.h"

using namespace LibScopeView;

TEST(Line, getAsText_Line) {
  Line Ln;
  EXPECT_EQ(Ln.getAsText(PrintSettings()), "{CodeLine}");
}

TEST(Line, getAsYAML_Line) {
  Line Ln;
  Ln.setLineNumber(52);
  Ln.setFilePath("test.cpp");
  Ln.setAddress(0x5555);
  Ln.setDiscriminator(5);

  EXPECT_EQ(Ln.getAsYAML(), "object: \"CodeLine\"\n"
                            "name: null\n"
                            "type: null\n"
                            "source:\n"
                            "  line: 52\n"
                            "  file: \"test.cpp\"\n"
                            "dwarf:\n"
                            "  offset: null\n"
                            "  tag: null\n"
                            "attributes:\n"
                            "  Address: 0x5555\n"
                            "  Discriminator: 5\n"
                            "  NewStatement: false\n"
                            "  PrologueEnd: false\n"
                            "  EndSequence: false\n"
                            "  BasicBlock: false\n"
                            "  EpilogueBegin: false");
}

TEST(Line, getAsText_Line_Attributes) {
  PrintSettings Settings;
  Settings.ShowCodelineAttributes = true;

  Line Ln;
  std::string Expected("{CodeLine}\n    - Discriminator 0");
  EXPECT_EQ(Ln.getAsText(Settings), Expected);

  Ln.setDiscriminator(5);
  Expected = "{CodeLine}\n    - Discriminator 5";
  EXPECT_EQ(Ln.getAsText(Settings), Expected);

  Ln.setIsNewStatement();
  Expected += "\n    - NewStatement";
  EXPECT_EQ(Ln.getAsText(Settings), Expected);

  Ln.setIsPrologueEnd();
  Expected += "\n    - PrologueEnd";
  EXPECT_EQ(Ln.getAsText(Settings), Expected);

  Ln.setIsLineEndSequence();
  Expected += "\n    - EndSequence";
  EXPECT_EQ(Ln.getAsText(Settings), Expected);

  Ln.setIsNewBasicBlock();
  Expected += "\n    - BasicBlock";
  EXPECT_EQ(Ln.getAsText(Settings), Expected);

  Ln.setIsEpilogueBegin();
  Expected += "\n    - EpilogueBegin";
  EXPECT_EQ(Ln.getAsText(Settings), Expected);
}

TEST(Line, getAsYAML_Line_Attributes) {
  Line Ln;
  Ln.setLineNumber(52);
  Ln.setFilePath("test.cpp");
  Ln.setAddress(0x5555);

  std::string Expected("object: \"CodeLine\"\n"
                       "name: null\n"
                       "type: null\n"
                       "source:\n"
                       "  line: 52\n"
                       "  file: \"test.cpp\"\n"
                       "dwarf:\n"
                       "  offset: null\n"
                       "  tag: null\n"
                       "attributes:\n"
                       "  Address: 0x5555");

  EXPECT_EQ(Ln.getAsYAML(), Expected + ("\n  Discriminator: 0"
                                        "\n  NewStatement: false"
                                        "\n  PrologueEnd: false"
                                        "\n  EndSequence: false"
                                        "\n  BasicBlock: false"
                                        "\n  EpilogueBegin: false"));

  Ln.setDiscriminator(10);
  EXPECT_EQ(Ln.getAsYAML(), Expected + ("\n  Discriminator: 10"
                                        "\n  NewStatement: false"
                                        "\n  PrologueEnd: false"
                                        "\n  EndSequence: false"
                                        "\n  BasicBlock: false"
                                        "\n  EpilogueBegin: false"));

  Ln.setIsNewStatement();
  EXPECT_EQ(Ln.getAsYAML(), Expected + ("\n  Discriminator: 10"
                                        "\n  NewStatement: true"
                                        "\n  PrologueEnd: false"
                                        "\n  EndSequence: false"
                                        "\n  BasicBlock: false"
                                        "\n  EpilogueBegin: false"));

  Ln.setIsPrologueEnd();
  EXPECT_EQ(Ln.getAsYAML(), Expected + ("\n  Discriminator: 10"
                                        "\n  NewStatement: true"
                                        "\n  PrologueEnd: true"
                                        "\n  EndSequence: false"
                                        "\n  BasicBlock: false"
                                        "\n  EpilogueBegin: false"));

  Ln.setIsLineEndSequence();
  EXPECT_EQ(Ln.getAsYAML(), Expected + ("\n  Discriminator: 10"
                                        "\n  NewStatement: true"
                                        "\n  PrologueEnd: true"
                                        "\n  EndSequence: true"
                                        "\n  BasicBlock: false"
                                        "\n  EpilogueBegin: false"));

  Ln.setIsNewBasicBlock();
  EXPECT_EQ(Ln.getAsYAML(), Expected + ("\n  Discriminator: 10"
                                        "\n  NewStatement: true"
                                        "\n  PrologueEnd: true"
                                        "\n  EndSequence: true"
                                        "\n  BasicBlock: true"
                                        "\n  EpilogueBegin: false"));

  Ln.setIsEpilogueBegin();
  EXPECT_EQ(Ln.getAsYAML(), Expected + ("\n  Discriminator: 10"
                                        "\n  NewStatement: true"
                                        "\n  PrologueEnd: true"
                                        "\n  EndSequence: true"
                                        "\n  BasicBlock: true"
                                        "\n  EpilogueBegin: true"));
}
