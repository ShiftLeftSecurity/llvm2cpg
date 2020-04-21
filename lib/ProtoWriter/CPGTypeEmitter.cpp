#include "CPGTypeEmitter.h"
#include "CPGProtoBuilder.h"
#include <llvm/BinaryFormat/Dwarf.h>
#include <llvm/IR/DebugInfo.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Intrinsics.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm2cpg/CPG/ObjCTypeHierarchy.h>
#include <llvm2cpg/LLVMExt/TypeEquality.h>
#include <llvm2cpg/Logger/CPGLogger.h>
#include <llvm2cpg/Traversals/ObjCTraversal.h>
#include <queue>
#include <set>
#include <sstream>
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

  if (type->isOpaque()) {
    if (canonicalOpaqueNames.count(type) == 0) {
      canonicalOpaqueNames[type] =
          std::string("opaque") + std::to_string(canonicalOpaqueNames.size());
    }
    return canonicalOpaqueNames[type];
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

CPGTypeEmitter::CPGTypeEmitter(CPGProtoBuilder &builder, CPGLogger &logger)
    : builder(builder), logger(logger) {
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
    std::vector<const llvm::Module *> &modules,
    std::unordered_map<llvm::Function *, CPGProtoNode *> &emittedMethods) {
  ObjCTypeHierarchy objCTypeHierarchy(logger, modules);

  /// Attach methods to the right classes
  for (const std::string &className : objCTypeHierarchy.getClasses()) {
    if (objCTypeHierarchy.isExternal(className)) {
      continue;
    }

    for (ObjCMethod &method : objCTypeHierarchy.getMethods(className)) {
      CPGProtoNode *methodNode = emittedMethods.at(method.function);
      (*methodNode)
          .setName(method.name)
          .setASTParentType("TYPE_DECL")
          .setASTParentFullName(className);
    }

    std::string metaclassName = objCTypeHierarchy.getMetaclass(className);
    for (ObjCMethod &method : objCTypeHierarchy.getMethods(metaclassName)) {
      CPGProtoNode *methodNode = emittedMethods.at(method.function);
      (*methodNode)
          .setName(method.name)
          .setASTParentType("TYPE_DECL")
          .setASTParentFullName(className);
    }
  }
  for (const std::string &category : objCTypeHierarchy.getCategories()) {
    for (ObjCMethod &method : objCTypeHierarchy.getMethods(category)) {
      CPGProtoNode *methodNode = emittedMethods.at(method.function);
      (*methodNode)
          .setName(method.name)
          .setASTParentType("TYPE_DECL")
          .setASTParentFullName(category);
    }
  }

  objCTypeHierarchy.propagateSubclassMethods();

  /// Attach method bindings
  for (const std::string &className : objCTypeHierarchy.getClasses()) {
    CPGProtoNode *typeDecl = namedTypeDecl(className);
    CPGProtoNode *typeDeclPtr = emitTypeDecl(className + "*", "<global>");

    for (ObjCMethod &method : objCTypeHierarchy.getMethods(className)) {
      CPGProtoNode *methodNode = emittedMethods.at(method.function);
      CPGProtoNode *binding = builder.bindingNode();
      (*binding).setName(method.name).setSignature("");
      builder.connectREF(binding, methodNode);
      builder.connectBinding(typeDecl, binding);
      builder.connectBinding(typeDeclPtr, binding);
    }

    std::string metaclassName = objCTypeHierarchy.getMetaclass(className);
    assert(!metaclassName.empty());
    CPGProtoNode *superclassTypeDecl = namedTypeDecl(metaclassName);
    CPGProtoNode *superclassTypeDeclPtr = emitTypeDecl(metaclassName + "*", "<global>");
    for (ObjCMethod &method : objCTypeHierarchy.getMethods(metaclassName)) {
      CPGProtoNode *methodNode = emittedMethods.at(method.function);
      CPGProtoNode *binding = builder.bindingNode();
      (*binding).setName(method.name).setSignature("");
      builder.connectREF(binding, methodNode);
      builder.connectBinding(superclassTypeDecl, binding);
      builder.connectBinding(superclassTypeDeclPtr, binding);
    }
  }
}

void CPGTypeEmitter::emitObjCTypes(std::vector<const llvm::Module *> &modules) {
  ObjCTypeHierarchy objCTypeHierarchy(logger, modules);
  for (const std::string &category : objCTypeHierarchy.getCategories()) {
    emitTypeDecl(category, "<global>");
    emitType(category);
  }

  for (const std::string &objcClass : objCTypeHierarchy.getRootClasses()) {
    emitObjCType(objcClass, objCTypeHierarchy);
  }
}

CPGProtoNode *CPGTypeEmitter::emitObjCType(const std::string &base,
                                           ObjCTypeHierarchy &typeHierarchy) {
  assert(!base.empty());
  CPGProtoNode *baseNode = emitTypeDecl(base, "<global>");
  baseNode->setIsExternal(typeHierarchy.isExternal(base));
  emitType(base);
  for (const std::string &subclass : typeHierarchy.getSubclasses(base)) {
    CPGProtoNode *subclassNode = emitObjCType(subclass, typeHierarchy);
    subclassNode->setInheritsFromTypeFullName(base);
  }
  return baseNode;
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

void CPGTypeEmitter::emitStructMembers(std::vector<const llvm::Module *> &modules) {
  std::unordered_set<std::string> emittedStructs;
  for (const llvm::Module *module : modules) {
    emitStructMembers(module, emittedStructs);
  }
}

void CPGTypeEmitter::emitStructMembers(const llvm::Module *module,
                                       std::unordered_set<std::string> &emittedStructs) {
  recordStructInformation(module);

  for (const llvm::StructType *structType : module->getIdentifiedStructTypes()) {
    if (structType->isOpaque() || !structType->hasName()) {
      continue;
    }

    std::string canonicalName = typeToString(structType);
    /// Because of type deduplication more than one llvm::struct may have the same canonical name
    /// Normally this should not be the case, but it happens with anonymous structs emitted by clang
    /// In this case we do want to emit members only once per canonical name
    if (emittedStructs.count(canonicalName) != 0) {
      continue;
    }
    emittedStructs.insert(canonicalName);

    for (std::string &alias : typeAliases[canonicalName]) {
      CPGProtoNode *aliasDecl = emitTypeDecl(alias, "<global>");
      aliasDecl->setAliasTypeFullName(canonicalName);
      emitType(alias);
    }

    CPGProtoNode *structDecl = emitTypeDecl(canonicalName, "<global>");
    emitType(canonicalName);
    assert(structDecl);
    std::vector<std::string> &members = getStructMembers(structType);
    for (unsigned i = 0; i < structType->getStructNumElements(); i++) {
      std::string memberTypeName = typeToString(structType->getStructElementType(i));
      emitTypeDecl(memberTypeName, "<global>");
      emitType(memberTypeName);
      std::string memberName = std::to_string(i);
      if (i < members.size()) {
        memberName = members[i];
      }
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

void CPGTypeEmitter::recordStructInformation(const llvm::Module *module) {
  std::queue<llvm::DIType *> worklist;

  llvm::StringRef debugDeclareName = llvm::Intrinsic::getName(llvm::Intrinsic::dbg_declare);
  llvm::Function *debugDeclare = module->getFunction(debugDeclareName);
  if (debugDeclare) {
    for (auto user : debugDeclare->users()) {
      if (auto call = llvm::dyn_cast<llvm::CallInst>(user)) {
        assert(call->getNumOperands() >= 2);
        auto *debugMetadata = llvm::dyn_cast<llvm::MetadataAsValue>(call->getOperand(1));
        if (debugMetadata) {
          auto variable = llvm::dyn_cast<llvm::DILocalVariable>(debugMetadata->getMetadata());
          if (variable) {
            worklist.push(variable->getType());
          }
        }
      }
    }
  }

  for (const llvm::Function &function : module->getFunctionList()) {
    if (function.hasMetadata(0)) {
      if (auto debug = llvm::dyn_cast<llvm::DISubprogram>(function.getMetadata(0))) {
        worklist.push(debug->getType());
      }
    }
  }

  std::unordered_set<llvm::DIType *> visitedTypes;
  std::vector<llvm::DICompositeType *> compositeTypes;
  while (!worklist.empty()) {
    llvm::DIType *type = worklist.front();
    worklist.pop();
    if (!type || visitedTypes.count(type) != 0) {
      continue;
    }
    visitedTypes.insert(type);

    if (auto compositeType = llvm::dyn_cast<llvm::DICompositeType>(type)) {
      compositeTypes.push_back(compositeType);
      for (auto element : compositeType->getElements()) {
        if (auto elementType = llvm::dyn_cast<llvm::DIType>(element)) {
          worklist.push(elementType);
        }
      }
    } else if (auto derivedType = llvm::dyn_cast<llvm::DIDerivedType>(type)) {
      llvm::DIType *baseType = derivedType->getBaseType();
      worklist.push(baseType);
      if (derivedType->getTag() == llvm::dwarf::DW_TAG_typedef && baseType) {
        std::string baseName = baseType->getName();
        std::string aliasName = derivedType->getName();
        if (!baseName.empty() && !aliasName.empty()) {
          typeAliases[baseName].push_back(aliasName);
        }
      }
    } else if (auto functionType = llvm::dyn_cast<llvm::DISubroutineType>(type)) {
      llvm::DITypeRefArray types = functionType->getTypeArray();
      for (auto t : types) {
        worklist.push(t);
      }
    }
  }

  for (llvm::DICompositeType *compositeType : compositeTypes) {
    std::string name = compositeType->getName();
    if (!name.empty()) {
      structMemberNames[name] = std::vector<std::string>();
      for (auto memberElement : compositeType->getElements()) {
        if (auto memberType = llvm::dyn_cast<llvm::DIDerivedType>(memberElement)) {
          structMemberNames[name].push_back(memberType->getName());
        }
      }
    }
  }

  /// Sanity check
  for (const llvm::StructType *type : module->getIdentifiedStructTypes()) {
    std::string canonicalName = typeToString(type);
    auto it = structMemberNames.find(canonicalName);
    if (it != structMemberNames.end() && it->second.size() != type->getStructNumElements()) {
      structMemberNames.erase(it);
    }
  }
}

CPGProtoNode *CPGTypeEmitter::namedTypeDecl(const std::string &typeName) {
  return namedTypeDecls.at(typeName);
}

std::vector<std::string> &CPGTypeEmitter::getStructMembers(const llvm::StructType *structType) {
  std::string canonicalName = typeToString(structType);
  return structMemberNames[canonicalName];
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
