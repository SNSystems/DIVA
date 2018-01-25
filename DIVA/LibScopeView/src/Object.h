//===-- LibScopeView/Object.h -----------------------------------*- C++ -*-===//
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
/// Definition of the Object and Element classes.
///
//===----------------------------------------------------------------------===//

#ifndef OBJECT_H
#define OBJECT_H

// Disable some clang warnings for libdwarf.h.
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#pragma clang diagnostic ignored "-Wundef"
#endif

#include "libdwarf.h"

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#include "StringPool.h"

#include <bitset>
#include <cassert>
#include <cstdint>

namespace LibScopeView {

class Object;
class PrintSettings;
class Scope;
class Type;

/// \brief Return true if Obj is an instance of T or its subclasses.
template <class T> bool isa(const Object &Obj) { return T::classof(&Obj); }

/// \brief Cast Obj to T. Obj must be instance of T or its subclasses.
template <class T> T *cast(Object *Obj) {
  assert(isa<T>(*Obj) && "Tried to cast Object instance to invalid type");
  return static_cast<T *>(Obj);
}
template <class T> const T *cast(const Object *Obj) {
  return cast<T>(const_cast<Object *>(Obj));
}
template <class T> const T &cast(const Object &Obj) {
  return *cast<T>(const_cast<Object *>(&Obj));
}
template <class T> T &cast(Object &Obj) { return *cast<T>(&Obj); }

/// \brief Cast Obj to T or return nullptr if it is not an instance of T.
template <class T> T *dyn_cast(Object *Obj) {
  return isa<T>(*Obj) ? static_cast<T *>(Obj) : nullptr;
}
template <class T> const T *dyn_cast(const Object *Obj) {
  return dyn_cast<T>(const_cast<Object *>(Obj));
}

/// \brief Print sizes and counts of allocated Objects.
void printAllocationInfo(const Object &Root, std::ostream &Out);

/// \brief Enum to represent C++ access specifiers.
enum class AccessSpecifier { Unspecified, Private, Protected, Public };

/// \brief Class to represent the basic information for a DIVA object.
class Object {
public:
  /// \brief Kind of Object.
  ///
  /// This enum is used to implement custom RTTI and is based on the LLVM style
  /// described here: http://llvm.org/docs/HowToSetUpLLVMStyleRTTI.html
  ///
  /// The enum is ordered as a preorder traversal of the class hierarchy tree to
  /// make checking classes more efficient. When modifying the list you need to
  /// be careful to check any effect you might have on the subclasses static
  /// 'classof' methods.
  enum ObjectKind {
    SV_Line,
    SV_Scope,
    SV_ScopeAggregate,
    SV_ScopeAlias,
    SV_ScopeArray,
    SV_ScopeCompileUnit,
    SV_ScopeEnumeration,
    SV_ScopeFunction,
    SV_ScopeFunctionInlined,
    SV_ScopeNamespace,
    SV_ScopeTemplatePack,
    SV_ScopeRoot,
    SV_Symbol,
    SV_Type,
    SV_TypeDefinition,
    SV_TypeEnumerator,
    SV_TypeImport,
    SV_TypeTemplateParam,
    SV_TypeSubrange,
  };

  /// \brief Get the kind of the Object.
  ObjectKind getKind() const { return Kind; }

  /// \brief Return true if Obj is an instance of Object.
  static bool classof(const Object *) { return true; }

  Object(ObjectKind K);
  virtual ~Object();

  Object(const Object &) = delete;
  Object &operator=(const Object &) = delete;

  Object(const Object &&) = delete;
  Object &operator=(const Object &&) = delete;

private:
  const ObjectKind Kind;

  // Flags specifying various properties of the Object.
  enum ObjectAttributes {
    IsGlobalReference,
    InvalidFilename,
    ObjectAttributesSize
  };
  // Flags specifying various properties of the Object.
  std::bitset<ObjectAttributesSize> ObjectAttributesFlags;

public:
  /// \brief Get the object kind as a string.
  const char *getKindAsString() const;

  /// \brief The Object is referenced from other CUs.
  bool getIsGlobalReference() const {
    return ObjectAttributesFlags[IsGlobalReference];
  }
  void setIsGlobalReference() { ObjectAttributesFlags.set(IsGlobalReference); }

  /// \brief The filename associated with the object is valid.
  bool getInvalidFileName() const {
    return ObjectAttributesFlags[InvalidFilename];
  }
  void setInvalidFileName() { ObjectAttributesFlags.set(InvalidFilename); }

private:
  // Source file associated with this object.
  StringPoolRef FilePathRef;
  // Line associated with this object.
  uint64_t LineNumber;

  // The parent of this object (nullptr if the root scope).
  Scope *Parent;

  // Information to link the object back to the DWARF.
  Dwarf_Off DieOffset; // Global Offset in Debug Info.
  Dwarf_Half DieTag;   // DWARF tag/attr for this object.

public:
  /// \brief DWARF Die tag.
  Dwarf_Half getDieTag() const { return DieTag; }
  void setDieTag(Dwarf_Half DWTag) { DieTag = DWTag; }

  /// \brief DWARF Die offset.
  Dwarf_Off getDieOffset() const { return DieOffset; }
  void setDieOffset(Dwarf_Off Offset) { DieOffset = Offset; }

  /// \brief The Object's name.
  virtual const std::string &getName() const;

  /// \brief The Object's qualified name.
  virtual const std::string &getQualifiedName() const;

  /// \brief The Object's file path.
  const std::string &getFilePath() const;
  StringPoolRef getFilePathPoolRef() const { return FilePathRef; }
  void setFilePath(const std::string &FilePath);
  void setFilePath(StringPoolRef FilePath) { FilePathRef = FilePath; }

  /// \brief The line for the object.
  uint64_t getLineNumber() const { return LineNumber; }
  void setLineNumber(uint64_t LnNumber) { LineNumber = LnNumber; }

  /// \brief The parent scope for this object.
  Scope *getParent() const { return Parent; }
  void setParent(Scope *ObjParent) { Parent = ObjParent; }

  // Get type info as string, handling the null case.
  std::string getTypeDieOffsetAsString(const PrintSettings &Settings) const;
  const std::string &getTypeAsString(const PrintSettings &Settings) const;

  const std::string &getTypeQualifiedName() const;

  virtual Object *getType() const { return nullptr; }

  /// \brief Should this object be printed under children?
  virtual bool getIsPrintedAsObject() const { return true; }
  /// \brief Returns a text representation of this DIVA Object.
  virtual std::string getAsText(const PrintSettings &Settings) const = 0;
  /// \brief Returns a YAML representation of this DIVA Object.
  virtual std::string getAsYAML() const = 0;

protected:
  /// \brief Returns a text representation of attribute information.
  static std::string formatAttributeText(const std::string &AttributeText);
  /// \brief Returns the common YAML information for this object.
  std::string getCommonYAML() const;
};

/// \brief Class to represent the basic data for an object.
class Element : public Object {
public:
  Element(ObjectKind K);

  /// Return true if Obj is an instance of Element.
  static bool classof(const Object *Obj) { return Obj->getKind() != SV_Line; }

private:
  // The name, type name, qualified name and filename in String Pool.
  StringPoolRef NameRef;
  StringPoolRef QualifiedRef;

  // Type of this object.
  Object *TheType;

public:
  /// \brief The Object's name.
  const std::string &getName() const override;
  StringPoolRef getNamePoolRef() const { return NameRef; }
  virtual void setName(const std::string &Name);
  void setName(StringPoolRef Name) { NameRef = Name; }

  /// \brief The Object's qualified name.
  const std::string &getQualifiedName() const override;
  void setQualifiedName(const std::string &Name);

  /// \brief Set the qualified name to include the parent's name.
  void resolveQualifiedName() { resolveQualifiedName(getParent()); }
  void resolveQualifiedName(const Scope *ExplicitParent);

  Object *getType() const override { return TheType; }
  void setType(Object *Obj) { TheType = Obj; }
};

} // namespace LibScopeView

#endif // OBJECT_H
