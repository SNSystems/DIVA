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
/// Tests for LibScopeView::ScopeTextPrinter.
///
//===----------------------------------------------------------------------===//

#include "Reader.h"
#include "ScopeTextPrinter.h"
#include "StringPool.h"
#include "UtilsForTesting.h"
#include "dwarf.h"
#include "gtest/gtest.h"

#include <assert.h>

using namespace LibScopeView;

namespace {

class FakeObject : public Scope {
public:
  FakeObject(std::string Name, uint64_t Line, size_t FileIndex,
             std::string FileName)
      : Scope(SV_Scope), FakeName(Name), FakeFileIndex(FileIndex),
        FakeFileName(FileName) {
    setLineNumber(Line);
    setIsBlock(); // For PrintSettings::printObject.
  }

  const char *getName() const override { return FakeName.c_str(); }
  std::string getAsText(const PrintSettings &Settings) const override {
    return std::string("{Fake} ") + FakeName + std::string("\n  - Attr");
  }
  size_t getFileNameIndex() const override { return FakeFileIndex; }
  std::string getFileName(bool format_options) const override {
    assert(format_options && "Text printer should always format file name");
    return FakeFileName;
  }

  std::string FakeName;
  size_t FakeFileIndex;
  std::string FakeFileName;
};

class FakeNoTextObject : public Scope {
public:
  bool getIsPrintedAsObject() const override { return false; }
};

} // namespace

TEST(ScopeTextPrinter, PrintNoChildren) {
  PrintSettings Settings;
  Settings.showAll();

  ScopeRoot Root;
  Root.addChild(new FakeObject("Top", 11, 1, "foo.cpp"));

  std::stringstream Output;
  ScopeTextPrinter(Settings, "In.o").print(&Root, Output);

  std::string Expected("{InputFile} \"In.o\"\n\n"
                       "{Source} \"foo.cpp\"\n"
                       "11  {Fake} Top\n"
                       "      - Attr\n");

  EXPECT_EQ(Output.str(), Expected);
}

TEST(ScopeTextPrinter, Print) {
  PrintSettings Settings;
  Settings.showAll();

  ScopeRoot Root;
  auto *Top = new FakeObject("Top", 11, 1, "foo.cpp");
  auto *Child1 = new FakeObject("Child1", 111, 2, "foo.cpp");
  auto *Child2 = new FakeObject("Child2", 1122, 2, "foo.cpp");
  auto *Child3 = new FakeObject("Child3", 2, 3, "bar.cpp");
  auto *Child4 = new FakeObject("Child4", 4, 3, "bar.cpp");
  Root.addChild(Top);
  Top->addChild(Child1);
  Top->addChild(Child2);
  Child1->addChild(Child3);
  Child1->addChild(Child4);

  std::stringstream Output;
  ScopeTextPrinter(Settings, "In.o").print(&Root, Output);

  std::string Expected("{InputFile} \"In.o\"\n\n"
                       "{Source} \"foo.cpp\"\n"
                       "  11  {Fake} Top\n"
                       "        - Attr\n\n"
                       "{Source} \"foo.cpp\"\n"
                       " 111    {Fake} Child1\n"
                       "          - Attr\n\n"
                       "{Source} \"bar.cpp\"\n"
                       "   2      {Fake} Child3\n"
                       "            - Attr\n"
                       "   4      {Fake} Child4\n"
                       "            - Attr\n\n"
                       "{Source} \"foo.cpp\"\n"
                       "1122    {Fake} Child2\n"
                       "          - Attr\n");

  EXPECT_EQ(Output.str(), Expected);
}

TEST(ScopeTextPrinter, SkipObjectsWithNoText) {
  PrintSettings Settings;
  Settings.showAll();

  ScopeRoot Root;
  auto *Top = new FakeObject("Top", 1, 1, "foo.cpp");
  auto *Child1 = new FakeObject("Child1", 2, 1, "foo.cpp");
  auto *Child2NoText = new FakeNoTextObject;
  auto *Child3 = new FakeObject("Child3", 3, 1, "foo.cpp");
  auto *Child4 = new FakeObject("Child4", 4, 1, "foo.cpp");
  Root.addChild(Top);
  Top->addChild(Child1);
  Top->addChild(Child2NoText);
  Child2NoText->addChild(Child3);
  Child2NoText->addChild(Child4);

  std::stringstream Output;
  ScopeTextPrinter(Settings, "In.o").print(&Root, Output);

  std::string Expected("{InputFile} \"In.o\"\n\n"
                       "{Source} \"foo.cpp\"\n"
                       "1  {Fake} Top\n"
                       "     - Attr\n"
                       "2    {Fake} Child1\n"
                       "       - Attr\n");

  EXPECT_EQ(Output.str(), Expected);
}

TEST(ScopeTextPrinter, SkipObjectsDependingOnSettings) {
  PrintSettings Settings;
  Settings.showAll();

  ScopeRoot Root;
  auto *Top = new FakeObject("Top", 11, 1, "foo.cpp");
  Top->setIsTemplate();
  auto *Child1 = new FakeObject("Child1", 111, 1, "foo.cpp");
  auto *Child2 = new FakeObject("Child2", 1122, 1, "foo.cpp");
  Root.addChild(Top);
  Top->addChild(Child1);
  Top->addChild(Child2);

  std::stringstream Output;

  Settings.ShowTemplate = true;
  ScopeTextPrinter(Settings, "In.o").print(&Root, Output);
  EXPECT_EQ(Output.str(), "{InputFile} \"In.o\"\n\n"
                          "{Source} \"foo.cpp\"\n"
                          "  11  {Fake} Top\n"
                          "        - Attr\n"
                          " 111    {Fake} Child1\n"
                          "          - Attr\n"
                          "1122    {Fake} Child2\n"
                          "          - Attr\n");

  Output.str("");
  Settings.ShowTemplate = false;
  ScopeTextPrinter(Settings, "In.o").print(&Root, Output);
  EXPECT_EQ(Output.str(), "{InputFile} \"In.o\"\n\n"
                          "{Source} \"foo.cpp\"\n"
                          " 111    {Fake} Child1\n"
                          "          - Attr\n"
                          "1122    {Fake} Child2\n"
                          "          - Attr\n");
}

TEST(ScopeTextPrinter, SkipObjectsDependingOnFilters) {
  PrintSettings Settings;
  Settings.showAll();

  ScopeRoot Root;
  auto *Top = new FakeObject("Top", 11, 1, "foo.cpp");
  auto *Child1 = new FakeObject("Child1", 111, 1, "foo.cpp");
  auto *Child2 = new FakeObject("Child2", 1122, 1, "foo.cpp");
  auto *Child3 = new FakeObject("Child3", 1, 1, "foo.cpp");
  auto *Child4 = new FakeObject("Child4", 2, 1, "foo.cpp");
  Root.addChild(Top);
  Top->addChild(Child1);
  Top->addChild(Child2);
  Child1->addChild(Child3);
  Child1->addChild(Child4);

  std::stringstream Output;

  Settings.Filters = {std::regex("Child1")};
  ScopeTextPrinter(Settings, "In.o").print(&Root, Output);
  EXPECT_EQ(Output.str(), "{InputFile} \"In.o\"\n\n"
                          "{Source} \"foo.cpp\"\n"
                          " 111    {Fake} Child1\n"
                          "          - Attr\n");

  Output.str("");
  Settings.Filters.clear();
  Settings.FilterAnys = {"Child"};
  ScopeTextPrinter(Settings, "In.o").print(&Root, Output);
  EXPECT_EQ(Output.str(), "{InputFile} \"In.o\"\n\n"
                          "{Source} \"foo.cpp\"\n"
                          " 111    {Fake} Child1\n"
                          "          - Attr\n"
                          "   1      {Fake} Child3\n"
                          "            - Attr\n"
                          "   2      {Fake} Child4\n"
                          "            - Attr\n"
                          "1122    {Fake} Child2\n"
                          "          - Attr\n");

  Output.str("");
  Settings.FilterAnys.clear();
  Settings.TreeFilters = {std::regex("Child1")};
  ScopeTextPrinter(Settings, "In.o").print(&Root, Output);
  EXPECT_EQ(Output.str(), "{InputFile} \"In.o\"\n\n"
                          "{Source} \"foo.cpp\"\n"
                          "  11  {Fake} Top\n"
                          "        - Attr\n"
                          " 111    {Fake} Child1\n"
                          "          - Attr\n"
                          "   1      {Fake} Child3\n"
                          "            - Attr\n"
                          "   2      {Fake} Child4\n"
                          "            - Attr\n");

  Output.str("");
  Settings.TreeFilters = {std::regex("Child2")};
  Settings.Filters = {std::regex("Child3")};
  ScopeTextPrinter(Settings, "In.o").print(&Root, Output);
  EXPECT_EQ(Output.str(), "{InputFile} \"In.o\"\n\n"
                          "{Source} \"foo.cpp\"\n"
                          "  11  {Fake} Top\n"
                          "        - Attr\n"
                          "   1      {Fake} Child3\n"
                          "            - Attr\n"
                          "1122    {Fake} Child2\n"
                          "          - Attr\n");
}

TEST(ScopeTextPrinter, PrintZeroLine) {
  PrintSettings Settings;
  Settings.showAll();

  ScopeRoot Root;
  auto *Top = new FakeObject("Top", 0, 1, "foo.cpp");
  auto *Child1 = new FakeObject("Child1", 1, 1, "foo.cpp");
  Root.addChild(Top);
  Top->addChild(Child1);

  std::stringstream Output;

  Settings.ShowZeroLine = false;
  ScopeTextPrinter(Settings, "In.o").print(&Root, Output);
  EXPECT_EQ(Output.str(), "{InputFile} \"In.o\"\n\n"
                          "{Source} \"foo.cpp\"\n"
                          "   {Fake} Top\n"
                          "     - Attr\n"
                          "1    {Fake} Child1\n"
                          "       - Attr\n");

  Output.str("");
  Settings.ShowZeroLine = true;
  ScopeTextPrinter(Settings, "In.o").print(&Root, Output);
  EXPECT_EQ(Output.str(), "{InputFile} \"In.o\"\n\n"
                          "{Source} \"foo.cpp\"\n"
                          "0  {Fake} Top\n"
                          "     - Attr\n"
                          "1    {Fake} Child1\n"
                          "       - Attr\n");
}

TEST(ScopeTextPrinter, PrintDWARFAttributes) {
  PrintSettings Settings;
  Settings.showAll();

  ScopeRoot Root;
  auto *Top = new FakeObject("Top", 11, 1, "foo.cpp");
  auto *Child1 = new FakeObject("Child1", 111, 1, "foo.cpp");
  auto *Child2 = new FakeObject("Child2", 1122, 1, "foo.cpp");
  Root.addChild(Top);
  Top->addChild(Child1);
  Top->addChild(Child2);

  Root.setDieOffset(0x0);
  Top->setDieOffset(0x1234);
  Child1->setDieOffset(0x11);
  Child2->setDieOffset(0x11111111);

  Top->setDieTag(DW_TAG_compile_unit);
  Child1->setDieTag(DW_TAG_namespace);
  Child2->setDieTag(DW_TAG_enumeration_type);

  std::stringstream Output;

  Settings.ShowDWARFOffset = true;
  ScopeTextPrinter(Settings, "In.o").print(&Root, Output);
  EXPECT_EQ(Output.str(), "              {InputFile} \"In.o\"\n\n"
                          "              {Source} \"foo.cpp\"\n"
                          "[0x00001234]    11  {Fake} Top\n"
                          "                      - Attr\n"
                          "[0x00000011]   111    {Fake} Child1\n"
                          "                        - Attr\n"
                          "[0x11111111]  1122    {Fake} Child2\n"
                          "                        - Attr\n");

  Output.str("");
  Settings.ShowDWARFOffset = false;
  Settings.ShowDWARFParent = true;
  ScopeTextPrinter(Settings, "In.o").print(&Root, Output);
  EXPECT_EQ(Output.str(), "              {InputFile} \"In.o\"\n\n"
                          "              {Source} \"foo.cpp\"\n"
                          "[0x00000000]    11  {Fake} Top\n"
                          "                      - Attr\n"
                          "[0x00001234]   111    {Fake} Child1\n"
                          "                        - Attr\n"
                          "[0x00001234]  1122    {Fake} Child2\n"
                          "                        - Attr\n");

  Output.str("");
  Settings.ShowDWARFParent = false;
  Settings.ShowLevel = true;
  ScopeTextPrinter(Settings, "In.o").print(&Root, Output);
  EXPECT_EQ(Output.str(), "    {InputFile} \"In.o\"\n\n"
                          "    {Source} \"foo.cpp\"\n"
                          "0     11  {Fake} Top\n"
                          "            - Attr\n"
                          "1    111    {Fake} Child1\n"
                          "              - Attr\n"
                          "1   1122    {Fake} Child2\n"
                          "              - Attr\n");

  Output.str("");
  Settings.ShowLevel = false;
  Settings.ShowDWARFTag = true;
  ScopeTextPrinter(Settings, "In.o").print(&Root, Output);
  EXPECT_EQ(Output.str(), "                           {InputFile} \"In.o\"\n\n"
                          "                           {Source} \"foo.cpp\"\n"
                          "[DW_TAG_compile_unit]        11  {Fake} Top\n"
                          "                                   - Attr\n"
                          "[DW_TAG_namespace]          111    {Fake} Child1\n"
                          "                                     - Attr\n"
                          "[DW_TAG_enumeration_type]  1122    {Fake} Child2\n"
                          "                                     - Attr\n");

  Output.str("");
  Settings.ShowDWARFOffset = true;
  Settings.ShowDWARFParent = true;
  Settings.ShowLevel = true;
  Settings.ShowDWARFTag = true;
  ScopeTextPrinter(Settings, "In.o").print(&Root, Output);
  // clang-format off
  std::string Expected(
      "                                                     {InputFile} \"In.o\"\n\n"
      "                                                     {Source} \"foo.cpp\"\n"
      "[0x00001234][0x00000000]0 [DW_TAG_compile_unit]        11  {Fake} Top\n"
      "                                                             - Attr\n"
      "[0x00000011][0x00001234]1 [DW_TAG_namespace]          111    {Fake} Child1\n"
      "                                                               - Attr\n"
      "[0x11111111][0x00001234]1 [DW_TAG_enumeration_type]  1122    {Fake} Child2\n"
      "                                                               - Attr\n");
  // clang-format on
  EXPECT_EQ(Output.str(), Expected);
}

TEST(ScopeTextPrinter, PrintFlagAttributes) {
  PrintSettings Settings;
  Settings.showAll();

  ScopeRoot Root;
  auto *Top = new FakeObject("Top", 1, 1, "foo.cpp");
  auto *Child1 = new FakeObject("Child1", 111, 1, "foo.cpp");
  auto *Child2 = new FakeObject("Child2", 222, 1, "foo.cpp");
  auto *Child3 = new FakeObject("Child3", 333, 1, "foo.cpp");
  Root.addChild(Top);
  Top->addChild(Child1);
  Top->addChild(Child2);
  Top->addChild(Child3);

  Child1->setIsGlobalReference();
  Child3->setIsGlobalReference();

  std::stringstream Output;

  Output.str("");
  Settings.ShowIsGlobal = true;
  ScopeTextPrinter(Settings, "In.o").print(&Root, Output);
  EXPECT_EQ(Output.str(), "  {InputFile} \"In.o\"\n\n"
                          "  {Source} \"foo.cpp\"\n"
                          "    1  {Fake} Top\n"
                          "         - Attr\n"
                          "X 111    {Fake} Child1\n"
                          "           - Attr\n"
                          "  222    {Fake} Child2\n"
                          "           - Attr\n"
                          "X 333    {Fake} Child3\n"
                          "           - Attr\n");

  Output.str("");
  Settings.ShowLevel = true;
  ScopeTextPrinter(Settings, "In.o").print(&Root, Output);
  EXPECT_EQ(Output.str(), "      {InputFile} \"In.o\"\n\n"
                          "      {Source} \"foo.cpp\"\n"
                          "0       1  {Fake} Top\n"
                          "             - Attr\n"
                          "1   X 111    {Fake} Child1\n"
                          "               - Attr\n"
                          "1     222    {Fake} Child2\n"
                          "               - Attr\n"
                          "1   X 333    {Fake} Child3\n"
                          "               - Attr\n");
}

TEST(ScopeTextPrinter, PrintNoIndent) {
  PrintSettings Settings;
  Settings.showAll();

  ScopeRoot Root;
  auto *Top = new FakeObject("Top", 1, 1, "foo.cpp");
  auto *Child1 = new FakeObject("Child1", 111, 1, "foo.cpp");
  auto *Child2 = new FakeObject("Child2", 222, 1, "foo.cpp");
  Root.addChild(Top);
  Top->addChild(Child1);
  Child1->addChild(Child2);

  Child1->setIsGlobalReference();

  std::stringstream Output;

  Output.str("");
  Settings.ShowLevel = true;
  Settings.ShowIsGlobal = true;
  Settings.ShowIndent = true;
  ScopeTextPrinter(Settings, "In.o").print(&Root, Output);
  EXPECT_EQ(Output.str(), "      {InputFile} \"In.o\"\n\n"
                          "      {Source} \"foo.cpp\"\n"
                          "0       1  {Fake} Top\n"
                          "             - Attr\n"
                          "1   X 111    {Fake} Child1\n"
                          "               - Attr\n"
                          "2     222      {Fake} Child2\n"
                          "                 - Attr\n");

  Output.str("");
  Settings.ShowIndent = false;
  ScopeTextPrinter(Settings, "In.o").print(&Root, Output);
  EXPECT_EQ(Output.str(), "      {InputFile} \"In.o\"\n\n"
                          "      {Source} \"foo.cpp\"\n"
                          "0       1  {Fake} Top\n"
                          "             - Attr\n"
                          "1   X 111  {Fake} Child1\n"
                          "             - Attr\n"
                          "2     222  {Fake} Child2\n"
                          "             - Attr\n");
}
