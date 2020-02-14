#include "CPGTypeEmitter.h"
#include "CPGProtoBuilder.h"
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm2cpg/CPG/ObjCTypeHierarchy.h>
#include <llvm2cpg/LLVMExt/TypeEquality.h>
#include <llvm2cpg/Traversals/ObjCTraversal.h>
#include <llvm2cpg/Logger/CPGLogger.h>
#include <sstream>
#include <set>
#include <unordered_map>
#include <utility>

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

std::string CPGTypeEmitter::defaultTypeToString(const llvm::Type *type) {
  std::string typeName;
  llvm::raw_string_ostream stream(typeName);
  type->print(stream);
  return stream.str();
}

std::string CPGTypeEmitter::typeToString(const llvm::PointerType *type) {
  return typeToString(type->getElementType()) + "*";
}

std::string CPGTypeEmitter::typeToString(const llvm::StructType *type) {
  if (type->hasName()) {
    assert(canonicalNames.count(type));
    return canonicalNames[type];
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

std::string CPGTypeEmitter::typeToString(const llvm::FunctionType *type) {
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

std::string CPGTypeEmitter::typeToString(const llvm::VectorType *type) {
  std::stringstream stream;
  stream << "<";
  if (type->isScalable()) {
    stream << "vscale x ";
  }
  stream << type->getVectorNumElements() << " x " << typeToString(type->getElementType()) << ">";
  return stream.str();
}

std::string CPGTypeEmitter::typeToString(const llvm::ArrayType *type) {
  std::stringstream stream;
  stream << "[" << type->getNumElements() << " x " << typeToString(type->getElementType()) << "]";
  return stream.str();
}

std::string CPGTypeEmitter::typeToString(const llvm::Type *type) {
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

CPGTypeEmitter::CPGTypeEmitter(CPGProtoBuilder &builder, CPGLogger &logger) : builder(builder), logger(logger) {
  recordType("ANY", "<global>");
}

std::string CPGTypeEmitter::recordType(const llvm::Type *type, const std::string &namespaceName) {
  /// TODO: Add some memoization to avoid type name calculation for each type over and over
  std::string typeName = typeToString(type);
  return recordType(typeName, namespaceName);
}

std::string CPGTypeEmitter::recordType(const std::string &typeName,
                                       const std::string &typeLocation) {
  assert(typeName.length() && "Every type is supposed to have a name");
  if (!recordedTypes.count(typeName)) {
    recordedTypes.insert(std::make_pair(typeName, typeLocation));
  }
  return typeName;
}

void CPGTypeEmitter::emitRecordedTypes() {
  for (const auto &pair : recordedTypes) {
    std::string typeName = pair.first;
    std::string typeLocation = pair.second;
    emitType(typeName);
    emitTypeDecl(typeName, typeLocation);
  }
}

void CPGTypeEmitter::emitObjCMethodBindings(
    const llvm::Module *module,
    std::unordered_map<llvm::Function *, CPGProtoNode *> &emittedMethods) {
  ObjCTypeHierarchy objCTypeHierarchy(module);

  /// Attach methods to the right classes
  for (ObjCClassDefinition *objcClass : objCTypeHierarchy.getClasses()) {
    if (objcClass->isExternal()) {
      continue;
    }

    std::string className = objcClass->getName();
    for (ObjCMethod &method : objCTypeHierarchy.getMethods(objcClass)) {
      CPGProtoNode *methodNode = emittedMethods.at(method.function);
      (*methodNode)
          .setName(method.name)
          .setASTParentType("TYPE_DECL")
          .setASTParentFullName(className);
    }

    ObjCClassDefinition *objcMetaclass = objCTypeHierarchy.getMetaclass(objcClass);
    std::string superclassName = objcMetaclass->getName();
    for (ObjCMethod &method : objCTypeHierarchy.getMethods(objcMetaclass)) {
      CPGProtoNode *methodNode = emittedMethods.at(method.function);
      (*methodNode)
          .setName(method.name)
          .setASTParentType("TYPE_DECL")
          .setASTParentFullName(className);
    }
  }

  objCTypeHierarchy.propagateSubclassMethods();

  /// Attach method bindings
  for (ObjCClassDefinition *objcClass : objCTypeHierarchy.getClasses()) {
    if (objcClass->isExternal()) {
      continue;
    }
    std::string className = objcClass->getName();
    CPGProtoNode *typeDecl = namedTypeDecl(className);
    CPGProtoNode *typeDeclPtr = emitTypeDecl(className + "*", "<global>");

    for (ObjCMethod &method : objCTypeHierarchy.getMethods(objcClass)) {
      CPGProtoNode *methodNode = emittedMethods.at(method.function);
      CPGProtoNode *binding = builder.bindingNode();
      (*binding).setName(method.name).setSignature("");
      builder.connectREF(binding, methodNode);
      builder.connectBinding(typeDecl, binding);
      builder.connectBinding(typeDeclPtr, binding);
    }

    ObjCClassDefinition *objcMetaclass = objCTypeHierarchy.getMetaclass(objcClass);
    std::string superclassName = objcMetaclass->getName();
    CPGProtoNode *superclassTypeDecl = namedTypeDecl(superclassName);
    CPGProtoNode *superclassTypeDeclPtr = emitTypeDecl(superclassName + "*", "<global>");
    for (ObjCMethod &method : objCTypeHierarchy.getMethods(objcMetaclass)) {
      CPGProtoNode *methodNode = emittedMethods.at(method.function);
      CPGProtoNode *binding = builder.bindingNode();
      (*binding).setName(method.name).setSignature("");
      builder.connectREF(binding, methodNode);
      builder.connectBinding(superclassTypeDecl, binding);
      builder.connectBinding(superclassTypeDeclPtr, binding);
    }
  }
}

void CPGTypeEmitter::emitObjCTypes(const llvm::Module &module) {
  ObjCTypeHierarchy objCTypeHierarchy(&module);

  for (ObjCClassDefinition *objcClass : objCTypeHierarchy.getRootClasses()) {
    emitObjCType(objcClass, objCTypeHierarchy);
  }
}

struct CanonicalNameCounter {
  std::string name;
  size_t counter;
  explicit CanonicalNameCounter(std::string n) : name(std::move(n)), counter(0) {}
  void bump() {
    counter += 1;
  }
  std::string getName() {
    if (counter == 0) {
      return name;
    } else {
      return name + "_" + std::to_string(counter);
    }
  }
};

void CPGTypeEmitter::recordCanonicalStructNames(std::vector<const llvm::Module *> &modules) {
  logger.uiInfo("Start type deduplication");
  /// Struct types in C/C++/ObjC are named in the following way:
  ///   - struct.Foo
  ///   - struct.Foo.0
  ///   - struct.Foo.152
  /// We consider the 'Foo' part to be the canonical name.
  /// There are cases in which structs of a different layout can have the same canonical names:
  /// %struct.Foo = { i32, i32 }
  /// %struct.Foo.120 = { float, float }
  /// In this case we emit Foo and Foo_1 to distinguish the types
  std::unordered_map<std::string, std::vector<const llvm::StructType *>> namedStructClusters;
  std::vector<const llvm::StructType *> allTypes;
  for (const llvm::Module *module : modules) {
    for (const llvm::StructType *structType : module->getIdentifiedStructTypes()) {
      allTypes.push_back(structType);
      std::string name = llvm_ext::getCanonicalName(structType);
      namedStructClusters[name].push_back(structType);
    }
  }

  llvm_ext::TypesComparator typesComparator;

  std::set<std::string> uniqCanonicalNames;
  for (auto &pair : namedStructClusters) {
    std::vector<const llvm::StructType *> &cluster = pair.second;
    auto it = cluster.begin();
    while (it != cluster.end()) {
      it = std::partition(it, cluster.end(), [&](const llvm::StructType *type) -> bool {
        return typesComparator.typesEqual(*it, type);
      });
    }

    const llvm::StructType *currentStruct = cluster.front();
    CanonicalNameCounter nameCounter(pair.first);
    canonicalNames[currentStruct] = nameCounter.getName();
    uniqCanonicalNames.insert(nameCounter.getName());
    for (size_t i = 1; i < cluster.size(); i++) {
      const llvm::StructType *nextStruct = cluster[i];
      if (!typesComparator.typesEqual(currentStruct, nextStruct)) {
        nameCounter.bump();
        currentStruct = nextStruct;
      }
      canonicalNames[nextStruct] = nameCounter.getName();

      uniqCanonicalNames.insert(nameCounter.getName());
    }
  }
  logger.uiInfo("Finish type deduplication");
}

void CPGTypeEmitter::emitStructMembers(const llvm::Module *module) {
  for (const llvm::StructType *structType : module->getIdentifiedStructTypes()) {
    if (structType->isOpaque() || !structType->hasName()) {
      continue;
    }
    std::string canonicalName = typeToString(structType);
    CPGProtoNode *structDecl = emitTypeDecl(canonicalName, "<global>");
    emitType(canonicalName);
    assert(structDecl);
    for (unsigned i = 0; i < structType->getStructNumElements(); i++) {
      std::string memberTypeName = typeToString(structType->getStructElementType(i));
      emitTypeDecl(memberTypeName, "<global>");
      emitType(memberTypeName);
      std::string memberName = std::to_string(i);
      CPGProtoNode *member = builder.memberNode();
      (*member) //
          .setName(memberName)
          .setCode(memberName)
          .setTypeFullName(memberTypeName)
          .setOrder(i);
      builder.connectAST(structDecl, member);
    }
  }
}

CPGProtoNode *CPGTypeEmitter::emitObjCType(ObjCClassDefinition *base,
                                           ObjCTypeHierarchy &typeHierarchy) {
  std::string baseName = base->getName();
  emitType(baseName);
  CPGProtoNode *baseNode = emitTypeDecl(baseName, "<global>");
  baseNode->setIsExternal(base->isExternal());
  for (ObjCClassDefinition *subclass : typeHierarchy.getSubclasses(base)) {
    CPGProtoNode *subclassNode = emitObjCType(subclass, typeHierarchy);
    subclassNode->setInheritsFromTypeFullName(baseName);
  }
  return baseNode;
}

CPGProtoNode *CPGTypeEmitter::namedTypeDecl(const std::string &typeName) {
  return namedTypeDecls.at(typeName);
}

CPGProtoNode *CPGTypeEmitter::emitType(const std::string &typeName) {
  CPGProtoNode *typeNode = builder.typeNode();
  (*typeNode) //
      .setName(typeName)
      .setFullName(typeName)
      .setTypeDeclFullName(typeName);
  return typeNode;
}

CPGProtoNode *CPGTypeEmitter::emitTypeDecl(const std::string &typeName,
                                           const std::string &typeLocation) {
  if (namedTypeDecls.count(typeName)) {
    return namedTypeDecls[typeName];
  }

  CPGProtoNode *typeDeclNode = builder.typeDeclNode();
  (*typeDeclNode) //
      .setName(typeName)
      .setFullName(typeName)
      .setIsExternal(false)
      .setOrder(0)
      .setASTParentType("NAMESPACE_BLOCK")
      .setASTParentFullName(typeLocation);

  namedTypeDecls[typeName] = typeDeclNode;

  return typeDeclNode;
}
