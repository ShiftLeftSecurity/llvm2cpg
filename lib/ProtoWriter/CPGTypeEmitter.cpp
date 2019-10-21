#include "CPGTypeEmitter.h"
#include "CPGProtoBuilder.h"
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Type.h>
#include <llvm/Support/raw_ostream.h>

using namespace llvm2cpg;

static std::string typeToString(const llvm::Type *type) {
  /// TODO: Make sure that the pointers are always encoded as `foo*`
  if (auto structType = llvm::dyn_cast<llvm::StructType>(type)) {
    if (structType->hasName()) {
      return structType->getStructName();
    }
  }
  std::string typeName;
  llvm::raw_string_ostream stream(typeName);
  type->print(stream);
  stream.flush();
  return typeName;
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
    std::string typeLocation = pair.second;
    auto typeDeclNode = builder.typeDeclNode();
    (*typeDeclNode) //
        .setName(typeName)
        .setFullName(typeName)
        .setIsExternal(false)
        .setOrder(0)
        .setASTParentType("NAMESPACE_BLOCK")
        .setASTParentFullName(typeLocation);

    auto typeNode = builder.typeNode();
    (*typeNode) //
        .setName(typeName)
        .setFullName(typeName)
        .setTypeDeclFullName(typeName);
  }
}
