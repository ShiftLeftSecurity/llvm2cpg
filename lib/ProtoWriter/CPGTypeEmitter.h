#pragma once

#include <string>
#include <unordered_map>

namespace llvm {
class Type;
class Function;
class Module;
} // namespace llvm

namespace llvm2cpg {
class CPGProtoBuilder;
class CPGProtoNode;
class ObjCClassDefinition;
class ObjCTypeHierarchy;

class CPGTypeEmitter {
public:
  explicit CPGTypeEmitter(CPGProtoBuilder &builder);
  void emitObjCTypes(const llvm::Module &module);
  void emitObjCMethodBindings(const llvm::Module *module,
                              std::unordered_map<llvm::Function *, CPGProtoNode *> &emittedMethods);
  std::string recordType(const llvm::Type *type, const std::string &namespaceName);
  void emitRecordedTypes();

  CPGProtoNode *namedTypeDecl(const std::string &className);

private:
  std::string recordType(const std::string &typeName, const std::string &typeLocation);
  CPGProtoNode *emitObjCType(ObjCClassDefinition *base, ObjCTypeHierarchy &typeHierarchy);

  CPGProtoNode *emitTypeDecl(const std::string &typeName, const std::string &typeLocation);
  CPGProtoNode *emitType(const std::string &typeName);

  CPGProtoBuilder &builder;
  std::unordered_map<std::string, std::string> recordedTypes;
  std::unordered_map<std::string, CPGProtoNode *> namedTypeDecls;
};

} // namespace llvm2cpg
