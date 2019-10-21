#pragma once

#include <string>
#include <unordered_map>

namespace llvm {
class Type;
}

namespace llvm2cpg {
class CPGProtoBuilder;

class CPGTypeEmitter {
public:
  explicit CPGTypeEmitter(CPGProtoBuilder &builder);
  std::string recordType(const llvm::Type *type, const std::string &namespaceName);
  void emitRecordedTypes();

private:
  std::string recordType(const std::string &typeName, const std::string &typeLocation);

  CPGProtoBuilder &builder;
  std::unordered_map<std::string, std::string> recordedTypes;
};

} // namespace llvm2cpg
