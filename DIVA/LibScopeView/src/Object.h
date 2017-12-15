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
void printAllocationInfo(std::ostream &Out);

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

  /// \brief Return true if Obj is an insance of Object.
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
    HasType,
    IsGlobalReference,
    InvalidFilename,
    HasReference,
    HasQualifiedName,
    ObjectAttributesSize
  };
  // Flags specifying various properties of the Object.
  std::bitset<ObjectAttributesSize> ObjectAttributesFlags;

public:
  /// \brief Get the object kind as a string.
  const char *getKindAsString() const;

  /// \brief The Object has a type.
  bool getHasType() const { return ObjectAttributesFlags[HasType]; }
  void setHasType() { ObjectAttributesFlags.set(HasType); }

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

  /// \brief The Object has a reference to another object.
  ///
  /// DW_AT_specification, DW_AT_abstract_origin, DW_AT_extension.
  bool getHasReference() const { return ObjectAttributesFlags[HasReference]; }
  void setHasReference() { ObjectAttributesFlags.set(HasReference); }

  /// \brief If the Object has a qualified name.
  bool getHasQualifiedName() const {
    return ObjectAttributesFlags[HasQualifiedName];
  }
  void setHasQualifiedName() { ObjectAttributesFlags.set(HasQualifiedName); }

private:
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
  virtual const char *getName() const = 0;
  virtual void setName(const char *Name) = 0;

  /// \brief StringPool index of the Object's name.
  virtual size_t getNameIndex() const = 0;
  virtual void setNameIndex(size_t NameIndex) = 0;

  /// \brief The Object's qualified name.
  virtual const char *getQualifiedName() const = 0;
  virtual void setQualifiedName(const char *Name) = 0;

  /// \brief The Object's type name (if any).
  virtual const char *getTypeName() const = 0;

  /// \brief The Object's filename.
  virtual std::string getFileName(bool FormatOptions) const = 0;
  virtual void setFileName(const char *FileName) = 0;

  /// \brief StringPool index of the Object's filename.
  virtual size_t getFileNameIndex() const = 0;
  virtual void setFileNameIndex(size_t FilenameIndex) = 0;

  /// \brief The Object's type qualified name.
  virtual const char *getTypeQualifiedName() const = 0;

  /// \brief If the Object has a name.
  virtual bool isNamed() const = 0;
  virtual bool isUnnamed() const { return !isNamed(); }

  bool isLined() const { return LineNumber != 0; }
  bool isUnlined() const { return !isLined(); }

  /// \brief Set the qualified name to include the parent's name.
  void resolveQualifiedName() { resolveQualifiedName(getParent()); }
  void resolveQualifiedName(const Scope *ExplicitParent);

  /// \brief The line for the object.
  uint64_t getLineNumber() const { return LineNumber; }
  void setLineNumber(uint64_t LnNumber) { LineNumber = LnNumber; }

  /// \brief The call line for the object (Inlined Functions).
  virtual uint64_t getCallLineNumber() const { return 0; }
  virtual void setCallLineNumber(uint64_t /*CallLineNumber*/) {}

  /// \brief The access DW_AT_GNU_discriminator attribute.
  virtual Dwarf_Half getDiscriminator() const { return 0; }
  virtual void setDiscriminator(Dwarf_Half /*Discriminator*/) {}

  /// \brief The parent scope for this object.
  Scope *getParent() const { return Parent; }
  void setParent(Scope *ObjParent) { Parent = ObjParent; }

  // Get some attributes as string.
  const char *getDieOffsetAsString(const PrintSettings &Settings) const;
  const char *getTypeDieOffsetAsString(const PrintSettings &Settings) const;
  const char *getTypeAsString(const PrintSettings &Settings) const;

  virtual Object *getType() const = 0;
  virtual void setType(Object *Obj) = 0;

  /// \brief Generate the full name for the object.
  bool setFullName(const PrintSettings &Settings, Type *BaseType,
                   Scope *BaseScope, Scope *SpecScope,
                   const char *BaseText = nullptr);

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

  /// Return true if Obj is an insance of Element.
  static bool classof(const Object *) { return true; }

private:
  // The name, type name, qualified name and filename in String Pool.
  size_t NameIndex;
  size_t QualifiedIndex;
  size_t FilenameIndex;

  // Type of this object.
  Object *TheType;

public:
  bool isNamed() const override { return NameIndex != 0; }
  bool isUnnamed() const override { return !isNamed(); }

  /// \brief The Object's name.
  const char *getName() const override;
  void setName(const char *Name) override;

  /// \brief The StringPool index of the Object's name.
  size_t getNameIndex() const override;
  void setNameIndex(size_t NameIndex) override;

  /// \brief The Object's qualified name.
  const char *getQualifiedName() const override;
  void setQualifiedName(const char *Name) override;

  /// \brief The Object's type name (if any).
  const char *getTypeName() const override;

  /// \brief The Object's filename.
  std::string getFileName(bool NameOnly) const override;
  void setFileName(const char *FileName) override;

  /// \brief The StringPool index of the Object's filename.
  size_t getFileNameIndex() const override;
  void setFileNameIndex(size_t FilenameIndex) override;

  /// \brief The Object's type qualified name.
  const char *getTypeQualifiedName() const override;

  void setType(Object *Obj) override {
    setHasType();
    TheType = Obj;
  }
  Object *getType() const override { return TheType; }
};

} // namespace LibScopeView

#endif // OBJECT_H
