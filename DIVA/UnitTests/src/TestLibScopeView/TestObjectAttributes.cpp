//===-- UnitTests/TestLibScopeView/TestAttributes.cpp -----------*- C++ -*-===//
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
/// Tests for LibScopeView::Object getAttributesAsText methods
///
//===----------------------------------------------------------------------===//

#include "Reader.h"
#include "Symbol.h"
#include "Type.h"

#include "dwarf.h"
#include "gtest/gtest.h"

using namespace LibScopeView;

TEST(ObjectAttributes, getAttributesAsText) {
  Reader R(nullptr);
  setReader(&R);

  ScopeRoot Root(-1);
  Root.setIsRoot();
  Root.setDieOffset(0x0cae);
  Root.setDieTag(/*DW_TAG_file*/0);
  Root.setIsGlobalReference();

  ScopeCompileUnit *CompileUnit = new ScopeCompileUnit(0U);
  CompileUnit->setIsCompileUnit();
  CompileUnit->setDieOffset(0x0b);
  CompileUnit->setDieTag(DW_TAG_compile_unit);
  CompileUnit->setIsGlobalReference();

  ScopeNamespace *Namespace = new ScopeNamespace(1U);
  Namespace->setIsNamespace();
  Namespace->setDieOffset(0x0c);
  Namespace->setDieTag(DW_TAG_namespace);
  Namespace->setIsGlobalReference();

  Symbol *Variable = new Symbol(2U);
  Variable->setIsVariable();
  Variable->setDieOffset(0xba);
  Variable->setDieTag(DW_TAG_variable);
  Variable->setIsGlobalReference();

  TypeDefinition *Typedef = new TypeDefinition(2U);
  Typedef->setIsTypedef();
  Typedef->setDieOffset(0xce);
  Typedef->setDieTag(DW_TAG_typedef);
  Typedef->setIsGlobalReference();

  Root.addObject(CompileUnit);
  CompileUnit->addObject(Namespace);
  Namespace->addObject(Variable);
  Namespace->addObject(Typedef);
  Root.setIsGlobalReference();

  // Test the object, parent DIE offsets;
  // Test the type and level;
  // Test the object DWARF tag.
  R.getOptions().setAttributeOffset();
  R.getOptions().setAttributeParent();
  R.getOptions().setAttributeType();
  R.getOptions().setAttributeLevel();
  R.getOptions().setAttributeGlobal();
  R.getOptions().setAttributeTag();

  EXPECT_EQ(Root.getAttributesAsText(),         "                        [SC]   X                                          ");
  EXPECT_EQ(CompileUnit->getAttributesAsText(), "[0x0000000b][0x00000cae][SC]000X[DW_TAG_compile_unit]                     ");
  EXPECT_EQ(Namespace->getAttributesAsText(),   "[0x0000000c][0x0000000b][SC]001X[DW_TAG_namespace]                        ");
  EXPECT_EQ(Variable->getAttributesAsText(),    "[0x000000ba][0x0000000c][ST]002X[DW_TAG_variable]                         ");
  EXPECT_EQ(Typedef->getAttributesAsText(),     "[0x000000ce][0x0000000c][TY]002X[DW_TAG_typedef]                          ");

  R.getOptions().resetAttributeOffset();
  R.getOptions().resetAttributeParent();
  R.getOptions().resetAttributeType();
  R.getOptions().resetAttributeLevel();
  R.getOptions().resetAttributeGlobal();
  R.getOptions().resetAttributeTag();

  EXPECT_EQ(Root.getAttributesAsText(),         "");
  EXPECT_EQ(CompileUnit->getAttributesAsText(), "");
  EXPECT_EQ(Namespace->getAttributesAsText(),   "");
  EXPECT_EQ(Variable->getAttributesAsText(),    "");
  EXPECT_EQ(Typedef->getAttributesAsText(),     "");

  // Test the object and parent DIE offsets.
  R.getOptions().setAttributeOffset();
  R.getOptions().setAttributeParent();

  EXPECT_EQ(Root.getAttributesAsText(),         "                        ");
  EXPECT_EQ(CompileUnit->getAttributesAsText(), "[0x0000000b][0x00000cae]");
  EXPECT_EQ(Namespace->getAttributesAsText(),   "[0x0000000c][0x0000000b]");
  EXPECT_EQ(Variable->getAttributesAsText(),    "[0x000000ba][0x0000000c]");
  EXPECT_EQ(Typedef->getAttributesAsText(),     "[0x000000ce][0x0000000c]");

  R.getOptions().resetAttributeOffset();
  R.getOptions().resetAttributeParent();

  EXPECT_EQ(Root.getAttributesAsText(),         "");
  EXPECT_EQ(CompileUnit->getAttributesAsText(), "");
  EXPECT_EQ(Namespace->getAttributesAsText(),   "");
  EXPECT_EQ(Variable->getAttributesAsText(),    "");
  EXPECT_EQ(Typedef->getAttributesAsText(),     "");

  // Test the object type and level.
  R.getOptions().setAttributeType();
  R.getOptions().setAttributeLevel();

  EXPECT_EQ(Root.getAttributesAsText(),         "[SC]   ");
  EXPECT_EQ(CompileUnit->getAttributesAsText(), "[SC]000");
  EXPECT_EQ(Namespace->getAttributesAsText(),   "[SC]001");
  EXPECT_EQ(Variable->getAttributesAsText(),    "[ST]002");
  EXPECT_EQ(Typedef->getAttributesAsText(),     "[TY]002");

  R.getOptions().resetAttributeType();
  R.getOptions().resetAttributeLevel();

  EXPECT_EQ(Root.getAttributesAsText(),         "");
  EXPECT_EQ(CompileUnit->getAttributesAsText(), "");
  EXPECT_EQ(Namespace->getAttributesAsText(),   "");
  EXPECT_EQ(Variable->getAttributesAsText(),    "");
  EXPECT_EQ(Typedef->getAttributesAsText(),     "");

  // Test the object 'global' status.
  R.getOptions().setAttributeGlobal();

  EXPECT_EQ(Root.getAttributesAsText(),         "X");
  EXPECT_EQ(CompileUnit->getAttributesAsText(), "X");
  EXPECT_EQ(Namespace->getAttributesAsText(),   "X");
  EXPECT_EQ(Variable->getAttributesAsText(),    "X");
  EXPECT_EQ(Typedef->getAttributesAsText(),     "X");

  R.getOptions().resetAttributeGlobal();

  EXPECT_EQ(Root.getAttributesAsText(),         "");
  EXPECT_EQ(CompileUnit->getAttributesAsText(), "");
  EXPECT_EQ(Namespace->getAttributesAsText(),   "");
  EXPECT_EQ(Variable->getAttributesAsText(),    "");
  EXPECT_EQ(Typedef->getAttributesAsText(),     "");

  // Test the object DIE offset.
  R.getOptions().setAttributeOffset();
  EXPECT_EQ(Root.getAttributesAsText(),         "            ");
  EXPECT_EQ(CompileUnit->getAttributesAsText(), "[0x0000000b]");
  EXPECT_EQ(Namespace->getAttributesAsText(),   "[0x0000000c]");
  EXPECT_EQ(Variable->getAttributesAsText(),    "[0x000000ba]");
  EXPECT_EQ(Typedef->getAttributesAsText(),     "[0x000000ce]");

  R.getOptions().resetAttributeOffset();
  EXPECT_EQ(Root.getAttributesAsText(),         "");
  EXPECT_EQ(CompileUnit->getAttributesAsText(), "");
  EXPECT_EQ(Namespace->getAttributesAsText(),   "");
  EXPECT_EQ(Variable->getAttributesAsText(),    "");
  EXPECT_EQ(Typedef->getAttributesAsText(),     "");

  // Test the object parent DIE offset.
  R.getOptions().setAttributeParent();
  EXPECT_EQ(Root.getAttributesAsText(),         "            ");
  EXPECT_EQ(CompileUnit->getAttributesAsText(), "[0x00000cae]");
  EXPECT_EQ(Namespace->getAttributesAsText(),   "[0x0000000b]");
  EXPECT_EQ(Variable->getAttributesAsText(),    "[0x0000000c]");
  EXPECT_EQ(Typedef->getAttributesAsText(),     "[0x0000000c]");

  R.getOptions().resetAttributeParent();
  EXPECT_EQ(Root.getAttributesAsText(),         "");
  EXPECT_EQ(CompileUnit->getAttributesAsText(), "");
  EXPECT_EQ(Namespace->getAttributesAsText(),   "");
  EXPECT_EQ(Variable->getAttributesAsText(),    "");
  EXPECT_EQ(Typedef->getAttributesAsText(),     "");

  // Test the object type.
  R.getOptions().setAttributeType();

  EXPECT_EQ(Root.getAttributesAsText(),         "[SC]");
  EXPECT_EQ(CompileUnit->getAttributesAsText(), "[SC]");
  EXPECT_EQ(Namespace->getAttributesAsText(),   "[SC]");
  EXPECT_EQ(Variable->getAttributesAsText(),    "[ST]");
  EXPECT_EQ(Typedef->getAttributesAsText(),     "[TY]");

  R.getOptions().resetAttributeType();

  EXPECT_EQ(Root.getAttributesAsText(),         "");
  EXPECT_EQ(CompileUnit->getAttributesAsText(), "");
  EXPECT_EQ(Namespace->getAttributesAsText(),   "");
  EXPECT_EQ(Variable->getAttributesAsText(),    "");
  EXPECT_EQ(Typedef->getAttributesAsText(),     "");

  // Test the object level.
  R.getOptions().setAttributeLevel();

  EXPECT_EQ(Root.getAttributesAsText(),         "   ");
  EXPECT_EQ(CompileUnit->getAttributesAsText(), "000");
  EXPECT_EQ(Namespace->getAttributesAsText(),   "001");
  EXPECT_EQ(Variable->getAttributesAsText(),    "002");
  EXPECT_EQ(Typedef->getAttributesAsText(),     "002");

  R.getOptions().resetAttributeLevel();

  EXPECT_EQ(Root.getAttributesAsText(),         "");
  EXPECT_EQ(CompileUnit->getAttributesAsText(), "");
  EXPECT_EQ(Namespace->getAttributesAsText(),   "");
  EXPECT_EQ(Variable->getAttributesAsText(),    "");
  EXPECT_EQ(Typedef->getAttributesAsText(),     "");

  // Test the object DWARF tag.
  R.getOptions().setAttributeTag();

  EXPECT_EQ(Root.getAttributesAsText(),         "                                          ");
  EXPECT_EQ(CompileUnit->getAttributesAsText(), "[DW_TAG_compile_unit]                     ");
  EXPECT_EQ(Namespace->getAttributesAsText(),   "[DW_TAG_namespace]                        ");
  EXPECT_EQ(Variable->getAttributesAsText(),    "[DW_TAG_variable]                         ");
  EXPECT_EQ(Typedef->getAttributesAsText(),     "[DW_TAG_typedef]                          ");

  R.getOptions().resetAttributeTag();

  EXPECT_EQ(Root.getAttributesAsText(),         "");
  EXPECT_EQ(CompileUnit->getAttributesAsText(), "");
  EXPECT_EQ(Namespace->getAttributesAsText(),   "");
  EXPECT_EQ(Variable->getAttributesAsText(),    "");
  EXPECT_EQ(Typedef->getAttributesAsText(),     "");

  EXPECT_EQ(Root.getAttributesAsText(),         "");
  EXPECT_EQ(CompileUnit->getAttributesAsText(), "");
  EXPECT_EQ(Namespace->getAttributesAsText(),   "");
  EXPECT_EQ(Variable->getAttributesAsText(),    "");
  EXPECT_EQ(Typedef->getAttributesAsText(),     "");

  // Test the object 'global' status.
  R.getOptions().setAttributeGlobal();
  EXPECT_EQ(Root.getAttributesAsText(),         "X");
  EXPECT_EQ(CompileUnit->getAttributesAsText(), "X");
  EXPECT_EQ(Namespace->getAttributesAsText(),   "X");
  EXPECT_EQ(Variable->getAttributesAsText(),    "X");
  EXPECT_EQ(Typedef->getAttributesAsText(),     "X");

  R.getOptions().resetAttributeGlobal();
  EXPECT_EQ(Root.getAttributesAsText(),         "");
  EXPECT_EQ(CompileUnit->getAttributesAsText(), "");
  EXPECT_EQ(Namespace->getAttributesAsText(),   "");
  EXPECT_EQ(Variable->getAttributesAsText(),    "");
  EXPECT_EQ(Typedef->getAttributesAsText(),     "");
}
