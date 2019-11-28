#include "CPGTypeEmitter.h"
#include "CPGProtoBuilder.h"
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Type.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm2cpg/Traversals/ObjCTraversal.h>
#include <sstream>

using namespace llvm2cpg;

static std::string concatStrings(const std::vector<std::string> &v) {
  if (v.empty()) {
    return std::string();
  }

  std::stringstream stream;
  std::copy(v.begin(), v.end() - 1, std::ostream_iterator<std::string>(stream, ", "));
  stream << *(v.end() - 1);
  return stream.str();
}

static std::string typeToString(const llvm::Type *type);
static std::string typeToString(const llvm::PointerType *type);
static std::string typeToString(const llvm::StructType *type);
static std::string typeToString(const llvm::FunctionType *type);
static std::string typeToString(const llvm::VectorType *type);
static std::string typeToString(const llvm::ArrayType *type);
static std::string defaultTypeToString(const llvm::Type *type);

static std::string defaultTypeToString(const llvm::Type *type) {
  std::string typeName;
  llvm::raw_string_ostream stream(typeName);
  type->print(stream);
  return stream.str();
}

static std::string typeToString(const llvm::PointerType *type) {
  return typeToString(type->getElementType()) + "*";
}

static std::string typeToString(const llvm::StructType *type) {
  if (type->hasName()) {
    return type->getStructName();
  }

  std::vector<std::string> types;
  types.reserve(type->getNumElements());
  for (unsigned i = 0; i < type->getNumElements(); i++) {
    types.push_back(typeToString(type->getTypeAtIndex(i)));
  }

  std::stringstream stream;
  stream << "{ " << concatStrings(types) << " }";
  return stream.str();
}

static std::string typeToString(const llvm::FunctionType *type) {
  std::vector<std::string> types;
  types.reserve(type->getFunctionNumParams());
  for (unsigned i = 0; i < type->getFunctionNumParams(); i++) {
    types.push_back(typeToString(type->getFunctionParamType(i)));
  }
  if (type->isVarArg()) {
    types.emplace_back("...");
  }

  std::stringstream stream;
  stream << typeToString(type->getReturnType()) << " (" << concatStrings(types) << ")";
  return stream.str();
}

static std::string typeToString(const llvm::VectorType *type) {
  std::stringstream stream;
  stream << "<";
  if (type->isScalable()) {
    stream << "vscale x ";
  }
  stream << type->getVectorNumElements() << " x " << typeToString(type->getElementType()) << ">";
  return stream.str();
}

static std::string typeToString(const llvm::ArrayType *type) {
  std::stringstream stream;
  stream << "[" << type->getNumElements() << " x " << typeToString(type->getElementType()) << "]";
  return stream.str();
}

static std::string typeToString(const llvm::Type *type) {
  switch (type->getTypeID()) {
  case llvm::Type::VoidTyID:
  case llvm::Type::HalfTyID:
  case llvm::Type::FloatTyID:
  case llvm::Type::DoubleTyID:
  case llvm::Type::X86_FP80TyID:
  case llvm::Type::FP128TyID:
  case llvm::Type::PPC_FP128TyID:
  case llvm::Type::LabelTyID:
  case llvm::Type::MetadataTyID:
  case llvm::Type::X86_MMXTyID:
  case llvm::Type::TokenTyID:
  case llvm::Type::IntegerTyID:
    return defaultTypeToString(type);

  case llvm::Type::FunctionTyID:
    return typeToString(llvm::cast<llvm::FunctionType>(type));

  case llvm::Type::StructTyID:
    return typeToString(llvm::cast<llvm::StructType>(type));

  case llvm::Type::ArrayTyID:
    return typeToString(llvm::cast<llvm::ArrayType>(type));

  case llvm::Type::PointerTyID:
    return typeToString(llvm::cast<llvm::PointerType>(type));

  case llvm::Type::VectorTyID:
    return typeToString(llvm::cast<llvm::VectorType>(type));
  }
}

CPGTypeEmitter::CPGTypeEmitter(CPGProtoBuilder &builder) : builder(builder) {
  recordType("ANY", "<global>");
}

std::string CPGTypeEmitter::recordType(const llvm::Type *type, const std::string &namespaceName) {
  /// TODO: Add some memoization to avoid type name calculation for each type over and over
  std::string typeName = typeToString(type);
  return recordType(typeName, namespaceName);
}

std::string CPGTypeEmitter::recordType(const std::string &typeName,
                                       const std::string &typeLocation) {
  assert(typeName.length() && "Evey type is supposed to have a name");
  if (!recordedTypes.count(typeName)) {
    recordedTypes.insert(std::make_pair(typeName, typeLocation));
  }
  return typeName;
}

void CPGTypeEmitter::emitRecordedTypes() {
  for (auto pair : recordedTypes) {
    std::string typeName = pair.first;
    if (namedTypeDecls.count(typeName)) {
      continue;
    }
    std::string typeLocation = pair.second;
    emitType(typeName);
    CPGProtoNode *typeDecl = emitTypeDecl(typeName, typeLocation);
    namedTypeDecls.insert(std::make_pair(typeName, typeDecl));
  }
}

void CPGTypeEmitter::emitObjCTypes(const llvm::Module &module) {
  ObjCTraversal traversal(&module);

  std::unordered_map<std::string, std::vector<std::string>> classes;
  std::vector<const llvm::ConstantStruct *> worklist = traversal.objcClasses();
  for (const llvm::ConstantStruct *objcClass : worklist) {
    const llvm::ConstantStruct *objcClassRO = traversal.objcClassROCounterpart(objcClass);
    std::string className = traversal.objcClassName(objcClassRO);

    classes[className] = std::vector<std::string>();

    const llvm::ConstantStruct *objcSuperclass = traversal.objcSuperclass(objcClass);
    if (!objcSuperclass) {
      continue;
    }
    const llvm::ConstantStruct *objcSuperclassRO = traversal.objcClassROCounterpart(objcSuperclass);
    std::string superclassName = traversal.objcClassName(objcSuperclassRO);

    classes[className].push_back(superclassName);
  }

  for (const auto &classPair : classes) {
    std::string className = classPair.first;
    CPGProtoNode *typeDecl = nullptr;
    if (namedTypeDecls.count(className)) {
      typeDecl = namedTypeDecls.find(className)->second;
    } else {
      emitType(className);
      typeDecl = emitTypeDecl(className, "<global>");
      namedTypeDecls.insert(std::make_pair(className, typeDecl));
    }

    for (const auto &parent : classPair.second) {
      typeDecl->setInheritsFromTypeFullName(parent);
    }
  }
}

CPGProtoNode *CPGTypeEmitter::namedTypeDecl(const std::string &className) {
  return namedTypeDecls.at(className);
}

CPGProtoNode *CPGTypeEmitter::emitType(const std::string &typeName) {
  auto typeNode = builder.typeNode();
  (*typeNode) //
      .setName(typeName)
      .setFullName(typeName)
      .setTypeDeclFullName(typeName);
  return typeNode;
}

CPGProtoNode *CPGTypeEmitter::emitTypeDecl(const std::string &typeName,
                                           const std::string &typeLocation) {
  auto typeDeclNode = builder.typeDeclNode();
  (*typeDeclNode) //
      .setName(typeName)
      .setFullName(typeName)
      .setIsExternal(false)
      .setOrder(0)
      .setASTParentType("NAMESPACE_BLOCK")
      .setASTParentFullName(typeLocation);
  return typeDeclNode;
}
