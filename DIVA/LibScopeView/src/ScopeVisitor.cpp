//===-- LibScopeView/ScopeVisitor.cpp ----------------------------- C++ -*-===//
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
/// This file contains the definitions of ScopeVisitor and ConstScopeVisitor's
/// methods.
///
//===----------------------------------------------------------------------===//

#include "ScopeVisitor.h"
#include "Line.h"
#include "Scope.h"

#include <assert.h>

using namespace LibScopeView;

// Defining the dtor here generates the vtable out of line in this file.
// This saves having to create vtables in all files that use the header.
ScopeVisitor::~ScopeVisitor() {}

void ScopeVisitor::visit(Object *Obj) {
  assert(Obj && "ScopeVisitor::visit passed nullptr");
  if (!Obj)
    return; // Handle gracefully in release.
  visitImpl(Obj);
}

void ScopeVisitor::visitChildren(Object *Obj) {
  assert(Obj && "ScopeVisitor::visitChildren passed nullptr");
  if (!Obj)
    return; // Handle gracefully in release.

  // Only scopes have children.
  if (!Obj->getIsScope())
    return;
  // Traverse.
  auto Scp = static_cast<Scope *>(Obj);
  for (Object *Child : Scp->getChildren()) {
    visit(Child);
  }
  for (Object *Ln : Scp->getLines()) {
    visit(Ln);
  }
}

// So the vtable can be out of line.
ConstScopeVisitor::~ConstScopeVisitor() {}
