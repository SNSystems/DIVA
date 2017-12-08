//===-- LibScopeView/Type.h -------------------------------------*- C++ -*-===//
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
/// Definitions of Type and its subclasses.
///
//===----------------------------------------------------------------------===//

#ifndef SCOPEVIEWTYPE_H
#define SCOPEVIEWTYPE_H

#include "Object.h"

namespace LibScopeView {

/// \brief Class to represent a DWARF Type object.
class Type : public Element {
public:
  Type() : Type(SV_Type) {}

  /// \brief Return true if Obj is an insance of Type.
  static bool classof(const Object *Obj) {
    return SV_Type <= Obj->getKind() && Obj->getKind() <= SV_TypeSubrange;
  }

protected:
  Type(ObjectKind K);

private:
  // Type Kind.
  static const char *KindBase;
  static const char *KindConst;
  static const char *KindEnumerator;
  static const char *KindImport;
  static const char *KindImportDeclaration;
  static const char *KindImportModule;
  static const char *KindInherits;
  static const char *KindPointer;
  static const char *KindPointerMember;
  static const char *KindReference;
  static const char *KindRestrict;
  static const char *KindRvalueReference;
  static const char *KindSubrange;
  static const char *KindTemplateTemplate;
  static const char *KindTemplateType;
  static const char *KindTemplateValue;
  static const char *KindTypedef;
  static const char *KindUndefined;
  static const char *KindUnspecified;
  static const char *KindVolatile;

  // Flags specifying various properties of the Type.
  enum TypeAttributes {
    IsBaseType,
    IsConstType,
    IsImported,
    IsImportedModule,
    IsImportedDeclaration,
    IsInheritance,
    IsPointerType,
    IsPointerMemberType,
    IsReferenceType,
    IsRvalueReferenceType,
    IsRestrictType,
    IsTemplateParam,
    IsTemplateTypeParam,
    IsTemplateValueParam,
    IsTemplateTemplateParam,
    IsTypedef,
    IsUnspecifiedType,
    IsVolatileType,
    IsEnumerator,
    IsSubrangeType,
    IncludeInPrint,
    TypeAttributesSize
  };
  std::bitset<TypeAttributesSize> TypeAttributesFlags;

public:
  /// \brief Gets the Type kind as a string (eg, "ARRAY").
  const char *getKindAsString() const override;

  bool getIsBaseType() const { return TypeAttributesFlags[IsBaseType]; }
  void setIsBaseType() { TypeAttributesFlags.set(IsBaseType); }
  bool getIsConstType() const { return TypeAttributesFlags[IsConstType]; }
  void setIsConstType() { TypeAttributesFlags.set(IsConstType); }
  bool getIsImported() const { return TypeAttributesFlags[IsImported]; }
  void setIsImported() { TypeAttributesFlags.set(IsImported); }
  bool getIsImportedDeclaration() const {
    return TypeAttributesFlags[IsImportedDeclaration];
  }
  void setIsImportedDeclaration() {
    TypeAttributesFlags.set(IsImportedDeclaration);
    setIsImported();
  }

  bool getIsImportedModule() const {
    return TypeAttributesFlags[IsImportedModule];
  }
  void setIsImportedModule() {
    TypeAttributesFlags.set(IsImportedModule);
    setIsImported();
  }

  bool getIsInheritance() const { return TypeAttributesFlags[IsInheritance]; }
  void setIsInheritance() { TypeAttributesFlags.set(IsInheritance); }
  bool getIsPointerType() const { return TypeAttributesFlags[IsPointerType]; }
  void setIsPointerType() { TypeAttributesFlags.set(IsPointerType); }
  bool getIsPointerMemberType() const {
    return TypeAttributesFlags[IsPointerMemberType];
  }
  void setIsPointerMemberType() {
    TypeAttributesFlags.set(IsPointerMemberType);
  }
  bool getIsReferenceType() const {
    return TypeAttributesFlags[IsReferenceType];
  }
  void setIsReferenceType() { TypeAttributesFlags.set(IsReferenceType); }
  bool getIsRestrictType() const { return TypeAttributesFlags[IsRestrictType]; }
  void setIsRestrictType() { TypeAttributesFlags.set(IsRestrictType); }
  bool getIsRvalueReferenceType() const {
    return TypeAttributesFlags[IsRvalueReferenceType];
  }
  void setIsRvalueReferenceType() {
    TypeAttributesFlags.set(IsRvalueReferenceType);
  }
  bool getIsTemplateParam() const {
    return TypeAttributesFlags[IsTemplateParam];
  }
  void setIsTemplateParam() { TypeAttributesFlags.set(IsTemplateParam); }
  bool getIsTemplateType() const {
    return TypeAttributesFlags[IsTemplateTypeParam];
  }
  void setIsTemplateType() {
    TypeAttributesFlags.set(IsTemplateTypeParam);
    setIsTemplateParam();
  }

  bool getIsTemplateValue() const {
    return TypeAttributesFlags[IsTemplateValueParam];
  }
  void setIsTemplateValue() {
    TypeAttributesFlags.set(IsTemplateValueParam);
    setIsTemplateParam();
  }

  bool getIsTemplateTemplate() const {
    return TypeAttributesFlags[IsTemplateTemplateParam];
  }
  void setIsTemplateTemplate() {
    TypeAttributesFlags.set(IsTemplateTemplateParam);
    setIsTemplateParam();
  }

  bool getIsTypedef() const { return TypeAttributesFlags[IsTypedef]; }
  void setIsTypedef() { TypeAttributesFlags.set(IsTypedef); }
  bool getIsUnspecifiedType() const {
    return TypeAttributesFlags[IsUnspecifiedType];
  }
  void setIsUnspecifiedType() { TypeAttributesFlags.set(IsUnspecifiedType); }
  bool getIsVolatileType() const { return TypeAttributesFlags[IsVolatileType]; }
  void setIsVolatileType() { TypeAttributesFlags.set(IsVolatileType); }
  bool getIsEnumerator() const { return TypeAttributesFlags[IsEnumerator]; }
  void setIsEnumerator() { TypeAttributesFlags.set(IsEnumerator); }
  bool getIsSubrangeType() const { return TypeAttributesFlags[IsSubrangeType]; }
  void setIsSubrangeType() { TypeAttributesFlags.set(IsSubrangeType); }
  bool getIncludeInPrint() const { return TypeAttributesFlags[IncludeInPrint]; }
  void setIncludeInPrint() { TypeAttributesFlags.set(IncludeInPrint); }

public:
  /// \brief Follow the chain of references given by DW_AT_abstract_origin
  /// and/or DW_AT_specification and update the symbol name.
  const char *resolveName();

  // Wrap SetFullName (Used by ElfReader) in a simpler call.
  using Object::setFullName;
  bool setFullName(const PrintSettings &Settings);

  // Functions to be implemented by derived classes.

  /// \brief Return the underlying type for a typedef.
  virtual Object *getUnderlyingType() { return nullptr; }
  virtual void setUnderlyingType(Object * /*Obj*/) {}
  /// \brief Process the values for a DW_TAG_enumerator.
  virtual const char *getValue() const { return nullptr; }
  virtual void setValue(const char * /*Value*/) {}
  virtual size_t getValueIndex() const { return 0; }

  bool getIsPrintedAsObject() const override;
  /// \brief Returns a text representation of this DIVA Object.
  std::string getAsText(const PrintSettings &Settings) const override;
  /// \brief Returns a YAML representation of this DIVA Object.
  std::string getAsYAML() const override;

private:
  static uint32_t TypesAllocated;

public:
  static uint32_t getInstanceCount() { return TypesAllocated; }

private:
  // DW_AT_byte_size for PrimitiveType.
  unsigned ByteSize;

public:
  unsigned getByteSize() const;
  void setByteSize(unsigned Size);
};

/// \brief Class to represent DW_TAG_typedef_type
class TypeDefinition : public Type {
public:
  TypeDefinition() : Type(SV_TypeDefinition) {}

  /// \brief Return true if Obj is an insance of TypeDefinition.
  static bool classof(const Object *Obj) {
    return Obj->getKind() == SV_TypeDefinition;
  }

  /// \brief Get the underlying type for a typedef.
  Object *getUnderlyingType() override;

  /// \brief Set the underlying type for a typedef.
  void setUnderlyingType(Object *Obj) override;

  bool getIsPrintedAsObject() const override { return true; }
  /// \brief Returns a text representation of this DIVA Object.
  std::string getAsText(const PrintSettings &Settings) const override;
  /// \brief Returns a YAML representation of this DIVA Object.
  std::string getAsYAML() const override;
};

/// \brief Class to represent a DW_TAG_enumerator
class TypeEnumerator : public Type {
public:
  TypeEnumerator() : Type(SV_TypeEnumerator), ValueIndex(0) {}

  /// Return true if Obj is an insance of TypeEnumerator.
  static bool classof(const Object *Obj) {
    return Obj->getKind() == SV_TypeEnumerator;
  }

private:
  size_t ValueIndex; // Enumerator value.

public:
  /// \brief Process the values for a DW_TAG_enumerator.
  const char *getValue() const override;
  void setValue(const char *Value) override;
  size_t getValueIndex() const override;

  bool getIsPrintedAsObject() const override { return false; }
  /// \brief Returns a text representation of this DIVA Object.
  std::string getAsText(const PrintSettings &Settings) const override;
  /// \brief Returns a YAML representation of this DIVA Object.
  std::string getAsYAML() const override;
};

/// \brief Class to represent DW_TAG_imported_module /
/// DW_TAG_imported_declaration
class TypeImport : public Type {
public:
  TypeImport()
      : Type(SV_TypeImport), InheritanceAccess(AccessSpecifier::Unspecified) {}

  /// \brief Return true if Obj is an insance of TypeImport.
  static bool classof(const Object *Obj) {
    return Obj->getKind() == SV_TypeImport;
  }

  /// \brief Access specifier, only valid for inheritance.
  AccessSpecifier getInheritanceAccess() const;
  void setInheritanceAccess(AccessSpecifier Access);

private:
  AccessSpecifier InheritanceAccess;

public:
  bool getIsPrintedAsObject() const override;

  /// \brief Returns a text representation of this DIVA Object.
  std::string getAsText(const PrintSettings &Settings) const override;
  /// \brief Returns a YAML representation of this DIVA Object.
  std::string getAsYAML() const override;

private:
  virtual std::string getInheritanceAsText() const;
  virtual std::string getUsingAsText(const PrintSettings &Settings) const;
  // Gets a YAML representation of DIVA Object as an Inheritance attribute.
  virtual std::string getInheritanceAsYAML() const;
  virtual std::string getUsingAsYAML() const;
};

/// \brief Class to represent a DWARF Template parameter holder.
///
/// Parameters can be values, types or templates.
class TypeParam : public Type {
public:
  TypeParam() : Type(SV_TypeParam), ValueIndex(0) {}

  /// \brief Return true if Obj is an insance of TypeParam.
  static bool classof(const Object *Obj) {
    return Obj->getKind() == SV_TypeParam;
  }

private:
  size_t ValueIndex; // Value in case of value or template parameters.

public:
  /// \brief Template parameter value
  const char *getValue() const override;
  void setValue(const char *Value) override;
  size_t getValueIndex() const override;

  bool getIsPrintedAsObject() const override;

  /// \brief Returns a text representation of this DIVA Object.
  std::string getAsText(const PrintSettings &Settings) const override;
  /// \brief Returns a YAML representation of this DIVA Object.
  std::string getAsYAML() const override;
};

/// \brief Class to represent a DW_TAG_subrange_type
class TypeSubrange : public Type {
public:
  TypeSubrange() : Type(SV_TypeSubrange) {}
  ~TypeSubrange();

  /// Return true if Obj is an insance of TypeSubrange.
  static bool classof(const Object *Obj) {
    return Obj->getKind() == SV_TypeSubrange;
  }
};

} // namespace LibScopeView

#endif // SCOPEVIEWTYPE_H
