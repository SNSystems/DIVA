//===-- UnitTests/TestLibScopeView/TestObject.cpp ---------------*- C++ -*-===//
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
/// Tests for LibScopeView::Object.
///
//===----------------------------------------------------------------------===//

#include "Reader.h"
#include "Symbol.h"
#include "Type.h"

#include "dwarf.h"
#include "gtest/gtest.h"

using namespace LibScopeView;

namespace {

// Object class where the properties can be set and getCommonYAML is public.
class TestObject : public Object {
public:
  TestObject() : Object(), Type(nullptr) {}

  const char *getKindAsString() const override {
    static std::string Kind("ObjKind");
    return Kind.c_str();
  }
  const char *getName() const override {
    return Name.c_str();
  }
  size_t getNameIndex() const override { return 0; }
  virtual void setNameIndex(size_t NameIndex) override {}
  void setName(const char *name) override {
    Name = name;
  }
  const char *getQualifiedName() const override {
    return QName.c_str();
  }
  void setQualifiedName(const char *name) override {
    setHasQualifiedName();
    QName = name;
  }
  const char *getTypeName() const override { return ""; }
  std::string getFileName(bool format_options) const override {
    return FileName.c_str();
  }
  void setFileName(const char *fileName) override {
    FileName = fileName;
  }
  size_t getFileNameIndex() const override { return 0; }
  virtual void setFileNameIndex(size_t FilenameIndex) override {};
  Object *getType() const override {
    return Type;
  }
  const char *getTypeQualifiedName() const override { return nullptr; }
  bool isNamed() const override { return false; }
  void setType(Object *object) override {
    Type = object;
  }

  std::string getAsText(const PrintSettings &) const override { return ""; };
  std::string getAsYAML() const override { return ""; };

  using Object::getCommonYAML;

private:
  std::string Name;
  std::string QName;
  std::string FileName;
  Object *Type;
};

}

TEST(Object, getCommonYAML) {
  Reader R;
  setReader(&R);

  TestObject TO;
  EXPECT_EQ(TO.getCommonYAML(),
            "object: \"ObjKind\"\n"
            "name: null\n"
            "type: null\n"
            "source:\n"
            "  line: null\n"
            "  file: null\n"
            "dwarf:\n"
            "  offset: 0x0\n"
            "  tag: null");

  TO.setName("VarName");
  EXPECT_EQ(TO.getCommonYAML(),
            "object: \"ObjKind\"\n"
            "name: \"VarName\"\n"
            "type: null\n"
            "source:\n"
            "  line: null\n"
            "  file: null\n"
            "dwarf:\n"
            "  offset: 0x0\n"
            "  tag: null");

  TO.setQualifiedName("Q::");
  EXPECT_EQ(TO.getCommonYAML(),
            "object: \"ObjKind\"\n"
            "name: \"Q::VarName\"\n"
            "type: null\n"
            "source:\n"
            "  line: null\n"
            "  file: null\n"
            "dwarf:\n"
            "  offset: 0x0\n"
            "  tag: null");

  Type Ty;
  Ty.setName("Ty");
  TO.setType(&Ty);
  EXPECT_EQ(TO.getCommonYAML(),
            "object: \"ObjKind\"\n"
            "name: \"Q::VarName\"\n"
            "type: \"Ty\"\n"
            "source:\n"
            "  line: null\n"
            "  file: null\n"
            "dwarf:\n"
            "  offset: 0x0\n"
            "  tag: null");

  Ty.setQualifiedName("Class::");
  Ty.setHasQualifiedName();
  EXPECT_EQ(TO.getCommonYAML(),
            "object: \"ObjKind\"\n"
            "name: \"Q::VarName\"\n"
            "type: \"Class::Ty\"\n"
            "source:\n"
            "  line: null\n"
            "  file: null\n"
            "dwarf:\n"
            "  offset: 0x0\n"
            "  tag: null");

  TO.setLineNumber(25);
  EXPECT_EQ(TO.getCommonYAML(),
            "object: \"ObjKind\"\n"
            "name: \"Q::VarName\"\n"
            "type: \"Class::Ty\"\n"
            "source:\n"
            "  line: 25\n"
            "  file: null\n"
            "dwarf:\n"
            "  offset: 0x0\n"
            "  tag: null");

  TO.setFileName("path/file.cpp");
  EXPECT_EQ(TO.getCommonYAML(),
            "object: \"ObjKind\"\n"
            "name: \"Q::VarName\"\n"
            "type: \"Class::Ty\"\n"
            "source:\n"
            "  line: 25\n"
            "  file: \"path/file.cpp\"\n"
            "dwarf:\n"
            "  offset: 0x0\n"
            "  tag: null");

  TO.setInvalidFileName();
  EXPECT_EQ(TO.getCommonYAML(),
            "object: \"ObjKind\"\n"
            "name: \"Q::VarName\"\n"
            "type: \"Class::Ty\"\n"
            "source:\n"
            "  line: 25\n"
            "  file: \"?\"\n"
            "dwarf:\n"
            "  offset: 0x0\n"
            "  tag: null");

  TO.setDieOffset(0x201);
  EXPECT_EQ(TO.getCommonYAML(),
            "object: \"ObjKind\"\n"
            "name: \"Q::VarName\"\n"
            "type: \"Class::Ty\"\n"
            "source:\n"
            "  line: 25\n"
            "  file: \"?\"\n"
            "dwarf:\n"
            "  offset: 0x201\n"
            "  tag: null");

  TO.setDieTag(DW_TAG_variable);
  EXPECT_EQ(TO.getCommonYAML(),
            "object: \"ObjKind\"\n"
            "name: \"Q::VarName\"\n"
            "type: \"Class::Ty\"\n"
            "source:\n"
            "  line: 25\n"
            "  file: \"?\"\n"
            "dwarf:\n"
            "  offset: 0x201\n"
            "  tag: \"DW_TAG_variable\"");
}

TEST(Object, ResolveQualifiedName) {
  Reader R;
  setReader(&R);

  ScopeNamespace NS1;
  NS1.setName("NS1");
  NS1.setIsNamespace();

  ScopeNamespace NS2;
  NS2.setName("NS2");
  NS2.setIsNamespace();

  ScopeFunction Func;
  Func.setName("Function");
  Func.setIsFunction();

  ScopeRoot Root;
  Root.setName("Root");
  Root.setIsRoot();

  ScopeCompileUnit CU;
  CU.setName("CU");
  CU.setIsCompileUnit();

  Scope Block;
  Block.setIsLexicalBlock();

  // Single parent.
  {
    Symbol Sym;
    Sym.resolveQualifiedName(&NS1);
    EXPECT_TRUE(Sym.getHasQualifiedName());
    EXPECT_STREQ(Sym.getQualifiedName(), "NS1::");
  }

  // Multiple parents.
  {
    NS2.setParent(&NS1);
    Symbol Sym;
    Sym.resolveQualifiedName(&NS2);
    EXPECT_TRUE(Sym.getHasQualifiedName());
    EXPECT_STREQ(Sym.getQualifiedName(), "NS1::NS2::");
  }

  // Function parent.
  {
    Func.setParent(&NS2);
    Symbol Sym;
    Sym.resolveQualifiedName(&Func);
    EXPECT_FALSE(Sym.getHasQualifiedName());
    EXPECT_STREQ(Sym.getQualifiedName(), "");
  }

  // Stop at ScopeRoot.
  {
    NS1.setParent(&Root);
    Symbol Sym;
    Sym.resolveQualifiedName(&NS2);
    EXPECT_TRUE(Sym.getHasQualifiedName());
    EXPECT_STREQ(Sym.getQualifiedName(), "NS1::NS2::");
  }

  // Stop at CU.
  {
    NS1.setParent(&CU);
    Symbol Sym;
    Sym.resolveQualifiedName(&NS2);
    EXPECT_TRUE(Sym.getHasQualifiedName());
    EXPECT_STREQ(Sym.getQualifiedName(), "NS1::NS2::");
  }

  // Parent with no name.
  {
    NS2.setParent(&Block);
    Block.setParent(&NS1);
    Symbol Sym;
    Sym.resolveQualifiedName(&NS2);
    EXPECT_TRUE(Sym.getHasQualifiedName());
    EXPECT_STREQ(Sym.getQualifiedName(), "NS1::NS2::");
  }
}
