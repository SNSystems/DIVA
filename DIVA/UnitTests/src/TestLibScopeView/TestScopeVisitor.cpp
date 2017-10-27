//===-- UnitTests/TestLibScopeView/TestScopeVisitor.cpp ---------*- C++ -*-===//
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
/// Tests for LibScopeView::ScopeVisitor.
///
//===----------------------------------------------------------------------===//

#include "Object.h"
#include "Scope.h"
#include "ScopeVisitor.h"
#include "Symbol.h"
#include "Line.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace LibScopeView;

using ::testing::InSequence;

namespace {

// Define a mock of ScopeVisitor that tracks how the visit methods are called.
class MockVisitor : public ScopeVisitor {
public:
  MOCK_METHOD1(visitImpl, void(Object *Obj));

  // Expose the visitChildren method for testing.
  void testVisitChildren(Object *Obj) { visitChildren(Obj); }
};

// Mock visitor that errors on unexpected calls.
typedef testing::StrictMock<MockVisitor> StrictMockVisitor;

// Define a mock of ConstScopeVisitor that tracks how visit(Object *) is called.
class MockConstVisitor : public ConstScopeVisitor {
public:
  MOCK_METHOD1(visitImpl, void(const Object *Obj));
};

typedef testing::StrictMock<MockConstVisitor> StrictMockConstVisitor;

} // end anonymous namespace

// Test visit calls visitImpl on objects.
TEST(ScopeVisitor, VisitObjects) {
  StrictMockVisitor Visitor;
  {
    Scope Scp;
    EXPECT_CALL(Visitor, visitImpl(&Scp));
    Visitor.visit(&Scp);
  }
  {
    Symbol Sym;
    EXPECT_CALL(Visitor, visitImpl(&Sym));
    Visitor.visit(&Sym);
  }
};

// Test ConstScopeVisitor.
TEST(ScopeVisitor, ConstScopeVisitor) {
  MockConstVisitor Visitor;
  {
    Scope Scp;
    const Scope *ConstScp = &Scp;
    EXPECT_CALL(Visitor, visitImpl(ConstScp));
    Visitor.visit(ConstScp);
  }
  {
    Symbol Sym;
    const Symbol *ConstSym = &Sym;
    EXPECT_CALL(Visitor, visitImpl(ConstSym));
    Visitor.visit(ConstSym);
  }
}

// Test using the visitChildren method.
TEST(ScopeVisitor, VisitChildren) {
  StrictMockVisitor Visitor;
  Scope Scp;
  // Allocate child scopes with new, will be deleted by the parent scope in its
  // destructor.
  Scope *Child1 = new Scope();
  Scope *Child1_Child1 = new Scope();
  Scope *Child1_Child2 = new Scope();
  Scope *Child2 = new Scope();
  Scope *Child3 = new Scope();

  Scp.addObject(Child1);
  Child1->addObject(Child1_Child1);
  Child1->addObject(Child1_Child2);
  Scp.addObject(Child2);
  Scp.addObject(Child3);

  {
    InSequence OrderedCalls;
    EXPECT_CALL(Visitor, visitImpl(Child1));
    EXPECT_CALL(Visitor, visitImpl(Child2));
    EXPECT_CALL(Visitor, visitImpl(Child3));
    Visitor.testVisitChildren(&Scp);
  }
  {
    InSequence OrderedCalls;
    EXPECT_CALL(Visitor, visitImpl(Child1_Child1));
    EXPECT_CALL(Visitor, visitImpl(Child1_Child2));
    Visitor.testVisitChildren(Child1);
  }
  {
    // No calls expected.
    Visitor.testVisitChildren(Child2);
  }
}

// Test lines are always visited after other children.
TEST(ScopeVisitor, VisitLineChildren) {
  StrictMockVisitor Visitor;
  Scope Scp;
  Line *Line1 = new Line();
  Line *Line2 = new Line();
  Scope *Child1 = new Scope();
  Scope *Child2 = new Scope();

  Scp.setCanHaveLines();
  Scp.addObject(Line1);
  Scp.addObject(Child1);
  Scp.addObject(Line2);
  Scp.addObject(Child2);

  {
    InSequence OrderedCalls;
    EXPECT_CALL(Visitor, visitImpl(Child1));
    EXPECT_CALL(Visitor, visitImpl(Child2));
    EXPECT_CALL(Visitor, visitImpl(Line1));
    EXPECT_CALL(Visitor, visitImpl(Line2));
    Visitor.testVisitChildren(&Scp);
  }
}

// Test that visitImpl isnt called when nullptr is passed to visit.
TEST(ScopeVisitor, VisitNullptr) {
  StrictMockVisitor Visitor;
#ifdef NDEBUG
  // Should do nothing in a Release build.
  Visitor.visit(nullptr);
#else
  // Test that there is an assertion in a Debug build.
  ASSERT_DEATH({ Visitor.visit(nullptr); },
               "Assertion.*ScopeVisitor::visit passed nullptr");
#endif
}

// Test that nullptr isn't dereferenced when passed to visitChildren.
TEST(ScopeVisitor, VisitChildrenNullptr) {
  StrictMockVisitor Visitor;
#ifdef NDEBUG
  // Should do nothing in a Release build.
  Visitor.testVisitChildren(nullptr);
#else
  // Test that there is an assertion in a Debug build.
  ASSERT_DEATH({ Visitor.testVisitChildren(nullptr); },
    "Assertion.*ScopeVisitor::visitChildren passed nullptr");
#endif
}
