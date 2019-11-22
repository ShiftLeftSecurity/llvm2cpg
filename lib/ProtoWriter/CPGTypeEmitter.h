#pragma once

#include <string>
#include <unordered_map>

namespace llvm {
class Type;
class Module;
}

namespace llvm2cpg {
class CPGProtoBuilder;
class CPGProtoNode;

class CPGTypeEmitter {
public:
  explicit CPGTypeEmitter(CPGProtoBuilder &builder);
  void emitObjCTypes(const llvm::Module &module);
  std::string recordType(const llvm::Type *type, const std::string &namespaceName);
  void emitRecordedTypes();

private:
  std::string recordType(const std::string &typeName, const std::string &typeLocation);
  CPGProtoNode *emitTypeDecl(const std::string &typeName, const std::string &typeLocation);
  CPGProtoNode *emitType(const std::string &typeName);

  CPGProtoBuilder &builder;
  std::unordered_map<std::string, std::string> recordedTypes;
};

} // namespace llvm2cpg
