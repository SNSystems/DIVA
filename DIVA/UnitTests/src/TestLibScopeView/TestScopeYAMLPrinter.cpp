//===-- UnitTests/TestLibScopeView/TestScopeYAMLPrinter.cpp -----*- C++ -*-===//
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
/// Tests for LibScopeView::ScopeYAMLPrinter.
///
//===----------------------------------------------------------------------===//

#include "Scope.h"
#include "ScopeYAMLPrinter.h"

#include "gtest/gtest.h"

using namespace LibScopeView;

namespace {

PrintSettings Settings;

class FakeObject : public Scope {
public:
  FakeObject(std::string FakeName) : FakeName(FakeName) {}
  std::string getAsYAML() const override {
    return std::string("object: Fake\nname: ") + FakeName;
  }
  std::string FakeName;
};

class FakeNoYAMLObject : public Scope {
public:
  bool getIsPrintedAsObject() const override {
    return false;
  }
};

}

TEST(ScopeYAMLPrinter, PrintNoChildren) {
  ScopeRoot Root;
  auto *Top = new FakeObject("Top");
  Root.addChild(Top);

  std::stringstream Output;
  ScopeYAMLPrinter(Settings, "In.o", "V0").print(&Root, Output);

  std::string ExpectedYAML("input_file: \"In.o\"\n");
  ExpectedYAML += "output_version: \"V0\"\n";
  ExpectedYAML += "objects:\n";
  ExpectedYAML += "  - object: Fake\n";
  ExpectedYAML += "    name: Top\n";
  ExpectedYAML += "    children: []\n";

  EXPECT_EQ(Output.str(), ExpectedYAML);
}

TEST(ScopeYAMLPrinter, Print) {
  ScopeRoot Root;
  auto *Top = new FakeObject("Top");
  auto *Child1 = new FakeObject("Child1");
  auto *Child2 = new FakeObject("Child2");
  auto *Child3 = new FakeObject("Child3");
  auto *Child4 = new FakeObject("Child4");
  Root.addChild(Top);
  Top->addChild(Child1);
  Top->addChild(Child2);
  Child1->addChild(Child3);
  Child1->addChild(Child4);

  std::stringstream Output;
  ScopeYAMLPrinter(Settings, "In.o", "V0").print(&Root, Output);

  std::string ExpectedYAML("input_file: \"In.o\"\n");
  ExpectedYAML += "output_version: \"V0\"\n";
  ExpectedYAML += "objects:\n";
  ExpectedYAML += "  - object: Fake\n";
  ExpectedYAML += "    name: Top\n";
  ExpectedYAML += "    children:\n";
  ExpectedYAML += "      - object: Fake\n";
  ExpectedYAML += "        name: Child1\n";
  ExpectedYAML += "        children:\n";
  ExpectedYAML += "          - object: Fake\n";
  ExpectedYAML += "            name: Child3\n";
  ExpectedYAML += "            children: []\n";
  ExpectedYAML += "          - object: Fake\n";
  ExpectedYAML += "            name: Child4\n";
  ExpectedYAML += "            children: []\n";
  ExpectedYAML += "      - object: Fake\n";
  ExpectedYAML += "        name: Child2\n";
  ExpectedYAML += "        children: []\n";

  EXPECT_EQ(Output.str(), ExpectedYAML);
}

TEST(ScopeYAMLPrinter, SkipObjectsWithNoYAML) {
  ScopeRoot Root;
  auto *Top = new FakeObject("Top");
  auto *Child1 = new FakeObject("Child1");
  auto *Child2NoYAML = new FakeNoYAMLObject;
  auto *Child3 = new FakeObject("Child3");
  auto *Child4 = new FakeObject("Child4");
  Root.addChild(Top);
  Top->addChild(Child1);
  Top->addChild(Child2NoYAML);
  Child2NoYAML->addChild(Child3);
  Child2NoYAML->addChild(Child4);

  std::stringstream Output;
  ScopeYAMLPrinter(Settings, "In.o", "V0").print(&Root, Output);

  std::string ExpectedYAML("input_file: \"In.o\"\n");
  ExpectedYAML += "output_version: \"V0\"\n";
  ExpectedYAML += "objects:\n";
  ExpectedYAML += "  - object: Fake\n";
  ExpectedYAML += "    name: Top\n";
  ExpectedYAML += "    children:\n";
  ExpectedYAML += "      - object: Fake\n";
  ExpectedYAML += "        name: Child1\n";
  ExpectedYAML += "        children: []\n";

  EXPECT_EQ(Output.str(), ExpectedYAML);
}

TEST(ScopeYAMLPrinter, PrintAllObjectWithNoYAML) {
  ScopeRoot Root;
  auto *Top = new FakeObject("Top");
  auto *Child2NoYAML = new FakeNoYAMLObject;
  Root.addChild(Top);
  Top->addChild(Child2NoYAML);

  std::stringstream Output;
  ScopeYAMLPrinter(Settings, "In.o", "V0").print(&Root, Output);

  std::string ExpectedYAML("input_file: \"In.o\"\n");
  ExpectedYAML += "output_version: \"V0\"\n";
  ExpectedYAML += "objects:\n";
  ExpectedYAML += "  - object: Fake\n";
  ExpectedYAML += "    name: Top\n";
  ExpectedYAML += "    children: []\n";

  EXPECT_EQ(Output.str(), ExpectedYAML);
}

TEST(ScopeYAMLPrinter, AddEscapeCharacterToBackSlash) {
  ScopeRoot Root;
  auto *Top = new FakeObject("Top");
  auto *Child1 = new FakeObject("Child1");
  auto *Child2NoYAML = new FakeNoYAMLObject;
  auto *Child3 = new FakeObject("Child3");
  auto *Child4 = new FakeObject("Child4");
  Root.addChild(Top);
  Top->addChild(Child1);
  Top->addChild(Child2NoYAML);
  Child2NoYAML->addChild(Child3);
  Child2NoYAML->addChild(Child4);

  std::stringstream Output;
  std::string inputFile = "..\\..\\file.o";
  ScopeYAMLPrinter(Settings, inputFile, "V0").print(&Root, Output);

  std::string ExpectedYAML("input_file: \"..\\\\..\\\\file.o\"\n");
  ExpectedYAML += "output_version: \"V0\"\n";
  ExpectedYAML += "objects:\n";
  ExpectedYAML += "  - object: Fake\n";
  ExpectedYAML += "    name: Top\n";
  ExpectedYAML += "    children:\n";
  ExpectedYAML += "      - object: Fake\n";
  ExpectedYAML += "        name: Child1\n";
  ExpectedYAML += "        children: []\n";

  EXPECT_EQ(Output.str(), ExpectedYAML);

  // Clear the output buffer.
  Output.str(std::string());
  inputFile = "..\\\\..\\\\file.o";
  ScopeYAMLPrinter(Settings, inputFile, "V0").print(&Root, Output);

  // Re-assign expected result.
  ExpectedYAML = std::string("input_file: \"..\\\\\\\\..\\\\\\\\file.o\"\n");
  ExpectedYAML += "output_version: \"V0\"\n";
  ExpectedYAML += "objects:\n";
  ExpectedYAML += "  - object: Fake\n";
  ExpectedYAML += "    name: Top\n";
  ExpectedYAML += "    children:\n";
  ExpectedYAML += "      - object: Fake\n";
  ExpectedYAML += "        name: Child1\n";
  ExpectedYAML += "        children: []\n";

  EXPECT_EQ(Output.str(), ExpectedYAML);
}
