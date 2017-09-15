//===-- LibScopeView/ScopeVisitor.h -----------------------------*- C++ -*-===//
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
/// This file contains the declaration of the ScopeVisitor and ConstScopeVisitor
/// classes, which are the base classes for all visitors over DIVA's internal
/// representation.
///
//===----------------------------------------------------------------------===//

#ifndef SCOPEVIEW_SCOPEVISITOR_H
#define SCOPEVIEW_SCOPEVISITOR_H

namespace LibScopeView {

class Object;

/// \brief An abstract base class for visiting Diva's internal representation.
///
/// Methods are provided to subclasses to allow traversal.
class ScopeVisitor {
public:
  virtual ~ScopeVisitor();

  /// \brief Visit an object.
  void visit(Object *Obj);

private:
  /// \brief Subclass interface for visiting an Object.
  virtual void visitImpl(Object *Obj) = 0;

protected:
  /// \brief Visit the children of an Object.
  void visitChildren(Object *Obj);
};

/// \brief A ScopeVisitor that traverses and visits const Objects.
class ConstScopeVisitor : private ScopeVisitor {
public:
  virtual ~ConstScopeVisitor();

  /// \brief Visit an object.
  void visit(const Object *Obj) {
    // Cast away the constness as ScopeVisitor::visit will call
    // ScopeVisitor::visitImpl which is overloaded below to call the const
    // visitImpl.
    ScopeVisitor::visit(const_cast<Object *>(Obj));
  }

private:
  /// \brief Subclass interface for visiting an Object.
  virtual void visitImpl(const Object *Obj) = 0;

  // Override the non-const visit (from ScopeVisitor) to call the const one.
  void visitImpl(Object *Obj) override {
    return visitImpl(static_cast<const Object *>(Obj));
  }

protected:
  /// \brief Visit the children of an Object.
  void visitChildren(const Object *Obj) {
    // Cast away the const-ness and call the non-const visit children. This is
    // safe as visitChildren itself doesn't modify the Object and the non-const
    // visit method is overridden to do nothing but call the const visit method.
    ScopeVisitor::visitChildren(const_cast<Object *>(Obj));
  }
};

} // end namespace LibScopeView

#endif // SCOPEVIEW_SCOPEVISITOR_H
