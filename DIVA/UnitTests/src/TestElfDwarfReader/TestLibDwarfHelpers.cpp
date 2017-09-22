//===-- ElfReader/TestLibDwarfHelpers.cpp -----------------------*- C++ -*-===//
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
/// Tests for LibDwarfHelpers.
///
//===----------------------------------------------------------------------===//

#include "FileUtilities.h"
#include "LibDwarfHelpers.h"
#include "UtilsForTesting.h"

#include "gtest/gtest.h"

using namespace ElfDwarfReader;

// IMPORTANT.
//
// These tests depend on the DWARF data in TestInputs/DwarfHelpers/test.elf.
// Changing test.elf in any way may cause these tests to fail.

namespace {

// Test fixture that creates a DwarfDebugData instance from
// DwarfHelpers/test.elf.
class LibDwarfHelpers : public ::testing::Test {
public:
  void SetUp() {
    // Check and open the elf file.
    std::string TestElfPath = getTestInputFilePath("DwarfHelpers/test.elf");
    ASSERT_TRUE(LibScopeView::doesFileExist(TestElfPath));
    LibScopeView::FileDescriptor FD(TestElfPath);
    ASSERT_GT(*FD, 0);

    TestDebugData = DwarfDebugData(*FD);
  }

protected:
  DwarfDebugData TestDebugData;

private:
  int FD;
};

}

TEST(DwarfHelpers, getDwarfTagAsString) {
  EXPECT_EQ(getDwarfTagAsString(DW_TAG_class_type), "DW_TAG_class_type");
  EXPECT_EQ(getDwarfTagAsString(DW_TAG_member), "DW_TAG_member");
  EXPECT_EQ(getDwarfTagAsString(0), "");
}

TEST(DwarfHelpers, getDwarfAttrAsString) {
  EXPECT_EQ(getDwarfAttrAsString(DW_AT_accessibility), "DW_AT_accessibility");
  EXPECT_EQ(getDwarfAttrAsString(DW_AT_member), "DW_AT_member");
  EXPECT_EQ(getDwarfAttrAsString(0), "");
}

TEST(DwarfHelpers, getDwarfFormAsString) {
  EXPECT_EQ(getDwarfFormAsString(DW_FORM_addr), "DW_FORM_addr");
  EXPECT_EQ(getDwarfFormAsString(DW_FORM_data1), "DW_FORM_data1");
  EXPECT_EQ(getDwarfFormAsString(0), "");
}

TEST(DwarfHelpers, Empty) {
  // repro8.o came from a crtbegin.o file as described in:
  // https://github.com/SNSystems/DIVA/issues/8
  std::string TestElfPath = getTestInputFilePath("DwarfHelpers/empty.o");
  ASSERT_TRUE(LibScopeView::doesFileExist(TestElfPath));
  LibScopeView::FileDescriptor FD(TestElfPath);
  ASSERT_GT(*FD, 0);

  DwarfDebugData EmptyDebugData(*FD);
  EXPECT_TRUE(EmptyDebugData.empty());
}

TEST_F(LibDwarfHelpers, DwarfDebugData) {
  EXPECT_NE(*TestDebugData, nullptr);

  auto CompileUnits = TestDebugData.getCompileUnits();
  // test.elf should have 3 compile units (test1.cpp, test2.cpp, test3.cpp).
  ASSERT_EQ(CompileUnits.size(), 3U);

  EXPECT_EQ(CompileUnits[0].CUDie.getName(), "test1.cpp");
  EXPECT_EQ(CompileUnits[0].HeaderOffset, 0U);
  EXPECT_EQ(CompileUnits[0].NextHeaderOffset, 0x5aU);

  EXPECT_EQ(CompileUnits[1].CUDie.getName(), "test2.cpp");
  EXPECT_EQ(CompileUnits[1].HeaderOffset, 0x5aU);
  EXPECT_EQ(CompileUnits[1].NextHeaderOffset, 0xa9U);

  EXPECT_EQ(CompileUnits[2].CUDie.getName(), "test3.cpp");
  EXPECT_EQ(CompileUnits[2].HeaderOffset, 0xa9U);
  EXPECT_GT(CompileUnits[2].NextHeaderOffset, CompileUnits[2].HeaderOffset + 0x56U)
      << "NextHeaderOffset of the last CU should be more than (HeaderOffset + "
         "last Die CU offset)";
}

TEST_F(LibDwarfHelpers, DwarfDie) {
  auto CompileUnits = TestDebugData.getCompileUnits();
  ASSERT_NE(CompileUnits.size(), 0U);

  const DwarfDie &TestDie = CompileUnits[0].CUDie;

  // Test basic Attributes.
  EXPECT_EQ(TestDie.getGlobalOffset(), 0x0bU);
  EXPECT_EQ(TestDie.getName(), "test1.cpp");
  EXPECT_EQ(TestDie.getTag(), DW_TAG_compile_unit);
  EXPECT_EQ(TestDie.getTagName(), "DW_TAG_compile_unit");

  // Test hasAttr.
  EXPECT_TRUE(TestDie.hasAttr(DW_AT_name));
  EXPECT_FALSE(TestDie.hasAttr(DW_AT_decl_file));

  // We need other DIEs to test other attribute types.
  auto IT = TestDie.childrenBegin();
  ASSERT_FALSE(IT.atEnd());
  const DwarfDie &TestDie2 = *IT;

  // Test getAttr.
  DwarfAttrValue Flag(TestDie2.getAttr(DW_AT_external));
  ASSERT_EQ(Flag.getKind(), DwarfAttrValueKind::Boolean);
  EXPECT_GT(Flag.getBool(), 0);

  DwarfAttrValue Addr(TestDie2.getAttr(DW_AT_low_pc));
  ASSERT_EQ(Addr.getKind(), DwarfAttrValueKind::Address);
  EXPECT_EQ(Addr.getAddress(), 0x004004e0U);

  DwarfAttrValue Off(TestDie2.getAttr(DW_AT_type));
  ASSERT_EQ(Off.getKind(), DwarfAttrValueKind::Reference);
  EXPECT_EQ(Off.getReference(), 0x52U);

  DwarfAttrValue Unsigned(TestDie2.getAttr(DW_AT_decl_file));
  ASSERT_EQ(Unsigned.getKind(), DwarfAttrValueKind::Unsigned);
  EXPECT_EQ(Unsigned.getUnsigned(), 1U);

  DwarfAttrValue String(TestDie.getAttr(DW_AT_name));
  ASSERT_EQ(String.getKind(), DwarfAttrValueKind::String);
  EXPECT_EQ(String.getString(), "test1.cpp");

  EXPECT_EQ(TestDie.getAttr(DW_AT_decl_line).getKind(),
            DwarfAttrValueKind::Empty);
}

// Simple tree of dwarf tags for testing.
struct TagTree {
  TagTree(Dwarf_Half Tag) : Tag(Tag) {}

  Dwarf_Half Tag;
  std::vector<TagTree> Children;
};

void buildTagTree(TagTree *root, const DwarfDie &Die) {
  root->Children.push_back(Die.getTag());
  for (auto IT = Die.childrenBegin(), End = Die.childrenEnd(); IT != End;
       ++IT) {
    buildTagTree(&(root->Children.back()), *IT);
  }
}

TEST_F(LibDwarfHelpers, DwarfDieChildIterator) {
  // The debug info should follow the following layout:
  //   DW_TAG_compile_unit
  //     DW_TAG_subprogram
  //       DW_TAG_variable
  //     DW_TAG_base_type
  //   DW_TAG_compile_unit
  //     DW_TAG_subprogram
  //     DW_TAG_base_type
  //   DW_TAG_compile_unit
  //     DW_TAG_subprogram
  //       DW_TAG_formal_parameter
  //     DW_TAG_base_type

  // Test that we read that layout.
  TagTree Tree(0);
  for (const auto &CU : TestDebugData.getCompileUnits()) {
    buildTagTree(&Tree, CU.CUDie);
  }

  // Check the number of nodes.
  ASSERT_EQ(Tree.Children.size(), 3U);
  const auto &CU1 = Tree.Children[0];
  const auto &CU2 = Tree.Children[1];
  const auto &CU3 = Tree.Children[2];

  ASSERT_EQ(CU1.Children.size(), 2U);
  const auto &CU1_Func = CU1.Children[0];
  const auto &CU1_Type = CU1.Children[1];
  ASSERT_EQ(CU1_Func.Children.size(), 1U);
  ASSERT_EQ(CU1_Type.Children.size(), 0U);
  ASSERT_EQ(CU1_Func.Children[0].Children.size(), 0U);

  ASSERT_EQ(CU2.Children.size(), 2U);
  const auto &CU2_Func = CU2.Children[0];
  const auto &CU2_Type = CU2.Children[1];
  ASSERT_EQ(CU2_Func.Children.size(), 0U);
  ASSERT_EQ(CU2_Type.Children.size(), 0U);

  ASSERT_EQ(CU3.Children.size(), 2U);
  const auto &CU3_Func = CU3.Children[0];
  const auto &CU3_Type = CU3.Children[1];
  ASSERT_EQ(CU3_Func.Children.size(), 1U);
  ASSERT_EQ(CU3_Type.Children.size(), 0U);
  ASSERT_EQ(CU3_Func.Children[0].Children.size(), 0U);

  // Check the node tags.
  EXPECT_EQ(CU1.Tag, DW_TAG_compile_unit);
  EXPECT_EQ(CU1_Func.Tag, DW_TAG_subprogram);
  EXPECT_EQ(CU1_Func.Children[0].Tag, DW_TAG_variable);
  EXPECT_EQ(CU1_Type.Tag, DW_TAG_base_type);

  EXPECT_EQ(CU2.Tag, DW_TAG_compile_unit);
  EXPECT_EQ(CU2_Func.Tag, DW_TAG_subprogram);
  EXPECT_EQ(CU2_Type.Tag, DW_TAG_base_type);

  EXPECT_EQ(CU3.Tag, DW_TAG_compile_unit);
  EXPECT_EQ(CU3_Func.Tag, DW_TAG_subprogram);
  EXPECT_EQ(CU3_Func.Children[0].Tag, DW_TAG_formal_parameter);
  EXPECT_EQ(CU3_Type.Tag, DW_TAG_base_type);
}

TEST_F(LibDwarfHelpers, DwarfLineTable) {
  auto CompileUnits = TestDebugData.getCompileUnits();
  ASSERT_FALSE(CompileUnits.empty());

  auto LineTable = CompileUnits[0].CUDie.getLineTable();

  // DwarfDump of the lines in the first CU we are checking against.
  //
  // <pc>        [lno,col] NS BB ET PE EB IS= DI= uri: file
  // 0x004004e0  [   4, 0] NS uri: 0x00000001
  // 0x004004ef  [   5,13] NS PE
  // 0x004004f9  [   5,21] DI=0x1
  // 0x00400503  [   5,19]
  // 0x00400508  [   5, 9]
  // 0x0040050b  [   6, 5] NS
  // 0x00400513  [   6, 5] NS ET

  ASSERT_FALSE(LineTable.empty());
  ASSERT_EQ(LineTable.size(), 7U);

  auto Line = LineTable[0];
  EXPECT_EQ(Line.LineNo, 4U);
  EXPECT_EQ(Line.SrcFileID, 1U);
  EXPECT_EQ(Line.LineAddr, 0x004004e0U);
  EXPECT_EQ(Line.IsBeginStatement, 1);
  EXPECT_EQ(Line.IsEndSequence, 0);
  EXPECT_EQ(Line.IsBeginBlock, 0);
  EXPECT_EQ(Line.IsPrologEnd, 0);
  EXPECT_EQ(Line.IsEpilogueBegin, 0);
  EXPECT_EQ(Line.ISA, 0U);
  EXPECT_EQ(Line.Discriminator, 0U);

  Line = LineTable[1];
  EXPECT_EQ(Line.LineNo, 5U);
  EXPECT_EQ(Line.SrcFileID, 1U);
  EXPECT_EQ(Line.LineAddr, 0x004004efU);
  EXPECT_EQ(Line.IsBeginStatement, 1);
  EXPECT_EQ(Line.IsEndSequence, 0);
  EXPECT_EQ(Line.IsBeginBlock, 0);
  EXPECT_EQ(Line.IsPrologEnd, 1);
  EXPECT_EQ(Line.IsEpilogueBegin, 0);
  EXPECT_EQ(Line.ISA, 0U);
  EXPECT_EQ(Line.Discriminator, 0U);

  Line = LineTable[2];
  EXPECT_EQ(Line.LineNo, 5U);
  EXPECT_EQ(Line.SrcFileID, 1U);
  EXPECT_EQ(Line.LineAddr, 0x004004f9U);
  EXPECT_EQ(Line.IsBeginStatement, 0);
  EXPECT_EQ(Line.IsEndSequence, 0);
  EXPECT_EQ(Line.IsBeginBlock, 0);
  EXPECT_EQ(Line.IsPrologEnd, 0);
  EXPECT_EQ(Line.IsEpilogueBegin, 0);
  EXPECT_EQ(Line.ISA, 0U);
  EXPECT_EQ(Line.Discriminator, 1U);

  Line = LineTable[3];
  EXPECT_EQ(Line.LineNo, 5U);
  EXPECT_EQ(Line.SrcFileID, 1U);
  EXPECT_EQ(Line.LineAddr, 0x00400503U);
  EXPECT_EQ(Line.IsBeginStatement, 0);
  EXPECT_EQ(Line.IsEndSequence, 0);
  EXPECT_EQ(Line.IsBeginBlock, 0);
  EXPECT_EQ(Line.IsPrologEnd, 0);
  EXPECT_EQ(Line.IsEpilogueBegin, 0);
  EXPECT_EQ(Line.ISA, 0U);
  EXPECT_EQ(Line.Discriminator, 0U);

  Line = LineTable[6];
  EXPECT_EQ(Line.LineNo, 6U);
  EXPECT_EQ(Line.SrcFileID, 1U);
  EXPECT_EQ(Line.LineAddr, 0x00400513U);
  EXPECT_EQ(Line.IsBeginStatement, 1);
  EXPECT_EQ(Line.IsEndSequence, 1);
  EXPECT_EQ(Line.IsBeginBlock, 0);
  EXPECT_EQ(Line.IsPrologEnd, 0);
  EXPECT_EQ(Line.IsEpilogueBegin, 0);
  EXPECT_EQ(Line.ISA, 0U);
  EXPECT_EQ(Line.Discriminator, 0U);
}
